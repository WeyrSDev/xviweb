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
#include <fcntl.h>
#include <sys/stat.h>
#include <xviweb/String.h>
#include "FileResponder.h"

using namespace std;

FileResponder::FileResponder()
{
	m_rootDirectory = ".";

	// add some standard MIME types
	addMimeType("text/plain", "txt");
	addMimeType("text/html", "html");
	addMimeType("text/css", "css");
	addMimeType("text/javascript", "js");
	addMimeType("image/png", "png");
	addMimeType("image/jpg", "jpeg");
	addMimeType("image/jpg", "jpg");
	addMimeType("image/gif", "gif");
}

FileResponder::~FileResponder()
{
}

void
FileResponder::setRootDirectory(const std::string &dir)
{
	m_rootDirectory = dir;
}

string
FileResponder::getRootDirectory() const
{
	return m_rootDirectory;
}

void
FileResponder::addMimeType(const string &type, const string &fileExtension)
{
	m_mimeTypes.push_back(type);
	m_mimeFileExtensions.push_back(string(".") + fileExtension);
}

string
FileResponder::getMimeTypeForFile(const string &path) const
{
	// find the type associated with the file extension
	for(unsigned int i = 0; i < m_mimeTypes.size(); ++i) {
		if(String::endsWith(path, m_mimeFileExtensions[i], true))
			return m_mimeTypes[i];
	}

	return string("");
}

static string
mapPath(const string &path)
{
	// not confident that the commented out code below
	// works right yet, so just return an empty string
	// if the requested path contains a ..
	if(path.find("..") != string::npos)
		return string("");

	string newPath = path;

	// replace back slashes with forward slashes
	size_t tmp;
	while((tmp = newPath.find('\\')) != string::npos)
		newPath[tmp] = '/';

	// replace double slashes with single slashes
	while((tmp = newPath.find("//")) != string::npos)
		newPath.erase(tmp, 1);

#if 0
	// make sure .. isn't used to go above the root directory
	int depth = 0;
	for(int i = 0; i < (int)newPath.length() - 3; ++i) {
		if(newPath[i] == '/') {
			if(newPath[i+1] == '.' && newPath[i+2] == '.' && newPath[i+3] == '/') {
				if(--depth < 0)
					return string("");
				i += 3;
			} else {
				++depth;
			}
		}
	}
#endif

	return newPath;
}

bool
FileResponder::matchesRequest(const HttpRequest * /*request*/) const
{
	return true;
}

ResponderContext *
FileResponder::respond(const HttpRequest *request, HttpResponse *response)
{
	// if the path returned by mapPath has a length
	// of zero, the path requested is invalid
	string path = mapPath(request->getPath());
	if(path.length() == 0) {
		response->sendErrorResponse(403, "Forbidden", "The provided file path is invalid.");
		return NULL;
	}

	path = m_rootDirectory + path;

	// open the file and get its status
	struct stat status;
	int fd = open(path.c_str(), O_RDONLY);
	if(fd == -1 || fstat(fd, &status) == -1) {
		if(fd != -1)
			close(fd);
		response->sendErrorResponse(404, "File Not Found", "The file that you requested does not exist.");
		return NULL;
	}

	// don't show directory listings
	if(S_ISDIR(status.st_mode)) {
		close(fd);
		response->sendErrorResponse(403, "Forbidden", "You do not have access to directory listings.");
		return NULL;
	}

	// get the MIME type for the file; if there's no
	// MIME type associated with the file extension,
	// just refuse to serve the file for security reasons
	string contentType = getMimeTypeForFile(path);
	if(contentType.length() == 0) {
		close(fd);
		response->sendErrorResponse(403, "Forbidden", "You do not have access to files of this type.");
		return NULL;
	}

	response->setStatus(200, "OK");
	response->setContentType(contentType);
	response->setContentLength((int)status.st_size);

	if(request->getVerb() != "HEAD") {
		// read the file and send it to the client
		char buf[512];
		ssize_t length;
		while((length = read(fd, buf, sizeof(buf))) > 0)
			response->sendString(buf, length);
	}

	response->endResponse();

	close(fd);
	return NULL;
}
