#include "Bot.hpp"

extern volatile bool running;

Bot::Bot() : fd(-1) {}

Bot::~Bot() {
	for (size_t i = 0; i < this->badWordsRegex.size(); ++i) {
		regfree(this->badWordsRegex[i]);
		delete this->badWordsRegex[i];
	}
	if (this->fd < 0)
		return ;
	this->sendMsg("QUIT :Bot disconnected");
	close(this->fd);
}

void	Bot::start() {
	std::ifstream file("badwords.txt");
	if (!file)
		throw std::runtime_error("Unable to open badwords.txt");

	std::string line;
	int reti;
	while (std::getline(file, line)) {
		if (line.empty())
			continue ;
		regex_t *regex = new regex_t;
		reti = regcomp(regex, line.c_str(), REG_EXTENDED | REG_NOSUB);
		if (reti) {
			delete regex;
			continue;
		}

		this->badWordsRegex.push_back(regex);
	}

	file.close();
}

void	Bot::connect(const std::string &password, int port, const std::string &ip) {
	this->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->fd < 0) {
		throw std::runtime_error("Socket creation failed");
	}

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

	if (::connect(this->fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
		close(this->fd);
		throw std::runtime_error("Connection failed");
	}

	this->sendMsg("CAP LS 302");
	this->sendMsg("CAP REQ :multi-prefix");
	this->sendMsg("CAP END");
	this->sendMsg("PASS " + password);
	this->sendMsg("NICK " + std::string(BOT_NAME));
	this->sendMsg("USER " + std::string(BOT_NAME) + " 0 * :" + BOT_NAME);
}

void	Bot::run() {
	while (running) {
		this->receiveMsg();
		this->processMsg();
	}
}

bool Bot::containsBadWords(const std::string &msg) {
	for (size_t i = 0; i < this->badWordsRegex.size(); ++i) {
		if (regexec(this->badWordsRegex[i], msg.c_str(), 0, NULL, 0) == 0)
			return true;
	}
	return false;
}

void	Bot::sendMsg(std::string msg) {
	if (msg.empty())
		return ;

	std::cout << YELLOW << "[BOT->SRV] `" << msg << "`" << RESET << std::endl;
	msg += "\r\n" ;
	send(this->fd, msg.c_str(), msg.size(), 0);
}

void	Bot::receiveMsg() {
	char buff[BUFFER_SIZE];
	int readBytes = recv(this->fd, buff, sizeof(buff) - 1, 0);

	if (readBytes < 0)
		throw std::runtime_error(strerror(errno));
	if (readBytes == 0)
		return ;

	buff[readBytes] = '\0';
	this->buffer += buff;
}

void	Bot::processMsg() {
	size_t pos = this->buffer.find("\r\n");
	while (pos != std::string::npos) {
		std::stringstream ssMessage(this->buffer.substr(0, pos));
		/* DEBUG */ std::cout << BLUE << "[SRV->BOT] `" << ssMessage.str() << "`" << RESET << std::endl;
		std::string command;
		std::string origin;

		ssMessage >> origin >> command;

		origin = origin.substr(0, origin.find('!')) ;

		this->buffer.erase(0, pos + 2);
		pos = this->buffer.find("\r\n");

		if (command == "ERROR") {
			running = false;
		} else if (command == "482") {
			std::string channel;
			ssMessage >> channel >> channel;
			this->sendMsg("PRIVMSG " + channel + " :I want to ban " + this->getLastBanned() + ", but I don't have permission to do it :(");
		} else if (command == "INVITE") {
			std::string channel;
			ssMessage >> channel >> channel;

			this->sendMsg("JOIN " + channel);
		} else if (command == "PRIVMSG") {
			std::string channel;
			ssMessage >> channel;
			if (channel[0] != '#')
				return ;

			ssMessage.ignore(2);
			std::string word;
			while (ssMessage >> word) {
				if (this->containsBadWords(word)) {
					origin.erase(0, 1);
					this->setLastBanned(origin);
					this->sendMsg("MODE " + channel + " +b " + origin);
					return ;
				}
			}
		}
		ssMessage.clear();
	}
}
