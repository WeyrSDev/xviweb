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

#include <dlfcn.h>
#include "ResponderModule.h"

ResponderModule::ResponderModule(const char *path)
{
	// open the module
	m_handle = dlopen(path, RTLD_NOW);
	if(!m_handle)
		throw "dlopen failed";

	// get getResponderName function pointer
	m_getResponderName = (GET_RESPONDER_NAME_FN)dlsym(m_handle, "getResponderName");
	if(!m_getResponderName) {
		dlclose(m_handle);
		throw "dlsym for getResponderName failed";
	}

	// get createResponder function pointer
	m_createResponder = (CREATE_RESPONDER_FN)dlsym(m_handle, "createResponder");
	if(!m_createResponder) {
		dlclose(m_handle);
		throw "dlsym for createResponder failed";
	}

	// get destroyResponder function pointer
	m_destroyResponder = (DESTROY_RESPONDER_FN)dlsym(m_handle, "destroyResponder");
	if(!m_destroyResponder) {
		dlclose(m_handle);
		throw "dlsym for destroyResponder failed";
	}

	m_responder = m_createResponder();
}

ResponderModule::~ResponderModule()
{
	m_destroyResponder(m_responder);
	dlclose(m_handle);
}

const char *
ResponderModule::getResponderName() const
{
	return m_getResponderName();
}

Responder *
ResponderModule::getResponder()
{
	return m_responder;
}
