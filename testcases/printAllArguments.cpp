#include <iostream>
#include <cstdlib>

int main(int argc, char ** argv) {
	std::cout << "arguments count: " << argc << std::endl;
	for (int i = 0; i < argc; i++) {
		std::cout << "@" << argv[i] << "@" << std::endl;
	}

	return EXIT_SUCCESS;
}
