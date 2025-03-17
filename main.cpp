#define MAX_CONNECTIONS 5
#define BUFFER_SIZE 1024
#define PORT 4016
#define TIMEOUT 30000

#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <netinet/in.h>

int main(void) {
	int servSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (servSocket < 0) {
		std::cerr << "socket error: " << strerror(errno) << std::endl;
		return EXIT_FAILURE;
	}

	struct sockaddr_in servAddr;
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = INADDR_ANY;
	servAddr.sin_port = htons(PORT);


	if (bind(servSocket, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
		std::cerr << "bind error: " << strerror(errno) << std::endl;
		return EXIT_FAILURE;
	}

	if (listen(servSocket, MAX_CONNECTIONS) < 0) {
		std::cerr << "listen error: " << strerror(errno) << std::endl;
		return EXIT_FAILURE;
	}

	int epollFd = epoll_create1(0);
	if (epollFd < 0) {
		std::cerr << "epoll_create error: " << strerror(errno) << std::endl;
		return EXIT_FAILURE;
	}

	epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = servSocket;

	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, servSocket, &event) < 0) {
		std::cerr << "epoll_ctl error: " << strerror(errno) << std::endl;
		return EXIT_FAILURE;
	}

	while (true) {
		std::cout << "Waiting for events..." << std::endl;

		int numEvents = epoll_wait(epollFd, &event, 1, TIMEOUT);
		if (numEvents < 0) {
			std::cerr << "epoll_wait error: " << strerror(errno) << std::endl;
			return EXIT_FAILURE;
		}
		if (numEvents == 0) {
			std::cout << "No events within timeout period" << std::endl;
			continue;
		}

		// std::cout << "Event received from fd " << event.data.fd << std::endl;
		if (event.data.fd == servSocket) {
			int clientSocket = accept(servSocket, NULL, NULL);
			if (clientSocket < 0) {
				std::cerr << "accept error: " << strerror(errno) << std::endl;
				continue;
			}
			std::cout << "Accepted new client: " << clientSocket << std::endl;

			epoll_event clientEvent;
			clientEvent.events = EPOLLIN;
			clientEvent.data.fd = clientSocket;
			if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &clientEvent) < 0) {
				std::cerr << "epoll_ctl (client) error: " << strerror(errno) << std::endl;
				close(clientSocket);
				continue;
			}
		} else if (event.events == EPOLLOUT) {
			if (send(event.data.fd, "CAP * LS", 8, 0) < 0) {
				std::cerr << "send error: " << strerror(errno) << std::endl;
				epoll_ctl(epollFd, EPOLL_CTL_DEL, event.data.fd, NULL);
				close(event.data.fd);
				continue;
			}
		} else {
			char buff[BUFFER_SIZE];
			int readBytes = recv(event.data.fd, buff, sizeof(buff) - 1, 0);
			if (readBytes < 0) {
				std::cerr << "read error on fd " << event.data.fd << ": " << strerror(errno) << std::endl;
				epoll_ctl(epollFd, EPOLL_CTL_DEL, event.data.fd, NULL);
				close(event.data.fd);
				continue;
			}
			if (readBytes == 0) {
				std::cout << "Client " << event.data.fd << " disconnected." << std::endl;
				epoll_ctl(epollFd, EPOLL_CTL_DEL, event.data.fd, NULL);
				close(event.data.fd);
				continue;
			}

			buff[readBytes] = '\0';
			std::cout << "Received from client " << event.data.fd << ": " << buff << std::endl;

		}


	}

	epoll_ctl(epollFd, EPOLL_CTL_DEL, servSocket, &event);

	close(epollFd);
	close(servSocket);

	return 0;
}
