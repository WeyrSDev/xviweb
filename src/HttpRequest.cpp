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
#include "HttpRequest.h"

using namespace std;

string
HttpRequest::getVerb() const
{
	return m_verb;
}

string
HttpRequest::getPath() const
{
	return m_path;
}

string
HttpRequest::getVersion() const
{
	return m_version;
}

string
HttpRequest::getQueryStringValue(const string &name) const
{
	HttpRequestMap::const_iterator iter = m_queryStringMap.find(name);
	return (iter != m_queryStringMap.end()) ? iter->second : string("");
}

string
HttpRequest::getHeaderValue(const string &name) const
{
	HttpRequestMap::const_iterator iter = m_headerMap.find(name);
	return (iter != m_headerMap.end()) ? iter->second : string("");
}

void
HttpRequest::parseQueryStringKeyValuePair(const string &pair)
{
	size_t tmp = pair.find('=');
	if(tmp != string::npos) {
		string key = pair.substr(0, tmp);
		string value = pair.substr(tmp + 1);
		m_queryStringMap.insert(make_pair(key, value));
	}
}

void
HttpRequest::parseQueryString(const string &queryString)
{
	size_t start = 0;
	size_t end;

	while((end = queryString.find('&', start)) != string::npos) {
		parseQueryStringKeyValuePair(queryString.substr(start, end - start));
		start = end + 1;
	}

	parseQueryStringKeyValuePair(queryString.substr(start));
}

bool
HttpRequest::parseRequestLine(const string &line)
{
	// parse the verb
	size_t end = line.find(' ');
	if(end == 0 || end == string::npos)
		return false;
	m_verb = line.substr(0, end);

	// parse the path
	size_t start = end + 1;
	end = line.find(' ', start);
	if(end == string::npos)
		return false;
	m_path = line.substr(start, end - start);
	if(m_path[0] != '/')
		return false;

	// parse the HTTP version
	start = end + 1;
	m_version = line.substr(start);

	// parse the query string from the path, if necessary
	start = m_path.find('?');
	if(start != string::npos) {
		string queryString = m_path.substr(start + 1);
		m_path = m_path.substr(0, start);

		parseQueryString(queryString);
	}

	return true;
}

bool
HttpRequest::parseHeaderLine(const string &line)
{
	// parse the name
	size_t end = line.find(':');
	if(end == 0 || end == string::npos)
		return false;
	string name = line.substr(0, end);

	// parse the value
	size_t start = end + 2;
	string value = line.substr(start);

	// add the name/value pair to the header map
	m_headerMap.insert(make_pair(name, value));

	return true;
}
