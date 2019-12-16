#include <iostream>
#include <cstdlib>

int main(void) {
	int array[10];
	int b = array[100000000000000];
	std::cout << b << std::endl;

	return EXIT_SUCCESS;
}
