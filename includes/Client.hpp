//
// Created by tjooris on 2/16/26.
//

#pragma once

#include <stdio.h>

#include "Server.hpp"
#include <string>
#include <sys/time.h>

class Server;

/**
 * File: Client.hpp\n\n
 * Description: The Client class represents a connected client to the server. \n\n
 * It manages the client's socket file descriptor, read and write buffers, and tracks the last active time for timeout purposes. \n\n
 * The Client class provides methods for reading from and writing to the client's socket, updating the last active time, checking for timeouts, and determining whether the client should continue to be processed or disconnected.
 */
class Client
{
	public:
	enum Status
	{
		READING,
		WRITING,
		PROCESSING,
		CLOSED
	};
	private:
		int	_fd;
		Server	*_server;

		std::string	_read_buffer;
		std::string	_write_buffer;

		struct timeval	_last_active_time;

		Status _status;
	public:
		Client();
		Client(int fd, Server *server);
		~Client();

		/**
		 * @return The number of bytes read from the socket, or -1 if an error occurred.
		 * @brief This method reads data from the client's socket and appends it to the read buffer. \n\n
		 * It uses the recv system call to read data from the socket, handling any potential errors that may occur during the read operation. \n\n
		 * If the read operation is successful, it updates the last active time for the client and returns the number of bytes read. \n\n
		 * If an error occurs, it returns -1 to indicate the failure.
		 */
		ssize_t	read_from_socket();

		/**
		 * @return The number of bytes written to the socket, or -1 if an error occurred.
		 * @brief This method writes data from the write buffer to the client's socket. \n\n
		 * It uses the send system call to write data to the socket, handling any potential errors that may occur during the write operation. \n\n
		 * If the write operation is successful, it updates the last active time for the client and returns the number of bytes written. \n\n
		 * If an error occurs, it returns -1 to indicate the failure.
		 */
		ssize_t	write_to_socket();


		/**
		 * @brief This method updates the last active time for the client to the current time. \n\n
		 * It should be called whenever the client performs an action that indicates activity (e.g., reading from or writing to the socket) to ensure that the client's timeout is properly tracked. \n\n
		 * This method does not return any value, but it modifies the internal state of the client to reflect the updated last active time.
		 */
		void	update();

		/**
		 * @return True if the client has timed out due to inactivity, or false otherwise.
		 * @brief This method checks if the client has timed out due to inactivity. \n\n
		 * It compares the current time with the last active time of the client and determines if the difference exceeds a predefined timeout threshold. \n\n
		 * If the client has been inactive for longer than the allowed timeout period, this method returns true, indicating that the client should be considered timed out.\n\n
		 * Otherwise, it returns false, indicating that the client is still active and should not be disconnected due to inactivity.
		 */
		bool	is_timed_out() const;

		/**
		 * @return True if the client should continue to be processed, or false if it should be disconnected.
		 * @brief This method determines whether the client should continue to be processed or if it should be disconnected. \n\n
		 * It checks the client's current status and other relevant conditions (e.g., if the client has timed out) to decide whether the client is still active and should be kept alive. \n\n
		 * If the client is in a state that indicates it should be disconnected (e.g., if it has timed out or if its status is CLOSED), this method returns false. \n\n
		 * Otherwise, it returns true, indicating that the client should continue to be processed and kept alive.
		 */
		bool	should_continue() const;


		/**
		 * @return The file descriptor associated with the client's socket.
		 * @brief This method returns the file descriptor that is associated with the client's socket. \n\n
		 * The file descriptor is used for performing read and write operations on the client's socket, as well as for monitoring the socket for events (e.g., using epoll). \n\n
		 * This method does not modify any internal state of the client; it simply provides access to the file descriptor for external use.
		 */
		int get_fd() const;

		/**
		 * @return A pointer to the Server object that is associated with the client.
		 * @brief This method returns a pointer to the Server object that is associated with the client. \n\n
		 * The Server object contains the configuration and logic for handling requests from the client, and it may be used to determine how to process incoming data and generate responses. \n\n
		 * This method does not modify any internal state of the client; it simply provides access to the associated Server object for external use.
		 */
		Server* get_server() const;

		/**
		 * @param server A pointer to the Server object to be associated with the client.
		 * @brief This method sets the Server object that is associated with the client. \n\n
		 * The provided Server pointer will be stored in the client's internal state, allowing the client to reference the server's configuration and logic when processing requests. \n\n
		 * This method modifies the internal state of the client by updating the pointer to the associated Server object.
		 */
		void	set_server(Server* server);


		/**
		 * @return A reference to the string that serves as the client's read buffer, which contains data that has been read from the socket but not yet processed.
		 * @brief This method provides access to the client's read buffer, which is a string that holds data that has been read from the client's socket. \n\n
		 * The read buffer may contain partial or complete requests from the client, and it can be used by external code to process incoming data. \n\n
		 * This method returns a reference to the read buffer, allowing external code to modify its contents as needed when processing client requests.
		 */
		std::string& get_read_buffer();

