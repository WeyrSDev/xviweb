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

#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include <string>
#include "Address.h"

class Connection
{
	private:
		int m_fd;
		Address m_address;
		unsigned short m_port;
		unsigned int m_readMilliseconds;

	protected:
		std::string m_line;

	public:
		Connection(int fd, const Address &address, unsigned short port);
		Connection(const Address &address, unsigned short port);
		virtual ~Connection();

		int getFileDescriptor() const;
		const Address &getAddress() const;
		unsigned short getPort() const;
		unsigned int getMillisecondsSinceLastRead() const;

		void doRead();
		void sendString(const char *s, size_t size);
		void sendString(const char *s);
		void sendString(const std::string &s);
		void sendLine(const std::string &line);
		void sendLine(const char *line);

		std::string toString() const;

	protected:
		virtual void closed();
		virtual void stringRead(const std::string &s);
		virtual void lineRead(const std::string &line);
};

#endif /* __CONNECTION_H__ */
