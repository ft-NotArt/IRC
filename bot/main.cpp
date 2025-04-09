#include <csignal>
#include "Bot.hpp"

volatile bool running = true;

void signalHandler(int signal) {
	(void) signal;
	running = false;
}

int main(int argc, char *argv[]) {
	if (argc < 3 || argc > 4) {
		std::cerr << "Usage: " << argv[0] << " <password> <port> [ip]" << std::endl ;
		return EXIT_FAILURE ;
	}

	struct sigaction sa;
	sa.sa_handler = signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	if (sigaction(SIGINT, &sa, NULL) == -1) {
		std::cerr << "sigaction" << std::endl;
		return EXIT_FAILURE;
	}

	const std::string ip = argc == 4 ? argv[3] : "127.0.0.1" ;

	Bot bot;
	try {
		bot.start() ;
		bot.connect(argv[1], std::atoi(argv[2]), ip) ;
		bot.run() ;
	} catch (std::exception &e) {
		std::cerr << BOLD_RED << std::endl << e.what() << RESET << std::endl ;
		return EXIT_FAILURE ;
	}

	return EXIT_SUCCESS ;
}
