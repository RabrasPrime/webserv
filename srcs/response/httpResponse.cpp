#include "httpResponse.hpp"

httpResponse::httpResponse()
: _statusCode(0){

	fillMapExtension(_mMimeTypes, PATH_FIlE_MIME);
	fillMapExtension(_mCgiTypes, PATH_FILE_CGI);
	fillMapError();
}

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

bool httpResponse::isCgiExtension(std::string currentPath){
	
	size_t end = currentPath.find_last_of(".");
	std::string extension = currentPath.substr(end + 1);
	for (std::map<std::string, std::string>::iterator it = _mCgiTypes.begin(); it != _mCgiTypes.end(); ++it)
	{
		if (extension == (*it).first)
		{
			_binary = (*it).second;
			return true;
		}
	}
	return false;
}

char** httpResponse::createEnv(HttpRequest &req, std::string path){

	std::vector<std::string> envList;
	envList.push_back("REQUEST_METHOD=" + req.method);
	envList.push_back("QUERY_STRING=" + req.queryString);
	std::stringstream ss;
	ss << req.body.size();
	envList.push_back("CONTENT_LENGTH=" + ss.str());
	envList.push_back("CONTENT_TYPE=" + req.headers["Content-Type"]);
	envList.push_back("PATH_INFO=" + path);
	envList.push_back("SCRIPT_NAME=" + path);
	envList.push_back("SERVER_PROTOCOL=" + req.version);
	envList.push_back("REDIRECT_STATUS=200");

	char **env = new char*[envList.size() + 1];
	for (size_t i = 0; i < envList.size(); ++i)
	{
		env[i] = new char[(envList[i].size() + 1)];
		std::copy(envList[i].begin(), envList[i].end(), env[i]);
		env[i][envList[i].size()] = '\0';
	}
	env[envList.size()] = NULL;

	return env;
}

void httpResponse::saveCgiOutput(int *pipeOut, pid_t pid){

	std::string outCgi;
	char buffer[4096];
	int bytesRead = read(pipeOut[0], buffer, sizeof(buffer) - 1);
	while ((bytesRead > 0))
	{
		buffer[bytesRead] = '\0';
		outCgi += buffer;
		bytesRead = read(pipeOut[0], buffer, sizeof(buffer) - 1);
	}
	close(pipeOut[0]);

	int status;
	waitpid(pid, &status, 0);
	status == 0 ? _statusCode = 200 : _statusCode = 500;
	_cgiOutput = outCgi;

}

int httpResponse::exeCgi(std::string path, HttpRequest &req){
	
	int pipeOut[2], pipeIn[2];
	
	if (pipe(pipeIn) == -1 || pipe(pipeOut) == -1)
		return 500;
	
	pid_t pid = fork();
	if (pid < 0)
	{
		close(pipeIn[1]);
		close(pipeIn[0]);
		close(pipeOut[0]);
		close(pipeOut[1]);
		return 500;
	}
	else if (pid == 0)
	{
		if (dup2(pipeIn[0], STDIN_FILENO) == -1)
			exit(EXIT_FAILURE);
		if (dup2(pipeOut[1], STDOUT_FILENO) == -1)
			exit(EXIT_FAILURE);
		close(pipeIn[1]);
		close(pipeIn[0]);
		close(pipeOut[0]);
		close(pipeOut[1]);

		char *arg[] = {const_cast<char *>(_binary.c_str()), const_cast<char *>(path.c_str()), NULL};
		char **env = createEnv(req, path);
		execve(_binary.c_str(), arg, env);
		delete []env;
		exit(1);
	}
	if (!req.body.empty() && req.method == "POST")
		write(pipeIn[1], req.body.c_str(), req.body.size());
	close(pipeOut[1]);
	close(pipeIn[1]);
	close(pipeIn[0]);

	saveCgiOutput(pipeOut, pid);

	return _statusCode;
}

int httpResponse::isCgi(HttpRequest &req, std::string path){

	size_t end = path.find_first_of("?");
	_cgiPath = path.substr(0, end);
	
	size_t pos = 0, nextSection = 0;
	std::string currentPath = req.root;
	while (pos < _cgiPath.size())
	{
		nextSection = _cgiPath.find('/', pos + 1);
		std::string section = _cgiPath.substr(pos, nextSection - pos);
		currentPath += section;
		struct stat s;
		if (stat(currentPath.c_str(), &s) == 0)
		{
			
			if (S_ISREG(s.st_mode))
			{
				if (isCgiExtension(currentPath))
				{
					return (exeCgi(currentPath, req));
				}
				break ;
			}
		}
		else
			return 0;
		if (nextSection == std::string::npos)
			break ;
		pos = nextSection;
	}
	return 0;
}


