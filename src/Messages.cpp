/* Includes */

#include "Server.hpp"


/* Messages */

void	Server::MSG_CAP_LS(const User *client) {
	std::string msg(":") ;

	msg += client->getNickname() ;
	msg += " " ;

	msg += "CAP * LS :multi-prefix" ;

	this->sendMsg(client->getFd(), msg) ;
}

void	Server::MSG_CAP_ACK(const User *client, const std::string &request_capa) {
	std::string msg(":") ;

	msg += client->getNickname() ;
	msg += " " ;

	msg += "CAP * ACK " ;

	msg += request_capa ;

	this->sendMsg(client->getFd(), msg) ;
}

void	Server::MSG_PONG(const User *client, const std::string &token) {
	std::string msg(":") ;

	msg += client->getNickname() ;

	msg += " PONG " ;

	msg += SERVER_NAME ;
	msg += " " ;

	msg += token ;

	this->sendMsg(client->getFd(), msg) ;
}

void	Server::MSG_INVITE(const User *client, const User *invited, const Channel &channel) {
	std::string msg(":") ;

	msg += client->getNickname() ;

	msg += " INVITE " ;

	msg += invited->getNickname() ;
	msg += " " ;

	msg += channel.getName() ;

	this->sendMsg(invited->getFd(), msg) ;
}

void	Server::MSG_ERROR(const User *client, const std::string &reason) {
	std::string msg(":") ;

	msg += client->getNickname() ;

	msg += " ERROR :" ;
	msg += reason ;

	this->sendMsg(client->getFd(), msg) ;
}
