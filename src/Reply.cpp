/* Includes */

#include "Server.hpp"


/* Replies */
// TODO: put every this->sendMsg in try catch when sendMsg will throw

void	Server::greetings(const User *user) const {
	this->RPL_WELCOME(user) ;
	this->RPL_YOURHOST(user) ;
	this->RPL_CREATED(user) ;
}

void	Server::RPL_WELCOME(const User *client) const {
	std::string rpl(":") ;

	rpl += SERVER_NAME ;
	rpl += " 001 " ;
	rpl += client->getNickname() ;

	rpl += " :Welcome to the " ;
	rpl += SERVER_NAME ;
	rpl += " Network, " ;

	rpl += client->getFullname() ;

	this->sendMsg(client->getFd(), rpl) ;
}

void	Server::RPL_YOURHOST(const User *client) const {
	std::string rpl(":") ;

	rpl += SERVER_NAME ;
	rpl += " 002 " ;
	rpl += client->getNickname() ;

	rpl += " :Your host is " ;
	rpl += SERVER_NAME ;

	rpl += ", running version " ;
	rpl += SERVER_VERSION ;

	this->sendMsg(client->getFd(), rpl) ;
}

void	Server::RPL_CREATED(const User *client) const {
	std::string rpl(":") ;

	rpl += SERVER_NAME ;
	rpl += " 003 " ;
	rpl += client->getNickname() ;

	rpl += " :This server was created " ;
	rpl += asctime(this->creationDate) ;
	rpl.erase(rpl.length() - 1) ;

	this->sendMsg(client->getFd(), rpl) ;
}

void	Server::RPL_NOTOPIC(const User *client, const Channel &channel) const {
	std::string rpl(":") ;

	rpl += SERVER_NAME ;
	rpl += " 331 " ;
	rpl += client->getNickname() ;

	rpl += " " ;
	rpl += channel.getName() ;

	rpl += " :No topic is set" ;

	this->sendMsg(client->getFd(), rpl) ;
}

// Use this reply only if a topic is set, otherwise, use RPL_NOTOPIC
void	Server::RPL_TOPIC(const User *client, const Channel &channel) const {
	std::string rpl(":") ;

	rpl += SERVER_NAME ;
	rpl += " 332 " ;
	rpl += client->getNickname() ;

	rpl += " " ;
	rpl += channel.getName() ;

	rpl += " :" ;
	rpl += channel.getTopic() ;

	this->sendMsg(client->getFd(), rpl) ;
}

// Use this reply only if a topic is set, otherwise, use RPL_NOTOPIC
void	Server::RPL_TOPICWHOTIME(const User *client, const Channel &channel) const {
	std::string rpl(":") ;

	rpl += SERVER_NAME ;
	rpl += " 333 " ;
	rpl += client->getNickname() ;

	rpl += " " ;
	rpl += channel.getName() ;

	std::pair<const User *, time_t>	topic_change = channel.getTopicChange();
	if (topic_change.first) {
		rpl += " " ;
		rpl += topic_change.first->getNickname() ;
	} else {
		rpl += " <Unknow>" ;
	}

	rpl += " " ;
	rpl += topic_change.second ;

	this->sendMsg(client->getFd(), rpl) ;
}

void	Server::RPL_INVITING(const User *client, const User *invited, const Channel &channel) const {
	std::string rpl(":") ;

	rpl += SERVER_NAME ;
	rpl += " 341 " ;
	rpl += client->getNickname() ;

	rpl += " " ;
	rpl += invited->getNickname() ;

	rpl += " " ;
	rpl += channel.getName() ;

	this->sendMsg(client->getFd(), rpl) ;
}

// We put '=' as the symbol for the type of the channel as it's the only type of channel we have
void	Server::RPL_NAMREPLY(const User *client, const Channel &channel) const {
	std::string rpl(":") ;

	rpl += SERVER_NAME ;
	rpl += " 353 " ;
	rpl += client->getNickname() ;

	rpl += " = " ;
	rpl += channel.getName() ;
	rpl += " :" ;

	rpl += channel.getUsers() ;

	this->sendMsg(client->getFd(), rpl) ;
}

void	Server::RPL_ENDOFNAMES(const User *client, const Channel &channel) const {
	std::string rpl(":") ;

	rpl += SERVER_NAME ;
	rpl += " 366 " ;
	rpl += client->getNickname() ;

	rpl += " " ;
	rpl += channel.getName() ;

	rpl += " :End of /NAMES list" ;

	this->sendMsg(client->getFd(), rpl) ;
}

// This is commonly used when using MODE command to retrieve all available modes of the channel (the mode function can take parameter for example MODE #<channel_name>)
void	Server::RPL_CHANNELMODEIS(const User *client, const std::string &channel, const std::string modes, const std::string modesArgs) const
{
	std::string rpl(":") ;

	rpl += SERVER_NAME ;
	rpl += " 324 " ;
	rpl += client->getNickname() ;

	rpl += " " ;
	rpl += channel ;

	rpl += " " ;
	rpl += modes ;
	rpl += " " ;
	rpl += modesArgs ;

	this->sendMsg(client->getFd(), rpl);
}

// This is replied when user is promoted to Operator
void	Server::RPL_YOUREOPER(const User *client) const
{
	std::string rpl(":") ;

	rpl += SERVER_NAME ;
	rpl += " 324 " ;
	rpl += client->getNickname() ;

	rpl += " :You are now an IRC operator" ;

	this->sendMsg(client->getFd(), rpl);
}
