/* Includes */

#include "Server.hpp"

extern volatile int running ;

/* Constructor */

Server::Server(const std::string &password, const int port) : password(password), port(port) {
	time_t now = time(NULL) ;
	this->creationDate = localtime(&now) ;
}

/* Destructor */

Server::~Server(void) {
	for (std::map<int, User *>::iterator it = this->users.begin(); it != this->users.end(); ) {
		this->QUIT((it++)->second, "Server closed.", false) ;
	}

	epoll_ctl(this->epollFd, EPOLL_CTL_DEL, this->socket, &(this->event));

	close(this->epollFd);
	close(this->socket);
}

/* Methods */

void Server::createSocket(void) {
	this->socket = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (this->socket < 0) {
		std::cerr << BOLD_RED << "socket error: " << strerror(errno) << RESET << std::endl;
		return;
	}

	struct sockaddr_in servAddr;
	bzero(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = INADDR_ANY;
	servAddr.sin_port = htons(this->port);

	if (bind(this->socket, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
		std::cerr << BOLD_RED << "bind error: " << strerror(errno) << RESET << std::endl;
		return;
	}

	if (listen(this->socket, MAX_CONNECTIONS) < 0) {
		std::cerr << BOLD_RED << "listen error: " << strerror(errno) << RESET << std::endl;
		return;
	}
}

void Server::createEpoll(void) {
	this->epollFd = epoll_create1(0);
	if (this->epollFd < 0) {
		std::cerr << BOLD_RED << "epoll_create error: " << strerror(errno) << RESET << std::endl;
		return;
	}

	this->event.events = EPOLLIN;
	this->event.data.fd = this->socket;

	if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, this->socket, &(this->event)) < 0) {
		std::cerr << BOLD_RED << "epoll_ctl error: " << strerror(errno) << RESET << std::endl;
		return;
	}
}

void Server::start(void) {
	this->createSocket();
	this->createEpoll();
}

void Server::run(void) {
	while (running) {
		int numEvents = epoll_wait(this->epollFd, this->events, MAX_EVENTS, -1);
		if (numEvents < 0) {
			std::cerr << std::endl << BOLD_RED << strerror(errno) << RESET << std::endl;
			return;
		}

		for (int i = 0; i < numEvents; i++) {

			if (this->events[i].events & (EPOLLHUP | EPOLLERR)) {
				/* DEBUG */ std::cout << BOLD_RED << "[DBUG|CLI[" << this->events[i].data.fd << "]] Client " << this->events[i].data.fd  << " disconnected unexpectedly." << RESET << std::endl;
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
	struct sockaddr_in clientAddr ;
	socklen_t clientAddrLen ;

	while ((clientSocket = accept(this->socket, (struct sockaddr *)&clientAddr, &clientAddrLen)) > 0) {
		std::cout << BOLD_GREEN << "[DBUG|CLI[" << clientSocket << "]] Client " << clientSocket << " connected." << RESET << std::endl;
		epoll_event clientEvent;
		clientEvent.events = EPOLLIN;
		clientEvent.data.fd = clientSocket;

		if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, clientSocket, &clientEvent) < 0) {
			std::cerr << BOLD_RED << "epoll_ctl (client) error: " << strerror(errno) << RESET << std::endl;
			close(clientSocket);
		}

		this->users[clientSocket] = new User(clientSocket) ;
		this->users[clientSocket]->setHostname((inet_ntoa(clientAddr.sin_addr))) ;
	}

	if (clientSocket == -1 && (errno != EAGAIN && errno != EWOULDBLOCK)) {
		std::cerr << BOLD_RED << "accept error: " << strerror(errno) << RESET << std::endl;
		return;
	}
}

/***
 * @warning Check twice wether you want to call Server::QUIT(..., ..., false) or this one
 */
void	Server::closeClient(int fd) {
	std::cout << BOLD_RED << "[DBUG|CLI[" << fd << "]] Client " << fd << " disconnected." << RESET << std::endl;

	epoll_ctl(this->epollFd, EPOLL_CTL_DEL, fd, NULL);
	close(fd);
	this->clientBuffers.erase(fd);
	this->users.erase(fd) ;
}

void	Server::receiveMsg(int fd) {
	char buff[BUFFER_SIZE];
	int readBytes = recv(fd, buff, sizeof(buff) - 1, 0);

	if (readBytes < 0) {
		std::cerr << BOLD_RED << "read error on fd " << fd << ": " << strerror(errno) << RESET << std::endl ;
		this->QUIT(this->getUserByFd(fd), "read error", false) ;
		return ;
	}
	if (readBytes == 0)
		return ;

	buff[readBytes] = '\0';
	this->clientBuffers[fd] += buff;
}

void	Server::processMsg(int fd) {
	size_t pos = this->clientBuffers[fd].find("\r\n");
	while (pos != std::string::npos) {
		std::stringstream ssMessage(this->clientBuffers[fd].substr(0, pos));
		/* DEBUG */ std::cout << YELLOW << "[CLI[" << fd << "]->SRV] `" << ssMessage.str() << "`" << RESET << std::endl;
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
				else if (command == MSG_CLI_MODE)
					this->handleMODE(ssMessage, user) ;
			}
		} catch (const Server::DisconnectClient &e) {
			this->QUIT(user, e.what(), false) ;
		} catch(const std::exception& e) {
			std::string except(e.what());
			replaceAll(except, "%client%", user->getFullname());
			try {
				this->sendMsg(user->getFd(), except);
			} catch (const std::exception &ex) {}
		}

		ssMessage.clear();
	}
}

void	Server::sendMsg(int fd, std::string msg) const {
	if (msg.empty() || !this->getUserByFd(fd))
		return ;

	std::cout << BLUE << "[SRV->CLI[" << fd << "]] `" << msg << "`" << RESET << std::endl;

	msg += "\r\n" ;
	if (send(fd, msg.c_str(), msg.size(), 0) < 0) {
		std::cerr << BOLD_RED << "send error: " << strerror(errno) << RESET << std::endl;
		throw Server::DisconnectClient("send error") ;
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
	std::map<std::string, Channel *>::iterator it = this->channels.find(channelName);

	if (it != channels.end())
	{
		return it->second ;
	}
	return NULL ;
}

User *Server::getUser(const std::string &userName)
{
	for (std::map<int, User *>::iterator it = this->users.begin(); it != this->users.end(); it++) {
		if ((*it).second->getNickname() == userName)
			return (*it).second ;
	}
	return NULL;
}
