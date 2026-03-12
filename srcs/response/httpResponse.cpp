#include "httpResponse.hpp"
#include "Server.hpp"
#include <algorithm>

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
	_statusCode = 0;
	// std::cout << resp << "\n\n\n" << std::endl;
	return resp;
}

bool httpResponse::isCgiExtension(std::string currentPath, HttpRequest &req){
	size_t end = currentPath.find_last_of(".");
	std::string extension = currentPath.substr(end);

	for (std::map<std::string, std::string>::iterator it = req.cgi_ext.begin(); it != req.cgi_ext.end(); ++it)
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

	if (req.method & METHOD_GET)
		envList.push_back("REQUEST_METHOD=GET");
	if (req.method & METHOD_POST)
		envList.push_back("REQUEST_METHOD=POST");
	if (req.method & METHOD_DELETE)
		envList.push_back("REQUEST_METHOD=DELETE");


	for (std::vector<std::string>::iterator it = req.env.begin(); it != req.env.end(); ++it)
		envList.push_back(*it);
	std::stringstream ss;
	ss << req.body.size();
	if (req.chunked == 0)
	{
		if (!(req.method & METHOD_GET))
		{
			envList.push_back("CONTENT_LENGTH=" + ss.str());
			std::cerr << GOLD BOLD "CONTENT_LENGTH=" << ss.str() << RESET << std::endl;
			// std::stringstream sa;
			// struct stat st;
			// if (stat(req.path.c_str(), &st) == 0)
			// {
			// 	long long taille = st.st_size;
			// 	sa << taille;
			// }
			// envList.push_back("CONTENT_LENGTH=" + sa.str());
			// std::cerr << GOLD BOLD "CONTENT_LENGTH=" << sa.str() << RESET << std::endl;
		}
		else
		{
			envList.push_back("PATH_TRANSLATED=" + req.path);
		}
	}
	std::string tmp;
	for (std::vector<std::string>::iterator it = req.mult["Content-Type"].begin();it != req.mult["Content-Type"].end();it++)
	{
		if (it + 1 != req.mult["Content-Type"].end())
			tmp += *it + ",";
		else
			tmp += *it;
	}
	envList.push_back("CONTENT_TYPE=" + tmp);
	std::cerr << GOLD BOLD "CONTENT_TYPE=" << tmp << RESET << std::endl;
	std::string scriptName = req.raw_path;
	size_t pos = scriptName.find(".bla");
	if (pos != std::string::npos)
		scriptName = scriptName.substr(0, pos + 4);
	envList.push_back("SCRIPT_NAME=" + scriptName);
	// std::string pathInfo = "";
	// if (pos != std::string::npos && pos + 4 < req.raw_path.size())
	// 	pathInfo = req.raw_path.substr(pos + 4);
	std::cerr << "Raw path>" << req.raw_path << std::endl;
	std::cerr << "Raw Path>" << req.path << std::endl;
	envList.push_back("PATH_INFO=" + req.raw_path);
	envList.push_back("REQUEST_URI=" + req.raw_path);
	envList.push_back("SCRIPT_FILENAME=" + path);
	envList.push_back("SERVER_PROTOCOL=" + req.version);
	envList.push_back("REDIRECT_STATUS=200");
	envList.push_back("QUERY_STRING=" + req.queryString);

	std::map<std::string,std::vector<std::string> >::iterator it;
	for (it = req.mult.begin();it != req.mult.end();it++)
	{
		std::string content;
		std::vector<std::string>::iterator itt;
		for (itt = it->second.begin();itt != it->second.end();itt++)
		{
			if (content.size() == 0)
			{
				content += *itt;
			}
			else
			{
				content += ", " + *itt;
			}
		}
		std::string ident(it->first);
		std::transform(ident.begin(),ident.end(),ident.begin(),::toupper);
		envList.push_back("HTTP_" + ident + "=" + content);
		// std::cout << RED BOLD "HTTP_" + ident + "=" + content + RESET << std::endl;
	}

	std::cerr << BLUE BOLD "Scriptfilename {" << path << "}" RESET << std::endl;
	std::cerr << BLUE BOLD "scriptname {" << scriptName << "}" RESET << std::endl;
	// std::cerr << BLUE BOLD "path info {" << pathInfo << "}" RESET << std::endl;

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

// void httpResponse::saveCgiOutput(int *pipeOut, pid_t pid){

// 	std::string outCgi;
// 	char buffer[4096];
// 	int bytesRead = read(pipeOut[0], buffer, sizeof(buffer) - 1);
// 	while ((bytesRead > 0))
// 	{
// 		buffer[bytesRead] = '\0';
// 		outCgi += buffer;
// 		bytesRead = read(pipeOut[0], buffer, sizeof(buffer) - 1);
// 	}
// 	close(pipeOut[0]);

// 	int status;
// 	std::cerr << LIME BOLD "_BEFORE WAITPID" RESET << std::endl;
// 	waitpid(pid, &status, 0);
// 	std::cerr << GREEN BOLD "_AFTER WAITPID" RESET << std::endl;
// 	int code = WEXITSTATUS(status);
// 	_statusCode = code;
// 	code == 0 ? _statusCode = 200 : _statusCode = 500;
// 	_cgiOutput = outCgi;
// }

int httpResponse::exeCgi(std::string path, HttpRequest &req){
	std::cerr << "START EXE CGI" << std::endl;
	if (req.cgi_pid > 0)
		return 0;
	int pipeOut[2];//, pipeIn[2];
	req.isCgi = true;
	if (req.chunked != 0 && req.chunked_size != 0)
		return 200;
	// if (req.chunked == 0)
	// {
	// 	if (pipe(pipeIn) == -1 || pipe(pipeOut) == -1)
	// 		return 500;
	// }
	// else
	// {
	if (pipe(pipeOut) == -1)
		return 500;
	// }
	pid_t pid = fork();
	if (pid < 0)
	{
		// if (req.chunked == 0)
		// {
		// 	close(pipeIn[1]);
		// 	close(pipeIn[0]);
		// }
		close(pipeOut[0]);
		close(pipeOut[1]);
		return 500;
	}
	else if (pid == 0)
	{
		// if (req.chunked == 0)
		// {
		// 	if (dup2(pipeIn[0], STDIN_FILENO) == -1)
		// 		exit(EXIT_FAILURE);
		// 	close(pipeIn[1]);
		// 	close(pipeIn[0]);
		// }
		int fd;
		if (req.method & METHOD_GET)
		{
			std::cerr << "IS A GET" << std::endl;
			fd = open(req.path.c_str(),O_RDONLY,0644);
			if (fd == -1)
				exit(EXIT_FAILURE);
			if (dup2(fd,STDIN_FILENO) == -1)
				exit(EXIT_FAILURE);
		}
		else
		{
			std::cerr << "DUP FILE" << std::endl;
			fd = open(req.tmpName.c_str(),O_RDONLY,644);
			unlink(req.tmpName.c_str());
			if (fd == -1)
			{
				std::cerr << "FAIL TO OPEN FILE" << std::endl;
				exit(EXIT_FAILURE);
			}
			if (dup2(fd,STDIN_FILENO) == -1)
			{
				std::cerr << "FAIL TO DUP FILE" << std::endl;
				exit(EXIT_FAILURE);
			}
		}
		close(fd);

		if (dup2(pipeOut[1], STDOUT_FILENO) == -1)
			exit(EXIT_FAILURE);
		close(pipeOut[0]);
		close(pipeOut[1]);

		char *arg[] = {const_cast<char *>(_binary.c_str()), const_cast<char *>(req.path.c_str()), NULL};
		char **env = createEnv(req, path);
		// sleep(5);
		std::cerr << "RETURN ENV AFTER" << std::endl;
		execve(_binary.c_str(), arg, env);
		std::cerr << RED BOLD "FAIL EXECVE" RESET << std::endl;
		delete []env;
		exit(1);
	}
	req.cgi_pid = pid;
	// if (!(req.method & METHOD_GET))
	// {
		// req.pipeIn[1] = pipeIn[1];
		// req.pipeIn[0] = pipeIn[0];
		// fcntl(req.pipeIn[1], F_SETFL, O_NONBLOCK);
	if (req.method & METHOD_GET)
	{
	}
	else
	{
	}
	// }
	req.pipeOut[0] = pipeOut[0];
	req.pipeOut[1] = pipeOut[1];
	fcntl(req.pipeOut[0], F_SETFL, O_NONBLOCK);
	return 200;
}

int httpResponse::isCgi(HttpRequest &req, std::string path){

	struct stat s;
	// || req.raw_path.find(".bla") != std::string::npos
	int status = stat(path.c_str(), &s);
	if (req.chunked_size == 0 && status != 0 && req.path.find(".bla") != std::string::npos)
	{
		int fd = open(req.path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
		if (fd != -1)
			close(fd);
	}
	if (stat(path.c_str(), &s) == 0)
	{
		if (S_ISREG(s.st_mode))
		{	
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


// void httpResponse::parseCgiOutput(std::string dat){

// 	size_t sep =_cgiOutput.find("\r\n\r\n");
// 	size_t sizeSpace = 4;
// 	if (sep == std::string::npos)
// 	{
// 		sep = _cgiOutput.find("\n\n");
// 		sizeSpace = 2;
// 	}
// 	if (sep != std::string::npos)
// 	{
// 		std::string headers = _cgiOutput.substr(0, sep);
// 		std::string strContent = "Content-Type:";
// 		size_t posContentType = headers.find(strContent);
// 		if (posContentType != std::string::npos)
// 		{
			
// 			size_t posEndContentType = headers.find("\n", posContentType + strContent.size());
// 			_headers["Content-Type"] =  headers.substr(posContentType + strContent.size(), posEndContentType);
// 		}
// 		_body = _cgiOutput.substr(sep + sizeSpace);
// 	}
// 	else
// 		_body = _cgiOutput;
// }

// void httpResponse::fillCgiResponse(HttpRequest &req){

// 	_statusMsg = _mErrorMsg[_statusCode];
// 	// parseCgiOutput();

// 	size_t pos = _version.find('.');
// 	if (req.mult["Connection"].size() == 0 && _version[pos + 1] == '1')
// 		_headers["Connection"] = "Keep-Alive";
// 	else if (req.mult["Connection"].size() == 0 && _version[pos + 1] == '0')
// 		_headers["Connection"] = "close";
// 	else
// 		_headers["Connection"] = req.mult["Connection"].front();
// 	if (_headers["Content-Type"].empty())
// 		_headers["Content-Type"] = "text/html";
// 	std::stringstream ss;
// 	ss << _body.size();
// 	_headers["Content-Length"] = ss.str();
// }

std::string httpResponse::handleResponse(HttpRequest &req, int code){
	// std::cout << "HANDLE RESPONSE code>" << code << std::endl;
	_version = req.version;
	if (req.loc)
	{
		if (req.loc->get_is_set_return())
			code = 301;
	}
	else
	{
		code = 404;
	}
	if (code == 1)
	{
		code = 0;
		_statusCode = 0;
	}
	if (code != 0)
	{
		_statusCode = code;
		handleError(req);
		if (code == 200)
		{
			std::cout << PURPLE BOLD << "__________________HERE PASS" RESET << std::endl;
			_headers["Transfer-Encoding"] = "chunked";
			_headers["Content-Type"] = "text/plain";
			// _headers.erase("Content-Length");
			// _headers["Content-Length"] = "19";
			// PATH_INFO incorrect
			_headers.erase("Content-Length");
		}
		return convertFinalResponse();
	}
	if (_statusCode != 0)
	{
		_statusMsg = "Internal Server Error";
		handleError(req);
		return convertFinalResponse();
	}
	_statusCode = 0;
	
	_statusCode = isCgi(req, req.path);
	if (_statusCode != 0)
	{
		if (_statusCode != 200)
		{
			handleError(req);
			req.ErrorCode = _statusCode;
			return convertFinalResponse();
		}
		return "START_CGI";
	}
	if (req.method & METHOD_GET)
		exeGet(req);
	else if (req.method & METHOD_POST)
		exePost(req);
	else if (req.method & METHOD_DELETE)
		exeDelete(req);
	else
	{
		_statusCode = 405;//HERE
		handleError(req);
	}
	return convertFinalResponse();
}

void httpResponse::fillMapError(){
	std::ifstream inFile;
	inFile.open(PATH_FILE_CODE);
	if (!inFile.is_open())
	{
		_statusCode = 500;
		return ;
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
		 //std::cout << LIME BOLD "_________________________HERE code >" << value << "    msg >" << errorMsg << RESET << std::endl;
	}
	inFile.close();
}

void httpResponse::fillMapExtension(std::map<std::string, std::string> &map, std::string pathFile){

	std::ifstream inFile;
	inFile.open(pathFile.c_str());
	if (!inFile.is_open())
	{
		_statusCode = 500;
		return ;
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
		_headers["Connection"] = "Keep-Alive";
	else if (mult["Connection"].size() == 0 && _version[pos + 1] == '0')
		_headers["Connection"] = "close";
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
		if (errno != 0)
			return (500);
		if (!readDir)
			break ;
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
			inFile.close();
			return 200;
		}
	}
	if (req.auto_index)
		return (generateAutoIndex(path));
	else
		return 404;//HERE
	return 200;
}


int httpResponse::fillBody(std::string &path, HttpRequest &req) {

	if (!(req.methods & req.method))
	{
		std::cout << PURPLE BOLD " __________________________HERE" RESET << std::endl;
		std::cout << "method >>> " << req.method << std::endl;
		return 405;
	}
	struct stat s;
	if (stat(path.c_str(), &s) == -1)
	{
//  std::cout << RED BOLD "PATH NOT FOUND" RESET << std::endl;
		if (req.path[req.path.size() - 1] != '/')
			return 301;
		return 404;
	}
	std::ifstream inFile;
	if (S_ISDIR(s.st_mode))
	{
 //std::cout << RED BOLD "REQ IS A DIR" RESET << std::endl;
		if (req.path[req.path.size() - 1] != '/')
			return 301;
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
		inFile.close();
		_bodyType = path.substr(path.find_last_of(".") + 1, path.size());
	}
	return 200;	
}

std::string httpResponse::setPathError(HttpRequest &req)
{
	std::string pathErrFile;
	if (req.error_pages.find(_statusCode) != req.error_pages.end())
		return req.error_pages[_statusCode];
	switch (_statusCode)
	{
		
		case 401:
			pathErrFile = "file/error_page/error_page_401.html";
			break ;
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
	}
	return pathErrFile;
}

void httpResponse::fillDefaultBody(HttpRequest &req){
	std::cout << PURPLE BOLD "____________________________________code >> " << _statusCode << RESET << std::endl;
	if (_statusCode == 405 || req.isCgi)
		return;//HERE
	std::string pathErrFile = setPathError(req);
	std::ifstream inFile;
	if (!pathErrFile.empty())
		inFile.open(pathErrFile.c_str(), std::ios::binary);
	if (pathErrFile.empty() || !inFile.is_open())
	{
		std::ostringstream index;
		index 	<< "<html><head><style> body {background-color: black; text-align: center;} h1 { font-size: 200px; color: #d70516; margin-top -50px; } p {font-size: 100px; font-weight: bold; margin-top: -60px; color: #d70516; } </style></head><body><div><h1>" << _statusCode << "</h1><p>" << _statusMsg << "</p></div></body></html>";
		_body = index.str();
		_bodyType = "html";
	}
	else
	{
		std::ostringstream oss;
		oss << inFile.rdbuf();
		_body = oss.str();
		_bodyType = pathErrFile.substr(pathErrFile.find_last_of(".") + 1);
		inFile.close();
	}
}

void httpResponse::handleError(HttpRequest &req)
{
 //std::cout << YELLOW BOLD "HANDLE ERROR CODE >>>" << _statusCode << RESET << std::endl;
	_statusMsg = _mErrorMsg[_statusCode];
 //std::cout << YELLOW BOLD "MESSAGE FOUND >>>" << _statusMsg << RESET << std::endl;
	if (_statusMsg.empty() && _statusCode == 0)
	{
		_statusCode = 500;
		_statusMsg = "Internal Server Error";
	}
	fillDefaultBody(req);
	if (_statusCode == 301)
	{
		if (req.loc->get_is_set_return())
			_headers["Location"] = req.loc->get_return_path();
		else
			_headers["Location"] = req.raw_path + '/' + req.queryString;
	}

	_headers["Content-Type"] = _mMimeTypes[_bodyType];
	if (_headers["Content-Type"].empty())
		_headers["Content-Type"] = DEFAULT_TYPE;
	if (_statusCode == 500)
		_headers["Connection"] = "close";
	else
	{
		size_t pos = req.version.find('.');
		if (req.mult["Connection"].size() == 0 && req.version[pos + 1] == '1')
			_headers["Connection"] = "Keep-Alive";
		else if (req.mult["Connection"].size() == 0 && _version[pos + 1] == '0')
			_headers["Connection"] = "close";
		else if (req.mult.find("Connection") != req.mult.end() && req.mult["Connection"].size() != 0)
			_headers["Connection"] = req.mult["Connection"].front();
		else
			_headers["Connection"] = "close";
	}
	std::stringstream ss;
	ss << _body.size();
	_headers["Content-Length"] = ss.str();
	if (_statusCode == 501 || _statusCode == 405)
	{
		std::string allowMethods;
 //std::cout << PURPLE BOLD "Allow methods >>>" RESET << req.methods << std::endl;

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

std::string httpResponse::getUserValue(HttpRequest &req, std::string const &key)
{
	if (req.mult.count("Cookie") == 0 || req.mult["Cookie"].empty())
		return "";
	std::string searchKey = key + "=";
	size_t start = req.mult["Cookie"].front().find(searchKey);
	if (start == std::string::npos)
		return "";
	start += searchKey.length();
	size_t end = req.mult["Cookie"].front().find(';', start);
	if (end == std::string::npos)
		return req.mult["Cookie"].front().substr(start);
	return req.mult["Cookie"].front().substr(start, end);
}

void httpResponse::exeGet(HttpRequest &req){

	if (req.raw_path == "/Account")
	{
		std::string username = getUserValue(req, "user_session");
		if (!username.empty() && req.tartgetServ->userExist(username))
		{
			User &user = req.tartgetServ->getUser(username);
			_statusCode = 200;
			fillSignBody(user);
			fillHeaders(req.mult);
			return ;
		}
	}
	_statusCode = fillBody(req.path, req);
	if (_statusCode != 200)
		handleError(req);
	fillHeaders(req.mult);
}

void httpResponse::fillBody(HttpRequest &req)
{
	if (_statusCode == 100)
		return;
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

	// std::ofstream outFiletmp;
	 //std::cout << BLUE BOLD "req.chunked" << req.chunked << RESET << std::endl;
	if (!req.outFile)
	{
		// req.outFile = &outFiletmp;
		req.outFile = new std::ofstream;
		req.outFile->open(path.c_str(), std::ios::binary | std::ios::trunc);
	}
	if (!req.outFile->is_open())
	{
 //std::cout << PURPLE BOLD "NOT OPEN" RESET << std::endl;
		return 500;
	}		
 //std::cout << RED BOLD "WRITE IN FILE" RESET << std::endl;
	req.outFile->write(reinterpret_cast<char*>(&req.body[0]), req.body.size());
	if (!req.outFile->good())
	{
 //std::cout << PURPLE BOLD "NOT GOOD" RESET << std::endl;
		req.outFile->close();
		return 500;
	}
	if (req.chunked == 0 || req.chunked_size == 0)
	{
		req.outFile->close();
		// std::cout << YELLOW BOLD "close fd" RESET << std::endl;
		delete req.outFile;
	}
	return fileCreated ? 201 : 200;
}

void httpResponse::exePost(HttpRequest &req)
{
	if (!(req.methods & req.method))
	{
		_statusCode = 405;
		handleError(req);
		return ;
	}
 	if (req.body.size() > req.maxSize)
	{
		_statusCode = 413;
		handleError(req);
		return ;
	}
	if (req.body.size() == 0 && req.raw_path != "/logout" && req.chunked == 0)
	{
		_statusCode = 411;
		handleError(req);
		return ;
	}
	if (req.raw_path == "/signup")
	{
		User newUser;

		std::string body(req.body.begin(), req.body.end());
		std::map<std::string, std::string> parsed = parseUser(body);
		newUser.email = parsed["email"];
		newUser.password = parsed["password"];
		newUser.UserName = parsed["username"];
		newUser.defeat = 0;
		newUser.victory = 0;
		req.tartgetServ->setNewUser(newUser);
		_headers["Set-Cookie"] = "user_session=" + newUser.UserName + "; Path=/; HttpOnly";
		fillSignBody(newUser);
		fillHeaders(req.mult);
		if (_statusCode == 0)
			_statusCode = 200;
		if (_statusCode != 200)
			handleError(req);
	}
	else if (req.raw_path == "/logout")
	{
		_headers["Set-Cookie"] = "user_session=; Path=/; Max-Age=0; HttpOnly";
		_statusCode = 303;
		_headers["Location"] = "/Account";
		_body = "";
		fillHeaders(req.mult);
	}
	else if (req.raw_path == "/signin")
	{
		std::string body(req.body.begin(), req.body.end());
		std::map<std::string, std::string> parsed = parseUser(body);
		std::string username = parsed["username"];
		std::string password = parsed["password"];
		if (req.tartgetServ->userExist(username))
		{
			User &user = req.tartgetServ->getUser(username);
			if (user.password == password)
			{
				_headers["Set-Cookie"] = "user_session=" + user.UserName + "; Path=/; HttpOnly";
				_statusCode = 200;
				fillSignBody(user);
			}
			else
			{
				_statusCode = 401;
				handleError(req);
			}
		}
		else
		{
			_statusCode = 401;
			handleError(req);
		}
		fillHeaders(req.mult);
	}
	else
	{
		_statusCode = isFileExist(req.path, req);
 //std::cout << GREEN BOLD "STATUS CODE>>" << _statusCode << RESET << std::endl;
		if (_statusCode != 200 && _statusCode != 201)
		{
			handleError(req);
			return ;
		}
		if (req.chunked != 0 && req.chunked_size != 0)
			_statusCode = 100;
		_statusMsg = _mErrorMsg[_statusCode];
		fillBody(req);
		fillHeaders(req.mult);
	}
}

void httpResponse::fillSignBody(User &newUser)
{
	std::ifstream inFile;
	std::stringstream ss;
	inFile.open(PATH_SIGNIN);
	if (!inFile.is_open())
	{
		_statusCode = 500;
		return ;
	}
	ss << inFile.rdbuf();
	std::string strHtml = ss.str();
	ss.str("");
	replaceData(strHtml, "{{USERNAME}}", newUser.UserName);
	size_t pos = newUser.email.find("%40");
	if (pos != std::string::npos)
		newUser.email.replace(pos, 3, "@");
	replaceData(strHtml, "{{EMAIL}}", newUser.email);
	ss << newUser.victory;
	replaceData(strHtml, "{{VICTORY}}", ss.str());
	ss.str("");
	ss << newUser.defeat;
	replaceData(strHtml, "{{DEFEAT}}", ss.str());
	ss.str("");
	ss << newUser.defeat + newUser.victory;
	replaceData(strHtml, "{{TOTAL}}", ss.str());
	_body = strHtml;
	_bodyType = "html";
	inFile.close();
}

void httpResponse::replaceData(std::string &str, const std::string &from, const std::string &to)
{
	size_t pos = str.find(from);
	if (pos != std::string::npos)
		str.replace(pos, from.length(), to);
}

std::map<std::string, std::string> httpResponse::parseUser(std::string const &body)
{

	std::map<std::string, std::string> res;
	std::stringstream ss(body);
	std::string data;

	while(std::getline(ss, data, '&'))
	{
		size_t pos = data.find('=');
		if (pos != std::string::npos)
		{
			std::string key = data.substr(0, pos);
			std::string value = data.substr(pos + 1);
			res[key] = value;
		}
	}
	return res;
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
