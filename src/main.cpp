#include <cstdlib>
#include <iostream>
#include "Server.hpp"

int main(int argc, char *argv[]) {
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <password> <port>" << std::endl;
		return EXIT_FAILURE;
	}

	Server server(argv[1], std::atoi(argv[2]));

	server.start();
	server.run();

	return EXIT_SUCCESS;
}
