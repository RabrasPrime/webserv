#include "httpResponse.hpp"

httpResponse::httpResponse()
{
	//remplir ErrorMsg 403 404 500 etc....
	// + Content Type 

}
// httpResponse::httpResponse(const HttpRequest &req)
// : _statusCode(200), _version(req.version), _statusMsg("OK"), _headers(req.headers), _body(req.body), _method(req.method){}

std::string httpResponse::convertFinalResponse()
{
	std::string resp = _version + std::to_string(_statusCode) + " " + _statusMsg + "\r\n";
	for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); ++it)
		resp += it->first + ": " + it->second + "\r\n";
	resp += "\r\n";
	resp += _body;
	return resp;
}

void httpResponse::handleRequest(HttpRequest &req)
{
	if (req.method == "GET")
		exeGet(req);
	// else if (_method == "POST")
	// 	exePost();
	// else if (_method == "DELETE")
	// 	exeDelete();
}

void httpResponse::fillHeaders(std::map<std::string, std::string> &headers)
{
	_headers["Content-Type"] = headers["Content-Type"];
	_headers["Connection"] = headers["Connection"];
	_headers["Content-Lenght"] = std::to_string(_body.size());
}

void httpResponse::fillBody(std::string &path)
{
	// if (forbbiden method ---> remplir le body en consequence)
	struct stat s;
	_statusCode = stat(path.c_str(), &s);
	std::ifstream inFile;
	if (_statusCode == 0 && S_ISDIR(s.st_mode))
	{
		DIR *currentDir = opendir(path.c_str());
		if (!currentDir)
		{
			_statusCode = 404;
			_statusMsg = "File Not found.";
			return ;
		}
		struct dirent *readFile;
		closedir(currentDir);
	}
	inFile.open(path, std::ios::binary);
	if (!inFile.is_open())
	// if (_statusCode != 0 && auto_index == true) --> create Content index.html
	
}

void httpResponse::exeGet(HttpRequest &req)
{
	//Check forbidden Method
	fillBody(req.path);
	fillHeaders(req.headers);
}

std::string httpResponse::getVersion() const{
	return _version;
}

std::string httpResponse::getStatusMsg() const{
	return _statusMsg;
}

std::string httpResponse::getBody() const{
	return _body;
}

// std::string httpResponse::getMethod() const{
// 	return _method;
// }
int httpResponse::getStatusCode() const{
	return _statusCode;
}

std::map<std::string, std::string> httpResponse::getHeaders() const{
	return _headers;
}
