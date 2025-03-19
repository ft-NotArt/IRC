/* Includes */

#include "Server.hpp"


/* Messages */

void	Server::MSG_INVITE(const User *client, const User *invited, const Channel &channel) {
	std::string msg(":") ;

	msg += client->getNickname() ;

	msg += " INVITE " ;

	msg += invited->getNickname() ;
	msg += " " ;

	msg += channel.getName() ;

	this->sendMsg(invited->getFd(), msg) ;
}

void	Server::MSG_PONG(const User *client, const std::string &token) {
	std::string msg(":") ;

	msg += client->getNickname() ;
	msg += " " ;

	msg += SERVER_NAME ;
	msg += " " ;

	msg += token ;

	this->sendMsg(client->getFd(), msg) ;
}
