#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, const char* argv[]) {
	int support = 3;
    int confidence = 65;
	
	if (argc == 3) {
		support = atoi(argv[1]);
		confidence = atoi(argv[2]);	
	}

    for (std::string line; std::getline(std::cin, line);) {
        // TODO
    }
}