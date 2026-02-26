#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

# include <iostream>
# include <fstream>
# include <sys/stat.h>
# include <exception>
# include <map>
# include <dirent.h>
# include <unistd.h>
# include <errno.h>
# include <sstream>
# include <vector>
# include <algorithm>
# include "httpRequest.hpp"
# include <sys/wait.h>
# include "Color.hpp"


# define DEFAULT_TYPE "application/octet-stream"
# define PATH_FILE_CODE "file/error_code.txt"
# define PATH_FIlE_MIME "file/mime_types.txt"
# define PATH_FILE_CGI "file/cgi_path.txt"

class httpResponse
{
	public:
		httpResponse();
		~httpResponse(){}
	
	private:
		int _statusCode;
		std::string _version;
		std::string _statusMsg;
		std::map<std::string, std::string> _headers;
		std::string _body;
		std::map<int, std::string> _mErrorMsg;
		std::string _bodyType;
		std::string _cgiPath;
		std::string _binary;
		std::string _cgiOutput;
		std::map<std::string, std::string> _mMimeTypes;
		std::map<std::string, std::string> _mCgiTypes;

	public:
		

		std::string handleResponse(HttpRequest &req, int code);
		std::string convertFinalResponse();
		
		void fillHeaders(std::map<std::string, std::vector<std::string> > &mult);
		int fillBody(std::string &path, HttpRequest &req);
		void fillDefaultBody();
		void fillMapError();
		void fillMapExtension(std::map<std::string, std::string> &map, std::string pathFile);
		void handleError(HttpRequest &req);
		std::string setPathError();

		void exeGet(HttpRequest &req);
		int searchFileInDir(std::string &path, HttpRequest &req);
		int generateAutoIndex(std::string &path);

		void exePost(HttpRequest &req);
		int isFileExist(std::string &path, HttpRequest &req);
		void fillBody(HttpRequest &req);

		void exeDelete(HttpRequest &req);
		int deleteFile(std::string &path, HttpRequest &req);

		int isCgi(HttpRequest &req, std::string path);
		bool isCgiExtension(std::string currentPath, HttpRequest &req);
		int exeCgi(std::string path, HttpRequest &req);
		char** createEnv(HttpRequest &req, std::string path);
		void saveCgiOutput(int *pipeOut, pid_t pid);
		void fillCgiResponse(HttpRequest &req);
		void parseCgiOutput();
};

#endif

