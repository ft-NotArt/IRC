#pragma once

#include "Channel.hpp"
#include "User.hpp"
#include "Errors.hpp"
#include "Utils.hpp"

class Channel ;
class User ;

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <netinet/in.h>

#define SERVER_NAME		"Internet_Relay_Chat"

#define MAX_CONNECTIONS 5
#define MAX_EVENTS 10
#define BUFFER_SIZE 1024

#define MSG_CLI_CAP_LS		"CAP LS"
#define MSG_CLI_CAP_REQ		"CAP REQ "
#define MSG_CLI_CAP_END		"CAP END"
#define MSG_CLI_PASS		"PASS "
#define MSG_CLI_NICK		"NICK "
#define MSG_CLI_USER		"USER "

// FIXME: the space at the end prevent us from detecting the msg as a cmd if no args (ex: "JOIN", no spaces)

#define MSG_CLI_PING		"PING "
#define MSG_CLI_QUIT		"QUIT "
#define MSG_CLI_JOIN		"JOIN "
#define MSG_CLI_PART		"PART "
#define MSG_CLI_PRIVMSG		"PRIVMSG "
#define MSG_CLI_TOPIC		"TOPIC "
#define MSG_CLI_KICK		"KICK "
#define MSG_CLI_MODE		"MODE "
#define MSG_CLI_INVITE		"INVITE "

class Server {
	private:
		const std::string					password ;
		const int							port ;
		int									socket ;
		int									epollFd ;
		epoll_event							event, events[MAX_EVENTS] ;
		std::map<int, std::string>			clientBuffers ;
		std::map<std::string, Channel *>	channels ;
		std::map<int, User *>				users ;

	public:
		Server(const std::string &password, const int port) ;
		~Server(void) ;

		const std::string					&getPassword()								const	{ return this->password ; } ;
		int			 						getPort()									const	{ return this->port ; } ;
		int			 						getSocket()									const	{ return this->socket ; } ;
		int			 						getEpollFd()								const	{ return this->epollFd ; } ;
		User								*getUserByFd(int fd)						const	;
		Channel								*getChannel(const std::string &channelName)			;
		User								*getUser(const std::string &userName)			;

		void	createSocket(void) ;
		void	createEpoll(void) ;

		void	start(void) ;
		void 	run(void) ;

		void	acceptClient() ;
		void	closeClient(int fd) ;
		void	receiveMsg(int fd) ;
		void	processMsg(int fd) ;
		void 	sendMsg(int fd, std::string msg) const ;

		// Commands
		void	QUIT(const User *client, const std::string &reason, bool requested);
		void 	PRIVMSG(const User *client, const std::vector<std::string> targets, std::string text);
		void 	JOIN(const User *client, const std::string &channel, const std::string &key);
		void	PART(const User *client, const std::string &channel, const std::string &reason);
		void	INVITE(const User *client, const std::vector<std::string> args);
		void	KICK(const User *client, const std::string &channel, const std::string &kickedUser, const std::string &comment);
		void	MODE(const User *client, const std::string &channel, const std::vector<std::string> &modesArgs);
		void	TOPIC(const User *client, const std::string &channel, const std::string &topic, bool modify);

		// Replies
		void	RPL_WELCOME(const User *client)																						const ;
		void	RPL_NAMREPLY(const User *client, const Channel &channel)															const ;
		void	RPL_ENDOFNAMES(const User *client, const Channel &channel)															const ;
		void	RPL_INVITING(const User *client, const User *invited, const Channel &channel)										const ;
		void	RPL_TOPIC(const User *client, const Channel &channel)																const ;
		void	RPL_NOTOPIC(const User *client, const Channel &channel)																const ;
		void	RPL_TOPICWHOTIME(const User *client, const Channel &channel)														const ;
		void	RPL_CHANNELMODEIS(const User *client, const Channel &channel, const std::string modes, const std::string params)	const ;
		void	RPL_YOUREOPER(const User *client)																					const ;

		// Messages
		void	MSG_CAP_LS(const User *client)													const ;
		void	MSG_CAP_ACK(const User *client, const std::string &request_capa)				const ;
		void	MSG_PONG(const User *client, const std::string &token)							const ;
		void	MSG_INVITE(const User *client, const User *invited, const Channel &channel)		const ;
		void	MSG_ERROR(const User *client, const std::string &reason)						const ;
		void	MSG_JOIN(const User *client, const Channel &channel)							const ;
		void	MSG_TOPIC(const User *client, const Channel &channel, const std::string &topic)	const ;
} ;
