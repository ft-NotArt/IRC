#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 512

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 5000
#define SERVER_PASS "gobelin123"

#define BOT_NAME "feurbot"

int main() {
	std::string channel = "#general";

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		std::cerr << "Socket creation failed" << std::endl;
		return EXIT_FAILURE;
	}

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

	if (connect(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
		std::cerr << "Connection failed" << std::endl;
		close(sockfd);
		return EXIT_FAILURE;
	}

	std::string msg;
	msg = "CAP LS 302\r\n";
	send(sockfd, msg.c_str(), msg.size(), 0);
	msg = "CAP REQ :multi-prefix\r\n";
	send(sockfd, msg.c_str(), msg.size(), 0);
	msg = "CAP END\r\n";
	send(sockfd, msg.c_str(), msg.size(), 0);
	msg = "PASS " + std::string(SERVER_PASS) + "\r\n";
	send(sockfd, msg.c_str(), msg.size(), 0);
	msg = "NICK " + std::string(BOT_NAME) + "\r\n";
	send(sockfd, msg.c_str(), msg.size(), 0);
	msg = "USER " + std::string(BOT_NAME) + " 0 * :" + BOT_NAME + "\r\n";
	send(sockfd, msg.c_str(), msg.size(), 0);

	sleep(1);

	msg = "JOIN " + channel + "\r\n";
	send(sockfd, msg.c_str(), msg.size(), 0);

	char buffer[BUFFER_SIZE];
	while (true) {
		std::memset(buffer, 0, BUFFER_SIZE);
		int received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
		if (received <= 0) break;

		std::string received_msg(buffer);
		std::cout << ">> " << received_msg;

		if (received_msg.find("PING") == 0) {
			std::string ping_response = "PONG" + received_msg.substr(4);
			send(sockfd, ping_response.c_str(), ping_response.size(), 0);
		}

		if (received_msg.find("PRIVMSG") != std::string::npos &&
			received_msg.find("hello bot") != std::string::npos) {
			msg = "PRIVMSG " + channel + " :Hello from C++ bot!\r\n";
			send(sockfd, msg.c_str(), msg.size(), 0);
		}
	}

	close(sockfd);
	return 0;
}
