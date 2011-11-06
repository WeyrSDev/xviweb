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

#ifndef __SERVERWORKER_H__
#define __SERVERWORKER_H__

#include <map>
#include <vector>
#include <pthread.h>
#include <xviweb/Mutex.h>
#include <xviweb/Responder.h>
#include "HttpConnection.h"
#include "HttpResponseImpl.h"

typedef std::map<std::string, std::string> ServerMap;

class ServerConnection
{
	public:
		HttpConnection *connection;
		HttpResponseImpl *response;
		ResponderContext *context;
		long wakeupTime;

		ServerConnection(HttpConnection *connectionValue, HttpResponseImpl *responseValue = NULL, ResponderContext *contextValue = NULL);
};

class ServerWorker
{
	private:
		std::string &m_defaultRoot;
		ServerMap &m_vhostMap;
		std::vector <Responder *> &m_responders;

		Mutex *m_connectionsMutex;
		std::vector <ServerConnection> m_connections;

		int m_pipefds[2];

		bool m_running;
		pthread_t m_thread;

		void processRequest(ServerConnection *conn);
		void breakPoll();
		void cycle();

	public:
		ServerWorker(std::string &defaultRoot, ServerMap &vhostMap, std::vector <Responder *> &responders);
		virtual ~ServerWorker();

		void addConnection(HttpConnection *connection);

		void run();
};

#endif /* __SERVERWORKER_H__ */
