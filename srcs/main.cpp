#include "Server.hpp"
#include <fstream>
#include <sys/stat.h>
#include <sstream>




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
	int read = 0;
	while (read || std::getline(file, line))
	{
		read = 0;
		std::cout << line << std::endl;
		if (line == "server")
		{
			Server serv;
			read = serv.fill_server_config(file, line);
			servers.push_back(serv);
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