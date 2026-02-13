#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <exception>
#include <map>
#include <dirent.h>
#include <sstream>
#include <vector>
#include <algorithm>
#include "httpRequest.hpp"

class httpResponse
{
	public:
		httpResponse();
		// httpResponse(const HttpRequest &req);
		~httpResponse(){}
	
	private:
		int _statusCode;
		std::string _version;
		std::string _statusMsg;
		std::map<std::string, std::string> _headers;
		std::string _body;
		std::map<int, std::string> ErrorMsg;
		std::string _bodyType;
		std::map<std::string, std::string> _mMimeTypes;
		bool _autoIndex;
		// std::string _method;

	public:
		std::string handleResponse(HttpRequest &req);
		std::string convertFinalResponse();
		void exeGet(HttpRequest &req);
		void fillHeaders(std::map<std::string, std::string> &headers);
		void fillBody(std::string &path);
		void fillMimeTypes();
		void searchFileInDir(std::string &path);
		void generateAutoIndex(std::vector<std::string> &fileName, std::string &path);
		void exePost();
		void exeDelete();

		std::string getVersion() const;
		std::string getStatusMsg() const;
		std::string getBody() const;
		// std::string getMethod() const;
		int getStatusCode() const;
		std::map<std::string, std::string> getHeaders() const;
};
#endif
