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

// TODO: Add throw and disconnect client if necessary
// Diconnect are done in closeClient but maybe we should throw
void	Server::receiveMsg(int fd) {
	char buff[BUFFER_SIZE];
	int readBytes = recv(fd, buff, sizeof(buff) - 1, 0);

	if (readBytes < 0) {
		std::cerr << "read error on fd " << fd << ": " << strerror(errno) << std::endl ;
		this->QUIT(this->getUserByFd(fd), "read error", false) ;
		return ;
	}
	if (readBytes == 0) {
		this->QUIT(this->getUserByFd(fd), "test", false) ;
		return ;
	}

	buff[readBytes] = '\0';
	/* DEBUG */ std::cout << GRAY << "[CLI[" << fd << "]->SRV/RAW] " << debugShowInvisibleChar(buff) << "\e[0m" << std::endl;
	this->clientBuffers[fd] += buff;
}

void	Server::processMsg(int fd) {
	size_t pos = this->clientBuffers[fd].find("\r\n");
	while (pos != std::string::npos) {
		std::stringstream ssMessage(this->clientBuffers[fd].substr(0, pos));
		/* DEBUG */ std::cout << GREEN << "[CLI[" << fd << "]->SRV] " << debugShowInvisibleChar(ssMessage.str()) << "\e[0m" << std::endl;
		std::string command;

		ssMessage >> command;

		this->clientBuffers[fd].erase(0, pos + 2);
		pos = this->clientBuffers[fd].find("\r\n");

		User *user = this->getUserByFd(fd) ;
		if (!user)
			continue ;

		try {
			if (command == MSG_CLI_CAP)
				this->handleCAP(ssMessage, user) ;
			else if (command == MSG_CLI_PASS)
				this->handlePASS(ssMessage, user) ;
			else if (command == MSG_CLI_NICK)
				this->handleNICK(ssMessage, user) ;
			else if (command == MSG_CLI_USER)
				this->handleUSER(ssMessage, user) ;
			else {
				if (!user->isRegistered())
					throw IrcException::NotRegistered() ;

				if (command == MSG_CLI_PING)
					this->handlePING(ssMessage, user) ;
				else if (command == MSG_CLI_QUIT)
					this->handleQUIT(ssMessage, user) ;
				else if (command == MSG_CLI_PRIVMSG)
					this->handlePRIVMSG(ssMessage, user) ;
				else if (command == MSG_CLI_JOIN)
					this->handleJOIN(ssMessage, user) ;
				else if (command == MSG_CLI_INVITE)
					this->handleINVITE(ssMessage, user) ;
				else if (command == MSG_CLI_PART)
					this->handlePART(ssMessage, user) ;
				else if (command == MSG_CLI_TOPIC)
					this->handleTOPIC(ssMessage, user) ;
				else if (command == MSG_CLI_KICK)
					this->handleKICK(ssMessage, user) ;
				else if (command == MSG_CLI_MODE) {
					// try
				// {
				// 	//  TODO handle when /mode only is sent, Irssi is the only one sending the message
				// 	std::stringstream ss(message.substr(std::strlen(MSG_CLI_MODE)));
				// 	std::string channel;

				// 	ss >> channel; // The channel target out here
				// 	if (channel.empty())
				// 	{
				// 		throw IrcException::NeedMoreParams();
				// 	}

				// 	std::vector<std::string> modesArgs;
				// 	std::string tmp; // Fkin tmp cause we have to operate like this

				// 	while (ss >> tmp)
				// 	{
				// 		modesArgs.push_back(tmp);
				// 	}

				// 	// TODOD if !modesArgs -> Reply channel modes is 324
				// 	this->MODE(user, channel, modesArgs);

				// } catch(const std::exception& e) {
				// 	std::string except(e.what());
				// 	replaceAll(except, "%client%", user->getNickname()) ;
				// 	replaceAll(except, "%command%", MSG_CLI_MODE) ;
				}
			}
		} catch (const std::exception &e) {
			/* DEBUG */ std::cout << LIGHT_RED << "[DBUG|CLI[" << fd << "]] Exception caught: " << e.what() << "\e[0m" << std::endl;
			if (e.what()[0] == ':') {
				std::string except(e.what());
				replaceAll(except, "%client%", user->getNickname()) ;
				try {
					this->sendMsg(fd, except) ;
				} catch (const std::exception &ex) {}
			}
			// this->closeClient(fd, "jsp gros on se chie dessus") ;
		}
		ssMessage.clear();
	}
}








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

		std::string cmd;
		std::getline(received_msg, cmd, ' ');


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
