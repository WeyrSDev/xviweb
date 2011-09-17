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

#include <stdlib.h>
#include <netdb.h>
#include "Address.h"
#include "String.h"

using namespace std;

Address::Address(const char *hostName)
{
	struct hostent *host;

	// try IPv6 and then IPv4
	host = gethostbyname2(hostName, AF_INET6);
	if(!host) {
		host = gethostbyname(hostName);
		if(!host)
			throw "gethostbyname2 failed";
	}

	m_type = (host->h_addrtype == AF_INET) ? ADDRESS_TYPE_IPV4 : ADDRESS_TYPE_IPV6;
	for(int i = 0; i < 16; ++i)
		m_address[i] = host->h_addr_list[0][i];
}

Address::Address(const uint8_t *address, AddressType type)
{
	m_type = type;
	int length = (type == ADDRESS_TYPE_IPV4) ? 4 : 16;
	for(int i = 0; i < length; ++i)
		m_address[i] = address[i];
}

AddressType
Address::getType() const
{
	return m_type;
}

const uint8_t *
Address::getAddress() const
{
	return m_address;
}

string
Address::toString() const
{
	string s;

	if(m_type == ADDRESS_TYPE_IPV4) {
		for(int i = 0; i < 4; ++i) {
			if(s.length() != 0)
				s += '.';

			s += String::fromUInt((unsigned int)m_address[i]);
		}
	} else {
		for(int i = 0; i < 16; i += 2) {
			if(s.length() != 0)
				s += ':';

			unsigned int n = (unsigned int)m_address[i] << 8;
			n |= (unsigned int)m_address[i+1];
			s += String::hexFromUInt(n);
		}
	}

	return s;
}
