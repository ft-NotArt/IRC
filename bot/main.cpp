#include "Bot.hpp"

int main(int argc, char *argv[]) {
	if (argc < 3 || argc > 4) {
		std::cerr << "Usage: " << argv[0] << " <password> <port> [ip]" << std::endl ;
		return EXIT_FAILURE ;
	}

	const std::string ip = argc == 4 ? argv[3] : "127.0.0.1" ;

	Bot bot;
	try {
		bot.start() ;
		bot.connect(argv[1], std::atoi(argv[2]), ip) ;
		bot.run() ;
	} catch (std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl ;
		return EXIT_FAILURE ;
	}

	return EXIT_SUCCESS ;
}
