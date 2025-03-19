/* Includes */

#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "Server.hpp"

/* Temp */
#define GREEN "\033[1;32m"
#define BLUE "\033[1;34m"

void printBuffer(const std::string& buffer, const std::string& colorStart = "") {
	const std::string colorEnd = "\033[0m"; // Reset color

	std::cout << "|1|" << colorStart;
	for (std::size_t i = 0; i < buffer.length(); i++) {
		if (buffer[i] == '\r')
			std::cout << "\\r";  // Make carriage return visible
		else if (buffer[i] == '\n')
			std::cout << "\\n";  // Make newline visible and move to next line
		else
			std::cout << buffer[i];
	}
	std::cout << colorEnd << "|2|" << std::endl;
}

// true = successs
bool sendMsg(int fd, int epollFd, const char *msg) {
	printBuffer(msg, BLUE);
	if (send(fd, msg, std::strlen(msg), 0) < 0) {
		std::cerr << "send error: " << strerror(errno) << std::endl;
		epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL);
		close(fd);
		return false;
	}
	return true;
}
/* End temp */

/* Constructor */

Server::Server(const std::string &password, const int port) : password(password), port(port) {}

/* Destructor */

Server::~Server(void) {
	epoll_ctl(this->epollFd, EPOLL_CTL_DEL, this->socket, &(this->event));

	close(this->epollFd);
	close(this->socket);
}

/* Methods */

void Server::createSocket(void) {
	this->socket = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (this->socket < 0) {
		std::cerr << "socket error: " << strerror(errno) << std::endl;
		return;
	}

	struct sockaddr_in servAddr;
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = INADDR_ANY;
	servAddr.sin_port = htons(this->port);

	if (bind(this->socket, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
		std::cerr << "bind error: " << strerror(errno) << std::endl;
		return;
	}

	if (listen(this->socket, MAX_CONNECTIONS) < 0) {
		std::cerr << "listen error: " << strerror(errno) << std::endl;
		return;
	}
}

void Server::createEpoll(void) {
	this->epollFd = epoll_create1(0);
	if (this->epollFd < 0) {
		std::cerr << "epoll_create error: " << strerror(errno) << std::endl;
		return;
	}

	this->event.events = EPOLLIN;
	this->event.data.fd = this->socket;

	if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, this->socket, &(this->event)) < 0) {
		std::cerr << "epoll_ctl error: " << strerror(errno) << std::endl;
		return;
	}
}

void Server::start(void) {
	this->createSocket();
	this->createEpoll();
}

void Server::run(void) {
	while (true) {
		std::cout << "Waiting for events..." << std::endl;

		int numEvents = epoll_wait(epollFd, events, MAX_EVENTS, -1);
		if (numEvents < 0) {
			std::cerr << "epoll_wait error: " << strerror(errno) << std::endl;
			return;
		}

		for (int i = 0; i < numEvents; i++) {

			if (this->events[i].events & (EPOLLHUP | EPOLLERR)) {
				std::cout << "Client " << this->events[i].data.fd << " disconnected unexpectedly." << std::endl;
				epoll_ctl(epollFd, EPOLL_CTL_DEL, this->events[i].data.fd, NULL);
				close(this->events[i].data.fd);
				this->clientBuffers.erase(this->events[i].data.fd);
				continue;
			}

			if (events[i].data.fd == this->socket) {

				int clientSocket ;
				while ((clientSocket = accept(this->socket, NULL, NULL)) > 0) {
					std::cout << "Accepted new client: " << clientSocket << std::endl;
					epoll_event clientEvent;
					clientEvent.events = EPOLLIN;
					clientEvent.data.fd = clientSocket;

					if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &clientEvent) < 0) {
						std::cerr << "epoll_ctl (client) error: " << strerror(errno) << std::endl;
						close(clientSocket);
					}
				}

				if (clientSocket == -1 && (errno != EAGAIN && errno != EWOULDBLOCK)) {
					std::cerr << "accept error: " << strerror(errno) << std::endl;
				}

			} else {
				char buff[BUFFER_SIZE];
				int readBytes = recv(events[i].data.fd, buff, sizeof(buff) - 1, 0);

				if (readBytes < 0) {
					std::cerr << "read error on fd " << events[i].data.fd << ": " << strerror(errno) << std::endl;
					epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
					close(events[i].data.fd);
					clientBuffers.erase(events[i].data.fd);
					continue;
				}
				if (readBytes == 0) {
					std::cout << "Client " << events[i].data.fd << " disconnected." << std::endl;
					epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
					close(events[i].data.fd);
					clientBuffers.erase(events[i].data.fd);
					continue;
				}

				buff[readBytes] = '\0';
				printBuffer(buff, GREEN);

				clientBuffers[events[i].data.fd] += buff;

				// TODO: I'm not sure messages are only terminated by \n, possibly also terminated by \r
				size_t pos = clientBuffers[events[i].data.fd].find("\r\n");
				while (pos != std::string::npos) {
					std::string message = clientBuffers[events[i].data.fd].substr(0, pos);

					std::cout << "Received from client " << events[i].data.fd << ": " << message << std::endl;
					// printBuffer(message, BLUE);

					// TODO: Here we should handle the message received before cleaning it

					if (std::strncmp(message.c_str(), MSG_CLI_CAP_LS, std::strlen(MSG_CLI_CAP_LS)) == 0) {
						if (!sendMsg(events[i].data.fd, epollFd, MSG_SERV_CAP_LS)) {
							continue;
						}
					} else if (std::strncmp(message.c_str(), MSG_CLI_CAP_END, std::strlen(MSG_CLI_CAP_END)) == 0) {
						if (!sendMsg(events[i].data.fd, epollFd, MSG_SERV_MOTD)) {
							continue;
						}
					} else if (std::strncmp(message.c_str(), MSG_CLI_CAP_REQ, std::strlen(MSG_CLI_CAP_REQ)) == 0) {
						if (!sendMsg(events[i].data.fd, epollFd, MSG_SERV_CAP_ACK)) {
							continue;
						}
					} else if (std::strncmp(message.c_str(), MSG_CLI_PING, std::strlen(MSG_CLI_PING)) == 0) {
						std::string token = message.substr(std::strlen(MSG_CLI_PING));
						std::cout << "Token: `" << token << "`" << std::endl;

						const std::string irc = "[Internet Relay Chat] ";
						std::string pongMsg = MSG_SERV_PONG + irc + token + "\r\n";
						if (!sendMsg(events[i].data.fd, epollFd, pongMsg.c_str())) {
							continue;
						}
					} else if (std::strncmp(message.c_str(), MSG_CLI_PASS, std::strlen(MSG_CLI_PASS)) == 0) {
						// Extract the password
						std::string receivedPass = clientBuffers[events[i].data.fd].substr(
							std::strlen(MSG_CLI_PASS), pos - std::strlen(MSG_CLI_PASS)
						);

						std::string correctPass = "gobelin123";

						if (receivedPass != correctPass) {
							std::string errorMsg = "464 * :Password incorrect\r\n";
							send(events[i].data.fd, errorMsg.c_str(), errorMsg.size(), 0);
							epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
							close(events[i].data.fd);
							clientBuffers.erase(events[i].data.fd);
							continue;
						}

						std::cout << "Client " << events[i].data.fd << " authenticated successfully." << std::endl;
						// Now, wait for NICK and USER commands from the client
					}
					clientBuffers[events[i].data.fd].erase(0, pos + 2);
					pos = clientBuffers[events[i].data.fd].find("\r\n");
				}
			}
		}
	}
}

void Server::addUser(int fd, const User *user) {
	this->users[fd] = user ;
}
