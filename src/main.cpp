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

#include <csignal>
#include <iostream>
#include "Server.h"
#include "FileResponder.h"

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
	showOptionDescription(stream, "--address <address>", "Sets the address that the server binds to.\nThe default value is 127.0.0.1.");
	showOptionDescription(stream, "--port <port>", "Sets the port that the server binds to.\nThe default value is 8080.");
	showOptionDescription(stream, "--rootDirectory <path>", "Sets the root directory path to serve files from.");
	showOptionDescription(stream, "--addMimeType <type> <file extension>", "Associates a MIME type with a file extension.");
	showOptionDescription(stream, "--help", "Show this help message.");
	showOptionDescription(stream, "--version", "Show version information.");
}

int
main(int argc, char *argv[])
{
	const char *addressString = "127.0.0.1";
	unsigned short port = 8080;
	FileResponder *fileResponder = new FileResponder();

	// parse command line options
	for(int i = 1; i < argc; ++i) {
		// show the help message
		if(strcmp(argv[i], "--help") == 0) {
			showUsageMessage(cout, argv[0]);
			return 0;
		}

		// show version information
		if(strcmp(argv[i], "--version") == 0) {
			cout << "xviweb " PROJECT_VERSION << endl;
			cout << "Copyright (C) 2011 Josh A. Beam <josh@joshbeam.com>" << endl;
			cout << "There is NO WARRANTY for this software." << endl;
			return 0;
		}

		// set the address
		if(strcmp(argv[i], "--address") == 0) {
			addressString = argv[++i];
			continue;
		}

		// set the port
		if(strcmp(argv[i], "--port") == 0) {
			port = (unsigned short)atoi(argv[++i]);
			continue;
		}

		// set the root directory
		if(strcmp(argv[i], "--rootDirectory") == 0) {
			fileResponder->setRootDirectory(string(argv[++i]));
			continue;
		}

		// add MIME type
		if(strcmp(argv[i], "--addMimeType") == 0) {
			string mimeType = argv[++i];
			string mimeFileExtension = argv[++i];
			fileResponder->addMimeType(mimeType, mimeFileExtension);
			continue;
		}

		cerr << "Unknown option: " << argv[i] << endl << endl;
		showUsageMessage(cerr, argv[0]);
		return 1;
	}

	signal(SIGINT, interrupt);

	Address address(addressString);
	Server *server;

	try {
		server = new Server(address, port);
	} catch(const char *ex) {
		cerr << "Error starting server: " << ex << endl;
		delete fileResponder;
		return 1;
	}

	// attach responders to the server
	server->attachResponder(fileResponder);

	cout << "Listening for connections at " << address.toString() << " port " << port << endl;

	while(g_running) {
		try {
			server->cycle();
		} catch(const char *ex) {
			cerr << "Error during cycle: " << ex << endl;
		}
	}

	cout << endl << "Stopping server..." << endl;
	delete server;

	cout << "Done" << endl;
	return 0;
}
