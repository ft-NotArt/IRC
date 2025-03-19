#pragma once

#include "Channel.hpp"
#include "User.hpp"

class Channel ;
class User ;

#include <string>
#include <map>
#include <set>
#include <sys/epoll.h>

#define MAX_CONNECTIONS 5
#define MAX_EVENTS 10
#define BUFFER_SIZE 1024

// TODO: double check on the capabilities specified, not sure we have to handle every of those
#define MSG_SERV_CAP_LS		"CAP * LS :multi-prefix\r\n"
#define MSG_SERV_CAP_ACK	"CAP * ACK multi-prefix\r\n"
#define MSG_SERV_MOTD		"001 gobelin :Welcome to the Internet Relay Chat Network gobelin\r\n"
#define MSG_SERV_PONG		"PONG "

#define MSG_CLI_CAP_LS		"CAP LS"
#define MSG_CLI_CAP_END		"CAP END"
#define MSG_CLI_PASS		"PASS "
#define MSG_CLI_CAP_REQ		"CAP REQ :multi-prefix"
#define MSG_CLI_PING		"PING "

class Server {
	private:
		const std::string			password ;
		const int					port ;
		int							socket ;
		int							epollFd ;
		epoll_event					event, events[MAX_EVENTS] ;
		std::map<int, std::string>	clientBuffers ;
		std::set<Channel>			channels ;
		std::map<int, const User *>	users ;

	public:
		Server(const std::string &password, const int port) ;
		~Server(void) ;

		const std::string			&getPassword()	const	{ return this->password ; } ;
		int			 				getPort()		const	{ return this->port ; } ;
		int			 				getSocket()		const	{ return this->socket ; } ;
		int			 				getEpollFd()	const	{ return this->epollFd ; } ;
		std::set<Channel>			getChannels()	const	{ return this->channels ; } ;
		std::map<int, const User *>	getUsers()		const	{ return this->users ; } ;

		void	createSocket(void) ;
		void	createEpoll(void) ;

		void	start(void) ;
		void 	run(void) ;

		void	acceptClient() ;
		void 	sendMsg(int fd, const std::string &buffer) ;
};
