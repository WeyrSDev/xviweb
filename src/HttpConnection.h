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

#ifndef __HTTPCONNECTION_H__
#define __HTTPCONNECTION_H__

#include "Connection.h"
#include "HttpRequest.h"

enum HttpConnectionState
{
	HTTP_CONNECTION_STATE_AWAITING_REQUEST = 0,
	HTTP_CONNECTION_STATE_READING_HEADERS,
	HTTP_CONNECTION_STATE_READING_POST_DATA,
	HTTP_CONNECTION_STATE_RECEIVED_REQUEST,
	HTTP_CONNECTION_STATE_SENDING_RESPONSE,
	HTTP_CONNECTION_STATE_DONE
};

class HttpConnection : public Connection
{
	private:
		HttpConnectionState m_state;
		HttpRequest m_request;
		unsigned int m_bytesRead;
		unsigned int m_contentLength;
		std::string m_postData;

	public:
		HttpConnection(int fd, const Address &address, unsigned short port);
		virtual ~HttpConnection();

		HttpConnectionState getState() const;
		const HttpRequest &getRequest() const;

		void beginResponse(int responseCode, const char *responseDesc);
		void endResponse();
		void sendResponse(int responseCode, const char *responseDesc, const char *contentType, const char *content);
		void sendErrorResponse(int errorCode, const char *errorDesc, const char *errorMessage);
		void sendBadRequestResponse();

	protected:
		virtual void closed();
		void postDataRead(const std::string &s);
		virtual void stringRead(const std::string &s);
		virtual void lineRead(const std::string &line);
};

#endif /* __HTTPCONNECTION_H__ */
