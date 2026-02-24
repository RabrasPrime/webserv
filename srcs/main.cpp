#include "Parsing.hpp"
#include "Color.hpp"
#include "httpRequest.hpp"
#include <sstream>
#include <map>
#include "Engine.hpp"

int is_end_head(std::vector<unsigned char>::iterator it, std::vector<unsigned char>& vect)
{
	std::vector<unsigned char>::iterator end = vect.end();
	if (*it == '\r'
		&& it + 1 != end && *(it + 1) == '\n'
		&& it + 2 != end && *(it + 2) == '\r'
		&& it + 3 != end && *(it + 3) == '\n')
		return (1);
	return (0);
}

std::vector<std::string> ft_split(std::string& str, std::string split)
{
	std::vector<std::string> vect;
	int index = 0;
	size_t pos;
	do
	{
		pos = str.find(split, index);
		if (pos == std::string::npos)
			pos = str.size();
		std::string ligne(str.substr(index, pos - index));
		std::cout << ligne << "    " << index << " " << pos << std::endl;
		vect.push_back(ligne);
		index = pos + 2;
	} while (pos != str.size());
	return (vect);
}

void	fill_headers(std::vector<std::string>& ligne, HttpRequest& req)
{
	for (std::vector<std::string>::iterator it = ligne.begin();it != ligne.end();it++)
	{
		std::string key;
		std::string value;
		size_t pos = it->find(':');
		if (pos != std::string::npos)
		{
			key = it->substr(0, pos);
			int i;
			for (i = pos + 1;(*it)[i] && std::isspace((*it)[i]);i++)
				;
			value = it->substr(i, it->size());
			req.mult.insert(std::make_pair(key, value));
		}
	}
}

int	parse_header(const std::string& str, HttpRequest& req, std::vector<Server>& servers)
{
	(void)servers;
	(void)req;
	std::string first_line;
	first_line = str.substr(0,str.find("\r\n"));
	// std::cout << "first line >> " << first_line << std::endl;
	std::stringstream ss(first_line);
	std::string method;
	std::string path;
	std::string version;
	if (ss >> method >> path >> version)
	{
		char extra;
		if (ss >> extra)
			return (1);
	}
	std::cout << "Method : " << method << std::endl;
	std::cout << "Path : " << path << std::endl;
	std::cout << "Version : " << version << std::endl;
	std::string header(str.substr(str.find("\r\n") + 2, str.size()));
	// std::cout << header << std::endl;
	std::vector<std::string> ligne = ft_split(header, "\r\n");
	// for (std::vector<std::string>::iterator it = ligne.begin();it != ligne.end();it++)
	// {
	// 	std::cout << *it << std::endl;
	// }
	fill_headers(ligne, req);
	// for (std::map<std::string, std::string>::iterator it = req.mult.begin();it != req.mult.end();it++)
	// {
	// 	std::cout << it->first << ":" << it->second << std::endl;
	// }

	// std::pair<std::map<std::string, std::string>::iterator,std::map<std::string, std::string>::iterator> range;
	// for (std::map<std::string, std::string>::iterator it = range.first;it != range.second;it++)
	// {
	// 	std::cout << it->first << " : " << it->second << std::endl;
	// }
	// Server server;
	// for (std::vector<Server>::iterator it = servers.begin();it != servers.end();it++)
	// {
	// 	it->get_server_name();
	// }
	return (0);
}

int main(int ac, char **av)
{
	std::vector<Server> servers;
	std::string path;
	if (ac > 1)
		path = av[1];
	else
		path = "config_file/config_file";
	if (parse(servers, path))
		return (1);
	std::cout << servers.front() << std::endl;
	Engine engine;
	for (std::vector<Server>::iterator it = servers.begin(); it != servers.end();it++)
		engine.add_server(*it);
	engine.init_listeners();
	engine.setup_epoll();
	engine.run();
// 	std::string tmp("
// GET /images/logo.png HTTP/1.0\r\n
// Host: localhost:8080\r\n
// User-Agent: Mozilla/5.0 (Linux; x86_64)\r\n
// Accept: image/png,image/*;q=0.8\r\n
// Connection: close\r\n
// \r\n
// Content of body");

// // 	std::string tmp("
// // GET /images/logo.png HTTP/1.0\r\n
// // Accept: image/png,image/*;q=0.8\r\n
// // \r\n
// // Content of body");
// 	std::vector<unsigned char> vect(tmp.begin(),tmp.end());
// 	std::string header;
// 	std::string str;
// 	for (std::vector<unsigned char>::iterator it = vect.begin();it != vect.end();it++ )
// 	{
// 		if (is_end_head(it, vect))
// 		{
// 			std::cout << RED << "End Header" << RESET << std::endl;
// 			break;
// 		}
// 		else
// 			str += *it;
// 	}
// 	// std::cout << str << std::endl;
// 	HttpRequest req;
// 	parse_header(str, req, servers);
}




// int main(int ac, char **av)
// {
// 	std::vector<Server> servers;
// 	std::string path;
// 	if (ac > 1)
// 		path = av[1];
// 	else
// 		path = "config_file/config_file";
// 	if (parse(servers, path))
// 		return (1);
// 	std::cout << servers.front() << std::endl;
// 	return (0);
// }


// #include <iostream>
// #include <map>
// #include <string>
// #include "Color.hpp"
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <unistd.h>
// #include <sstream>
// #include "httpResponse.hpp"
// #include "httpRequest.hpp"

// int main()
// {
// 	HttpRequest req;
// 	req.method = "POST";
// 	req.path = "/home/samaouch/Documents/api_local/42_location.py?samaouch";
// 	req.version = "HTTP/1.1";
// 	req.headers["Host"] = "localhost:8080";
// 	req.headers["Content-Type"] = "text/html";
// 	req.headers["Connection"] = "Keep-Alive";
// 	req.body = "<h1>Hello World!<h1>";
// 	req.type = "html";
// 	req.queryString = "samaouch";
// 	req.auto_index = true;
// 	req.indexes.push_back("default_page.html");
// 	req.indexes.push_back("index.html");
// 	req.methods.push_back("GET");
// 	req.methods.push_back("POST");
// 	req.methods.push_back("DELETE");
// 	req.maxSize = 458;
// 	std::stringstream ss;
// 	ss << req.body.size();
// 	req.headers["Content-Length"] = ss.str();
// 	httpResponse resp;
// 	// std::string response = resp.handleResponse(req);
// 	// std::cout << std::endl << PURPLE BOLD "Final Response" RESET << std::endl << response << std::endl;

// 	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
// 	struct sockaddr_in addr;
// 	addr.sin_family = AF_INET;
// 	addr.sin_addr.s_addr = INADDR_ANY;
// 	addr.sin_port = htons(8080);
// 	bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));

// 	listen(server_fd, 10);
// 	std::cout << YELLOW BOLD "Run Server to http://localhost:8080" RESET << std::endl;

// 	while (1)
// 	{
// 		int client_fd = accept(server_fd, NULL, NULL);

// 		char buffer[4096] = {0};
// 		read(client_fd, buffer, sizeof(buffer));
// 		std::cout << BLUE BOLD "Buffer: " RESET << std::endl << buffer << std::endl;
// 		std::string response = resp.handleResponse(req);
// 		std::cout << std::endl << PURPLE BOLD "Final Response" RESET << std::endl << response << std::endl;
// 		write(client_fd, response.c_str(), response.size());
// 		close(client_fd);
// 	}
// 	return 0;
// }

