#pragma once

#include <map>
#include <vector>
#include <sys/epoll.h>
#include "Server.hpp"
#include "Listener.hpp"
#include "Client.hpp"

class Client;

/**
 * File: Engine.hpp\n\n
 * Description: The Engine class is responsible for managing the server's main event loop, handling incoming connections, and coordinating the interactions between listeners and clients.\n\n
 * It uses epoll for efficient event handling and maintains a mapping of file descriptors to their corresponding listeners and clients.\n\n
 * The Engine class also provides methods for adding servers, matching incoming requests to the appropriate server configuration, and gracefully shutting down the server when needed.
 */
class Engine
{
	private:
		int _epoll_fd;
		std::vector<Server> _servers_config;

		std::map<int, Client> _clients;
		std::map<int, Listener> _listeners;

		enum FdType
		{
			FD_LISTENER,
			FD_CLIENT
		};
		std::map<int, FdType> _fd_types;

		bool _is_running;
		static const int MAX_EVENTS = 64;

	public:
		Engine();
		~Engine();
		/**
		 * @brief the server by setting up listeners based on the provided server configurations and preparing the epoll instance for event handling.\n\n
		 * This method creates sockets for each listener, binds them to the appropriate addresses and ports, and starts listening for incoming connections.\n\n
		 * It also adds the listener file descriptors to the epoll instance to monitor for events. \n\n
		 * If any step fails during initialization, the method will clean up resources and return without starting the server.
		 */
		void  init_listeners();
		/**
		 * @brief Sets up the epoll instance for monitoring file descriptors. \n\n
		 * This method creates an epoll instance and stores its file descriptor in the _epoll_fd member variable.\n\n
		 * It also checks for errors during the creation of the epoll instance and handles them appropriately. \n\n
		 * This method should be called before entering the main event loop to ensure that.
		 */
		void setup_epoll();

		/**
		 * @return A boolean value indicating whether the server is currently running and able to accept connections and process events.
		 * @brief This method checks the _is_running member variable, which is set to true when the server is started and false when it is stopped.
		 */
		bool is_running() const;


		/**
		 * @param listener_fd The file descriptor of the listener that has received a new incoming connection. \n\n
		 * @brief This method is responsible for handling new incoming connections on the specified listener file descriptor. \n\n
		 * It accepts the new connection, creates a new Client object for the accepted connection, and adds the client's file descriptor to the epoll instance for monitoring. \n\n
		 * The method also retrieves the appropriate Server configuration for the new client based on the listener it connected to and associates the client with that server. \n\n
		 * If any errors occur during the acceptance of the new connection, they will be logged, and the method will return without adding the client to the server.
		 */
		void handle_new_connection(int listener_fd);

		/**
		 * @param client_fd The file descriptor of the client connection that has data available to read.
		 * @brief Handles read events for a client connection. \n\n
		 * This method is called when a client connection has data available to read. \n\n
		 * It reads the incoming data from the client, processes the request according to the associated Server configuration, and prepares a response to be sent back to the client. \n\n
		 */
		void handle_client_read(int client_fd);

		/**
		 * @param client_fd
		 * @brief Handles write events for a client connection. \n\n
		 * This method is called when a client connection is ready to send data back to the client. \n\n
		 * It retrieves the response data that has been prepared for the client, sends it over the network, and updates the client's state accordingly. \n\n
		 * If the response has been fully sent, the method may also modify the epoll events for the client to stop monitoring for write events until there is more data to send.
		 */
		void handle_client_write(int client_fd);

		/**
		 * @param client_fd The file descriptor of the client connection that has been disconnected.
		 * @brief Handles the disconnection of a client connection. \n\n
		 * This method is called when a client connection has been closed or has encountered an error that requires it to be disconnected. \n\n
		 * It removes the client's file descriptor from the epoll instance, cleans up any associated resources, and updates the internal data structures to reflect the disconnection. \n\n
		 * If the client was associated with a Server configuration, it may also perform any necessary cleanup related to that association.
		 */
		void handle_client_disconnect(int client_fd);


