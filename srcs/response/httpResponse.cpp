#include "httpResponse.hpp"

httpResponse::httpResponse()
: _statusCode(0){

	fillMimeTypes();
	fillMapError();
}
// httpResponse::httpResponse(const HttpRequest &req)
// : _statusCode(200), _version(req.version), _statusMsg("OK"), _headers(req.headers), _body(req.body), _method(req.method){}

std::string httpResponse::convertFinalResponse(){

	std::stringstream ss;
	ss << _statusCode;
	std::string resp = _version + " " + ss.str() + " " + _statusMsg + "\r\n";
	for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); ++it)
		resp += it->first + ": " + it->second + "\r\n";
	resp += "\r\n";
	resp += _body;
	return resp;
}

std::string httpResponse::handleResponse(HttpRequest &req){

	_version = req.version;
	if (req.method == "GET")
		exeGet(req);
	else // Voir pour le  body etc..
	{
		_statusCode = 501;
		handleError(req);
	}
	// else if (_method == "POST")
	// 	exePost();
	// else if (_method == "DELETE")
	// 	exeDelete();
	return convertFinalResponse();
}

void httpResponse::fillMapError(){
	std::ifstream inFile;
	inFile.open(PATH_FILE_CODE);
	// if (!inFile.is_open())
	// 	// voir quoi faire EXIT ?
	std::string line;
	while(std::getline(inFile, line))
	{
		if (line.empty())
			continue;
		std::string::size_type endCode = line.find_first_of(" ");
		std::string errorCode = line.substr(0, endCode);
		std::string::size_type startMsg = line.find_first_not_of(" ", endCode);
		std::string errorMsg = line.substr(startMsg);
		std::stringstream ss(errorCode);
		int value;
		if (ss >> value)
			_mErrorMsg[value] = errorMsg;
	}
}

void httpResponse::fillMimeTypes(){

	std::ifstream inFile;
	inFile.open(PATH_FIlE_MIME);
	// if (!inFile.is_open())
	// 	// voir quoi faire EXIT ?
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

	_headers["Content-Type"] = _mMimeTypes[_bodyType];
	if (_headers["Content-Type"].empty())
		_headers["Content-Type"] = DEFAULT_TYPE;
	
	_headers["Connection"] = headers["Connection"];
	if (_headers["Connection"].empty())
		_headers["Connection"] = "Keep-Alive"; // --> verif ce qu on fait dans ce cas la
	std::stringstream ss;
	ss << _body.size();
	_headers["Content-Length"] = ss.str();
}

int httpResponse::generateAutoIndex(std::string &path)
{
	DIR *currentDir = opendir(path.c_str());
	if (!currentDir)
		return 403;
	struct dirent *readDir;
	std::vector<std::string> fileName;
	while (1)
	{
		readDir = readdir(currentDir); // secu ?
		if (!readDir)
			break ;
		fileName.push_back(readDir->d_name);
	}
	closedir(currentDir);
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
	_bodyType = "html";
	return 200;
}

int httpResponse::searchFileInDir(std::string &path, HttpRequest &req)
{
	struct stat s;
	for (std::vector<std::string>::iterator it = req.indexes.begin(); it != req.indexes.end(); ++it)
	{
		std::string tryPath = path + "/" + *it;
		if (stat(tryPath.c_str(), &s) == 0)
		{
			std::ifstream inFile;
			inFile.open(tryPath.c_str() , std::ios::binary);
			if (!inFile.is_open())
				return 403;
			std::ostringstream oss;
			oss << inFile.rdbuf();
			_body = oss.str();
			_bodyType = tryPath.substr(tryPath.find_first_of(".") + 1);
			return 200;
		}
	}
	if (req.auto_index)
		generateAutoIndex(path);
	else
		return 403;
	return 200;
}


bool httpResponse::isForbiddenMethod(HttpRequest &req)
{
	for (std::vector<std::string>::iterator it = req.methods.begin(); it != req.methods.end(); ++it)
	{
		if (*it == req.method)
			return false;
	}
	return true;
}

