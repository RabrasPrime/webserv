#include "Parsing.hpp"
#include "Color.hpp"
#include "httpRequest.hpp"
#include <sstream>
#include <map>
#include "Engine.hpp"

int is_end_line(std::vector<unsigned char>::iterator it, std::vector<unsigned char>& vect)
{
	std::vector<unsigned char>::iterator end = vect.end();
	if (*it == '\r' && it + 1 != end && *(it + 1) == '\n')
		return (1);
	return (0);
}

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
		 //std::cout << ligne << "    " << index << " " << pos << std::endl;
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

int	parse_header(const std::string& tmp, HttpRequest& req, std::vector<Server*> servers)
{
	(void)req;
	size_t i;
	for (i = 0;(tmp[i] == '\n' || tmp[i] == '\r') && i < tmp.size();i++)
		;
	std::string str(&tmp[i]);
 std::cout << YELLOW BOLD "\n\nSTR >> " << str << RESET << std::endl;
	std::string first_line;
	if (str.find("\r\n") != std::string::npos)
		first_line = str.substr(0,str.find("\r\n"));
	else
		first_line = str;
 //std::cout << "first line >> " << first_line << std::endl;
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
	if (!method.size() || !path.size() || !version.size())
		return (400);
 //std::cout << "Method : " << method << std::endl;
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
 //std::cout << "Path : " << path << std::endl;
	req.raw_path = path;
 //std::cout << "Version : " << version << std::endl;
 //std::cout << "___________Query String >" << req.queryString << std::endl;

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
	//  //std::cout << *it << std::endl;
	// }
	fill_headers(ligne, req);
	for (std::map<std::string,std::vector<std::string> >::iterator it = req.mult.begin();it != req.mult.end();it++)
	{
 //std::cout << it->first << " : " <<  std::endl;
		for (std::vector<std::string>::iterator itt = it->second.begin();itt != it->second.end();itt++)
		{
 //std::cout << "\t" << *itt;
		}
 //std::cout << std::endl;
	}
	//HERE identifier le bon server
	Server* server = NULL;
	for (std::vector<Server *>::iterator it = servers.begin();server == NULL && it != servers.end();it++)
	{
		std::vector<std::string> list_name = (*it)->get_server_name();
		for (std::vector<std::string>::iterator itt = list_name.begin();server == NULL && itt != list_name.end();itt++)
		{
			if (req.mult["Host"].size() != 0 && *itt == req.mult["Host"].front())
			{
				server = *it;
			}
		}
	}
	if (server == NULL)
	{
 //std::cout << RED BOLD "NULL" RESET << std::endl;
		for (std::vector<Server *>::iterator it = servers.begin();server == NULL && it != servers.end();it++)
		{
			std::vector<std::string> list_name = (*it)->get_server_name();
			if (list_name.size() == 0)
				server = *it;
		}
	}
	if (server == NULL && servers.size() != 0)
		server = servers.front();
	req.tartgetServ = server;
 //std::cout << RED BOLD "HERE" RESET << std::endl;
	const std::map<std::string, Location>& locations = server->get_locations();
	Location* best = NULL;
	size_t size_match = 0;
	for (std::map<std::string, Location>::const_iterator it = locations.begin(); it != locations.end(); it++)
    {
        std::string p = it->second.get_path();
        if (path.compare(0, p.size(), p) == 0)
        {
            if (p.size() > size_match)
            {
                best = const_cast<Location*>(&it->second);
                size_match = p.size();
            }
        }
    }
 //std::cout << "Path >> " << path << std::endl;
	if (!best)
	{
		req.location_match = 0;
		req.error_pages = server->get_error_pages();
		req.methods = server->get_methods();
		req.auto_index = server->get_auto_index();
		req.indexes = server->get_indexes();
		req.maxSize = server->get_client_max_body_size();
		if (path[1])
			req.path = server->get_root() + &path[1];
		else
			req.path = server->get_root();
		req.cgi_ext = server->get_cgi_ext();
	}
	else
	{
 //std::cout << "Found Best ____________" << std::endl;

		req.location_match = 1;
		req.error_pages = best->get_error_pages();
		req.methods = best->get_methods();
		req.auto_index = best->get_auto_index();
		req.indexes = best->get_indexes();
		req.maxSize = best->get_client_max_body_size();
		req.cgi_ext = best->get_cgi_ext();
		
		req.loc = best;

 //std::cout << *best << std::endl;

		if (best->get_use_alias())
		{
 //std::cout << RED BOLD "USE ALIAS" RESET << std::endl;
			req.path = best->get_alias() + &path[best->get_path().size()];
		}
		else
			req.path = best->get_root() + &path[1];
	}
 //std::cout << "__________________Path>" << req.path << std::endl;
	// for (std::multimap<std::string, std::string>::iterator it = req.mult.begin();it != req.mult.end();it++)
	// {
	//  //std::cout << it->first << ":" << it->second << std::endl;
	// }

	// std::pair<std::map<std::string, std::string>::iterator,std::map<std::string, std::string>::iterator> range;
	// for (std::map<std::string, std::string>::iterator it = range.first;it != range.second;it++)
	// {
	//  //std::cout << it->first << " : " << it->second << std::endl;
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
	// signal(SIGPIPE, SIG_IGN);
	std::vector<Server> servers;
	std::string path;
	if (ac > 1)
		path = av[1];
	else
		path = "config_file/config_file";
	if (parse(servers, path))
		return (1);
	 //std::cout << servers.front() << std::endl;

	Engine engine;
	for (std::vector<Server>::iterator it = servers.begin(); it != servers.end();it++)
		engine.add_server(*it);
	engine.init_listeners();
	engine.setup_epoll();
	signal(SIGINT, Engine::signal_handler);
	signal(SIGTERM, Engine::signal_handler);
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
	// 	//std::cout << RED << "End Header" << RESET << std::endl;
	// 		break;
	// 	}
	// 	else
	// 		str += *it;
	// }
	//  //std::cout << str << std::endl;
	// HttpRequest req;
	// parse_header(str, req, servers);
}


