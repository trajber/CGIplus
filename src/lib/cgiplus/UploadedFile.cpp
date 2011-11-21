#include <sstream>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <cgiplus/UploadedFile.hpp>

CGIPLUS_NS_BEGIN

string UploadedFile::getData() const
{
	return _data;
}

string UploadedFile::getFilename() const
{
	return _filename;
}

string UploadedFile::getControlName() const
{
	return _controlName;
}

void UploadedFile::setData(const string &data)
{
	_data = data;
}

void UploadedFile::setFilename(const string &filename)
{
	_filename = filename;
}

void UploadedFile::setControlName(const string &controlName)
{
	_controlName = controlName;
}

void UploadedFile::setMultipart(const string &multipart)
{
	std::stringstream ss(multipart);
	string line;

	// Each part is expected to contain "Content-disposition"
	bool disposition_found = false;

	// try to find headers
	while (std::getline(ss, line)) {
		if (line.find("Content-") != string::npos) {
			// content-disposition is a special case
			if (line.find("Content-Disposition") != string::npos) {
				disposition_found = true;
			}
			parseContentHeader(line);
			continue;
		}

		// CRLF is used to separate lines of data
		boost::trim(line);
		// content-disposition is mandatory
		if (line == "" && disposition_found) {
			break;
		}
	}

	// payload ends with "--"
	string payload = multipart.substr(ss.tellg(),
	                                  multipart.find_last_of("--") - 
	                                  ss.tellg() - 1);

	_data = payload;
}

void UploadedFile::parseContentHeader(const string &contentHeader)
{
	std::vector<string> keysValues;
	boost::split(keysValues, contentHeader, boost::is_any_of(":"));

	if (keysValues.size() != 2) {
		return; // malformed header
	}

	if (keysValues[0] == "Content-Disposition") {
		boost::split(keysValues, keysValues[1], boost::is_any_of(";"));
		foreach(string kv, keysValues) {
			boost::trim(kv);
			if (boost::starts_with(kv, "name=")) {
				size_t pos = kv.find("=");
				_controlName = kv.substr(pos + 2, kv.size() - pos - 3);
			} else if (boost::starts_with(kv,"filename=")) {
				size_t pos = kv.find("=");
				_filename = kv.substr(pos + 2, kv.size() - pos - 3);
			}
		}
	}
}


CGIPLUS_NS_END
