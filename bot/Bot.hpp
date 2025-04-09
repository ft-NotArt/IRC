#pragma once

#include "colors.h"

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <regex.h>
#include <sstream>
#include <vector>
#include <fstream>

#define BUFFER_SIZE 128

#define BOT_NAME "B-Bot"

class Bot {
	private:
		int	fd ;
		std::string buffer;
		std::vector<regex_t *> badWordsRegex ;
		std::string lastBanned;

	public:
		Bot() ;
		~Bot() ;

		void	start() ;
		void	connect(const std::string &password, int port, const std::string &ip) ;
		void	run() ;

		void	sendMsg(std::string msg) ;
		void	receiveMsg() ;
		void	processMsg() ;

		bool	containsBadWords(const std::string &msg) ;

		std::string	&getLastBanned() { return this->lastBanned ; }
		void	setLastBanned(std::string &lastBanned) { this->lastBanned = lastBanned ; }
} ;