void Engine::handle_chunked(std::vector<unsigned char>& vect, Client& client, const int client_fd)
{
	std::cout << BLUE BOLD "HANDLE CHUNCK" RESET << std::endl;
	do
	{
		std::cout << ORANGE BOLD "HANDLE CHUNCK" RESET << std::endl;
		client.req.chunked_size = -1;
		if (client.req.chunked <= 1)
		{
			// std::string s(vect.begin(), vect.end());
			// std::cout << ORANGE BOLD "BODY CHUCK>>>" << s << "END" << RESET << std::endl;
			//size
			client.req.chunked = 1;
			std::vector<unsigned char>::iterator it;
			std::string hexa;
			for (it = vect.begin();it != vect.end();it++)
			{
				if (is_end_line(it, vect))
				{
					if (client.req.chunked == 0)
						client.req.chunked = 5;
					else
						client.req.chunked = 3;
		//  std::cout << ORANGE BOLD "HEXA >>>" << hexa << RESET <<std::endl;
					// std::string hexa = "FF";
					size_t valeur;
					std::stringstream ss;
					ss << std::hex << hexa;
					ss >> valeur;
					// std::cerr << ORANGE BOLD "INT >>>" << valeur << RESET <<std::endl;
					if (!client.req.isCgi)
						std::cerr << ORANGE BOLD "INT >>>" << valeur << RESET <<std::endl;
					client.req.chunked_size = valeur;
					client.req.total_size += valeur;
					if (client.req.total_size > client.req.maxSize && !client.req.ErrorCode)
					{
						std::cerr << ORANGE BOLD "REQ BODY TOO BIG" RESET << std::endl;
						client.get_read().clear();
						vect.clear();
						if (client.req.isCgi)
						{
							close(client.req.pipeIn[1]);
							close(client.req.pipeIn[0]);
							close(client.req.pipeOut[1]);
							client.req.pipeIn[1] = -1;
							// int pipefd = client.req.pipeOut[0];
							// _fd_types[pipefd] = FD_CGI_PIPE;
							// _map_cgi_pid[pipefd] = client.req.cgi_pid;
							// _cgi_to_client[pipefd] = client_fd;
							// std::cerr << "ADD FD BEFORE" << std::endl;
							// add_to_epoll(pipefd, EPOLLIN);
							// std::cerr << "ADD FD AFTER" << std::endl;
							return;
						}
						client.req.ErrorCode = 413;
						client._write_buffer = client.res.handleResponse(client.req, 413);
						// modify_epoll(client_fd, EPOLLOUT | EPOLLET);
						return;
					}
					// std::cerr << "SIZE VECT>" << vect.size() << std::endl;

					vect.erase(vect.begin(),it + 2);
					client.req.chunked--;
					break;
				}
				else
					hexa += *it;
			}
			if (client.req.chunked == 1)
				return ;
		}
		if (client.req.chunked_size == 0 && client.req.chunked > 1)
		{
			if (client.req.ErrorCode != 0)
				modify_epoll(client_fd, EPOLLOUT | EPOLLET);
			// for (size_t i = 0;i < vect.size();i++)
			// {
			// 	std::cerr << "int val>" << (int)vect[i] << std::endl;
			// }
			std::cerr << "SIZE VECT>" << vect.size() << std::endl;
			// if (vect.size() == 0)
			// {
			// 	return ;
			// }
			std::string str(vect.begin(),vect.end());
			// std::cerr << BLUE BOLD "CALL RESPONSE END FUNC CHUNCKED HERE>";
			// std::cerr << str ;
			// std::cerr << "<END" RESET << std::endl;
			client.get_read().clear();
			vect.clear();
			if (client.req.isCgi)
			{
				close(client.req.pipeIn[1]);
				close(client.req.pipeIn[0]);
				close(client.req.pipeOut[1]);
				client.req.pipeIn[1] = -1;
				// int pipefd = client.req.pipeOut[0];
				// _fd_types[pipefd] = FD_CGI_PIPE;
				// _map_cgi_pid[pipefd] = client.req.cgi_pid;
				// _cgi_to_client[pipefd] = client_fd;
				// std::cerr << "ADD FD BEFORE" << std::endl;
				// add_to_epoll(pipefd, EPOLLIN);
				// std::cerr << "ADD FD AFTER" << std::endl;
				return;
			}
			client.req.body = client.get_read();
			// if (!client.req.ErrorCode)
				client._write_buffer = client.res.handleResponse(client.req, 0);
			client.get_read().clear();
			// std::cout << PURPLE BOLD << client.get_read().size() << RESET << std::endl;
			modify_epoll(client_fd, EPOLLOUT | EPOLLET);
			return ;
		}
		if (client.req.chunked > 1)
		{
			//content

			std::vector<unsigned char>::iterator it;
			for (it = vect.begin();it != vect.end();it++)
			{
				if (is_end_line(it, vect))
				{
					client.req.body.clear();
					client.req.body.insert(client.req.body.end(), vect.begin(), it);

					std::string s(client.req.body.begin(), client.req.body.end());
					//  std::cout << LIME BOLD "BODY>>>" << s << RESET << std::endl;

					vect.erase(vect.begin(),it + 2);
					if (client.req.outFile)
					{
						client.req.outFile->write(reinterpret_cast<char*>(&client.req.body[0]),client.req.body.size());
						// il nous manquait un clear je crois 
						client.req.body.clear();
					}
					else if (client.req.isCgi)
					{
						std::cout << "WRITE HERE" << std::endl;
						write(client.req.pipeIn[1], reinterpret_cast<char*>(&client.req.body[0]), client.req.body.size());
						client.req.body.clear();
					}
					else
					{
						std::cout << ORANGE BOLD "CALL REPONSE HERE" RESET << std::endl;
						client.res.handleResponse(client.req, client.req.ErrorCode);
						// if (client.req.ErrorCode == 404)
						// {
						// 	modify_epoll(client_fd, EPOLLOUT | EPOLLET);
						// 	return ;
						// }

						int pipefd = client.req.pipeOut[0];
						_fd_types[pipefd] = FD_CGI_PIPE;
						_map_cgi_pid[pipefd] = client.req.cgi_pid;
						_cgi_to_client[pipefd] = client_fd;
						std::cout << BROWN "ADD FD BEFORE" RESET << std::endl;
						add_to_epoll(pipefd, EPOLLIN);
						std::cout << BROWN "ADD FD AFTER" RESET << std::endl;

						// write(client.req.pipeIn[1], reinterpret_cast<char*>(&client.req.body[0]), client.req.body.size());

						while (client.req.body.size() > 0)
						{
							size_t bytes_write = write(client.req.pipeIn[1], reinterpret_cast<char*>(&client.req.body[0]), client.req.body.size());
							if (bytes_write > 0)
							{
								client.req.body.erase(client.req.body.begin(),client.req.body.begin() + bytes_write);
							}
							else if (errno == EAGAIN || errno == EWOULDBLOCK)
							{
								remove_from_epoll(client_fd);
								int pipefd = client.req.pipeIn[1];
								_fd_types[pipefd] = FD_CGI_PIPE_WRITE;
								_map_cgi_pid[pipefd] = client.req.cgi_pid;
								_cgi_to_client[pipefd] = client_fd;
								std::cout << LIME "ADD FD BEFORE" RESET << std::endl;
								add_to_epoll(pipefd, EPOLLIN);
								std::cout << LIME "ADD FD AFTER" RESET << std::endl;
							}
							else
							{
								std::cerr << RED BOLD "AIE MAIN" RESET << std::endl;
							}
							std::cout << "HERE" << std::endl;
						}

						// client.req.body.clear();
					}
					client.req.chunked = 1;
					break;
				}
			}
		}
	}
	while (client.req.chunked == 1);
	
	// std::string hexa;
	// for (it = vect.begin();it != vect.end();it++)
	// {
	// 	if (is_end_line(it, vect))
	// 	{
	// 		client.req.chunked = 2;
	// 	//std::cout << ORANGE BOLD "HEXA >>>" << hexa << RESET <<std::endl;
	// 		// std::string hexa = "FF";
	// 		size_t valeur;
	// 		std::stringstream ss;
	// 		ss << std::hex << hexa;
	// 		ss >> valeur;
	// 	//std::cout << ORANGE BOLD "INT >>>" << valeur << RESET <<std::endl;
	// 		client.req.chunked_size = valeur;
	// 		vect.erase(vect.begin(),it + 2);
	// 	}
	// 	else
	// 		hexa += *it;
	// }
	// if (client.req.chunked_size == 0)
	// {
	// 	client.req.chunked = 0;
	// 	client.req.body = client.get_read();
	// 	client._write_buffer = client.res.handleResponse(client.req, client.req.ErrorCode);
	// 	client.get_read().clear();
	// 	modify_epoll(client_fd, EPOLLOUT | EPOLLET);
	// }
	// else if (it == vect.end())
	// {
	 //std::cout << GREEN BOLD "MISSING END" RESET << std::endl;
	// 	return;
	// }
}
