#include "httpResponse.hpp"
#include "Server.hpp"

httpResponse::httpResponse()
: _statusCode(0){

	fillMapExtension(_mMimeTypes, PATH_FIlE_MIME);
	fillMapExtension(_mCgiTypes, PATH_FILE_CGI);
	for (std::map<std::string,std::string>::iterator it = _mCgiTypes.begin();it != _mCgiTypes.end();it++)
	{
		std::cout << "first >" << it->first << "<      second >" << it->second << "<" << std::endl;
	}
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

bool httpResponse::isCgiExtension(std::string currentPath, HttpRequest &req){
	(void)req;
	size_t end = currentPath.find_last_of(".");
	std::string extension = currentPath.substr(end);

	std::cout << "ext > " << extension << "    size  > " << req.cgi_ext.size() << std::endl;
	for (std::map<std::string, std::string>::iterator it = req.cgi_ext.begin(); it != req.cgi_ext.end(); ++it)
	{
		std::cout << "(*it).first >> " << (*it).first  << std::endl;
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

	if (req.methods & METHOD_GET)
		envList.push_back("REQUEST_METHOD=GET");
	if (req.methods & METHOD_POST)
		envList.push_back("REQUEST_METHOD=POST");
	if (req.methods & METHOD_DELETE)
		envList.push_back("REQUEST_METHOD=DELETE");
	for (std::vector<std::string>::iterator it = req.env.begin(); it != req.env.end(); ++it)
		envList.push_back(*it);
	std::stringstream ss;
	ss << req.body.size();
	envList.push_back("CONTENT_LENGTH=" + ss.str());
	std::string tmp;
	for (std::vector<std::string>::iterator it = req.mult["Content-Type"].begin();it != req.mult["Content-Type"].end();it++)
	{
		if (it + 1 != req.mult["Content-Type"].end())
			tmp += *it + ",";
		else
			tmp += *it;
	}
	envList.push_back("CONTENT_TYPE=" + tmp);
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
	int code = WEXITSTATUS(status);
	code == 0 ? _statusCode = 200 : _statusCode = 500;

	std::cout << BOLD GREEN << "STATUS CODE"  << _statusCode  << std::endl << "STATUS" RESET << code << std::endl;

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
	if (!req.body.empty() && req.method & METHOD_POST)
		write(pipeIn[1], reinterpret_cast<char*>(&req.body), req.body.size());
	close(pipeOut[1]);
	close(pipeIn[1]);
	close(pipeIn[0]);

	saveCgiOutput(pipeOut, pid);

	return _statusCode;
}

int httpResponse::isCgi(HttpRequest &req, std::string path){

	// size_t end = path.find_first_of("?");
	// _cgiPath = path.substr(0, end);
	
	// size_t pos = 0, nextSection = 0;
	// std::string currentPath = req.root;
	// while (pos < _cgiPath.size())
	// {
	// 	nextSection = _cgiPath.find('/', pos + 1);
	// 	std::string section = _cgiPath.substr(pos, nextSection - pos);
	// 	currentPath += section;
		// struct stat s;
		// if (stat(currentPath.c_str(), &s) == 0)
		// {
			
		// 	if (S_ISREG(s.st_mode))
		// 	{
		// 		if (isCgiExtension(currentPath))
		// 		{
		// 			return (exeCgi(currentPath, req));
		// 		}
		// 		break ;
		// 	}
		// }
		// else
		// 	return 0;
		// if (nextSection == std::string::npos)
		// 	break ;
		// pos = nextSection;
	// }
	struct stat s;
	if (stat(path.c_str(), &s) == 0)
	{
		std::cout << "1" << std::endl;
		if (S_ISREG(s.st_mode))
		{	
			std::cout << "2" << std::endl;
			std::ifstream inFile;
			inFile.open(path.c_str(), std::ios::binary);
			if (!inFile.is_open())
				return 403;
			inFile.close();
			if (isCgiExtension(path, req))
				return (exeCgi(path, req));
		}
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
	parseCgiOutput();
	if (req.mult["Connection"].size() == 0)
		_headers["Connection"] = "Keep-Alive"; //a modif selon la version http
	else
		_headers["Connection"] = req.mult["Connection"].front();
	if (_headers["Content-Type"].empty())
		_headers["Content-Type"] = "text/html";
	std::stringstream ss;
	ss << _body.size();
	_headers["Content-Length"] = ss.str();
}

std::string httpResponse::handleResponse(HttpRequest &req, int code){

	_version = req.version;
	_statusCode = 0;
	(void)code;
	if (code != 0)
	{
		_statusCode = code;
		handleError(req);
		return convertFinalResponse();
	}
	std::cout << BLUE BOLD "HANDLE RESPONSE BEFORE   > " << _statusCode << RESET << std::endl;
	_statusCode = isCgi(req, req.path);
	std::cout << BLUE BOLD "HANDLE RESPONSE AFTER    > " << _statusCode << RESET << std::endl;
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
	// std::cout << RED << "sdasdasda" << req.method << RESET << std::endl;
	if (req.method & METHOD_GET)
		exeGet(req);
	else if (req.method & METHOD_POST)
		exePost(req);
	else if (req.method & METHOD_DELETE)
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
		size_t endCode = line.find(" ");
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
		size_t endExtension = line.find(" ");
		std::string extensionType = line.substr(0, endExtension);
		size_t startContent = line.find_first_not_of(" ", endExtension);
		std::string contentType = line.substr(startContent);
		map[extensionType] = contentType;
	}
	inFile.close();
}

void httpResponse::fillHeaders(std::map<std::string, std::vector<std::string> > &mult){

	_headers["Content-Type"] = _mMimeTypes[_bodyType];
	if (_headers["Content-Type"].empty())
		_headers["Content-Type"] = DEFAULT_TYPE;
	if (_headers["Content-Type"].find("text/") == 0)
		_headers["Content-Type"] += "; charset=utf-8";

	size_t pos = _version.find('.');
	if (mult["Connection"].size() == 0 && _version[pos + 1] == '1')
		_headers["Connection"] = "Keep-Alive"; //a modif selon la version http
	else
		_headers["Connection"] = mult["Connection"].front();
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
		errno = 0;
		readDir = readdir(currentDir);
		std::cout << "ERRNO >" << errno << std::endl;
		if (errno != 0)
			return (500);
		if (!readDir)
			break ;
		std::cout << "ReadDir result >" << readDir->d_name << std::endl;
		fileName.push_back(readDir->d_name);
	}
	closedir(currentDir);
	std::sort(fileName.begin(), fileName.end());
	std::ostringstream index;
	index	<< "<html><head><title>Index of " << path << "</title></head><body>"
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
		std::cout << BOLD BLUE << " IT" << std::endl;
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
			_bodyType = tryPath.substr(tryPath.find_last_of(".") + 1);
			return 200;
		}
	}
	std::cout << BOLD BLUE << " INDEX > " << req.auto_index << std::endl;
	if (req.auto_index)
	{
		std::cout << PURPLE "GENERATE AUTOIDNEX" RESET << std::endl;
		return (generateAutoIndex(path));
	}
	else
		return 403;
	return 200;
}


