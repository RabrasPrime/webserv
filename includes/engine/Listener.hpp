#pragma once

#include <vector>
#include <string>
#include <netinet/in.h>

class Server;

/**
 * File: Listener.hpp\n\n
 * Description: The Listener class is responsible for managing a listening socket on a specific host and port. \n\n
 * It handles the creation, binding, and listening of the socket, as well as accepting incoming connections. \n\n
 * The Listener class also maintains a list of associated Server objects that define the configurations for handling incoming requests on that listener.
 */
class Listener
{
private:
	int _fd;
	int _port;
	int _host;
	struct sockaddr_storage addr;
	std::vector<Server*> _servers;

	static const int BACKLOG = 128;

public:
	Listener();
	Listener(int host, int port, struct sockaddr_storage addrs);
	~Listener();

	/**
	 * @return True if the socket was successfully created, false otherwise.
	 * @brief This method creates a socket for the listener using the socket system call. \n\n
	 * It sets the appropriate socket options (e.g., SO_REUSEADDR) and configures the socket for non-blocking mode. \n\n
	 * If the socket is successfully created and configured, it returns true. If any step fails during the socket creation or configuration, it logs an error message and returns false.
	 */
	bool create_socket();

	/**
	 * @return True if the socket was successfully bound to the specified host and port, false otherwise.
	 * @brief This method binds the listener's socket to the specified host and port using the bind system call. \n\n
	 * It constructs the appropriate sockaddr structure based on the host and port, and attempts to bind the socket to that address. \n\n
	 * If the bind operation is successful, it returns true. If the bind operation fails (e.g., due to the port being in use or insufficient permissions), it logs an error message and returns false.
	 */
	bool bind_socket();

	/**
	 * @return True if the socket was successfully set to listen for incoming connections, false otherwise.
	 * @brief This method sets the listener's socket to listen for incoming connections using the listen system call. \n\n
	 * It specifies a backlog value (e.g., 128) to indicate the maximum number of pending connections that can be queued. \n\n
	 * If the listen operation is successful, it returns true and logs a message indicating that the listener is now listening on the specified port. If the listen operation fails, it logs an error message and returns false.
	 */
	bool listen_socket();

	/**
	 * @brief Closes the listener's socket if it is currently open. \n\n
	 * This method checks if the socket file descriptor is valid (i.e., not -1) and, if so, it closes the socket using the close system call and sets the file descriptor to -1 to indicate that it is no longer valid. \n\n
	 * After calling this method, the listener's socket will be closed and should not be used for accepting new connections until a new socket is created and bound.
	 */
	void close_socket();


	/**
	 * @return The file descriptor of the accepted client connection, or -1 if an error occurred.
	 * @brief This method accepts a new incoming connection on the listener's socket using the accept system call. \n\n
	 * It retrieves the client's address information and logs a message indicating the source of the new connection. \n\n
	 * If the accept operation is successful, it sets the accepted client socket to non-blocking mode and returns the file descriptor of the accepted connection. If an error occurs during acceptance (e.g., if there are no pending connections or if an error occurs), it logs an error message and returns -1.
	 */
	int accept_connection();


	/**
	 * @param server A pointer to a Server object to be added to the listener's list of associated servers.
	 * @brief This method adds a Server object to the listener's internal list of associated servers. \n\n
	 * The listener may have multiple Server configurations associated with it, and this method allows external code to add new Server objects to that list. \n\n
	 * The method takes a pointer to a Server object as a parameter and appends it to the _servers vector, allowing the listener to manage multiple server configurations and match incoming requests to the appropriate server based on criteria defined in the Server objects.
	 */
	void add_server(Server* server);

	/**
	 * @param server A pointer to a Server object to be removed from the listener's list of associated servers.
	 * @brief This method removes a Server object from the listener's internal list of associated servers. \n\n
	 * It iterates through the _servers vector to find the specified Server pointer and removes it from the list if found. \n\n
	 * If the specified Server pointer is not found in the list, the method does nothing. After calling this method, the specified Server will no longer be associated with the listener, and incoming requests will not be matched to that server configuration.
	 */
	void remove_server(Server* server);

	/**
	 * @param host_header The value of the Host header from an incoming HTTP request.
	 * @return A pointer to the Server object that matches the provided host header, or NULL if no match is found.
	 * @brief This method matches an incoming request to the appropriate Server configuration based on the Host header. \n\n
	 * It iterates through the list of Server objects associated with the listener and compares the host header with the server names defined in each Server. \n\n
	 * If a match is found (i.e., if a Server's name matches the host header), it returns a pointer to that Server object. If no match is found after checking all associated servers, it returns NULL, indicating that there is no specific server configuration for the given host header.
	 */
	Server* match_server(const std::string& host_header) const;


	/**
	 * @return The file descriptor of the listener's socket.
	 * @brief This method returns the file descriptor that is associated with the listener's socket. \n\n
	 * The file descriptor is used for accepting new connections and for monitoring the socket for events (e.g., using epoll). \n\n
	 * This method does not modify any internal state of the listener; it simply provides access to the file descriptor for external use.
	 */
	int get_fd() const;

	/**
	 * @return The port number that the listener is bound to.
	 * @brief This method returns the port number that the listener is configured to listen on. \n\n
	 * The port number is used for accepting incoming connections and is part of the listener's configuration. \n\n
	 * This method does not modify any internal state of the listener; it simply provides access to the configured port number for external use.
	 */
	int get_port() const;

	/**
	 * @return The host address that the listener is bound to, represented as an integer.
	 * @brief This method returns the host address that the listener is configured to listen on, represented as an integer. \n\n
	 * The host address is part of the listener's configuration and is used for accepting incoming connections. \n\n
	 * This method does not modify any internal state of the listener; it simply provides access to the configured host address for external use.
	 */
	int get_host() const;

	/**
	 * @return A const reference to the vector of Server pointers that are associated with the listener.
	 * @brief This method returns a const reference to the internal vector of Server pointers that are associated with the listener. \n\n
	 * The returned vector contains pointers to the Server objects that are configured to handle requests for this listener. \n\n
	 * This method does not modify any internal state of the listener; it simply provides read-only access to the list of associated servers for external use.
	 */
	const std::vector<Server*>& get_servers() const;

	/**
	 * @return True if the listener's socket is valid and ready to accept connections, false otherwise.
	 * @brief This method checks whether the listener's socket is valid (i.e., the file descriptor is not -1) and is properly configured to accept incoming connections. \n\n
	 * It may perform additional checks to ensure that the socket is in a state where it can successfully accept new connections. \n\n
	 * If the socket is valid and ready, it returns true. If the socket is invalid or not properly configured, it returns false, indicating that the listener cannot currently accept new connections.
	 */
	bool is_valid() const;

	/**
	 * @brief This method sets the listener's socket to non-blocking mode. \n\n
	 * Non-blocking mode allows the server to perform accept operations on the listener's socket without blocking the execution of the server, which is essential for handling multiple listeners and clients concurrently. \n\n
	 * This method modifies the internal state of the listener by changing the socket's file descriptor to non-blocking mode using the fcntl system call. If an error occurs while setting the socket to non-blocking mode, it logs an error message.
	 */
	void set_non_blocking();
};
