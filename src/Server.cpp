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
#include "Util.h"

using namespace std;

ServerConnection::ServerConnection(HttpConnection *connectionValue,
                                   HttpResponseImpl *responseValue,
                                   ResponderContext *contextValue)
{
	connection = connectionValue;
	response = responseValue;
	context = contextValue;
}

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

	// delete all connection data
	for(unsigned int i = 0; i < m_connections.size(); ++i) {
		if(m_connections[i].context != NULL)
			delete m_connections[i].context;
		if(m_connections[i].response != NULL)
			delete m_connections[i].response;
		if(m_connections[i].connection != NULL)
			delete m_connections[i].connection;
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
Server::processRequest(ServerConnection *conn)
{
	// create HttpResponse for the connection
	conn->response = new HttpResponseImpl(conn->connection);

	// loop through responders and stop when
	// one matches the request
	for(unsigned int i = 0; i < m_responders.size(); ++i) {
		Responder *responder = m_responders[i];
		if(responder->matchesRequest(conn->connection->getRequest())) {
			// respond to the request
			conn->context = responder->respond(conn->connection->getRequest(), conn->response);
			break;
		}
	}
}

void
Server::cycle()
{
	long currentTime = getMilliseconds();
	long sleepTime = 1000;

	// process connections
	for(unsigned int i = 0; i < m_connections.size(); ++i) {
		ServerConnection *sconn = &(m_connections[i]);
		HttpConnection *conn = sconn->connection;

		HttpConnectionState state = conn->getState();
		bool done = (state == HTTP_CONNECTION_STATE_DONE) ||
		            (state != HTTP_CONNECTION_STATE_SENDING_RESPONSE &&
		             conn->getMillisecondsSinceLastRead() > 10000);

		// remove connections in the done state or continue
		// responses for ones that have associated contexts
		if(done) {
			delete conn;
			if(sconn->response != NULL)
				delete sconn->response;
			if(sconn->context != NULL)
				delete sconn->context;
			m_connections.erase(m_connections.begin() + (i--));
		} else if(sconn->context != NULL) {
			long wakeupTime = sconn->context->getWakeupTime();
			if(wakeupTime <= currentTime) {
				// continue the context's response; it may return
				// a pointer to itself, a pointer to a new context,
				// or null if it's done
				sconn->context = sconn->context->continueResponse(conn->getRequest(), sconn->response);
			} else {
				long timeDiff = wakeupTime - currentTime;
				if(timeDiff < sleepTime)
					sleepTime = timeDiff;
			}
		}
	}

	// create pollfd array
	nfds_t bsIndex = (nfds_t)m_connections.size();
	nfds_t nfds = bsIndex + 1;
	struct pollfd *fds = new struct pollfd[nfds];

	// add each connection to fds
	for(unsigned int i = 0; i < m_connections.size(); ++i) {
		fds[i].fd = m_connections[i].connection->getFileDescriptor();
		fds[i].events = POLLIN;
		fds[i].revents = 0;
	}

	// add the bound socket to fds
	fds[bsIndex].fd = m_fd;
	fds[bsIndex].events = POLLIN;
	fds[bsIndex].revents = 0;

	// poll bound socket and connection sockets
	if(poll(fds, nfds, sleepTime) > 0) {
		// handle connections to the bound socket
		if(fds[bsIndex].revents & POLLIN) {
			HttpConnection *conn = acceptHttpConnection();
			m_connections.push_back(ServerConnection(conn));
		}

		// read from connections
		for(unsigned int i = 0; i < m_connections.size(); ++i) {
			if((fds[i].revents & POLLIN) == 0)
				continue;

			// read from the connection
			HttpConnection *conn = m_connections[i].connection;
			conn->doRead();

			switch(conn->getState()) {
				default:
					break;
				case HTTP_CONNECTION_STATE_RECEIVED_REQUEST:
					// full request received
					processRequest(&m_connections[i]);
					break;
			}
		}
	}

	delete [] fds;
}
