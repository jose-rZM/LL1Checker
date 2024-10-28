#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

#include "../include/ll1_parser.hpp"

int print_file_to_stdout(const std::string& filename) {
    std::ifstream file(filename);
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

void show_usage(const char* program_name) {
    std::cout << "Usage: " << program_name
              << " <grammar_filename> [<text_filename>] [--debug] [-h]"
              << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h             Show this help message" << std::endl;
    std::cout << "  --debug        Enable debug mode: print ll1 table, input "
                 "file and parser stack trace"
              << std::endl;
    std::cout << "  <grammar_filename>  Path to the grammar file" << std::endl;
    std::cout
        << "  <text_filename>     Path to the text file to be parsed (optional)"
        << std::endl;
}

int main(int argc, char* argv[]) {
    std::string grammar_filename, text_filename;
    bool        debug_mode = false;

    try {
        if (argc < 2 || argc > 4) {
            std::cerr << "Error: Incorrect number of arguments. Use 'll1 -h' "
                         "for usage information"
                      << std::endl;
            return 1;
        }

        for (int i = 1; i < argc; ++i) {
            const std::string& arg = argv[i];
            if (arg == "-h") {
                show_usage(argv[0]);
                return 0;
            } else if (arg == "--debug") {
                debug_mode = true;
            } else if (grammar_filename.empty()) {
                grammar_filename = arg;
            } else if (text_filename.empty()) {
                text_filename = arg;
            }
        }

        std::ifstream grammar_file_check(grammar_filename);
        if (!grammar_file_check) {
            std::cerr << "Error: Grammar file '" << grammar_filename
                      << "' does not exists or it can not be opened."
                      << std::endl;
            return 1;
        }

        LL1Parser ll1_p{grammar_filename, text_filename};
        std::cout << "Grammar is LL(1)" << std::endl;
        if (debug_mode) {
            std::cout << "-----------------------------------------------\n";
            std::cout << "LL1 Table (Debug Mode):" << std::endl;
            ll1_p.print_table();
            std::cout << "-----------------------------------------------\n";
            if (!text_filename.empty()) {
                std::cout << "Input (Debug Mode):" << std::endl;
                if (print_file_to_stdout(text_filename)) {
                    std::cerr << "Error: File does not exists!" << std::endl;
                    return 1;
                }
                std::cout
                    << "-----------------------------------------------\n";
            }
        }

        if (!text_filename.empty()) {
            if (ll1_p.parse()) {
                std::cout << "Parsing was successful" << std::endl;
                if (debug_mode) {
                    ll1_p.print_stack_trace();
                }
            } else {
                std::cerr << "Parsing encountered an error." << std::endl;
                ll1_p.print_stack_trace();
                ll1_p.print_symbol_hist();
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
