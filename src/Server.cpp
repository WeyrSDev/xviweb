/*
 * Copyright (C) 2011 Josh A. Beam
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include "Server.h"

using namespace std;

Server::Server(const Address &address, unsigned short port)
 : m_address(address), m_port(port)
{
	if(address.getType() == ADDRESS_TYPE_IPV4) {
		// create IPv4 socket
		m_fd = socket(AF_INET, SOCK_STREAM, 0);
		if(m_fd == -1)
			throw "socket() failed";

		int value = 1;
		setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));

		struct sockaddr_in sin;
		bzero(&sin, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		memcpy(&sin.sin_addr, address.getAddress(), 4);
		socklen_t len = sizeof(sin);

		if(bind(m_fd, (struct sockaddr *)&sin, len) == -1) {
			close(m_fd);
			throw "bind() failed";
		}
	} else {
		// create IPv6 socket
		m_fd = socket(AF_INET6, SOCK_STREAM, 0);
		if(m_fd == -1)
			throw "socket() failed";

		int value = 1;
		setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));

		struct sockaddr_in6 sin;
		bzero(&sin, sizeof(sin));
		sin.sin6_family = AF_INET6;
		sin.sin6_port = htons(port);
		memcpy(&sin.sin6_addr, address.getAddress(), 16);
		socklen_t len = sizeof(sin);

		if(bind(m_fd, (struct sockaddr *)&sin, len) == -1) {
			close(m_fd);
			throw "bind() failed";
		}
	}

	if(listen(m_fd, 0) == -1) {
		close(m_fd);
		throw "listen() failed";
	}
}

Server::~Server()
{
	// close bound socket
	close(m_fd);

	// delete all responders
	for(unsigned int i = 0; i < m_responders.size(); ++i)
		delete m_responders[i];

	// delete all connections
	for(unsigned int i = 0; i < m_connections.size(); ++i) {
		if(m_connections[i] != NULL)
			delete m_connections[i];
	}
}

const Address &
Server::getAddress() const
{
	return m_address;
}

unsigned short
Server::getPort() const
{
	return m_port;
}

void
Server::attachResponder(Responder *responder)
{
	m_responders.insert(m_responders.begin(), responder);
}

HttpConnection *
Server::acceptHttpConnection()
{
	int fd;
	uint8_t address[16];
	AddressType type;
	unsigned short port;

	if(m_address.getType() == ADDRESS_TYPE_IPV4) {
		// accept IPv4 connection
		struct sockaddr_in sin;
		bzero(&sin, sizeof(sin));
		socklen_t len = sizeof(sin);

		fd = accept(m_fd, (struct sockaddr *)&sin, &len);
		if(fd == -1)
			throw "accept() failed";

		memcpy(address, &sin.sin_addr, 4);
		type = ADDRESS_TYPE_IPV4;
		port = ntohs(sin.sin_port);
	} else {
		// accept IPv6 connection
		struct sockaddr_in6 sin;
		bzero(&sin, sizeof(sin));
		socklen_t len = sizeof(sin);

		fd = accept(m_fd, (struct sockaddr *)&sin, &len);
		if(fd == -1)
			throw "accept() failed";

		memcpy(address, &sin.sin6_addr, 4);
		type = ADDRESS_TYPE_IPV6;
		port = ntohs(sin.sin6_port);
	}

	return new HttpConnection(fd, Address(address, type), port);
}

void
Server::processRequest(HttpConnection *conn)
{
	// loop through responders and stop when
	// one matches the request
	for(unsigned int i = 0; i < m_responders.size(); ++i) {
		Responder *responder = m_responders[i];
		if(responder->matchesRequest(conn->getRequest())) {
			responder->respond(conn);
			break;
		}
	}
}

void
Server::cycle()
{
	nfds_t nfds = (nfds_t)m_connections.size() + 1;
	struct pollfd *fds = new struct pollfd[nfds];

	// add the bound socket and each connection to fds
	fds[0].fd = m_fd;
	fds[0].events = POLLIN;
	fds[0].revents = 0;
	for(unsigned int i = 0; i < m_connections.size(); ++i) {
		HttpConnection *conn = m_connections[i];

		// remove null elements (representing connections that
		// were deleted) from the vector
		if(conn == NULL) {
			m_connections.erase(m_connections.begin() + (i--));
			continue;
		}

		// remove connections that haven't had any activity
		// in 10 seconds and haven't reached the responding
		// state yet
		if(conn->getState() != HTTP_CONNECTION_STATE_SENDING_RESPONSE &&
		   conn->getMillisecondsSinceLastRead() > 10000) {
			delete conn;
			m_connections.erase(m_connections.begin() + (i--));
			continue;
		}

		int tmp = i + 1;
		fds[tmp].fd = conn->getFileDescriptor();
		fds[tmp].events = POLLIN;
		fds[tmp].revents = 0;
	}

	if(poll(fds, nfds, 1000) > 0) {
		// handle connections to the bound socket
		if(fds[0].revents & POLLIN) {
			HttpConnection *conn = acceptHttpConnection();
			m_connections.push_back(conn);
		}

		// read from connections
		for(unsigned int i = 0; i < m_connections.size(); ++i) {
			if((fds[i+1].revents & POLLIN) == 0)
				continue;

			// read from the connection
			HttpConnection *conn = m_connections[i];
			conn->doRead();

			switch(conn->getState()) {
				default:
					break;
				case HTTP_CONNECTION_STATE_SENDING_RESPONSE:
					processRequest(conn);
				case HTTP_CONNECTION_STATE_DONE:
					delete conn;
					m_connections[i] = NULL;
					break;
			}
		}
	}

	delete [] fds;
}
