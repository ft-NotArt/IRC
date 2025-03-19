// This whole file could also be a part of Server.cpp, we should see how we manage that. However, it tends to be big...
// TODO: Idk if numeric values have to be at the start of those replies

/* Includes */

#include "Server.hpp"


/* Replies */
// TODO: put every this->sendMsg in try catch when sendMsg will throw

void	Server::RPL_WELCOME(const User *client) {
	std::string rpl("001 ") ;

	rpl += client->getNickname() ;

	rpl += " :Welcome to the " ;
	rpl += SERVER_NAME ;
	rpl += " Network, " ;

	rpl += client->getNickname() ;

	this->sendMsg(client->getFd(), rpl) ;
}

void	Server::RPL_NOTOPIC(const User *client, const Channel &channel) {
	std::string rpl("331 ") ;

	rpl += client->getNickname() ;

	rpl += " " ;
	rpl += channel.getName() ;

	rpl += " :No topic is set" ;

	this->sendMsg(client->getFd(), rpl) ;
}

// Use this reply only if a topic is set, otherwise, use RPL_NOTOPIC
void	Server::RPL_TOPIC(const User *client, const Channel &channel) {
	std::string rpl("332 ") ;

	rpl += client->getNickname() ;

	rpl += " " ;
	rpl += channel.getName() ;

	rpl += " :" ;
	rpl += channel.getTopic() ;

	this->sendMsg(client->getFd(), rpl) ;
}

// Use this reply only if a topic is set, otherwise, use RPL_NOTOPIC
void	Server::RPL_TOPICWHOTIME(const User *client, const Channel &channel) {
	std::string rpl("333 ") ;

	rpl += client->getNickname() ;

	rpl += " " ;
	rpl += channel.getName() ;


	std::pair<const User *, time_t>	topic_change = channel.getTopicChange();
	if (topic_change.first) {
		rpl += " " ;
		rpl += topic_change.first->getNickname() ;

		rpl += " " ;
		rpl += topic_change.second ;
	}

	this->sendMsg(client->getFd(), rpl) ;
}
