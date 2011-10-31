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

#ifndef __HTTPREQUESTIMPL_H__
#define __HTTPREQUESTIMPL_H__

#include <map>
#include <xviweb/HttpRequest.h>

typedef std::map<std::string, std::string> HttpRequestMap;

class HttpRequestImpl : public HttpRequest
{
	private:
		std::string m_verb;
		std::string m_path;
		std::string m_version;
		std::string m_vhostRoot;

		HttpRequestMap m_queryStringMap;
		HttpRequestMap m_headerMap;
		HttpRequestMap m_postDataMap;

	public:
		std::string getVerb() const;
		std::string getPath() const;
		std::string getVersion() const;
		std::string getVHostRoot() const;
		std::string getQueryStringValue(const std::string &name) const;
		std::string getHeaderValue(const std::string &name) const;
		std::string getPostDataValue(const std::string &name) const;

		bool parseRequestLine(const std::string &line);
		bool parseHeaderLine(const std::string &line);
		bool parsePostData(const std::string &line);
		void setVHostRoot(const std::string &root);
};

#endif /* __HTTPREQUESTIMPL_H__ */
