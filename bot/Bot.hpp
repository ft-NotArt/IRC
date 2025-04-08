#pragma once

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

#define GREEN "\e[1;32m"
#define BLUE "\e[1;34m"
#define GRAY "\e[1;90m"
#define CYAN "\e[1;96m"
#define RESET "\e[0m"

#define BUFFER_SIZE 128

#define BOT_NAME "B-Bot"

class Bot {
	private:
		int	fd ;
		std::string buffer;
		std::vector<std::string> badWords ;

	public:
		Bot() ;

		void	start() ;
		void	connect(const std::string &password, int port, const std::string &ip) ;
		void	run() ;
		void	disconnect() ;

		void	sendMsg(std::string msg) ;
		void	receiveMsg() ;
		void	processMsg() ;

		bool	containsBadWords(const std::string &msg) ;
} ;
