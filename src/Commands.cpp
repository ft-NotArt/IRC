#include "Server.hpp"

void	Server::QUIT(const User *client, const std::string &reason, bool requested) {
	this->MSG_ERROR(client, reason) ;

	this->closeClient(client->getFd()) ;

	(void) requested ;
	// TODO send quit msg to every channels
}

void	Server::JOIN(const User *client, const std::string &channel, const std::string &key) {
	Channel *chan = this->getChannel(channel) ;
	if (!chan) { // Creating the channel (making the client operator)
		Channel *newChan = new Channel(channel, *this, client) ;
		this->channels.insert(std::pair<std::string, Channel *>(channel, newChan)) ;
	} else {
		chan->join(client, key) ;
	}
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

void Server::MODE(const User *client, const std::string &channel, const std::vector<std::string> &modesArgs)
{
	if (!this->getChannel(channel))
	{
		throw IrcException::NoSuchNick(channel);
	}


	(void) client;
	(void) channel;
	(void) modesArgs;
}

void Server::TOPIC(const User *client, const std::string &channel, const std::string &topic, bool modify) {
	Channel *chan = this->getChannel(channel) ;
	if (!chan)
		throw IrcException::NoSuchChannel(channel) ;
	
	if (modify) {
		chan->changeTopic(client, topic) ;
	} else {
		chan->sendTopic(client) ;
	}
}

void Server::PRIVMSG(const User *client, const std::vector<std::string> targets, std::string text) {
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
