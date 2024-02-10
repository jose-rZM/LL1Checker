#include "lexer.hpp"
#include "ll1_parser.hpp"
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

int main(int argc, char *argv[]) {
    std::string grammar_filename, text_filename;
    if (argc == 2) {
        grammar_filename = argv[1];
        LL1Parser ll1_p{grammar_filename};
    } else if (argc == 3) {
        grammar_filename = argv[1];
        text_filename = argv[2];
        LL1Parser ll1_p{grammar_filename, text_filename};
        std::cout << "Input:" << std::endl;
        std::ifstream file;
        file.open(text_filename);
        std::string line;
        while (getline(file, line)) {
            std::cout << line << std::endl;
        }
        file.close();

        if (ll1_p.parse()) {
            std::cout << "Parsing was successful";
        } else {
            std::cerr << "Parsing encountered an error.";
        }

    }
}
