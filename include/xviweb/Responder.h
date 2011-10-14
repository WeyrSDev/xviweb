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

#ifndef __XVIWEB_RESPONDER_H__
#define __XVIWEB_RESPONDER_H__

#include "HttpRequest.h"
#include "HttpResponse.h"

class ResponderContext
{
	public:
		ResponderContext();
		virtual ~ResponderContext();

		virtual ResponderContext *continueResponse(const HttpRequest *request, HttpResponse *response) = 0;
		virtual long getResponseInterval() const;
};

class Responder
{
	public:
		Responder();
		virtual ~Responder();

		virtual void addOption(const std::string &option, const std::string &value);

		virtual bool matchesRequest(const HttpRequest *request) const = 0;
		virtual ResponderContext *respond(const HttpRequest *request, HttpResponse *response) = 0;
};

#define XVIWEB_RESPONDER(CLASSNAME) extern "C" { const char *getResponderName() { return #CLASSNAME; } Responder *createResponder() { return new CLASSNAME(); } void destroyResponder(Responder *p) { delete p; } }

#endif /* __XVIWEB_RESPONDER_H__ */
