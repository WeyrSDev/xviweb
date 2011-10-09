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

#ifndef __XVIWEB_HTTPRESPONSE_H__
#define __XVIWEB_HTTPRESPONSE_H__

#include <string>

class HttpResponse
{
	public:
		virtual int getStatusCode() const = 0;
		virtual std::string getStatusMessage() const = 0;
		virtual void setStatus(int statusCode, const std::string &statusMessage) = 0;

		virtual std::string getContentType() const = 0;
		virtual void setContentType(const std::string &contentType) = 0;

		virtual int getContentLength() const = 0;
		virtual void setContentLength(int contentLength) = 0;

		virtual std::string getHeaderValue(const std::string &headerName) const = 0;
		virtual void setHeaderValue(const std::string &headerName, const std::string &headerValue) = 0;

		virtual void sendString(const char *s, size_t length) = 0;
		virtual void sendString(const char *s) = 0;
		virtual void sendString(const std::string &s) = 0;
		virtual void sendLine(const char *line) = 0;
		virtual void sendLine(const std::string &line) = 0;

		virtual void sendResponse(int statusCode, const char *statusMessage, const char *contentType, const char *content) = 0;
		virtual void sendErrorResponse(int errorCode, const char *errorDesc, const char *errorMessage) = 0;

		virtual void endResponse() = 0;
};

#endif /* __XVIWEB_HTTPRESPONSE_H__ */
