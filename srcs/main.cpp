#include "Parsing.hpp"
#include "Color.hpp"
#include "httpRequest.hpp"
#include <sstream>
#include <map>
#include "Engine.hpp"
#include <string.h>




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
	// std::cout << servers.front() << std::endl;
	Engine engine;
	for (std::vector<Server>::iterator it = servers.begin(); it != servers.end();it++)
		engine.add_server(*it);
	engine.init_listeners();
	engine.setup_epoll();
	signal(SIGINT, Engine::signal_handler);
	signal(SIGTERM, Engine::signal_handler);
	engine.run();
}

