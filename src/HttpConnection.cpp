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
#include "HttpConnection.h"
#include "String.h"

using namespace std;

HttpConnection::HttpConnection(int fd, const Address &address,
                               unsigned short port)
 : Connection(fd, address, port)
{
	m_state = HTTP_CONNECTION_STATE_AWAITING_REQUEST;
	m_bytesRead = 0;
	m_contentLength = 0;
}

HttpConnection::~HttpConnection()
{
}

HttpConnectionState
HttpConnection::getState() const
{
	return m_state;
}

const HttpRequest &
HttpConnection::getRequest() const
{
	return m_request;
}

void
HttpConnection::closed()
{
	m_state = HTTP_CONNECTION_STATE_DONE;
}

void
HttpConnection::beginResponse(int responseCode, const char *responseDesc)
{
	m_state = HTTP_CONNECTION_STATE_SENDING_RESPONSE;

	sendString("HTTP/1.1 ");
	sendString(String::fromInt(responseCode));
	sendString(" ");
	sendLine(responseDesc);

#ifdef PROJECT_VERSION
	sendLine("Server: xviweb/" PROJECT_VERSION);
#else
	sendLine("Server: xviweb");
#endif
}

void
HttpConnection::endResponse()
{
	m_state = HTTP_CONNECTION_STATE_DONE;
}

void
HttpConnection::sendResponse(int responseCode, const char *responseDesc,
                             const char *contentType, const char *content)
{
	beginResponse(responseCode, responseDesc);

	// send Content-Type header
	sendString("Content-Type: ");
	sendLine(contentType);

	// send Content-Length header
	sendString("Content-Length: ");
	sendLine(String::fromUInt(strlen(content)));

	// send content
	sendLine("");
	if(m_request.getVerb() != "HEAD")
		sendLine(content);

	endResponse();
}

void
HttpConnection::sendErrorResponse(int errorCode, const char *errorDesc,
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
HttpConnection::sendBadRequestResponse()
{
	cerr << toString() << ": Bad request" << endl;
	sendErrorResponse(400, "Bad Request", "Your request could not be understood.");
}

void
HttpConnection::postDataRead(const string &s)
{
	m_postData += s;

	// parse post data if all of it has been read
	if(m_postData.length() == m_contentLength) {
		if(m_request.parsePostData(m_postData) == false)
			sendBadRequestResponse();
		else
			m_state = HTTP_CONNECTION_STATE_RECEIVED_REQUEST;
	} else if(m_postData.length() > m_contentLength) {
		sendBadRequestResponse();
	}
}

void
HttpConnection::stringRead(const string &s)
{
	const unsigned int maxRequestSize = 8 * 1024;

	m_bytesRead += s.length();
	if(m_bytesRead > maxRequestSize) {
		m_state = HTTP_CONNECTION_STATE_DONE;
		cerr << toString() << ": Maximum request size exceeded" << endl;
	}

	if(m_state == HTTP_CONNECTION_STATE_READING_POST_DATA)
		postDataRead(s);
}

void
HttpConnection::lineRead(const string &line)
{
	// perform the appropriate action based on the current state
	switch(m_state) {
		default:
			break;

		// parse the first line of the request containing
		// the verb, path, and HTTP version
		case HTTP_CONNECTION_STATE_AWAITING_REQUEST:
			if(m_request.parseRequestLine(line) == false) {
				sendBadRequestResponse();
			} else {
				m_state = HTTP_CONNECTION_STATE_READING_HEADERS;
				cout << toString() << ": Received request: " << line << endl;
			}
			break;

		// parse headers until a blank line is received
		case HTTP_CONNECTION_STATE_READING_HEADERS:
			if(line.size() == 0) {
				if(m_request.getVerb() == "POST") {
					// start reading post data
					m_state = HTTP_CONNECTION_STATE_READING_POST_DATA;
					m_contentLength = String::toUInt(m_request.getHeaderValue("Content-Length"));
					postDataRead(m_line);
				} else {
					m_state = HTTP_CONNECTION_STATE_RECEIVED_REQUEST;
				}
			} else {
				if(m_request.parseHeaderLine(line) == false)
					sendBadRequestResponse();
			}
			break;
	}
}
