#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

#include "lexer.hpp"
#include "ll1_parser.hpp"

int print_file_to_stdout(const std::string& filename) {
    std::ifstream file;
    file.open(filename);
    if (!file) {
        return 1;
    }
    std::string line;
    while (getline(file, line)) {
        std::cout << line << std::endl;
    }
    file.close();
    return 0;
}

int main(int argc, char* argv[]) {
    std::string grammar_filename, text_filename;

    try {
        if (argc == 2) {
            grammar_filename = argv[1];
            LL1Parser ll1_p{grammar_filename};
            std::cout << "Grammar is LL1.\nLL1 Table:" << std::endl;
            ll1_p.print_table();
        } else if (argc == 3) {
            grammar_filename = argv[1];
            text_filename    = argv[2];
            LL1Parser ll1_p{grammar_filename, text_filename};
            std::cout << "Grammar is LL1.\nLL1 Table:" << std::endl;
            ll1_p.print_table();
            std::cout << "Input:" << std::endl;
            if (print_file_to_stdout(text_filename)) {
                std::cerr << "Input file does not exists!";
                return 1;
            }
            if (ll1_p.parse()) {
                std::cout << "Parsing was successful" << std::endl;
            } else {
                std::cerr << "Parsing encountered an error." << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}
