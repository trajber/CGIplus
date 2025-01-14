/*
  CGIplus Copyright (C) 2011 Rafael Dantas Justo

  This file is part of CGIplus.

  CGIplus is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  CGIplus is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with CGIplus.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>
#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>

#include <boost/assign/std/vector.hpp>
#include <boost/typeof/std/string.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

#include <cgiplus/Cgi.hpp>

using namespace boost::assign; // bring 'operator+=()' into scope

CGIPLUS_NS_BEGIN

Cgi::Cgi() :
	_method(Method::UNKNOWN),
	_remoteAddress("")
{
	readInputs();
}

string Cgi::operator[](const string &key)
{
	BOOST_AUTO(input, _inputs.find(key));
	if (input != _inputs.end()) {
		return input->second;
	}

	return "";
}

string Cgi::operator()(const string &key)
{
	BOOST_AUTO(cookie, _cookies.find(key));
	if (cookie != _cookies.end()) {
		return cookie->second;
	}

	return "";
}

void Cgi::readInputs()
{
	clearInputs();
	readMethod();
	readGetInputs();
	readPostInputs();
	readCookies();
	readRemoteAddress();
}

Cgi::Method::Value Cgi::getMethod() const
{
	return _method;
}

unsigned int Cgi::getNumberOfInputs() const
{
	return _inputs.size();
}

unsigned int Cgi::getNumberOfCookies() const
{
	return _cookies.size();
}

string Cgi::getRemoteAddress() const
{
	return _remoteAddress;
}

void Cgi::clearInputs()
{
	_method = Method::UNKNOWN;
	_inputs.clear();
	_cookies.clear();
	_remoteAddress.clear();
}

void Cgi::readMethod()
{
	const char *methodPtr = getenv("REQUEST_METHOD");
	if (methodPtr != NULL) {
		string method = boost::to_upper_copy((string) methodPtr);
		if (method == "GET") {
			_method = Method::GET;
		} else if (method == "POST") {
			_method = Method::POST;
		}
	}
}

void Cgi::readGetInputs()
{
	const char *inputsPtr = getenv("QUERY_STRING");
	if (inputsPtr == NULL) {
		return;
	}

	string inputs = inputsPtr;
	parse(inputs);
}

void Cgi::readPostInputs()
{
	const char *sizePtr = getenv("CONTENT_LENGTH");
	const char *typePtr = getenv("CONTENT_TYPE");

	if (_method != Method::POST || sizePtr == NULL || typePtr == NULL) {
		return;
	}

	string type = typePtr;

	int size = 0;
	try {
		size = boost::lexical_cast<int>(sizePtr);
	} catch (const  boost::bad_lexical_cast &e) {}

	if (size == 0) {
		return;
	}

	char inputsPtr[size + 1];
	memset(inputsPtr, 0, size + 1);

	std::cin.read(inputsPtr, size);
	if (std::cin.good() == false) {
		return;
	}

	string inputs = inputsPtr;

	if (type == "application/x-www-form-urlencoded") {
		parse(inputs);
	}

	if (type.find("multipart/form-data") != string::npos) {
		std::vector<string> splittedData;
		boost::split(splittedData, type, boost::is_any_of(";"));

		if (splittedData.size() != 2) {
			// TODO: malformed multipart data
		}

		string boundaryText = splittedData[1];

		boost::split(splittedData, boundaryText, boost::is_any_of("="));
		if (splittedData.size() != 2) {
			// TODO: malformed multipart data
		}

		string boundary = splittedData[1];

		UploadedFile uploadedFile = parseMultipart(inputs, boundary);
	}
}

void Cgi::readCookies()
{
	const char *cookiesPtr = getenv("HTTP_COOKIE");
	if (cookiesPtr == NULL) {
		return;
	}

	string cookies = cookiesPtr;

	std::vector<string> keysValues;
	boost::split(keysValues, cookies, boost::is_any_of("; "));

	foreach (string &keyValue, keysValues) {
		std::vector<string> keyValueSplitted;
		boost::split(keyValueSplitted, keyValue, boost::is_any_of("="));
		if (keyValueSplitted.size() == 2) {
			_cookies[keyValueSplitted[0]] = keyValueSplitted[1];
		}
	}
}

void Cgi::readRemoteAddress()
{
	const char *remoteAddressPtr = getenv("REMOTE_ADDR");
	if (remoteAddressPtr == NULL) {
		return;
	}

	_remoteAddress = remoteAddressPtr;
}

void Cgi::parse(string inputs)
{
	decode(inputs);

	std::vector<string> keysValues;
	boost::split(keysValues, inputs, boost::is_any_of("&"));

	foreach (string keyValue, keysValues) {
		std::vector<string> keyValueSplitted;
		boost::split(keyValueSplitted, keyValue, boost::is_any_of("="));
		if (keyValueSplitted.size() == 2) {
			_inputs[keyValueSplitted[0]] = keyValueSplitted[1];
		}
	}
}

UploadedFile Cgi::parseMultipart(string &inputs, string &boundary)
{
	UploadedFile uploadedFile;
	size_t pos = 0;
	size_t first_occurrence, second_occurrence = 0;

	while (1) {
		first_occurrence = inputs.find(boundary, pos);
		if (first_occurrence == string::npos) {
			break;
		}

		pos += first_occurrence + boundary.size();
		second_occurrence = inputs.find(boundary, pos);
		if (second_occurrence == string::npos) {
			break;
		}

		string partial_data;
		partial_data = inputs.substr(first_occurrence + boundary.size(),
		                             second_occurrence -
		                             (first_occurrence + boundary.size()));

		uploadedFile.setMultipart(partial_data);

		pos += second_occurrence + boundary.size();
	}

	return uploadedFile;
}

void Cgi::decode(string &inputs)
{
	boost::trim(inputs);
	decodeSpecialSymbols(inputs);
	decodeHexadecimal(inputs);
	removeDangerousHtmlCharacters(inputs);
}

void Cgi::decodeSpecialSymbols(string &inputs)
{
	boost::replace_all(inputs, "+", " ");
}

void Cgi::decodeHexadecimal(string &inputs)
{
	boost::match_results<string::const_iterator> found;
	boost::regex hexadecimal("%[0-9A-F][0-9A-F]");

	while (boost::regex_search(inputs, found, hexadecimal)) {
		string finalHexadecimal = boost::to_upper_copy(found.str());
		string text = hexadecimalToText(finalHexadecimal);
		boost::replace_all(inputs, found.str(), text);
	}
}

string Cgi::hexadecimalToText(const string &hexadecimal)
{
	std::stringstream hexadecimalStream;
	hexadecimalStream << std::hex << hexadecimal.substr(1);

	unsigned int hexadecimalNumber = 0;
	hexadecimalStream >> hexadecimalNumber;

	string text("");
	text += char(hexadecimalNumber);
	return text;
}

void Cgi::removeDangerousHtmlCharacters(string &inputs)
{
	boost::replace_all(inputs, "'", "");
	boost::replace_all(inputs, "\"", "");
	boost::replace_all(inputs, "<", "");
	boost::replace_all(inputs, ">", "");
}

CGIPLUS_NS_END