int httpResponse::fillBody(std::string &path, HttpRequest &req) {

	if (isForbiddenMethod(req))
		return 405;
	struct stat s;
	if (stat(path.c_str(), &s) == -1)
		return 404;
	std::ifstream inFile;
	if (S_ISDIR(s.st_mode))
		searchFileInDir(path, req);
	else
	{
		inFile.open(path.c_str(), std::ios::binary);
		if (!inFile.is_open())
			return 403;
		std::ostringstream oss;
		oss << inFile.rdbuf();
		_body = oss.str();
		_bodyType = path.substr(path.find_first_of(".") + 1);
	}
	return 200;	
}

std::string httpResponse::setPathError()
{
	std::string pathErrFile;
	switch (_statusCode)
	{
		case 403:
			pathErrFile = "file/error_page/error_page_403.html";
			break ;
		case 404:
			pathErrFile = "file/error_page/error_page_404.html";
			break ;
		case 405:
			pathErrFile = "file/error_page/error_page_405.html";
			break ;
		case 500:
			pathErrFile = "file/error_page/error_page_500.html";
			break ;
		case 501:
			pathErrFile = "file/error_page/error_page_501.html";
			break;
		default:
			_statusCode = 500;
			pathErrFile = "file/error_page 500";
			_statusMsg = _mErrorMsg[500];
	}
	return pathErrFile;
}

void httpResponse::fillDefaultBody(){

	std::string pathErrFile = setPathError();
	std::cout << pathErrFile << std::endl;
	std::ifstream inFile;
	inFile.open(pathErrFile.c_str(), std::ios::binary);
	if (!inFile.is_open())
	{
		std::ostringstream index;
		index 	<< "<html><head><title>" << _statusCode << " "<< _statusMsg + "</title></head>"
				<< "<center><h1>Index of " << _statusCode << " " << _statusMsg << "</h1></center><hr><pre>";
		_body = index.str();
		_bodyType = "html";
	}
	else
	{
		std::ostringstream oss;
		oss << inFile.rdbuf();
		_body = oss.str();
		_bodyType = pathErrFile.substr(pathErrFile.find_first_of(".") + 1);
	}
}

void httpResponse::handleError(HttpRequest &req)
{
	(void)req;
	_statusMsg = _mErrorMsg[_statusCode];
	if (_statusMsg.empty() && _statusCode == 0)
	{
		_statusCode = 500;
		_statusMsg = "Internal Server Error";
	}
	fillDefaultBody();

	_headers["Content-Type"] = _mMimeTypes[_bodyType];
	if (_headers["Content-Type"].empty())
		_headers["Content-Type"] = DEFAULT_TYPE;
	if (_statusCode == 500)
		_headers["Connection"] = "close"; // --> verif ce qu on fait dans ce cas la
	else
		_headers["Connection"] = "Keep-Alive"; // --> verif ce qu on fait dans ce cas la
	std::stringstream ss;
	ss << _body.size();
	_headers["Content-Length"] = ss.str();
	if (_statusCode == 501 || _statusCode == 405)
	{
		std::string allowMethods;
		for (std::vector<std::string>::iterator it = req.methods.begin(); it != req.methods.end(); ++it)
		{
			if (it + 1 != req.methods.end())
				allowMethods += *it + ", ";
			else
				allowMethods += *it;
		}
		_headers["Allow"] = allowMethods; 
	}
}

void httpResponse::exeGet(HttpRequest &req){

	_statusCode = fillBody(req.path, req);
	if (_statusCode != 200)
		handleError(req);
	fillHeaders(req.headers);
}

// std::string httpResponse::getVersion() const{
// 	return _version;
// }

// std::string httpResponse::getStatusMsg() const{
// 	return _statusMsg;
// }

// std::string httpResponse::getBody() const{
// 	return _body;
// }

// int httpResponse::getStatusCode() const{
// 	return _statusCode;
// }

// std::map<std::string, std::string> httpResponse::getHeaders() const{
// 	return _headers;
// }