		/**
		 * @param fd The file descriptor to be added, modified, or removed from the epoll instance.
		 * @param events The events to be monitored for the specified file descriptor (e.g., EPOLLIN, EPOLLOUT, EPOLLET).
		 * @brief These methods manage the registration of file descriptors with the epoll instance. \n\n
		 * The add_to_epoll method adds a new file descriptor to the epoll instance with the specified events. \n\n
		 * The modify_epoll method modifies the events being monitored for an existing file descriptor in the epoll instance. \n\n
		 * The remove_from_epoll method removes a file descriptor from the epoll instance, stopping all monitoring for that descriptor.
		 */
		void add_to_epoll(int fd, uint32_t events) const;

		/**
		 *
		 * @param fd The file descriptor for which the events are being modified in the epoll instance.
		 * @param events The new set of events to be monitored for the specified file descriptor (e.g., EPOLLIN, EPOLLOUT, EPOLLET).
		 * @brief This method modifies the events being monitored for an existing file descriptor in the epoll instance. \n\n
		 * It updates the epoll registration for the specified file descriptor to reflect the new set of events that should trigger notifications. \n\n
		 * If the modification fails, it logs an error message indicating the failure and the file descriptor involved.
		 */
		void modify_epoll(int fd, uint32_t events) const;

		/**
		 * @param fd The file descriptor to be removed from the epoll instance.
		 * @brief This method removes a file descriptor from the epoll instance, stopping all monitoring for that descriptor. \n\n
		 * It calls epoll_ctl with the EPOLL_CTL_DEL operation to remove the specified file descriptor from the epoll instance. \n\n
		 * If the removal fails, it logs an error message indicating the failure and the file descriptor involved.
		 */
		void remove_from_epoll(int fd) const;

		/**
		 * @param server The Server object to be added to the Engine's list of server configurations.
		 * @brief This method adds a new Server configuration to the Engine's internal list of server configurations. \n\n
		 * It takes a Server object as a parameter and appends it to the _servers_config vector. \n\n
		 * This allows the Engine to manage multiple server configurations and match incoming requests to the appropriate server based on the host header and other criteria defined in the Server configurations.
		 */
		void add_server(const Server& server);

		/**
		 * @param host_header The value of the Host header from an incoming HTTP request.
		 * @return A pointer to the Server object that matches the provided host header, or NULL if no match is found.
		 * @brief This method matches an incoming request to the appropriate Server configuration based on the Host header. \n\n
		 * It iterates through the list of Server configurations and compares the host header with the server names defined in each Server. \n\n
		 * If a match is found, it returns a pointer to the corresponding Server object. If no match is found, it may return a default Server configuration or NULL, depending on the implementation. \n\n
		 * This method is crucial for routing incoming requests to the correct server configuration based on the requested hostname.
		 */
		Server* match_server(const std::string& host_header);


		/**
		 * @brief Starts the main event loop of the server. \n\n
		 * This method continuously waits for events on the monitored file descriptors using epoll_wait. \n\n
		 * When an event occurs, it checks the type of the file descriptor (listener or client) and calls the appropriate handler method to process the event. \n\n
		 * The loop continues until the server is stopped, at which point it will clean up resources and exit gracefully. \n\n
		 * If any errors occur during the event loop, they will be logged, and the server will attempt to continue running if possible.
		 */
		void run();

		/**
		 * @brief Stops the server by setting the _is_running flag to false, closing all client connections, and cleaning up resources. \n\n
		 * This method iterates through all active clients and listeners, closing their respective file descriptors and clearing the associated data structures. \n\n
		 * After calling this method, the server will no longer accept new connections or process events, and the main event loop will exit gracefully. \n\n
		 * This method should be called when the server needs to be shut down, either due to an error or as part of a planned shutdown process.
		 */
		void stop();
};