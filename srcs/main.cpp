#include <iostream>
#include <map>
#include <string>
#include "Color.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include "httpResponse.hpp"
#include "httpRequest.hpp"

int main()
{
	HttpRequest req;
	req.method = "PUSH";
	req.path = "/home/samaouch/index.html";
	req.version = "HTTP/1.1";
	req.headers["Host"] = "localhost:8080";
	req.headers["Content-Type"] = "text/html";
	req.headers["Connection"] = "close";
	req.body = "<h1>Hello World!<h1>";
	req.type = "html";
	req.auto_index = true;
	req.indexes.push_back("default_page.html");
	req.indexes.push_back("index.html");
	req.methods.push_back("GET");
	req.methods.push_back("POST");
	req.methods.push_back("DELETE");
	std::stringstream ss;
	ss << req.body.size();
	req.headers["Content-Length"] = ss.str();
	httpResponse resp;
	resp.handleResponse(req);
	std::cout << PURPLE BOLD "Display Status Code/Message" RESET << std::endl;
	std::cout << GREEN BOLD "HTTP/1.1 " RESET << resp.getStatusCode() << " " << resp.getStatusMsg() << std::endl;
	std::cout << std::endl << PURPLE BOLD "Display Headers" RESET << std::endl;
	std::map<std::string, std::string> headers = resp.getHeaders();
	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
		std::cout << (*it).first << ": " << (*it).second << std::endl;
	std::cout << std::endl << PURPLE BOLD "Display Body" RESET << std::endl;
	std::cout << resp.getBody() << std::endl;


	// int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	// struct sockaddr_in addr;
	// addr.sin_family = AF_INET;
	// addr.sin_addr.s_addr = INADDR_ANY;
	// addr.sin_port = htons(8080);
	// bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));

	// listen(server_fd, 10);
	// std::cout << YELLOW BOLD "Run Server to http://localhost:8080" RESET << std::endl;

	// while (1)
	// {
	// 	int client_fd = accept(server_fd, NULL, NULL);

	// 	char buffer[4096] = {0};
	// 	read(client_fd, buffer, sizeof(buffer));
	// 	std::cout << BLUE BOLD "Buffer: " RESET << std::endl << buffer << std::endl;
	// 	resp.handleRequest(req);
	// 	std::string response = resp.convertFinalResponse(); // call handleRequest plutot
	// 	std::cout << std::endl << PURPLE BOLD "Final Response" RESET << std::endl << response << std::endl;
	// 	write(client_fd, response.c_str(), response.size());
	// 	close(client_fd);
	// }
	return 0;
}
