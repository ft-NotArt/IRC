/* Includes */

#include "Server.hpp"
#include "Errors.hpp"
#include "Utils.hpp"

/* Temp */
#define GREEN "\e[1;32m"
#define LIGHT_GREEN "\e[1;92m"
#define LIGHT_RED "\e[1;91m"
#define BLUE "\e[1;34m"
#define GRAY "\e[1;90m"
#define RESET "\e[0m"

#include <sstream>
static std::string debugShowInvisibleChar(const std::string& buffer) {
	std::ostringstream oss;

	oss << "`";
	for (std::size_t i = 0; i < buffer.length(); i++) {
		if (buffer[i] == '\r')
			oss << "\\r";  // Make carriage return visible
		else if (buffer[i] == '\n')
			oss << "\\n";  // Make newline visible and move to next line
		else
			oss << buffer[i];
	}
	oss << "`";
	return oss.str();
}
/* End temp */

// TODO: Move to another place
void replaceAll(std::string& str, const std::string& from, const std::string& to) {
	if (from.empty()) return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Advance position
	}
}

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
	bzero(&servAddr, sizeof(servAddr));
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

		int numEvents = epoll_wait(this->epollFd, this->events, MAX_EVENTS, -1);
		if (numEvents < 0) {
			std::cerr << "epoll_wait error: " << strerror(errno) << std::endl;
			return;
		}

		for (int i = 0; i < numEvents; i++) {

			if (this->events[i].events & (EPOLLHUP | EPOLLERR)) {
				std::cout << "Client " << this->events[i].data.fd << " disconnected unexpectedly." << std::endl;
				epoll_ctl(this->epollFd, EPOLL_CTL_DEL, this->events[i].data.fd, NULL);
				close(this->events[i].data.fd);
				this->clientBuffers.erase(this->events[i].data.fd);
				continue;
			}

			if (this->events[i].data.fd == this->socket) {
				this->acceptClient();
			} else {
				this->receiveMsg(this->events[i].data.fd);
				this->processMsg(this->events[i].data.fd);
			}
		}
	}
}

void Server::acceptClient() {
	int clientSocket ;
	while ((clientSocket = accept(this->socket, NULL, NULL)) > 0) {
		std::cout << "Accepted new client: " << clientSocket << std::endl;
		epoll_event clientEvent;
		clientEvent.events = EPOLLIN;
		clientEvent.data.fd = clientSocket;

		if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, clientSocket, &clientEvent) < 0) {
			std::cerr << "epoll_ctl (client) error: " << strerror(errno) << std::endl;
			close(clientSocket);
		}

		this->users[clientSocket] = new User(clientSocket) ;
	}

	if (clientSocket == -1 && (errno != EAGAIN && errno != EWOULDBLOCK)) {
		std::cerr << "accept error: " << strerror(errno) << std::endl;
		return;
	}
}

// TODO: Remove user from channels variables
void	Server::closeClient(int fd, const std::string &reason) {
	std::cout << "Client " << fd << " disconnected." << std::endl;

	epoll_ctl(this->epollFd, EPOLL_CTL_DEL, fd, NULL);
	close(fd);
	this->clientBuffers.erase(fd);
	this->users.erase(fd) ;
}

// TODO: Add throw and disconnect client if necessary
// Diconnect are done in closeClient but maybe we should throw
void	Server::receiveMsg(int fd) {
	char buff[BUFFER_SIZE];
	int readBytes = recv(fd, buff, sizeof(buff) - 1, 0);

	if (readBytes < 0) {
		std::cerr << "read error on fd " << fd << ": " << strerror(errno) << std::endl;
		this->QUIT(this->getUserByFd(fd), "read error", false);
		return;
	}
	if (readBytes == 0) {
		this->QUIT(this->getUserByFd(fd), "test", false); // FIXME: Why should we close here ? Prolly will be handled by QUIT
		return;
	}

	buff[readBytes] = '\0';
	/* DEBUG */ std::cout << GRAY << "[CLI[" << fd << "]->SRV/RAW] " << debugShowInvisibleChar(buff) << "\e[0m" << std::endl;
	this->clientBuffers[fd] += buff;
}

