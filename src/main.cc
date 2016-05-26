#include <cstddef>
#include <iostream>

#include "view_synthesis.h"

using namespace vs;

int main(int argc, const char* argv[]) {
	std::cout << "mf: view_syn\n" << std::endl;
	
	if(argc < 2) {
		std::cout << "must provide configuration file argument!" << std::endl;
		return EXIT_FAILURE;
	}
	
	std::cout << "setting up..." << std::endl;
	view_synthesis syn(argv[1]);
	syn.setup();
	
	std::cout << "running..." << std::endl;
	syn.run();
	
	std::cout << "finished." << std::endl;
	return EXIT_SUCCESS;
}
