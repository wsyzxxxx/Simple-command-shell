#include <iostream>
#include <string>
#include <cstdlib>

int main(void) {
	std::string line;
	std::cout << "inputs from stdin: " << std::endl;
	while (getline(std::cin, line)) {
		std::cout << line << std::endl;
	}
	std::cout << "end of inputs! " << std::endl;

	std::cout << "This additional line should be printed to stdout!" << std::endl;
	std::cerr << "This additional line should be printed to stderr!" << std::endl;
	return EXIT_SUCCESS;
}
