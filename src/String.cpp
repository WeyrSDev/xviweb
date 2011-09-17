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
	int n = 0;
	stringstream stream(s);
	stream >> n;

	return n;
}

unsigned int
String::toUInt(const string &s)
{
	unsigned int n = 0;
	stringstream stream(s);
	stream >> n;

	return n;
}

unsigned int
String::hexToUInt(const string &s, size_t index, size_t length)
{
	unsigned int value = 0;

	// set length to be one higher than the index
	// of the last character to consider; if length
	// is zero, go to the end of the string
	if(length == 0) {
		length = s.length();
	} else {
		length += index;
		if(length > s.length())
			length = s.length();
	}

	while(index < length) {
		char c = s[index++];
		if(c >= '0' && c <= '9') {
			value *= 16;
			value += (unsigned int)(c - '0');
		} else if(c >= 'a' && c <= 'f') {
			value *= 16;
			value += (unsigned int)(c - 'a' + 10);
		} else if(c >= 'A' && c <= 'F') {
			value *= 16;
			value += (unsigned int)(c - 'A' + 10);
		} else {
			break;
		}
	}

	return value;
}

unsigned int
String::hexToUInt(const string &s, size_t index)
{
	return hexToUInt(s, index, 0);
}

unsigned int
String::hexToUInt(const string &s)
{
	return hexToUInt(s, 0, 0);
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

string
String::htmlEncode(const string &s)
{
	const char *specialChars = "<>&\"";

	size_t tmp = s.find_first_of(specialChars);
	if(tmp == string::npos) {
		return s;
	} else {
		string t = s;

		do {
			switch(t[tmp]) {
				default:
					++tmp;
					break;
				case '<':
					t.replace(tmp, 1, "&lt;");
					tmp += 4;
					break;
				case '>':
					t.replace(tmp, 1, "&gt;");
					tmp += 4;
					break;
				case '&':
					t.replace(tmp, 1, "&amp;");
					tmp += 5;
					break;
				case '"':
					t.replace(tmp, 1, "&quot;");
					tmp += 6;
					break;
			}
		} while((tmp = t.find_first_of(specialChars, tmp)) != string::npos);

		return t;
	}
}

string
String::urlDecode(const string &s)
{
	const char *specialChars = "+%";

	size_t tmp = s.find_first_of(specialChars);
	if(tmp == string::npos) {
		return s;
	} else {
		string t = s;
		size_t length = t.length();

		do {
			switch(t[tmp]) {
				default:
					break;
				case '+':
					t[tmp] = ' ';
					break;
				case '%':
					if(tmp + 2 < length)
						t.replace(tmp, 3, 1, (char)hexToUInt(t, tmp + 1, 2));
					break;
			}

			++tmp;
		} while((tmp = t.find_first_of(specialChars, tmp)) != string::npos);

		return t;
	}
}

bool
String::endsWith(const string &s1, const string &s2, bool ignoreCase)
{
	const char diff = 'a' - 'A';

	if(s1.length() < s2.length())
		return false;

	size_t i = s1.length() - s2.length();

	if(ignoreCase) {
		// do a case insensitive comparison of the
		// end of the first string to the second
		for(size_t j = 0; j < s2.length(); ++j) {
			char c1 = s1[i++];
			char c2 = s2[j];

			// convert upper-case characters to lower-case
			if(c1 >= 'A' && c1 <= 'Z')
				c1 += diff;
			if(c2 >= 'A' && c2 <= 'Z')
				c2 += diff;

			if(c1 != c2)
				return false;
		}
	} else {
		// do a case sensitive comparison of the
		// end of the first string to the second
		for(size_t j = 0; j < s2.length(); ++j) {
			char c1 = s1[i++];
			char c2 = s2[j];

			if(c1 != c2)
				return false;
		}
	}

	return true;
}

bool
String::endsWith(const string &s1, const string &s2)
{
	return endsWith(s1, s2, false);
}

vector <string>
String::split(const string &s, const string &delimiter)
{
	vector <string> v;
	size_t length = delimiter.length();
	size_t start = 0;
	size_t end;

	while((end = s.find(delimiter, start)) != string::npos) {
		v.push_back(s.substr(start, end - start));
		start = end + length;
	}

	v.push_back(s.substr(start));
	return v;
}
