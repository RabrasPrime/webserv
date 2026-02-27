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
		// std::cout << ligne << "    " << index << " " << pos << std::endl;
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
			req.mult[key];
			req.mult[key].push_back(value);
			// req.mult.insert(std::make_pair(key, value));
		}
	}
}

int	parse_header(const std::string& str, HttpRequest& req, Server* server)
{
	(void)server;
	(void)req;
	std::string first_line;
	first_line = str.substr(0,str.find("\r\n"));
	std::cout << "first line >> " << first_line << std::endl;
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
	size_t index = path.find('?');
	if (index != std::string::npos)
	{
		index++;
		req.queryString = path.substr(index, path.size() - index);

		size_t pos;
		do
		{
			pos = path.find(index, '&');
			if (pos == std::string::npos)
				pos = path.size();
			std::string s = path.substr(index, pos - index);
			req.env.push_back(s);
			index = pos + 1;
		} while (pos != path.size());
	}
	std::cout << "Path : " << path << std::endl;
	req.raw_path = path;
	std::cout << "Version : " << version << std::endl;
	std::cout << "___________Query String >" << req.queryString << std::endl;

	path = path.substr(0, path.find('?'));
	
	if (method == "GET")
		req.method = METHOD_GET;
	if (method == "DELETE")
		req.method = METHOD_DELETE;
	if (method == "POST")
		req.method = METHOD_POST;
	req.path = path;
	req.version = version;
	std::string header(str.substr(str.find("\r\n") + 2, str.size()));
	// std::cout << header << std::endl;
	std::vector<std::string> ligne = ft_split(header, "\r\n");
	// for (std::vector<std::string>::iterator it = ligne.begin();it != ligne.end();it++)
	// {
	// 	std::cout << *it << std::endl;
	// }
	fill_headers(ligne, req);
	for (std::map<std::string,std::vector<std::string> >::iterator it = req.mult.begin();it != req.mult.end();it++)
	{
		std::cout << it->first << " : " <<  std::endl;
		for (std::vector<std::string>::iterator itt = it->second.begin();itt != it->second.end();itt++)
		{
			std::cout << "\t" << *itt;
		}
		std::cout << std::endl;
	}
	//HERE remove query string
	std::map<std::string, Location> locations = server->get_locations();
	Location* best = NULL;
	size_t size_match = 0;
	for (std::map<std::string, Location>::iterator it = locations.begin(); it != locations.end(); it++)
	{
		std::string p = it->second.get_path();
		if (path.compare(0, p.size(), p) == 0)
		{
			if (p.size() > size_match)
			{
				best = &it->second;
				size_match = p.size();
			}
		}
	}
	if (!best)
	{
		req.location_match = 0;
		req.error_pages = server->get_error_pages();
		req.methods = server->get_methods();
		req.auto_index = server->get_auto_index();
		req.indexes = server->get_indexes();
		req.maxSize = server->get_client_max_body_size();
		req.path = server->get_root() + &path[1];
		req.cgi_ext = server->get_cgi_ext();
	}
	else
	{
		std::cout << "Found Best ____________" << std::endl;

		req.location_match = 1;
		req.error_pages = best->get_error_pages();
		req.methods = best->get_methods();
		req.auto_index = best->get_auto_index();
		req.indexes = best->get_indexes();
		req.maxSize = best->get_client_max_body_size();
		req.cgi_ext = best->get_cgi_ext();
		req.loc = best;

		std::cout << *best << std::endl;

		if (best->get_use_alias())
		{
			std::cout << RED BOLD "USE ALIAS" RESET << std::endl;
			req.path = best->get_alias() + &path[best->get_path().size()];
		}
		else
			req.path = best->get_root() + &path[1];
	}
	std::cout << "__________________Path>" << req.path << std::endl;
	// for (std::multimap<std::string, std::string>::iterator it = req.mult.begin();it != req.mult.end();it++)
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
	// std::cout << servers.front() << std::endl;

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

// 	std::string tmp("
// GET /images/logo.png HTTP/1.0\r\n
// Accept: image/png,image/*;q=0.8\r\n
// \r\n
// Content of body");
	// std::vector<unsigned char> vect(tmp.begin(),tmp.end());
	// std::string header;
	// std::string str;
	// for (std::vector<unsigned char>::iterator it = vect.begin();it != vect.end();it++ )
	// {
	// 	if (is_end_head(it, vect))
	// 	{
	// 		std::cout << RED << "End Header" << RESET << std::endl;
	// 		break;
	// 	}
	// 	else
	// 		str += *it;
	// }
	// std::cout << str << std::endl;
	// HttpRequest req;
	// parse_header(str, req, servers);
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

