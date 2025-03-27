#include "Server.hpp"

void	Server::QUIT(const User *client, const std::string &reason, bool requested) {
	this->MSG_ERROR(client, reason) ;

	this->closeClient(client->getFd()) ;

	(void) requested ;
	// TODO send quit msg to every channels
}

void	Server::JOIN(const User *client, const std::map<std::string, std::string> args) {

	for (std::map<std::string, std::string>::const_iterator it = args.begin(); it != args.end(); it++) {
		if (!this->getChannel((*it).first))
			throw IrcException::NoSuchChannel((*it).first) ;
	}

	(void) client ;
	(void) args ;
	// TODO: channel.join or create it
}

// void Server::INVITE(const User *client, const std::vector<std::string> args)
// {
// 	(void) client;
// 	(void) args ;

	// std::string target = args.at(0);
	// std::string channel = args.at(1);

	// if (channels.find(channel) == channels.end())
	// {
	// 	throw(IrcException::NoSuchChannel());
	// }

	// Check if client is in the channel or not ?

	// Find target fd by name and let know the server if target is already in channel and if user does exist maybe in the order
// }

// void Server::MODE(const User *client, const std::vector<std::string> args)
// {
// 	(void) client;
// 	(void) args;
// }

// void Server::TOPIC(const User *client, const std::vector<std::string> args)
// {
// 	(void) client;
// 	(void) topic;
	// if (args.empty())
	// {
	// 	throw(IrcException::NeedMoreParams());
	// }

	// std::string channelName = args.at(0);
	// Channel *channel = getChannel(channelName);

	// if (!channel) // Tat's mean channel is not found
	// {
	// 	throw(IrcException::NoSuchChannel());
	// }

	// if (args.size() == 1) // That's means we have no TOPIC so we are just retrieving the TOPIC of the channel
	// {
	// 	if (channel->getTopic().empty())
	// 	{
	// 		RPL_NOTOPIC(client, *channel);
	// 	}
	// 	else
	// 	{
	// 		RPL_TOPIC(client, *channel);
	// 	}
	// 	return ;
	// }

	// if (channel->getTopicRestriction()) // + Check if user is not operator
	// {
	// 	throw(IrcException::ChanoPrivNeeded());
	// }

	// channel->setTopic(args.at(0));
	// // maybe inform all users of channels that TOPIC has been changed

// }

void Server::PRIVMSG(const User *client, const std::vector<std::string> targets, std::string text)
{
	for (std::vector<std::string>::const_iterator it = targets.begin(); it != targets.end(); it++) {
		if ((*it)[0] == '#') {
			if (!this->getChannel(*it))
				throw IrcException::NoSuchChannel(*it) ;
		} else {
			if (!this->getUser(*it))
				throw IrcException::NoSuchNick(*it) ;
		}
	}

	for (std::vector<std::string>::const_iterator it = targets.begin(); it != targets.end(); it++) {
		if ((*it)[0] == '#') {
			try {
				this->getChannel(*it)->sendMsg(client, text) ;
			} catch(const std::exception& e) {
				throw e ;
			}
		} else {
			this->sendMsg(this->getUser(*it)->getFd(), text) ;
		}
	}
}
