#include "Client.hpp"
#include "Engine.hpp"

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
		}
	}
}

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

void Engine::handle_chunked(std::vector<unsigned char>& vect, Client& client, const int client_fd)
{
	do
	{
		client.req.chunked_size = -1;
		if (client.req.chunked <= 1)
		{
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
					size_t valeur;
					std::stringstream ss;
					ss << std::hex << hexa;
					ss >> valeur;
					client.req.chunked_size = valeur;
					client.req.total_size += valeur;
					if (client.req.total_size > client.req.maxSize && !client.req.ErrorCode)
					{
						client.get_read().clear();
						vect.clear();
						if (client.req.isCgi)
						{
							close(client.req.pipeOut[1]);
							client.req.pipeIn[1] = -1;
							return;
						}
						client.req.ErrorCode = 413;
						client._write_buffer = client.res.handleResponse(client.req, 413);
						return;
					}

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
			std::string str(vect.begin(),vect.end());
			client.get_read().clear();
			vect.clear();
			if (client.req.isCgi)
			{
				close(client.req.fd);
				client.res.handleResponse(client.req, 1);

				int pipefd = client.req.pipeOut[0];
				_fd_types[pipefd] = FD_CGI_PIPE;
				_map_cgi_pid[pipefd] = client.req.cgi_pid;
				_cgi_to_client[pipefd] = client_fd;
				add_to_epoll(pipefd, EPOLLIN);
				close(client.req.pipeOut[1]);
				return;
			}
			client.req.body = client.get_read();
			client._write_buffer = client.res.handleResponse(client.req, 0);
			client.get_read().clear();
			modify_epoll(client_fd, EPOLLOUT | EPOLLET);
			return ;
		}
		if (client.req.chunked > 1)
		{
			std::vector<unsigned char>::iterator it;
			for (it = vect.begin();it != vect.end();it++)
			{
				if (is_end_line(it, vect))
				{
					client.req.body.insert(client.req.body.end(), vect.begin(), it);

					vect.erase(vect.begin(),it + 2);
					if (client.req.outFile)
					{
						client.req.outFile->write(reinterpret_cast<char*>(&client.req.body[0]),client.req.body.size());
						client.req.body.clear();
					}
					else if (client.req.isCgi)
					{
						write(client.req.fd, &client.req.body[0], client.req.body.size());
						client.req.body.clear();
					}
					else
					{
						client.res.handleResponse(client.req, client.req.ErrorCode);
						if (client.req.isCgi)
						{
							size_t i = 0;
							std::stringstream ss;
							ss << i;
							client.req.tmpName = "/tmp/" + ss.str() + ".tmp" ;
							struct stat st;
							while (stat(client.req.tmpName.c_str(), &st) != -1)
							{
								i++;
								ss.str("");
								ss.clear();
								ss << i;
								client.req.tmpName = "/tmp/" + ss.str() + ".tmp" ;
							}
							client.req.fd = open(client.req.tmpName.c_str(), O_CREAT | O_WRONLY, 6044);
						}
					}
					client.req.chunked = 1;
					break;
				}
			}
		}
	}
	while (client.req.chunked == 1);
}

int	parse_header(const std::string& tmp, HttpRequest& req, std::vector<Server*> servers)
{
	(void)req;
	size_t i;
	for (i = 0;(tmp[i] == '\n' || tmp[i] == '\r') && i < tmp.size();i++)
		;
	std::string str(&tmp[i]);
	std::string first_line;
	if (str.find("\r\n") != std::string::npos)
		first_line = str.substr(0,str.find("\r\n"));
	else
		first_line = str;
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
	req.raw_path = path;

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
	std::vector<std::string> ligne = ft_split(header, "\r\n");
	fill_headers(ligne, req);

	Server* server = NULL;
	for (std::vector<Server *>::iterator it = servers.begin();server == NULL && it != servers.end();it++)
	{
		std::vector<std::string> list_name = (*it)->get_server_name();
		for (std::vector<std::string>::iterator itt = list_name.begin();server == NULL && itt != list_name.end();itt++)
		{
			if (req.mult["Host"].size() != 0 && *itt == req.mult["Host"].front())
				server = *it;
		}
	}
	if (server == NULL)
	{
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

		req.location_match = 1;
		req.error_pages = best->get_error_pages();
		req.methods = best->get_methods();
		req.auto_index = best->get_auto_index();
		req.indexes = best->get_indexes();
		req.maxSize = best->get_client_max_body_size();
		req.cgi_ext = best->get_cgi_ext();
		
		req.loc = best;

		if (best->get_use_alias())
		{
			req.path = best->get_alias() + &path[best->get_path().size()];
		}
		else
			req.path = best->get_root() + &path[1];
	}
	return (0);
}
