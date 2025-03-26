#include "Server.hpp"
#include "Errors.hpp"
#include "Channel.hpp"
#include "Utils.hpp"

// That should looks like this
void Server::QUIT(const User *client, const std::string &args) {
	(void) client ;

	std::string reason(trim(args)) ;

	// TODO quit every channels
}

// void Server::INVITE(const User *client, std::vector<std::string> args)
// {
// 	if (args.size() < 2)
// 	{
// 		throw(IrcException::NeedMoreParams());
// 	}

// 	std::string target = args.at(0);
// 	std::string channel = args.at(1);

// 	if (channels.find(channel) == channels.end())
// 	{
// 		throw(IrcException::NoSuchChannel());
// 	}

// 	// Check if client is in the channel or not ?

// 	// Find target fd by name and let know the server if target is already in channel and if user does exist maybe in the order
// }

// void Server::MODE(const User *client, std::vector<std::string> args)
// {

// }

// void Server::TOPIC(const User *client, std::vector<std::string> args)
// {
// 	if (args.empty())
// 	{
// 		throw(IrcException::NeedMoreParams());
// 	}

// 	std::string channelName = args.at(0);
// 	Channel *channel = getChannel(channelName);

// 	if (!channel) // That's mean channel is not found
// 	{
// 		throw(IrcException::NoSuchChannel());
// 	}

// 	if (args.size() == 1) // That's means we have no TOPIC so we are just retrieving the TOPIC of the channel
// 	{
// 		if (channel->getTopic().empty())
// 		{
// 			RPL_NOTOPIC(client, *channel);
// 		}
// 		else
// 		{
// 			RPL_TOPIC(client, *channel);
// 		}
// 		return ;
// 	}

// 	if (channel->getTopicRestriction()) // + Check if user is not operator
// 	{
// 		throw(IrcException::ChanoPrivNeeded());
// 	}

// 	channel->setTopic(args.at(0));
// 	// maybe inform all users of channels that TOPIC has been changed

// }

// void Server::PRIVMSG(const User *client, std::vector<std::string> args)
// {
// 	if (args.empty())
// 	{
// 		throw(IrcException::NeedMoreParams());
// 	}

// 	if (args.size() > 2)
// 	{
// 		// maybe handle for too many params
// 	}


// }
