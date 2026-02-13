#include "httpResponse.hpp"

httpResponse::httpResponse(){
	//remplir ErrorMsg 403 404 500 etc....
	_statusCode = 200; // a virer
	_statusMsg = "OK"; // a virer
	fillMimeTypes();

}
// httpResponse::httpResponse(const HttpRequest &req)
// : _statusCode(200), _version(req.version), _statusMsg("OK"), _headers(req.headers), _body(req.body), _method(req.method){}

std::string httpResponse::convertFinalResponse(){
	std::stringstream ss;
	ss << _statusCode;
	std::string resp = _version + ss.str() + " " + _statusMsg + "\r\n";
	for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); ++it)
		resp += it->first + ": " + it->second + "\r\n";
	resp += "\r\n";
	resp += _body;
	return resp;
}

std::string httpResponse::handleResponse(HttpRequest &req){
	if (req.method == "GET")
		exeGet(req);
	// else if (_method == "POST")
	// 	exePost();
	// else if (_method == "DELETE")
	// 	exeDelete();
	return convertFinalResponse();
}

void httpResponse::fillMimeTypes(){

	std::ifstream inFile;
	inFile.open("file/mime_types.txt");
	// if (!inFile.is_open())
	// 	std::cout << "error open file" << std::endl;
	// 	// voir quoi faire
	std::string line;
	while (std::getline(inFile, line))
	{
		if (line.empty())
			continue ;
		std::string::size_type endExtension = line.find_first_of(" ");
		std::string extensionType = line.substr(0, endExtension);
		std::string::size_type startContent = line.find_first_not_of(" ", endExtension);
		std::string contentType = line.substr(startContent);
		_mMimeTypes[extensionType] = contentType;
	}
	inFile.close();
}

void httpResponse::fillHeaders(std::map<std::string, std::string> &headers){
	std::stringstream ss;
	ss << _body.size();
	_headers["Content-Type"] = _mMimeTypes[_bodyType];
	if (_headers["Content-Type"].empty())
		_headers["Content-Type"] = _mMimeTypes["bin"];
	_headers["Connection"] = headers["Connection"];
	_headers["Content-Lenght"] = ss.str();
}

void httpResponse::generateAutoIndex(std::vector<std::string> &fileName, std::string &path)
{
	std::sort(fileName.begin(), fileName.end());
	std::ostringstream index;
	index	<< "<html><head><title>Index of " << path << "</title</head><body>"
			<< "<h1>Index of " << path << "</h1><hr><pre>";
	for (std::vector<std::string>::iterator it = fileName.begin(); it != fileName.end(); ++it)
	{
		if (*it == ".")
			continue ;
		index << "<a href=\"" << *it << "\">" << *it << "</a>\n";
	}
	index << "</pre><hr></body></html>";
	_body = index.str();
}

void httpResponse::searchFileInDir(std::string &path)
{
	DIR *currentDir = opendir(path.c_str());
	if (!currentDir)
	{
		_statusCode = 403;
		_statusMsg = "Forbidden";
		return ;
	}
	struct dirent *readFile;
	std::vector<std::string> fileName;
	while (1)
	{
		readFile = readdir(currentDir); // secu ?
		fileName.push_back(readFile->d_name);
		if (!readFile)
			break ;
		if (readFile->d_name == "index.html") // std::vector<std::string> index;
		{	
			path += readFile->d_name;
			closedir(currentDir);
			std::ifstream inFile;
			inFile.open(path.c_str() , std::ios::binary);
			if (!inFile.is_open())
			{
				_statusCode = 403;
				return ;
			}
			std::ostringstream oss;
			oss << inFile.rdbuf();
			_body = oss.str();
			return ;
		}
	}
	if (_autoIndex)
		generateAutoIndex(fileName, path);
	else
		_statusCode = 403;
	closedir(currentDir);
	return ;
}


void httpResponse::fillBody(std::string &path) {
	// if (forbbiden method ---> remplir le body en consequence)
	struct stat s;
	if (stat(path.c_str(), &s) == -1)
	{
		_statusMsg = "Not found."; // a virer
		_statusCode = 404;
		return ;
	}
	std::ifstream inFile;
	if (_statusCode == 0 && S_ISDIR(s.st_mode))
		searchFileInDir(path);
	else
	{
		inFile.open(path.c_str(), std::ios::binary);
		if (!inFile.is_open())
		{
			_statusCode = 403;
			return ;
		}
		std::ostringstream oss;
		oss << inFile.rdbuf();
		_body = oss.str();
	}
	
	
}

void httpResponse::exeGet(HttpRequest &req){
	//Check forbidden Method
	// fillBody(req.path);
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
