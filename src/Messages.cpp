/* Includes */

#include "Server.hpp"


/* Messages */

void	Server::MSG_CAP_LS(const User *client) const {
	std::string msg(":") ;

	msg += client->getFullname() ;
	msg += " " ;

	msg += "CAP * LS :" ;
	msg += SERVER_CAP ;

	this->sendMsg(client->getFd(), msg) ;
}

void	Server::MSG_CAP_ACK(const User *client, const std::string &request_capa) const {
	std::string msg(":") ;

	msg += client->getFullname() ;
	msg += " " ;

	msg += "CAP * ACK " ;

	msg += request_capa ;

	this->sendMsg(client->getFd(), msg) ;
}

void	Server::MSG_PONG(const User *client, const std::string &token) const {
	std::string msg(":") ;

	msg += client->getFullname() ;

	msg += " PONG " ;

	msg += SERVER_NAME ;
	msg += " " ;

	msg += token ;

	this->sendMsg(client->getFd(), msg) ;
}

void	Server::MSG_INVITE(const User *client, const User *invited, const Channel &channel) const {
	std::string msg(":") ;

	msg += client->getFullname() ;

	msg += " INVITE " ;

	msg += invited->getNickname() ;
	msg += " " ;

	msg += channel.getName() ;

	this->sendMsg(invited->getFd(), msg) ;
}

void	Server::MSG_ERROR(const User *client, const std::string &reason) const {
	std::string msg(":") ;

	msg += client->getFullname() ;

	msg += " ERROR :" ;
	msg += reason ;

	this->sendMsg(client->getFd(), msg) ;
}

void	Server::MSG_JOIN(const User *client, const Channel &channel) const {
	std::string msg(":") ;

	msg += client->getFullname() ;

	msg += " JOIN " ;
	msg += channel.getName() ;

	channel.sendMsg(client, msg) ;
}

void	Server::MSG_TOPIC(const User *client, const Channel &channel, const std::string &topic)	const {
	std::string msg(":") ;

	msg += client->getFullname() ;

	msg += " TOPIC " ;
	msg += channel.getName() ;

	msg += " " ;
	msg += topic ;

	channel.sendMsg(client, msg) ;
}