void httpResponse::parseCgiOutput(){

	size_t sep =_cgiOutput.find("\r\n\r\n");
	size_t sizeSpace = 4;
	if (sep == std::string::npos)
	{
		sep = _cgiOutput.find("\n\n");
		sizeSpace = 2;
	}
	// std::cout << "PARSE CGI OUTPUT" << std::endl;
	if (sep != std::string::npos)
	{
		std::string headers = _cgiOutput.substr(0, sep);
		std::string strContent = "Content-Type:";
		size_t posContentType = headers.find(strContent);
		if (posContentType != std::string::npos)
		{
			
			size_t posEndContentType = headers.find("\n", posContentType + strContent.size());
			_headers["Content-Type"] =  headers.substr(posContentType + strContent.size(), posEndContentType);
			std::cout << BLUE BOLD "CONTENT TyPE:" RESET << _headers["Content-Type"] << std::endl;
		}
		std::cout << BLUE BOLD " HEADER HERE " RESET << std::endl << headers << BLUE BOLD "END HEADER" RESET << std::endl;
		_body = _cgiOutput.substr(sep + sizeSpace);
		// std::cout << BLUE BOLD " BODY HERE " RESET << std::endl << _body << BLUE BOLD "END BODY" RESET << std::endl;
	}
	else
	{
		// std::cout << "FILL BODY" << std::endl;
		_body = _cgiOutput;
	}
}

void httpResponse::fillCgiResponse(HttpRequest &req){

	// std::cout << RED BOLD "FILL CGI RESPONSE" RESET << std::endl;
	_statusMsg = _mErrorMsg[_statusCode];
	(void)req;
	parseCgiOutput();
	_headers["Connection"] = req.headers["Connection"];
	if (_headers["Connection"].empty())
		_headers["Connection"] = "Keep-Alive";
	if (_headers["Content-Type"].empty())
		_headers["Content-Type"] = "text/html";
	std::stringstream ss;
	ss << _body.size();
	_headers["Content-Length"] = ss.str();
}

std::string httpResponse::handleResponse(HttpRequest &req){

	_version = req.version;
	_statusCode = isCgi(req, req.path);
	if (_statusCode != 0)
	{
		if (_statusCode != 200)
		{
			handleError(req);
			return convertFinalResponse();
		}
		fillCgiResponse(req);
		return convertFinalResponse();
	}
	if (req.method == "GET")
		exeGet(req);
	else if (req.method == "POST")
		exePost(req);
	else if (req.method == "DELETE")
		exeDelete(req);
	else
	{
		_statusCode = 501;
		handleError(req);
	}
	return convertFinalResponse();
}

void httpResponse::fillMapError(){
	std::ifstream inFile;
	inFile.open(PATH_FILE_CODE);
	if (!inFile.is_open())
	{
		std::cerr << RED BOLD "Error fail to open file" RESET << std::endl;
		exit(EXIT_FAILURE);
	}
	std::string line;
	while(std::getline(inFile, line))
	{
		if (line.empty())
			continue ;
		size_t endCode = line.find_first_of(" ");
		std::string errorCode = line.substr(0, endCode);
		size_t startMsg = line.find_first_not_of(" ", endCode);
		std::string errorMsg = line.substr(startMsg);
		std::stringstream ss(errorCode);
		int value;
		if (ss >> value)
			_mErrorMsg[value] = errorMsg;
	}
}

void httpResponse::fillMapExtension(std::map<std::string, std::string> &map, std::string pathFile){

	std::ifstream inFile;
	inFile.open(pathFile.c_str());
	if (!inFile.is_open())
	{
		std::cerr << RED BOLD "Error fail to open file" RESET << std::endl;
		exit(EXIT_FAILURE);
	}
	std::string line;
	while (std::getline(inFile, line))
	{
		if (line.empty())
			continue ;
		size_t endExtension = line.find_first_of(" ");
		std::string extensionType = line.substr(0, endExtension);
		size_t startContent = line.find_first_not_of(" ", endExtension);
		std::string contentType = line.substr(startContent);
		map[extensionType] = contentType;
	}
	inFile.close();
}

