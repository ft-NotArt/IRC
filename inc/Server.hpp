#pragma once

#include "Channel.hpp"
#include "User.hpp"

class Channel ;
class User ;

#include <string>
#include <vector>
#include <map>
#include <set>
#include <sys/epoll.h>

#define SERVER_NAME		"Internet_Relay_Chat"

#define MAX_CONNECTIONS 5
#define MAX_EVENTS 10
#define BUFFER_SIZE 1024

#define MSG_CLI_CAP_LS		"CAP LS"
#define MSG_CLI_CAP_END		"CAP END"
#define MSG_CLI_PASS		"PASS "
#define MSG_CLI_CAP_REQ		"CAP REQ :multi-prefix"
#define MSG_CLI_PING		"PING "
#define MSG_CLI_NICK		"NICK "
#define MSG_CLI_USER		"USER "

class Server {
	private:
		const std::string			password ;
		const int					port ;
		int							socket ;
		int							epollFd ;
		epoll_event					event, events[MAX_EVENTS] ;
		std::map<int, std::string>	clientBuffers ;
		std::map<std::string, Channel>	channels ; // Shouldn't this be a std::map<std::string name, Channel> in order to know what channel already exist ?
		std::map<int, User *>		users ;

	public:
		Server(const std::string &password, const int port) ;
		~Server(void) ;

		const std::string			&getPassword()			const	{ return this->password ; } ;
		int			 				getPort()				const	{ return this->port ; } ;
		int			 				getSocket()				const	{ return this->socket ; } ;
		int			 				getEpollFd()			const	{ return this->epollFd ; } ;
		User						*getUserByFd(int fd)	const	;

		void	createSocket(void) ;
		void	createEpoll(void) ;

		void	start(void) ;
		void 	run(void) ;

		void	acceptClient() ;
		void	receiveMsg(int fd) ;
		void	processMsg(int fd) ;
		void 	sendMsg(int fd, std::string msg) const ;

		// Commands
		void	QUIT(const User *client, std::vector<std::string> args);
		void	PRIVMSG(const User *client, std::vector<std::string> args);
		void	INVITE(const User *client, std::vector<std::string> args);
		void	KICK(const User *client, std::vector<std::string> args);
		void	MODE(const User *client, std::vector<std::string> args);
		void	TOPIC(const User *client, std::vector<std::string> args);

		void	RPL_WELCOME(const User *client)																						const ;
		void	RPL_NAMREPLY(const User *client, const Channel &channel)															const ;
		void	RPL_ENDOFNAMES(const User *client, const Channel &channel)															const ;
		void	RPL_INVITING(const User *client, const User *invited, const Channel &channel)										const ;
		void	RPL_TOPIC(const User *client, const Channel &channel)																const ;
		void	RPL_NOTOPIC(const User *client, const Channel &channel)																const ;
		void	RPL_TOPICWHOTIME(const User *client, const Channel &channel)														const ;
		void	RPL_CHANNELMODEIS(const User *client, const Channel &channel, const std::string modes, const std::string params) 	const ;
		void	RPL_YOUREOPER(const User *client)																					const ;

		void	MSG_CAP_LS(const User *client) ;
		void	MSG_CAP_ACK(const User *client, const std::string &request_capa) ;
		void	MSG_PONG(const User *client, const std::string &token) ;
		void	MSG_INVITE(const User *client, const User *invited, const Channel &channel) ;
} ;
