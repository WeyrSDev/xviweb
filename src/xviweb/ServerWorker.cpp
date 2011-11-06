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
#include <unistd.h>
#include <poll.h>
#include <xviweb/String.h>
#include "ServerWorker.h"
#include "Util.h"

using namespace std;

ServerConnection::ServerConnection(HttpConnection *connectionValue,
                                   HttpResponseImpl *responseValue,
                                   ResponderContext *contextValue)
{
	connection = connectionValue;
	response = responseValue;
	context = contextValue;
	wakeupTime = 0;
}

static void *
runWorker(void *vworker)
{
	ServerWorker *worker = (ServerWorker *)vworker;
	worker->run();
	return NULL;
}

ServerWorker::ServerWorker(string &defaultRoot, ServerMap &vhostMap,
                           vector <Responder *> &responders)
 : m_defaultRoot(defaultRoot), m_vhostMap(vhostMap),
   m_responders(responders)
{
	m_connectionsMutex = Mutex::create();

	// initialize pthread attributes
	pthread_attr_t attr;
	if(pthread_attr_init(&attr) != 0)
		throw "pthread_attr_init failed";

	// use system scope
	if(pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM) != 0) {
		pthread_attr_destroy(&attr);
		throw "pthread_attr_setscope failed";
	}

	// create a pipe; this is just used to interrupt
	// the poll in the cycle function
	if(pipe(m_pipefds) == -1) {
		pthread_attr_destroy(&attr);
		throw "pipe creation failed";
	}

	// start the worker thread
	m_running = true;
	if(pthread_create(&m_thread, NULL, runWorker, this) != 0) {
		close(m_pipefds[0]);
		close(m_pipefds[1]);
		pthread_attr_destroy(&attr);
		throw "pthread_create failed";
	}

	pthread_attr_destroy(&attr);
}

ServerWorker::~ServerWorker()
{
	// stop the worker thread
	m_running = false;
	breakPoll();
	if(pthread_join(m_thread, NULL) == -1)
		throw "pthread_join failed";

	// close pipe
	close(m_pipefds[0]);
	close(m_pipefds[1]);

	delete m_connectionsMutex;

	// delete all connection data
	for(unsigned int i = 0; i < m_connections.size(); ++i) {
		if(m_connections[i].context != NULL)
			delete m_connections[i].context;
		if(m_connections[i].response != NULL)
			delete m_connections[i].response;
		if(m_connections[i].connection != NULL)
			delete m_connections[i].connection;
	}
}

void
ServerWorker::addConnection(HttpConnection *connection)
{
	breakPoll();
	m_connectionsMutex->lock();
	m_connections.push_back(ServerConnection(connection));
	m_connectionsMutex->unlock();
}

void
ServerWorker::processRequest(ServerConnection *conn)
{
	// create HttpResponse for the connection
	conn->response = new HttpResponseImpl(conn->connection);

	// set the request's vhost root
	HttpRequestImpl *request = conn->connection->getRequest();
	ServerMap::const_iterator iter = m_vhostMap.find(String::toLower(request->getHeaderValue("Host")));
	if(iter != m_vhostMap.end()) {
		request->setVHostRoot(iter->second);
	} else {
		// no vhost found for the given host; use the
		// default root if one is set, otherwise end
		// the response
		if(m_defaultRoot.length() != 0) {
			request->setVHostRoot(m_defaultRoot);
		} else {
			string message = "Your request could not be processed because there is no virtual host associated with " + request->getHeaderValue("Host") + ".";
			conn->response->sendErrorResponse(500, "No Virtual Host", message.c_str());
			return;
		}
	}

	// loop through responders and stop when
	// one matches the request
	unsigned int i;
	for(i = 0; i < m_responders.size(); ++i) {
		Responder *responder = m_responders[i];
		if(responder->matchesRequest(conn->connection->getRequest())) {
			// respond to the request
			conn->context = responder->respond(conn->connection->getRequest(), conn->response);
			if(conn->context != NULL)
				conn->wakeupTime = getMilliseconds() + conn->context->getResponseInterval();
			break;
		}
	}

	// just end the response if no responders handled it
	if(i == m_responders.size())
		conn->response->sendErrorResponse(500, "No Responder", "Your request could not be processed because there is no module loaded that is capable of handing the request.");
}

void
ServerWorker::breakPoll()
{
	// write a byte to the write end
	// of the pipe to interrupt the poll
	char tmp = 0;
	write(m_pipefds[1], &tmp, 1);
}

void
ServerWorker::cycle()
{
	long currentTime = getMilliseconds();
	long sleepTime = 1000;

	// process connections
	for(unsigned int i = 0; i < m_connections.size(); ++i) {
		ServerConnection *sconn = &(m_connections[i]);
		HttpConnection *conn = sconn->connection;

		HttpConnectionState state = conn->getState();
		bool done = (state == HTTP_CONNECTION_STATE_DONE) ||
		            (state != HTTP_CONNECTION_STATE_SENDING_RESPONSE &&
		             conn->getMillisecondsSinceLastRead() > 10000);

		// remove connections in the done state or continue
		// responses for ones that have associated contexts
		if(done) {
			delete conn;
			if(sconn->response != NULL)
				delete sconn->response;
			if(sconn->context != NULL)
				delete sconn->context;
			m_connections.erase(m_connections.begin() + (i--));
		} else if(sconn->context != NULL) {
			if(sconn->wakeupTime <= currentTime) {
				// continue the context's response; it may return
				// a pointer to itself, a pointer to a new context,
				// or null if it's done
				sconn->context = sconn->context->continueResponse(conn->getRequest(), sconn->response);
				if(sconn->context != NULL)
					sconn->wakeupTime = currentTime + sconn->context->getResponseInterval();
			} else {
				long timeDiff = sconn->wakeupTime - currentTime;
				if(timeDiff < sleepTime)
					sleepTime = timeDiff;
			}
		}
	}

	// create pollfd array
	nfds_t pipefdIndex = (nfds_t)m_connections.size();
	nfds_t nfds = pipefdIndex + 1;
	struct pollfd *fds = new struct pollfd[nfds];

	// poll the read end of the pipe so
	// that the poll can be interrupted
	fds[pipefdIndex].fd = m_pipefds[0];
	fds[pipefdIndex].events = POLLIN;
	fds[pipefdIndex].revents = 0;

	// add each connection to fds
	for(unsigned int i = 0; i < m_connections.size(); ++i) {
		fds[i].fd = m_connections[i].connection->getFileDescriptor();
		fds[i].events = POLLIN;
		fds[i].revents = 0;
	}

	// poll connection sockets
	if(poll(fds, nfds, sleepTime) > 0) {
		// read byte from pipe if necessary
		if((fds[pipefdIndex].revents & POLLIN) != 0) {
			char tmp;
			read(m_pipefds[0], &tmp, 1);
		}

		// read from connections
		for(unsigned int i = 0; i < m_connections.size(); ++i) {
			if((fds[i].revents & POLLIN) == 0)
				continue;

			// read from the connection
			HttpConnection *conn = m_connections[i].connection;
			conn->doRead();

			switch(conn->getState()) {
				default:
					break;
				case HTTP_CONNECTION_STATE_RECEIVED_REQUEST:
					// full request received
					processRequest(&m_connections[i]);
					break;
			}
		}
	}

	delete [] fds;
}

void
ServerWorker::run()
{
	while(m_running) {
		m_connectionsMutex->lock();
		cycle();
		m_connectionsMutex->unlock();
	}
}
