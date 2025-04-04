/* Includes */

#include "Server.hpp"


/* Temp */
#define GREEN "\e[1;32m"
#define LIGHT_GREEN "\e[1;92m"
#define LIGHT_RED "\e[1;91m"
#define BLUE "\e[1;34m"
#define GRAY "\e[1;90m"
#define YELLOW "\e[1;93m"
#define CYAN "\e[1;96m"
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


/* Constructor */

Server::Server(const std::string &password, const int port) : password(password), port(port) {
	time_t now = time(NULL) ;
	this->creationDate = localtime(&now) ;
}

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
		std::cout << std::endl << CYAN "[DBUG|SERVER] Waiting for events..." << std::endl;

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

/***
 * @warning Check twice wether you want to call Server::QUIT(..., ..., false) or this one
 */
void	Server::closeClient(int fd) {
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
			if (command == "CAP") {
				std::string capName;
				ssMessage >> capName;

				if (capName == "LS") {
					this->MSG_CAP_LS(user);
				}
				else if (capName == "REQ") {
					this->MSG_CAP_ACK(user, "multi-prefix") ; // TODO: Refactor for better implementation
				}
				else if (capName == "END") {
					user->setRequestCap(true) ;
					if (!user->getUsername().empty())
						this->greetings(user);
				}
			}
			else if (command == MSG_CLI_PASS) {
				std::string password;
				ssMessage >> password;

				/* DEBUG */ std::cout << YELLOW "[DBUG|CLI[" << fd << "]] Password: `" << password << "`" << std::endl;
				user->setPassword(password) ;
			}
			else if (command == MSG_CLI_NICK) {
				std::string nick;
				ssMessage >> nick;

				/* DEBUG */ std::cout << YELLOW "[DBUG|CLI[" << fd << "]] Nick: `" << nick << "`" << std::endl;
				user->setNickname(nick) ;
			}
			else if (command == MSG_CLI_USER) {
				std::string username;
				ssMessage >> username;

				// /* DEBUG */ std::cout << YELLOW "[DBUG|CLI[" << fd << "]] User Old: `" << message.substr(std::strlen(MSG_CLI_USER) + 1, message.find(' ', std::strlen(MSG_CLI_USER) + 1)) << "`" << std::endl;
				/* DEBUG */ std::cout << YELLOW "[DBUG|CLI[" << fd << "]] User: `" << username << "`" << std::endl;
				user->setUsername(username) ;

				if (user->getPassword() != this->password)
					throw IrcException::PasswdMismatch() ;

				/* DEBUG */ std::cout << LIGHT_GREEN << "[DBUG|CLI[" << fd << "]] Client " << fd << " authenticated successfully." << "\e[0m" << std::endl;
				user->setRegistered(true) ;

				if (user->hasRequestCap())
					this->greetings(user) ;
			}
			else {
				if (!user->isRegistered())
					throw IrcException::NotRegistered() ;

				// *** COMMANDS *** // // TODO : Uncomment when implemented
				if (command == MSG_CLI_PING) {
					std::string token;
					ssMessage >> token;

					/* DEBUG */ std::cout << YELLOW "[DBUG|CLI[" << fd << "]] PING: Token: `" << token << "`" << std::endl;
					this->MSG_PONG(user, token) ;
				}
				else if (command == MSG_CLI_QUIT) {
					std::size_t colon_pos = ssMessage.str().find(':') ;

					std::string reason ;
					try {
						reason = trim(ssMessage.str().substr(colon_pos + 1));
					} catch (std::out_of_range &e) {
						reason = "no reason" ;
					}

					/* DEBUG */ std::cout << YELLOW "[DBUG|CLI[" << fd << "]] QUIT: Reason: `" << reason << "`" << std::endl;
					this->QUIT(user, reason, true) ;
				}
				else if (command == MSG_CLI_JOIN) {
					try {
						std::string tmp ;

						ssMessage >> tmp ;
						if (tmp.empty())
							throw IrcException::NeedMoreParams() ;
						std::stringstream channels(tmp) ;

						ssMessage >> tmp ;
						std::stringstream keys(tmp) ;

						std::string channel ;
						std::string key ;
						while (std::getline(channels, channel, ',') && std::getline(keys, key, ',')) {
							try {
								if (channel[0] != '#')
									throw IrcException::BadChanMask(channel) ;
								this->JOIN(user, channel, key) ;
							} catch(const std::exception& e) {
								std::string except(e.what());
								replaceAll(except, "%client%", user->getNickname()) ;
								replaceAll(except, "%command%", MSG_CLI_JOIN) ;
								try {
									this->sendMsg(fd, except) ;
								} catch (const std::exception &ex) {}
							}
							key.clear() ; // Reset to ensure we don't keep the key from before
						}
					} catch(const std::exception& e) {
						std::string except(e.what());
						replaceAll(except, "%client%", user->getNickname()) ;
						replaceAll(except, "%command%", MSG_CLI_JOIN) ;
						try {
							this->sendMsg(fd, except) ;
						} catch (const std::exception &ex) {}
					}
				}
				else if (command == MSG_CLI_PART) {
					try {
						std::string strChannels;
						std::getline(ssMessage, strChannels, ':') ;
						strChannels = trim(strChannels) ;
						if (strChannels.empty())
							throw IrcException::NeedMoreParams() ;

						std::string reason;
						std::getline(ssMessage, reason) ;
						reason = trim(reason) ;

						std::stringstream ssCchannels(strChannels) ;
						std::string channel ;
						while (std::getline(ssCchannels, channel, ',')) {
							try {
								if (channel[0] != '#')
									throw IrcException::BadChanMask(channel) ;

								this->PART(user, channel, reason) ;
							} catch(const std::exception& e) {
								std::string except(e.what());
								replaceAll(except, "%client%", user->getNickname()) ;
								replaceAll(except, "%command%", MSG_CLI_PART) ;
								try {
									this->sendMsg(fd, except) ;
								} catch (const std::exception &ex) {}
							}
						}
					} catch(const std::exception& e) {
						std::string except(e.what());
						replaceAll(except, "%client%", user->getNickname()) ;
						replaceAll(except, "%command%", MSG_CLI_PART) ;
						try {
							this->sendMsg(fd, except) ;
						} catch (const std::exception &ex) {}
					}
				}
				else if (command == MSG_CLI_PRIVMSG) {
					try {
						std::string tmp ;
						std::getline(ssMessage, tmp, ':') ;
						if (tmp.empty())
							throw IrcException::NoRecipient() ;

						std::stringstream ssTargets(trim(tmp)) ;
						std::vector<std::string> targets ;
						while (std::getline(ssTargets, tmp, ','))
							targets.push_back(tmp) ;
						if (targets.empty())
							throw IrcException::NoRecipient() ;

						/* DEBUG */ std::cout << YELLOW << "[DBUG|CLI[" << fd << "]] PRIVMSG: Targets: ";
						/* DEBUG */ for (std::vector<std::string>::iterator it = targets.begin(); it != targets.end(); it++) {
						/* DEBUG */ 	std::cout << "`" << *it << "`";
						/* DEBUG */ 	if (it != targets.end() - 1)
						/* DEBUG */ 		std::cout << ", ";
						/* DEBUG */ }
						/* DEBUG */ std::cout << RESET << std::endl;

						std::getline(ssMessage, tmp);
						tmp = trim(tmp);
						if (tmp.empty())
							throw IrcException::NoTextToSend() ;

						/* DEBUG */ std::cout << YELLOW "[DBUG|CLI[" << fd << "]] PRIVMSG: Text: `" << tmp << "`" << RESET << std::endl;

						this->PRIVMSG(user, targets, tmp) ;
					} catch(const std::exception& e) {
						std::string except(e.what());
						replaceAll(except, "%client%", user->getNickname()) ;
						replaceAll(except, "%command%", MSG_CLI_PRIVMSG) ;
						try {
							this->sendMsg(fd, except) ;
						} catch (const std::exception &ex) {}
					}
				}
				else if (command == MSG_CLI_TOPIC) {
					try {
						std::string	channel;
						std::getline(ssMessage, channel, ':') ;
						channel = trim(channel);

						if (channel.empty())
							throw IrcException::NeedMoreParams() ;
						else if (channel[0] != '#')
							throw IrcException::BadChanMask(channel) ;
						else if (!this->getChannel(channel))
							throw IrcException::NoSuchChannel(channel) ;

						std::string	topic;
						std::getline(ssMessage, topic) ;
						topic = trim(topic);

						this->TOPIC(user, channel, topic, !topic.empty()) ;
					} catch(const std::exception& e) {
						std::string except(e.what());
						replaceAll(except, "%client%", user->getNickname()) ;
						replaceAll(except, "%command%", MSG_CLI_TOPIC) ;
						try {
							this->sendMsg(fd, except) ;
						} catch (const std::exception &ex) {}
					}
				}
				else if (command == MSG_CLI_KICK) {
					try {
						std::string tmp;
						std::getline(ssMessage, tmp, ':') ;
						tmp = trim(tmp) ;

						std::stringstream ssTmp(tmp) ;
						std::string channel ;
						ssTmp >> channel ;
						if (channel.empty())
							throw IrcException::NeedMoreParams() ;

						ssTmp >> tmp ;
						if (tmp.empty())
							throw IrcException::NeedMoreParams() ;

						if (channel[0] != '#')
							throw IrcException::BadChanMask(channel) ;

						std::string comment ;
						std::getline(ssMessage, comment) ;
						comment = trim(comment) ;
						if (comment.empty())
							comment = "pas sage" ;

						ssTmp.str(tmp) ;
						while (std::getline(ssTmp, tmp, ',')) {
							try {
								this->KICK(user, channel, tmp, comment) ;
							} catch(const std::exception& e) {
								std::string except(e.what());
								replaceAll(except, "%client%", user->getNickname()) ;
								replaceAll(except, "%command%", MSG_CLI_KICK) ;
								try {
									this->sendMsg(fd, except) ;
								} catch (const std::exception &ex) {}
							}
						}
					} catch(const std::exception& e) {
						std::string except(e.what());
						replaceAll(except, "%client%", user->getNickname()) ;
						replaceAll(except, "%command%", MSG_CLI_KICK) ;
						try {
							this->sendMsg(fd, except) ;
						} catch (const std::exception &ex) {}
					}
				}
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
				else if (command == MSG_CLI_INVITE) {
					// this->INVITE(user, message.substr(std::strlen(MSG_CLI_INVITE))) ;
				}
				else if (command == MSG_CLI_NICK) {
					// this->NICK(user, message.substr(std::strlen(MSG_CLI_NICK))) ;
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

void	Server::sendMsg(int fd, std::string msg) const {
	if (msg.empty())
		return ;

	msg += "\r\n" ;

	std::cout << BLUE << "[SRV->CLI[" << fd << "]] " << debugShowInvisibleChar(msg) << "\e[0m" << std::endl;
	if (send(fd, msg.c_str(), msg.size(), 0) < 0) {
		std::cerr << "send error: " << strerror(errno) << std::endl;
		// TODO: Change this to better exception | Think to change every catch after use of sendMsg when changing the exception it throws
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
