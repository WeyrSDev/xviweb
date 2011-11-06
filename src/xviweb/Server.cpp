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
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <xviweb/String.h>
#include "Server.h"

using namespace std;

Server::Server()
 : m_address("127.0.0.1"), m_port(8080)
{
	m_fd = -1;
	m_numWorkers = 2;
}

Server::~Server()
{
	stop();
}

const Address &
Server::getAddress() const
{
	return m_address;
}

void
Server::setAddress(const Address &address)
{
	m_address = address;
}

unsigned short
Server::getPort() const
{
	return m_port;
}

void
Server::setPort(unsigned short port)
{
	m_port = port;
}

void
Server::setDefaultRoot(const string &root)
{
	m_defaultRoot = root;
}

void
Server::addVHost(const string &hostname, const string &root)
{
	m_vhostMap.insert(make_pair(String::toLower(hostname), root));
}

void
Server::attachResponder(Responder *responder)
{
	m_responders.insert(m_responders.begin(), responder);
}

unsigned int
Server::getNumWorkers() const
{
	return m_numWorkers;
}

void
Server::setNumWorkers(unsigned int numWorkers)
{
	m_numWorkers = numWorkers;
}

void
Server::start()
{
	if(m_fd != -1)
		throw "Server already started";

	if(m_address.getType() == ADDRESS_TYPE_IPV4) {
		// create IPv4 socket
		m_fd = socket(AF_INET, SOCK_STREAM, 0);
		if(m_fd == -1)
			throw "socket() failed";

		int value = 1;
		setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));

		struct sockaddr_in sin;
		bzero(&sin, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_port = htons(m_port);
		memcpy(&sin.sin_addr, m_address.getAddress(), 4);
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
		sin.sin6_port = htons(m_port);
		memcpy(&sin.sin6_addr, m_address.getAddress(), 16);
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

	// create workers
	m_nextWorker = 0;
	for(unsigned int i = 0; i < m_numWorkers; ++i)
		m_workers.push_back(new ServerWorker(m_defaultRoot, m_vhostMap, m_responders));
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
Server::cycle()
{
	struct pollfd pfd;
	pfd.fd = m_fd;
	pfd.events = POLLIN;
	pfd.revents = 0;

	// poll for connection
	if(poll(&pfd, 1, -1) > 0) {
		if(pfd.revents & POLLIN != 0) {
			// accept the new connection and add it to a worker
			HttpConnection *conn = acceptHttpConnection();
			m_workers[m_nextWorker]->addConnection(conn);
			m_nextWorker = (m_nextWorker + 1) % m_workers.size();
		}
	}
}

void
Server::stop()
{
	if(m_fd == -1)
		return;

	// close bound socket
	close(m_fd);
	m_fd = -1;

	// delete all workers
	for(unsigned int i = 0; i < m_workers.size(); ++i)
		delete m_workers[i];
	m_workers.clear();
}
