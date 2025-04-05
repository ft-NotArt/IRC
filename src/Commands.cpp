#include "Server.hpp"

// TODO: delete channel if nobody in anymore

void	Server::QUIT(const User *client, const std::string &reason, bool requested) {
	this->MSG_ERROR(client, reason) ;

	std::string msg(":") ;
	msg += client->getNickname() ;
	msg += " QUIT :" ;
	if (requested)
		msg += "Quit: " ;
	msg += reason ;

	for (std::map<std::string, Channel *>::iterator it = this->channels.begin(); it != this->channels.end(); it++) {
		try {
			(*it).second->leave(client, msg) ;
		} catch(const std::exception& e) {}
	}

	this->closeClient(client->getFd()) ;
	delete client ;
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

void	Server::PART(const User *client, const std::string &channel, const std::string &reason) {
	Channel *chan = this->getChannel(channel) ;
	if (!chan)
		throw IrcException::NoSuchChannel(channel) ;
	if (!chan->isUserIn(client))
		throw IrcException::NotOnChannel(channel) ;

	std::string msg(":") ;
	msg += client->getNickname() ;
	msg += " PART " ;
	msg += chan->getName() ;
	if (reason != "") {
		msg += " :" ;
		msg += reason ;
	}

	chan->leave(client, msg) ;
	this->sendMsg(client->getFd(), msg) ;
}

void	Server::INVITE(const User *client, const std::string &nickname, const std::string &channel) {
	Channel *chan = this->getChannel(channel) ;
	if (!chan)
		throw IrcException::NoSuchChannel(channel) ;
	
	User *invited = this->getUser(nickname) ;
	if (!invited)
		throw IrcException::NoSuchNick(nickname) ;
	
	std::string msg(":") ;
	msg += client->getNickname() ;
	msg += " INVITE " ;
	msg += nickname ;
	msg += " " ;
	msg += channel ;

	chan->invite(client, invited) ;
	this->RPL_INVITING(client, invited, *chan) ;
	this->sendMsg(invited->getFd(), msg) ;
}

void	Server::KICK(const User *client, const std::string &channel, const std::string &kickedUser, const std::string &comment) {
	Channel *chan = this->getChannel(channel) ;
	if (!chan)
		throw IrcException::NoSuchChannel(channel) ;

	if (!chan->isUserIn(client))
		throw IrcException::NotOnChannel(channel) ;

	User *kicked = this->getUser(kickedUser) ;
	if (!kicked)
		throw IrcException::NoSuchNick(kickedUser) ;
	
	std::string msg(":") ;
	msg += client->getNickname() ;
	msg += " KICK " ;
	msg += chan->getName() ;
	msg += " " ;
	msg += kickedUser ;
	msg += " :" ;
	msg += comment ;

	chan->kick(client, kicked, msg) ;
}

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
		std::string msg(":") ;
		msg += client->getNickname() ;
		msg += " PRIVMSG " ;
		msg += (*it) ;
		msg += " :" ;
		msg += text ;

		if ((*it)[0] == '#') {
			this->getChannel(*it)->sendMsg(client, msg) ;
		} else {
			this->sendMsg(this->getUser(*it)->getFd(), msg) ;
		}
	}
}
