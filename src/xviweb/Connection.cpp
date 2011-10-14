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
#include <xviweb/String.h>
#include "Connection.h"
#include "Util.h"

using namespace std;

Connection::Connection(int fd, const Address &address, unsigned short port)
 : m_fd(fd), m_address(address), m_port(port)
{
	m_readMilliseconds = getMilliseconds();

	cout << toString() << ": Connection opened" << endl;
}

Connection::Connection(const Address &address, unsigned short port)
 : m_address(address), m_port(port)
{
	m_readMilliseconds = getMilliseconds();

	if(address.getType() == ADDRESS_TYPE_IPV4) {
		// open TCP connection over IPv4
		m_fd = socket(AF_INET, SOCK_STREAM, 0);
		if(m_fd == -1)
			throw "socket() failed";

		struct sockaddr_in sin;
		bzero(&sin, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		memcpy(&sin.sin_addr, address.getAddress(), 4);

		if(connect(m_fd, (struct sockaddr *)&sin, sizeof(sin)) == -1) {
			close(m_fd);
			throw "connect() failed";
		}
	} else {
		// open TCP connection over IPv6
		m_fd = socket(AF_INET6, SOCK_STREAM, 0);
		if(m_fd == -1)
			throw "socket() failed";

		struct sockaddr_in6 sin;
		bzero(&sin, sizeof(sin));
		sin.sin6_family = AF_INET6;
		sin.sin6_port = htons(port);
		memcpy(&sin.sin6_addr, address.getAddress(), 16);

		if(connect(m_fd, (struct sockaddr *)&sin, sizeof(sin)) == -1) {
			close(m_fd);
			throw "connect() failed";
		}
	}

	cout << toString() << ": Connection opened" << endl;
}

Connection::~Connection()
{
	close(m_fd);
	cout << toString() << ": Connection closed" << endl;
}

int
Connection::getFileDescriptor() const
{
	return m_fd;
}

const Address &
Connection::getAddress() const
{
	return m_address;
}

unsigned short
Connection::getPort() const
{
	return m_port;
}

long
Connection::getMillisecondsSinceLastRead() const
{
	return getMilliseconds() - m_readMilliseconds;
}

void
Connection::doRead()
{
	string s;

	// read from the socket
	char buf[512];
	ssize_t length;
	while((length = recv(m_fd, buf, sizeof(buf) - 1, 0)) > 0) {
		buf[length] = '\0';
		s += buf;

		if(length != sizeof(buf) - 1)
			break;
	}

	if(s.length() != 0) {
		m_readMilliseconds = getMilliseconds();

		stringRead(s);

		// append the string to the buffered line and
		// see if the line has been completely read
		m_line += s;
		size_t tmp;
		while((tmp = m_line.find("\r\n")) != string::npos) {
			s = m_line.substr(0, tmp);
			m_line = m_line.substr(tmp + 2);
			lineRead(s);
		}
	}

	// check if the connection was closed
	if(length == 0)
		closed();
}

void
Connection::sendString(const char *s, size_t size)
{
	while(size != 0) {
		ssize_t length = send(m_fd, s, size, 0);
		if(length == -1)
			break;

		s += length;
		size -= (size_t)length;
	}
}

void
Connection::sendString(const char *s)
{
	sendString(s, strlen(s));
}

void
Connection::sendString(const string &s)
{
	sendString(s.c_str(), s.length());
}

void
Connection::sendLine(const string &line)
{
	sendString(line + "\r\n");
}

void
Connection::sendLine(const char *line)
{
	sendLine(string(line));
}

string
Connection::toString() const
{
	return m_address.toString() + " port " + String::fromUInt(m_port);
}

void
Connection::closed()
{
}

void
Connection::stringRead(const string &/*s*/)
{
}

void
Connection::lineRead(const string &/*line*/)
{
}
