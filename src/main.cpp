#include <cstdlib>
#include <iostream>
#include <csignal>
#include "Server.hpp"

volatile bool running = true;

void signalHandler(int signal) {
	(void) signal;
	running = false;
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <password> <port>" << std::endl;
		return EXIT_FAILURE;
	}

	struct sigaction sa;
	sa.sa_handler = signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	if (sigaction(SIGINT, &sa, NULL) == -1) {
		std::cerr << "sigaction" << std::endl;
		return 1;
	}

	Server server(argv[1], std::atoi(argv[2]));

	server.start();
	server.run();

	return EXIT_SUCCESS;
}
