#include "Server.hpp"
#include <sys/stat.h>
#include <fstream>

int parse(std::vector<Server>& servers, std::string path)
{
	struct stat info;

	if (stat(path.c_str(), &info) != 0)
		return (1);
	if (info.st_mode & S_IFDIR)
		return (1);

	std::ifstream file(path.c_str());

	if (!file.is_open())
		return (1);
	std::string line;
	int read = 0;
	while (read || std::getline(file, line))
	{
		read = 0;
		if (line == "server")
		{
			Server serv;
			read = serv.fill_server_config(file, line);
			servers.push_back(serv);
		}
	}

	return (0);
}
