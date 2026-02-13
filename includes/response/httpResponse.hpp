#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

# include <iostream>
# include <fstream>
# include <sys/stat.h>
# include <exception>
# include <map>
# include <dirent.h>
# include <sstream>
# include <vector>
# include <algorithm>
# include "httpRequest.hpp"

# define DEFAULT_TYPE "application/octet-stream"
# define PATH_FILE_CODE "file/error_code.txt"
# define PATH_FIlE_MIME "file/mime_types.txt"

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
		std::map<int, std::string> _mErrorMsg;
		std::string _bodyType;
		std::map<std::string, std::string> _mMimeTypes;
		bool _autoIndex;

	public:
		std::string handleResponse(HttpRequest &req);
		std::string convertFinalResponse();
		void exeGet(HttpRequest &req);
		void fillHeaders(std::map<std::string, std::string> &headers);
		int fillBody(std::string &path, HttpRequest &req);
		void fillDefaultBody();
		void fillMimeTypes();
		void fillMapError();
		int searchFileInDir(std::string &path, HttpRequest &req);
		int generateAutoIndex(std::string &path);
		bool isForbiddenMethod(HttpRequest &req);
		std::string setPathError();
		void handleError(HttpRequest &req);

		void exePost();
		void exeDelete();

		// std::string getVersion() const;
		// std::string getStatusMsg() const;
		// std::string getBody() const;
		// // std::string getMethod() const;
		// int getStatusCode() const;
		// std::map<std::string, std::string> getHeaders() const;
};
#endif

/*#ifndef CONFIG_HPP
# define CONFIG_HPP

#include <vector>
#include <map>
#include <iostream>

class Config
{
	public:
		Config(){}
		~Config(){}

	private:
		std::string							_root;
		size_t								_client_max_body_size;
		std::map<int, std::string>			_error_pages; -> 
		std::vector<std::string>			_methods; ->
		bool								_auto_index; -> 
		std::vector<std::string>			_indexes; -> 
		std::string							_upload_store;
		bool								_cgi_enabled;
		std::map<std::string, std::string>	_cgi_ext;
		std::string							_cgi_working_dir;
		std::string							_cgi_upload_path;
		int									_cgi_timeout;

	public:
		// GET
		const std::string							get_root() const;
		size_t										get_client_max_body_size() const;
		const std::map<int, std::string>			get_error_pages() const;
		const std::vector<std::string>				get_methods() const;
		bool										get_auto_index() const;
		const std::vector<std::string>				get_indexes() const;
		const std::string							get_upload_store() const;
		bool										get_cgi_enabled() const;
		const std::map<std::string, std::string>	get_cgi_ext() const;
		const std::string							get_cgi_working_dir() const;
		const std::string							get_cgi_upload_path() const;
		int											get_cgi_timeout() const;

		// SET
};

#endif*/

