#pragma once

#include "Channel.hpp"
#include "User.hpp"

class Channel ;
class User ;

#include <string>
#include <map>
#include <set>
#include <sys/epoll.h>

#define SERVER_NAME		"Internet Relay Chat"

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
		std::set<Channel>			channels ; // Shouldn't this be a std::map<std::string name, Channel> in order to know what channel already exist ?
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

		// THIS DOES WORKS ?? If yes and you guys are agree maybe this is more optimized :D
		#define RPL_WELCOME_TEST(client)							SERVER_NAME + " 001 " + client + " :Welcome " + client + " to the ft_irc network"
		#define RPL_NAMREPLY_TEST(client, channel, usersList)		SERVER_NAME + " 353 " + client + " = " + channel + " :" usersList
		#define RPL_ENDOFNAMES_TEST(client, channel)				SERVER_NAME + " 366 " + client + " " + channel + " :End of /NAMES list"
		#define RPL_QUIT_TEST(client, message)						":" + client + " Quit: " + message

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
};