		/**
		 * @return A const reference to the string that serves as the client's read buffer, which contains data that has been read from the socket but not yet processed.
		 * @brief This method provides read-only access to the client's read buffer, which is a string that holds data that has been read from the client's socket. \n\n
		 * The read buffer may contain partial or complete requests from the client, and it can be used by external code to process incoming data without modifying the buffer. \n\n
		 * This method returns a const reference to the read buffer, ensuring that external code cannot modify its contents when processing client requests.
		 */
		const std::string& get_read_buffer() const;


		/**
		 * @return A reference to the string that serves as the client's write buffer, which contains data that is prepared to be sent to the client but has not yet been written to the socket.
		 * @brief This method provides access to the client's write buffer, which is a string that holds data that is prepared to be sent back to the client. \n\n
		 * The write buffer may contain responses or other data that needs to be sent to the client, and it can be used by external code to prepare outgoing data. \n\n
		 * This method returns a reference to the write buffer, allowing external code to modify its contents as needed when preparing responses for the client.
		 */
		std::string& get_write_buffer();

		/**
		 * @return A const reference to the string that serves as the client's write buffer, which contains data that is prepared to be sent to the client but has not yet been written to the socket.
		 * @brief This method provides read-only access to the client's write buffer, which is a string that holds data that is prepared to be sent back to the client. \n\n
		 * The write buffer may contain responses or other data that needs to be sent to the client, and it can be used by external code to prepare outgoing data without modifying the buffer. \n\n
		 * This method returns a const reference to the write buffer, ensuring that external code cannot modify its contents when preparing responses for the client.
		 */
		const std::string& get_write_buffer() const;


		/**
		 * @return The current status of the client, which indicates whether the client is currently reading, writing, processing, or closed.
		 * @brief This method returns the current status of the client, which is represented by an enum value. \n\n
		 * The status can indicate whether the client is currently in a state of reading from the socket, writing to the socket, processing a request, or if it has been closed. \n\n
		 * This method does not modify any internal state of the client; it simply provides access to the current status for external use.
		 */
		Status	get_status() const;

		/**
		 * @param new_status The new status to be set for the client, which should be one of the values defined in the Status enum (READING, WRITING, PROCESSING, CLOSED).
		 * @brief This method sets the current status of the client to the specified new status. \n\n
		 * The new_status parameter should be a valid value from the Status enum, and it will update the client's internal state to reflect the new status. \n\n
		 * This method modifies the internal state of the client by changing its current status to the provided new status.
		 */
		void	set_status(Status new_status);


		/**
		 * @param index The index of the character in the client's read buffer to be accessed.
		 * @return A reference to the character at the specified index in the client's read buffer, allowing for modification of that character.
		 * @brief This method provides access to a specific character in the client's read buffer based on the provided index. \n\n
		 * It returns a reference to the character at the specified index, allowing external code to modify that character if needed when processing client requests. \n\n
		 * If the index is out of bounds for the read buffer, this method may throw an exception or result in undefined behavior, so it should be used with caution.
		 */
		char& operator[](size_t index);

		/**
		 * @param index The index of the character in the client's read buffer to be accessed.
		 * @return A const reference to the character at the specified index in the client's read buffer, providing read-only access to that character.
		 * @brief This method provides read-only access to a specific character in the client's read buffer based on the provided index. \n\n
		 * It returns a const reference to the character at the specified index, ensuring that external code cannot modify that character when processing client requests. \n\n
		 * If the index is out of bounds for the read buffer, this method may throw an exception or result in undefined behavior, so it should be used with caution.
		 */
		const char& operator[](size_t index) const;

		/**
		 * @return True if the client's socket file descriptor is valid (i.e., not -1), or false otherwise.
		 * @brief This method checks whether the client's socket file descriptor is valid. \n\n
		 * A valid file descriptor indicates that the client has an active socket connection that can be used for reading and writing data. \n\n
		 * If the file descriptor is -1, it typically indicates that the client has been closed or that there was an error in creating the socket, and therefore this method would return false. \n\n
		 * Otherwise, if the file descriptor is a non-negative integer, this method returns true, indicating that the client's socket is valid and can be used for communication.
		 */
		bool is_valid() const;

		/**
		 * @brief Closes the client's socket connection and updates the client's status to CLOSED. \n\n
		 * This method should be called when the client is no longer active or when it needs to be disconnected. \n\n
		 * It will close the socket associated with the client, set the file descriptor to -1, and update the client's status to indicate that it has been closed. \n\n
		 * After calling this method, the client should be considered inactive and should not be processed further for reading or writing until it is reinitialized with a new socket connection.
		 */
		void close();

		/**
		 * @brief This method sets the client's socket to non-blocking mode. \n\n
		 * Non-blocking mode allows the server to perform read and write operations on the client's socket without blocking the execution of the server, which is essential for handling multiple clients concurrently. \n\n
		 * This method uses the fcntl system call to modify the file descriptor flags for the client's socket, adding the O_NONBLOCK flag to enable non-blocking behavior. \n\n
		 */
		void set_non_blocking();
};
