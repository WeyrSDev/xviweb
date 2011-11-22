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

#ifndef __HTTPRESPONSEIMPL_H__
#define __HTTPRESPONSEIMPL_H__

#include <map>
#include <xviweb/HttpResponse.h>
#include "HttpConnection.h"

typedef std::map<std::string, std::string> HttpResponseMap;

class HttpResponseImpl : public HttpResponse
{
	private:
		HttpConnection *m_conn;
		bool m_responding;

		int m_statusCode;
		std::string m_statusMessage;

		HttpResponseMap m_headerMap;

		void beginResponse();

	public:
		HttpResponseImpl(HttpConnection *conn);

		int getStatusCode() const;
		std::string getStatusMessage() const;
		void setStatus(int statusCode, const std::string &statusMessage);

		std::string getContentType() const;
		void setContentType(const std::string &contentType);

		int getContentLength() const;
		void setContentLength(int contentLength);

		std::string getHeaderValue(const std::string &headerName) const;
		void setHeaderValue(const std::string &headerName, const std::string &headerValue);

		void redirect(const std::string &location);

		void sendString(const char *s, size_t length);
		void sendString(const char *s);
		void sendString(const std::string &s);
		void sendLine(const char *line);
		void sendLine(const std::string &line);

		void sendResponse(int statusCode, const char *statusMessage, const char *contentType, const char *content);
		void sendErrorResponse(int errorCode, const char *errorDesc, const char *errorMessage);

		void endResponse();
};

#endif /* __HTTPRESPONSEIMPL_H__ */
