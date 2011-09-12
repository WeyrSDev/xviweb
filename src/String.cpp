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

#include <sstream>
#include "String.h"

using namespace std;

string
String::fromInt(int n)
{
	stringstream stream;
	stream << n;

	return stream.str();
}

string
String::fromUInt(unsigned int n)
{
	stringstream stream;
	stream << n;

	return stream.str();
}

string
String::hexFromUInt(unsigned int n)
{
	if(n == 0)
		return string("0");

	string s;
	while(n > 0) {
		char c;
		unsigned int tmp = n % 16;
		if(tmp > 9)
			c = ('a' + (char)(tmp - 10));
		else
			c = ('0' + (char)tmp);

		s = c + s;
		n /= 16;
	}

	return s;
}

int
String::toInt(const string &s)
{
	int n;
	stringstream stream(s);
	stream >> n;

	return n;
}

unsigned int
String::toUInt(const string &s)
{
	unsigned int n;
	stringstream stream(s);
	stream >> n;

	return n;
}

string
String::toLower(const string &s)
{
	const char diff = 'a' - 'A';
	string t = s;

	size_t length = t.length();
	for(size_t i = 0; i < length; ++i) {
		if(t[i] >= 'A' && t[i] <= 'Z')
			t[i] += diff;
	}

	return t;
}

string
String::toUpper(const string &s)
{
	const char diff = 'A' - 'a';
	string t = s;

	size_t length = t.length();
	for(size_t i = 0; i < length; ++i) {
		if(t[i] >= 'a' && t[i] <= 'z')
			t[i] += diff;
	}

	return t;
}

bool
String::isWhitespace(char c)
{
	return (c == ' ' || c == '\t' || c == '\r' || c == '\n');
}

string
String::trim(const string &s)
{
	size_t length = s.length();
	if(length == 0)
		return s;

	// find the first non-whitespace character
	size_t start = 0;
	while(start != length && isWhitespace(s[start]))
		++start;

	// find the last non-whitespace character
	size_t end = length - 1;
	while(end != 0 && isWhitespace(s[end]))
		--end;

	if(start > end)
		return s;

	return s.substr(start, end - start + 1);
}
