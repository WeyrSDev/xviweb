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

#include <cstring>
#include <xviweb/String.h>
#include "HttpResponseImpl.h"

using namespace std;

HttpResponseImpl::HttpResponseImpl(HttpConnection *conn)
{
	m_conn = conn;
	m_responding = false;

	// set some default values
	setStatus(200, "OK");
	setHeaderValue("Server", "xviweb");
	setContentType("text/html");
}

int
HttpResponseImpl::getStatusCode() const
{
	return m_statusCode;
}

string
HttpResponseImpl::getStatusMessage() const
{
	return m_statusMessage;
}

void
HttpResponseImpl::setStatus(int statusCode, const string &statusMessage)
{
	m_statusCode = statusCode;
	m_statusMessage = statusMessage;
}

string
HttpResponseImpl::getContentType() const
{
	return getHeaderValue("Content-Type");
}

void
HttpResponseImpl::setContentType(const string &contentType)
{
	setHeaderValue("Content-Type", contentType);
}

int
HttpResponseImpl::getContentLength() const
{
	return String::toInt(getHeaderValue("Content-Length"));
}

void
HttpResponseImpl::setContentLength(int contentLength)
{
	setHeaderValue("Content-Length", String::fromInt(contentLength));
}

string
HttpResponseImpl::getHeaderValue(const string &headerName) const
{
	HttpResponseMap::const_iterator iter = m_headerMap.find(headerName);
	return (iter != m_headerMap.end()) ? iter->second : string("");
}

void
HttpResponseImpl::setHeaderValue(const string &headerName,
                                 const string &headerValue)
{
	// see if there's already a value for
	// this header; if so, remove it
	HttpResponseMap::iterator iter = m_headerMap.find(headerName);
	if(iter != m_headerMap.end())
		m_headerMap.erase(iter);

	// insert the new header value
	m_headerMap.insert(make_pair(headerName, headerValue));
}

void
HttpResponseImpl::beginResponse()
{
	m_responding = true;
	m_conn->beginResponse();

	// send status code/message
	string status = "HTTP/1.1 " + String::fromInt(m_statusCode) + " " + m_statusMessage + "\r\n";
	sendString(status);

	// send headers
	HttpResponseMap::iterator iter = m_headerMap.begin();
	while(iter != m_headerMap.end()) {
		string header = iter->first + ": " + iter->second + "\r\n";
		sendString(header);
		++iter;
	}

	// send empty line between headers and response body
	sendString("\r\n");
}

void
HttpResponseImpl::sendString(const char *s, size_t length)
{
	if(m_responding == false)
		beginResponse();

	m_conn->sendString(s, length);
}

void
HttpResponseImpl::sendString(const char *s)
{
	sendString(s, strlen(s));
}

void
HttpResponseImpl::sendString(const string &s)
{
	sendString(s.c_str(), s.length());
}

void
HttpResponseImpl::sendLine(const char *line)
{
	sendLine(string(line));
}

void
HttpResponseImpl::sendLine(const string &line)
{
	sendString(line + "\r\n");
}

void
HttpResponseImpl::sendResponse(int statusCode, const char *statusMessage,
                               const char *contentType, const char *content)
{
	m_conn->beginResponse();

	// set the status, content type, and content length
	setStatus(statusCode, statusMessage);
	setContentType(contentType);
	setContentLength(strlen(content));

	// send content
	sendString(content);
	m_conn->endResponse();
}

void
HttpResponseImpl::sendErrorResponse(int errorCode, const char *errorDesc,
                                    const char *errorMessage)
{
	string code = String::fromInt(errorCode);

	string response = string() +
		"<!DOCTYPE html>\r\n"
		"<html lang=\"en\">\r\n"
		"<head>\r\n"
		"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\r\n"
		"<title>" + code + " " + errorDesc + "</title>\r\n"
		"<style type=\"text/css\">\r\n"
		"body { margin: 0; background-color: white; color: black; font-family: Arial, Helvetica, sans-serif; }\r\n"
		"h1 { margin: 0; padding: 0.5em; background-color: #dedede; color: inherit; text-shadow: gray 1px 1px 4px; }\r\n"
		"p { margin: 0.5em; }\r\n"
		"</style>\r\n"
		"</head>\r\n"
		"<body>\r\n\r\n"
		"<h1>" + errorDesc + "</h1>\r\n"
		"<p>" + errorMessage + "</p>\r\n\r\n"
		"</body>\r\n"
		"</html>\r\n";

	sendResponse(errorCode, errorDesc, "text/html", response.c_str());
}

void
HttpResponseImpl::endResponse()
{
	if(m_responding == false)
		beginResponse();

	m_conn->endResponse();
}
