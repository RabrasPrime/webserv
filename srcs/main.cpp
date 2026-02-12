#include "Server.hpp"
#include <fstream>
#include <sys/stat.h>
#include <sstream>

int count_char(std::string line, char c)
{
	int i = 0;
	while (line[i] == c)
		i++;
	return (i);
}

void handle_server(std::ifstream& file, std::vector<Server>& servers)
{
	Server serv;
	std::string line;
	while (std::getline(file, line))
	{
		if (count_char(line, '\t') < 1)
		{
			std::cout << "end serv scope" << std::endl;
			servers.push_back(serv);
			return ;
		}
		size_t i = 0;
		while (std::isspace(line[i]))
			i++;
		std::string key;
		while (!std::isspace(line[i]) && i < line.size())
		{
			key += line[i];
			i++;
		}
		std::string value(&line[i]);
		std::cout << "	" << key << "  -->  " << value << std::endl;

	}
}

int parse(std::vector<Server>& servers, std::string path)
{
	struct stat info;

	if (stat(path.c_str(), &info) != 0)
	{
		std::cout << "Invalid config file path" << std::endl;
		return (1);
	}
	if (info.st_mode & S_IFDIR)
	{
		std::cout << "Config file path is a directory" << std::endl;
		return (1);
	}

	std::ifstream file(path.c_str());

	if (!file.is_open())
	{
		std::cout << "Failed to open file" << std::endl;
		return (1);
	}
	std::string line;
	while (std::getline(file, line))
	{
		std::cout << line << std::endl;
		if (line == "server")
		{
			handle_server(file, servers);
		}
	}

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
	return (0);
}