int httpResponse::fillBody(std::string &path, HttpRequest &req) {

	if (!(req.methods & req.method))
		return 405;
	struct stat s;
	if (stat(path.c_str(), &s) == -1)
		return 404;
	std::ifstream inFile;
	if (S_ISDIR(s.st_mode))
	{
		std::cout << ORANGE BOLD " IS DIR " << std::endl;
		if (req.path[req.path.size() - 1] != '/')
		{
			return 301;
		}
		return(searchFileInDir(path, req));
	}
	else
	{
		inFile.open(path.c_str(), std::ios::binary);
		if (!inFile.is_open())
			return 403;
		std::ostringstream oss;
		oss << inFile.rdbuf();
		_body = oss.str();
		_bodyType = path.substr(path.find_last_of(".") + 1, path.size());
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
		// default:
		// 	_statusCode = 500;
		// 	pathErrFile = "file/error_page 500";
		// 	_statusMsg = _mErrorMsg[500];
	}
	return pathErrFile;
}

void httpResponse::fillDefaultBody(){

	std::string pathErrFile = setPathError();
	std::ifstream inFile;
	if (!pathErrFile.empty())
		inFile.open(pathErrFile.c_str(), std::ios::binary);
	if (pathErrFile.empty() || !inFile.is_open())
	{
		std::cout << "Specific file not found take default file >" << _statusCode << std::endl;
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
		_bodyType = pathErrFile.substr(pathErrFile.find_last_of(".") + 1);
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
	std::cout << "ERROR CODE > " << _statusCode << std::endl;
	fillDefaultBody();
	if (_statusCode == 301)
	{
		_headers["Location"] = req.raw_path + '/' + req.queryString;
	}

	_headers["Content-Type"] = _mMimeTypes[_bodyType];
	if (_headers["Content-Type"].empty())
		_headers["Content-Type"] = DEFAULT_TYPE;
	if (_statusCode == 500)
		_headers["Connection"] = "close"; // --> verif ce qu on fait dans ce cas la
	else
	{
		size_t pos = req.version.find('.');
		if (req.mult["Connection"].size() == 0 && req.version[pos + 1] == '1')
			_headers["Connection"] = "Keep-Alive"; //a modif selon la version http
		else
			_headers["Connection"] = req.mult["Connection"].front();
	}
	std::stringstream ss;
	ss << _body.size();
	_headers["Content-Length"] = ss.str();
	if (_statusCode == 501 || _statusCode == 405)
	{
		std::string allowMethods;
		// for (std::vector<std::string>::iterator it = req.methods.begin(); it != req.methods.end(); ++it)
		// {
		// 	if (it + 1 != req.methods.end())
		// 		allowMethods += *it + ", ";
		// 	else
		// 		allowMethods += *it;
		// }

		if (req.methods & METHOD_GET)
			allowMethods += "GET";
		if (req.methods & METHOD_POST)
		{
			if (allowMethods.size() == 0)
				allowMethods += "POST";
			else
				allowMethods += ", POST";
		}
		if (req.methods & METHOD_DELETE)
		{
			if (allowMethods.size() == 0)
				allowMethods += "DELETE";
			else
				allowMethods += ", DELETE";
		}
		_headers["Allow"] = allowMethods; 
	}
}

void httpResponse::exeGet(HttpRequest &req){

	_statusCode = fillBody(req.path, req);
	std::cout << RED BOLD "ERROR CODE OUT FILL BODY >" << _statusCode << RESET << std::endl; 
	if (_statusCode != 200)
		handleError(req);
	fillHeaders(req.mult);
}

void httpResponse::fillBody(HttpRequest &req)
{
	std::ostringstream index;
	index << "<html><head><style>body{ background: #1a1a1a; color: white; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; font-family: sans-serif; flex-direction: column;} h1 {font-size: 100px; color: #16d705; margin-top: -50px; } </style></head><body> <h1>";
	if (_statusCode == 200 && req.method & METHOD_DELETE)
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
	if (!(req.methods & req.method))
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
	outFile.write(reinterpret_cast<char*>(&req.body[0]), req.body.size());
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
	fillHeaders(req.mult);
}

int httpResponse::deleteFile(std::string &path, HttpRequest &req)
{
	if (!(req.methods & req.method))
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
	fillHeaders(req.mult);
}
