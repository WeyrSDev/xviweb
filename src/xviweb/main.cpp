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
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <xviweb/String.h>
#include "ResponderModule.h"
#include "Server.h"

#ifndef PROJECT_VERSION
	#define PROJECT_VERSION ""
#endif

using namespace std;

bool g_running = true;

static void
interrupt(int /*param*/)
{
	g_running = false;
}

static void
showOptionDescription(ostream &stream, const string &option,
                      const string &desc)
{
	const unsigned int OPTION_COLUMNS = 25;

	// print the option name
	if(option.length() > OPTION_COLUMNS - 4) {
		stream << "  " << option << endl;
		for(unsigned int i = 0; i < OPTION_COLUMNS; ++i)
			stream << " ";
	} else {
		stream << "  " << option;
		for(unsigned int i = 0; i < OPTION_COLUMNS - 2 - option.length(); ++i)
			stream << " ";
	}

	// print the option description
	size_t start = 0;
	size_t end;
	while((end = desc.find('\n', start)) != string::npos) {
		stream << desc.substr(start, end - start) << endl;
		for(unsigned int i = 0; i < OPTION_COLUMNS; ++i)
			stream << " ";
		start = end + 1;
	}
	stream << desc.substr(start) << endl << endl;
}

static void
showOptionDescription(ostream &stream, const char *option,
                      const char *desc)
{
	showOptionDescription(stream, string(option), string(desc));
}

static void
showUsageMessage(ostream &stream, const char *executableName)
{
	stream << "Usage: " << executableName << " [options]" << endl << endl;

	stream << "Options:" << endl;
	showOptionDescription(stream, "--numWorkers <number>", "Sets the number of worker threads to the given number.\nThe number must be greater than or equal to 1.\nThe default value is 2.");
	showOptionDescription(stream, "--address <address>", "Sets the address that the server binds to.\nThe default value is 127.0.0.1.");
	showOptionDescription(stream, "--port <port>", "Sets the port that the server binds to.\nThe default value is 8080.");
	showOptionDescription(stream, "--defaultRoot <root>", "Sets the default root directory.\nThe default behavior is to have\nno default root directory.");
	showOptionDescription(stream, "--addVHost <hostname> <root>", "Adds a virtual host with the given\nhostname and root directory.");
	showOptionDescription(stream, "--help", "Show this help message.");
	showOptionDescription(stream, "--version", "Show version information.");
}

static bool
missingParameters(const char *path, const char *option,
                  int argc, int i, int numParams)
{
	if(argc <= i + numParams) {
		cerr << "Error: Missing parameter(s) for option " << option << endl << endl;
		showUsageMessage(cerr, path);
		return true;
	}

	return false;
}

int
main(int argc, char *argv[])
{
	vector <ResponderModule *> modules;
	Server *server = new Server();

	// parse command line options
	for(int i = 1; i < argc; ++i) {
		// show the help message
		if(strcmp(argv[i], "--help") == 0) {
			showUsageMessage(cout, argv[0]);
			delete server;
			return 0;
		}

		// show version information
		if(strcmp(argv[i], "--version") == 0) {
			cout << "xviweb " PROJECT_VERSION << endl;
			cout << "Copyright (C) 2011 Josh A. Beam <josh@joshbeam.com>" << endl;
			cout << "There is NO WARRANTY for this software." << endl;
			delete server;
			return 0;
		}

		// set the number of worker threads
		if(strcmp(argv[i], "--numWorkers") == 0) {
			if(missingParameters(argv[0], "--numWorkers", argc, i, 1)) {
				delete server;
				return 1;
			}

			unsigned int numWorkers = String::toUInt(argv[++i]);
			if(numWorkers < 1) {
				cerr << "Error: Argument to --numWorkers option must be greater than or equal to 1." << endl;
				delete server;
				return 1;
			}

			server->setNumWorkers(numWorkers);
			continue;
		}

		// set the address
		if(strcmp(argv[i], "--address") == 0) {
			if(missingParameters(argv[0], "--address", argc, i, 1)) {
				delete server;
				return 1;
			}

			server->setAddress(argv[++i]);
			continue;
		}

		// set the port
		if(strcmp(argv[i], "--port") == 0) {
			if(missingParameters(argv[0], "--port", argc, i, 1)) {
				delete server;
				return 1;
			}

			server->setPort((unsigned short)String::toUInt(argv[++i]));
			continue;
		}

		// set the default root directory
		if(strcmp(argv[i], "--defaultRoot") == 0) {
			if(missingParameters(argv[0], "--defaultRoot", argc, i, 1)) {
				delete server;
				return 1;
			}

			server->setDefaultRoot(argv[++i]);
			continue;
		}

		// add a vhost
		if(strcmp(argv[i], "--addVHost") == 0) {
			if(missingParameters(argv[0], "--addVHost", argc, i, 2)) {
				delete server;
				return 1;
			}

			const char *hostname = argv[++i];
			const char *root = argv[++i];
			server->addVHost(hostname, root);
			continue;
		}

		// load responder
		if(strcmp(argv[i], "--loadResponder") == 0) {
			if(missingParameters(argv[0], "--loadResponder", argc, i, 1)) {
				delete server;
				return 1;
			}

			const char *path = argv[++i];
			try {
				ResponderModule *module = new ResponderModule(path);
				modules.push_back(module);
				cout << "Loaded responder " << module->getResponderName() << " from " << path << endl;
			} catch(const char *ex) {
				cerr << "Error loading " << path << ": " << ex << endl;
			}
			continue;
		}

		cerr << "Unknown option: " << argv[i] << endl << endl;
		showUsageMessage(cerr, argv[0]);
		return 1;
	}

	signal(SIGINT, interrupt);

	// attach responders to the server
	for(unsigned int i = 0; i < modules.size(); ++i)
		server->attachResponder(modules[i]->getResponder());

	// start the server
	try {
		server->start();
	} catch(const char *ex) {
		cerr << "Error starting server: " << ex << endl;

		// delete server and responder modules
		delete server;
		for(unsigned int i = 0; i < modules.size(); ++i)
			delete modules[i];

		return 1;
	}

	cout << "Listening for connections at " << server->getAddress().toString() << " port " << server->getPort() << endl;

	while(g_running) {
		try {
			server->cycle();
		} catch(const char *ex) {
			cerr << "Error during cycle: " << ex << endl;
		}
	}

	cout << endl << "Stopping server..." << endl;
	delete server;

	// delete responder modules
	for(unsigned int i = 0; i < modules.size(); ++i) {
		cout << "Unloading responder " << modules[i]->getResponderName() << "..." << endl;
		delete modules[i];
	}

	cout << "Done" << endl;
	return 0;
}
