#ifndef __CGIPLUS_UPLOADED_FILE_H__
#define __CGIPLUS_UPLOADED_FILE_H__

#include "Cgiplus.hpp"

#include <string>

using std::string;

CGIPLUS_NS_BEGIN

class UploadedFile
{
public:
	string getData() const;
	string getFilename() const;
	string getControlName() const;

	void setData(const string &data);
	void setFilename(const string &filename);
	void setControlName(const string &inputName);

	void setMultipart(const string &multipart);
private:
	void parseContentHeader(const string &contentHeader);
	string _data;
	string _filename;
	string _controlName;
};

CGIPLUS_NS_END


#endif // __CGIPLUS_UPLOADED_FILE_H__