void	Server::processMsg(int fd) {
	size_t pos = this->clientBuffers[fd].find("\r\n");
	while (pos != std::string::npos) {
		std::string message = this->clientBuffers[fd].substr(0, pos);
		/* DEBUG */ std::cout << GREEN << "[CLI[" << fd << "]->SRV] " << debugShowInvisibleChar(message) << "\e[0m" << std::endl;

		this->clientBuffers[fd].erase(0, pos + 2);
		pos = this->clientBuffers[fd].find("\r\n");

		User *user = this->getUserByFd(fd) ;
		if (!user)
			continue ;

		try {
			if (message.compare(0, std::strlen(MSG_CLI_CAP_LS), MSG_CLI_CAP_LS) == 0) {
				this->MSG_CAP_LS(user) ;
			}
			else if (message.compare(0, std::strlen(MSG_CLI_CAP_REQ), MSG_CLI_CAP_REQ) == 0) {
				this->MSG_CAP_ACK(user, "multi-prefix") ; // TODO: Refactor for better implementation
			}
			else if (message.compare(0, std::strlen(MSG_CLI_CAP_END), MSG_CLI_CAP_END) == 0) {
				user->setRequestCap(true) ;
				if (!user->getUsername().empty())
					this->RPL_WELCOME(this->getUserByFd(fd));
			}
			else if (std::strncmp(message.c_str(), MSG_CLI_PASS, std::strlen(MSG_CLI_PASS)) == 0) {
				user->setPassword(message.substr(std::strlen(MSG_CLI_PASS)));
			}
			else if (message.compare(0, std::strlen(MSG_CLI_NICK), MSG_CLI_NICK) == 0) {
				user->setNickname(message.substr(std::strlen(MSG_CLI_NICK)));
			}
			else if (message.compare(0, std::strlen(MSG_CLI_USER), MSG_CLI_USER) == 0) {
				user->setUsername(message.substr(std::strlen(MSG_CLI_USER), message.find(' ', std::strlen(MSG_CLI_USER))));

				if (user->getPassword() != this->password)
					throw IrcException::PasswdMismatch() ;

				/* DEBUG */ std::cout << LIGHT_GREEN << "[DBUG|CLI[" << fd << "]] Client " << fd << " authenticated successfully." << "\e[0m" << std::endl;

				if (user->hasRequestCap())
					this->RPL_WELCOME(this->getUserByFd(fd));
			}
			// *** COMMANDS *** // // TODO : Uncomment when implemented
			else if (std::strncmp(message.c_str(), MSG_CLI_PING, std::strlen(MSG_CLI_PING)) == 0) {
				this->MSG_PONG(user, message.substr(std::strlen(MSG_CLI_PING))) ;
			}
			else if (std::strncmp(message.c_str(), MSG_CLI_QUIT, std::strlen(MSG_CLI_QUIT)) == 0) {
				std::size_t colon_pos = message.find(':') ;
			
				std::string reason ;
				if (colon_pos == std::string::npos)
					reason = "no reason" ;
				else
					reason = trim(message.substr(colon_pos + 1)) ;
				if (reason.empty())
					reason = "no reason" ;

				this->QUIT(user, reason, true) ;
			}
			else if (std::strncmp(message.c_str(), MSG_CLI_JOIN, std::strlen(MSG_CLI_JOIN)) == 0) {
				// this->JOIN(user, message.substr(std::strlen(MSG_CLI_JOIN))) ;
			}
			else if (std::strncmp(message.c_str(), MSG_CLI_PART, std::strlen(MSG_CLI_PART)) == 0) {
				// this->PART(user, message.substr(std::strlen(MSG_CLI_PART))) ;
			}
			else if (std::strncmp(message.c_str(), MSG_CLI_PRIVMSG, std::strlen(MSG_CLI_PRIVMSG)) == 0) {
				// this->PRIVMSG(user, message.substr(std::strlen(MSG_CLI_PRIVMSG))) ;
			}
			else if (std::strncmp(message.c_str(), MSG_CLI_TOPIC, std::strlen(MSG_CLI_TOPIC)) == 0) {
				// this->TOPIC(user, message.substr(std::strlen(MSG_CLI_TOPIC))) ;
			}
			else if (std::strncmp(message.c_str(), MSG_CLI_KICK, std::strlen(MSG_CLI_KICK)) == 0) {
				// this->KICK(user, message.substr(std::strlen(MSG_CLI_KICK))x) ;
			}
			else if (std::strncmp(message.c_str(), MSG_CLI_MODE, std::strlen(MSG_CLI_MODE)) == 0) {
				// this->MODE(user, message.substr(std::strlen(MSG_CLI_MODE))) ;
			}
			else if (std::strncmp(message.c_str(), MSG_CLI_INVITE, std::strlen(MSG_CLI_INVITE)) == 0) {
				// this->INVITE(user, message.substr(std::strlen(MSG_CLI_INVITE))) ;
			}
			else if (std::strncmp(message.c_str(), MSG_CLI_NICK, std::strlen(MSG_CLI_NICK)) == 0) {
				// this->NICK(user, message.substr(std::strlen(MSG_CLI_NICK))) ;
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
	}
}

void	Server::sendMsg(int fd, std::string msg) const {
	if (msg.empty())
		return ;

	msg += "\r\n" ;

	std::cout << BLUE << "[SRV->CLI[" << fd << "]] " << debugShowInvisibleChar(msg) << "\e[0m" << std::endl;
	if (send(fd, msg.c_str(), msg.size(), 0) < 0) {
		std::cerr << "send error: " << strerror(errno) << std::endl;
		// TODO: Change this to better exception
		throw std::exception() ;
	}
}

/* Getters */

User *Server::getUserByFd(int fd) const {
	try {
		return this->users.at(fd) ;
	} catch(const std::exception& e) {
		return NULL ;
	}
}

Channel *Server::getChannel(const std::string &channelName)
{
	std::map<std::string, Channel>::iterator it = channels.find(channelName);

	if (it != channels.end())
	{
		return &(it->second);
	}
	return NULL;
}
