/*

Copyright (c) 2017, Arvid Norberg, Steven Siloti
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in
the documentation and/or other materials provided with the distribution.
* Neither the name of the author nor the names of its
contributors may be used to endorse or promote products derived
from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef TORRENT_SESSION_UDP_SOCKETS_HPP_INCLUDED
#define TORRENT_SESSION_UDP_SOCKETS_HPP_INCLUDED

#include "libtorrent/utp_socket_manager.hpp"
#include "libtorrent/config.hpp"
#include <boost/asio/io_service.hpp>
#include <vector>

namespace libtorrent { namespace aux {

	struct listen_endpoint_t;
	struct proxy_settings;

	struct session_udp_socket : utp_socket_interface
	{
		explicit session_udp_socket(io_service& ios)
			: sock(ios) {}

		udp::endpoint local_endpoint() override { return sock.local_endpoint(); }

		udp_socket sock;

		// this is true when the udp socket send() has failed with EAGAIN or
		// EWOULDBLOCK. i.e. we're currently waiting for the socket to become
		// writeable again. Once it is, we'll set it to false and notify the utp
		// socket manager
		bool write_blocked = false;
	};

	struct outgoing_udp_socket final : session_udp_socket
	{
		outgoing_udp_socket(io_service& ios, std::string const& dev, bool ssl_)
			: session_udp_socket(ios), device(dev), ssl(ssl_) {}

		// the name of the device the socket is bound to, may be empty
		// if the socket is not bound to a device
		std::string const device;

		// set to true if this is an SSL socket
		bool const ssl;
	};

	// sockets used for outoing utp connections
	struct TORRENT_EXTRA_EXPORT outgoing_sockets
	{
		// partitions sockets based on whether they match one of the given endpoints
		// all matched sockets are ordered before unmatched sockets
		// matched endpoints are removed from the vector
		// returns an iterator to the first unmatched socket
		std::vector<std::shared_ptr<outgoing_udp_socket>>::iterator
		partition_outgoing_sockets(std::vector<listen_endpoint_t>& eps);

		tcp::endpoint bind(socket_type& s, address const& remote_address) const;

		void update_proxy(proxy_settings const& settings);

		// close all sockets
		void close();

		std::vector<std::shared_ptr<outgoing_udp_socket>> sockets;
	private:
		// round-robin index into sockets
		// one dimention for IPv4/IPv6 and a second for SSL/non-SSL
		mutable std::uint8_t index[2][2] = { {0, 0}, {0, 0} };
	};

} }

#endif
