#pragma once

#include "User.hpp"

#include <vector>

// TODO Have to add required parameters in here, depends on how we are gonna implement it !
class Command{
	public:
		static void		QUIT(const User *client, std::vector<std::string> args);
		static void		PRIVMSG(const User *client, std::vector<std::string> args);
		static void		INVITE(const User *client, std::vector<std::string> args);
		static void		KICK(const User *client, std::vector<std::string> args);
		static void		MODE(const User *client, std::vector<std::string> args);
		static void		TOPIC(const User *client, std::vector<std::string> args);
};