void httpResponse::fillHeaders(std::map<std::string, std::string> &headers){

	_headers["Content-Type"] = _mMimeTypes[_bodyType];
	if (_headers["Content-Type"].empty())
		_headers["Content-Type"] = DEFAULT_TYPE;
	if (_headers["Content-Type"].find("text/") == 0)
		_headers["Content-Type"] += "; charset=utf-8";
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
		readDir = readdir(currentDir);
		if (errno != 0)
			return (500);
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
			inFile.open(tryPath.c_str(), std::ios::binary);
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
		case 409:
			pathErrFile = "file/error_page/error_page_409.html";
			break ;
		case 411:
			pathErrFile = "file/error_page/error_page_411.html";
			break ;
		case 413:
			pathErrFile = "file/error_page/error_page_413.html";
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
	std::ifstream inFile;
	inFile.open(pathErrFile.c_str(), std::ios::binary);
	if (!inFile.is_open())
	{
		std::ostringstream index;
		index 	<< "<html><head><title>" << _statusCode << " "<< _statusMsg + "</title></head>"
				<< "<center><h1>Index of " << _statusCode << " " << _statusMsg << "</h1></center><hr><pre></html>";
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

void httpResponse::fillBody(HttpRequest &req)
{
	std::ostringstream index;
	index << "<html><head><style>body{ background: #1a1a1a; color: white; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; font-family: sans-serif; flex-direction: column;} h1 {font-size: 100px; color: #16d705; margin-top: -50px; } </style></head><body> <h1>";
	if (_statusCode == 200 && req.method == "DELETE")
		index << " File deleted sucessfully";
	else if (_statusCode == 200)
		index << " File updated sucessfully";
	else
		index << " File created sucessfully";
	index <<  "</h1></body></html>";
	_body = index.str();
	_bodyType = "html";
}

int httpResponse::isFileExist(std::string &path, HttpRequest &req)
{
	if (isForbiddenMethod(req))
		return 405;
	bool fileCreated = false;
	struct stat s;

	size_t parentDir = path.find_last_of("/\\");
	std::string dirPath;
	if (parentDir == std::string::npos)
		dirPath = ".";
	else if (parentDir == 0)
		dirPath = "/";
	else
		dirPath = path.substr(0, parentDir);
	
	if (stat(path.c_str(), &s) == 0)
	{
		if (S_ISDIR(s.st_mode))
			return 409;
		if (access(path.c_str(), W_OK) == -1)
			return 403;
	}
	else
	{
		if (stat(dirPath.c_str(), &s) == -1 || !S_ISDIR(s.st_mode))
			return 404;
		if (access(dirPath.c_str(), W_OK) == -1)
			return 403;
		fileCreated = true;
	}

	std::ofstream outFile;
	outFile.open(path.c_str(), std::ios::binary | std::ios::trunc);
	if (!outFile.is_open())
		return 500;
	outFile.write(req.body.c_str(), req.body.size());
	if (!outFile.good())
	{
		outFile.close();
		return 500;
	}
	outFile.close();
	return fileCreated ? 201 : 200;
}

void httpResponse::exePost(HttpRequest &req)
{
 	if (req.body.size() > req.maxSize)
	{
		_statusCode = 413;
		handleError(req);
		return ;
	}
	if (req.body.size() == 0)
	{
		_statusCode = 411;
		handleError(req);
		return ;
	}
	_statusCode = isFileExist(req.path, req);
	if (_statusCode != 200 && _statusCode != 201)
	{
		handleError(req);
		return ;
	}
	_statusMsg = _mErrorMsg[_statusCode];
	fillBody(req);
	fillHeaders(req.headers);
}

int httpResponse::deleteFile(std::string &path, HttpRequest &req)
{
	if (isForbiddenMethod(req))
		return 405;
	size_t parentDir = path.find_last_of("/\\");
	std::string dirPath;
	if (parentDir == std::string::npos)
		dirPath = ".";
	else if (parentDir == 0)
		dirPath = "/";
	else
		dirPath = path.substr(0, parentDir);
	if (access(dirPath.c_str(), X_OK) == -1)
		return 403;
	struct stat s;
	if (stat(path.c_str(), &s) == -1)
		return 404;
	if (S_ISDIR(s.st_mode))
		return 403;
	if (access(dirPath.c_str(), W_OK) == -1)
		return 404;
	if (unlink(path.c_str()) == -1)
		return 500;
	return 200;
}

void httpResponse::exeDelete(HttpRequest &req)
{
	_statusCode = deleteFile(req.path, req);
	if (_statusCode != 200 && _statusCode != 201)
	{
		handleError(req);
		return ;
	}
	_statusCode = 200;
	_statusMsg = _mErrorMsg[_statusCode];
	fillBody(req);
	fillHeaders(req.headers);
}
