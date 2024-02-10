#include "lexer.hpp"
#include "ll1_parser.hpp"
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

int main() {
    std::cout << "\nInput:" << std::endl;
    std::ifstream file;
    file.open("text.txt");
    std::string line;
    while (getline(file, line)) {
        std::cout << line << std::endl;
    }
    file.close();

    LL1Parser parser("input.txt", "text.txt");

    if (parser.parse()) {
        std::cout << "Parsing was successful";
    } else {
        std::cerr << "Parsing encountered an error.";
    }

    std::cout << std::endl;
}
