#define MAX_CONNECTIONS 5
#define BUFFER_SIZE 1024
#define PORT 5000
#define TIMEOUT -1
#define MAX_EVENTS 10

// TODO: double check on the capabilities specified, not sure we have to handle every of those
#define MSG_SERV_CAP_LS		"CAP * LS :multi-prefix\r\n"
#define MSG_SERV_CAP_END	"CAP END\r\n"

#define MSG_CLI_CAP_LS		"CAP LS"
#define MSG_CLI_CAP_END		"CAP END"
#define MSG_CLI_PASS		"PASS "

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <string>
#include <map>


// TODO: When passing to proper Object-oriented code, find a correct place for this
std::map<int, std::string> clientBuffers;


int main(void) {
	int servSocket = socket(AF_INET, SOCK_STREAM | O_NONBLOCK, 0);
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

	epoll_event event, events[MAX_EVENTS];
	event.events = EPOLLIN;
	event.data.fd = servSocket;

	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, servSocket, &event) < 0) {
		std::cerr << "epoll_ctl error: " << strerror(errno) << std::endl;
		return EXIT_FAILURE;
	}

	while (true) {
		std::cout << "Waiting for events..." << std::endl;

		int numEvents = epoll_wait(epollFd, events, MAX_EVENTS, TIMEOUT);
		if (numEvents < 0) {
			std::cerr << "epoll_wait error: " << strerror(errno) << std::endl;
			return EXIT_FAILURE;
		}

		// TODO: prolly remove this as it won't happen
		if (numEvents == 0) {
			std::cout << "No events within timeout period" << std::endl;
			continue;
		}

		for (int i = 0; i < numEvents; i++) {
			if (events[i].events & (EPOLLHUP | EPOLLERR)) {
				std::cout << "Client " << events[i].data.fd << " disconnected unexpectedly." << std::endl;
				epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
				close(events[i].data.fd);
				clientBuffers.erase(events[i].data.fd);
				continue;
			}
			if (events[i].data.fd == servSocket) {
				int clientSocket ;
				while ((clientSocket = accept(servSocket, NULL, NULL)) > 0) {
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
			}
			// else if (events[i].events & EPOLLOUT) {
				// if (send(events[i].data.fd, "CAP * LS", 8, 0) < 0) {
					// std::cerr << "send error: " << strerror(errno) << std::endl;
					// epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
					// close(events[i].data.fd);
					// continue;
				// }
			// }
			else {
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

				clientBuffers[events[i].data.fd] += buff;

				// TODO: I'm not sure messages are only terminated by \n, possibly also terminated by \r
				size_t pos = clientBuffers[events[i].data.fd].find("\r\n");
				if (pos != std::string::npos) {
					std::string message = clientBuffers[events[i].data.fd].substr(0, pos);
					std::cout << "Received from client " << events[i].data.fd << ": " << message << std::endl;

					// TODO: Here we should handle the message received before cleaning it

					if (std::strncmp(message.c_str(), MSG_CLI_CAP_LS, std::strlen(MSG_CLI_CAP_LS)) == 0) {
						if (send(events[i].data.fd, MSG_SERV_CAP_LS, std::strlen(MSG_SERV_CAP_LS), 0) < 0) {
							std::cerr << "send error: " << strerror(errno) << std::endl;
							epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
							close(events[i].data.fd);
							continue;
						}
					} else if (std::strncmp(message.c_str(), MSG_CLI_CAP_END, std::strlen(MSG_CLI_CAP_END)) == 0) {
						if (send(events[i].data.fd, MSG_SERV_CAP_END, std::strlen(MSG_SERV_CAP_END), 0) < 0) {
							std::cerr << "send error: " << strerror(errno) << std::endl;
							epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
							close(events[i].data.fd);
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
				}
			}
		}
	}

	epoll_ctl(epollFd, EPOLL_CTL_DEL, servSocket, &event);

	close(epollFd);
	close(servSocket);

	return 0;
}
