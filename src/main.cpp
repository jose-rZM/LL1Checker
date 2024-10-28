#include <fstream>
#include <getopt.h>
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
    return 0;
}

void show_usage(const char* program_name) {
    std::cout << "Usage: " << program_name
              << " <grammar_filename> [<text_filename>] [-v] [-h]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h             Show this help message" << std::endl;
    std::cout << "  -v             Enable verbose mode: print ll1 table, input "
                 "file and parser stack trace"
              << std::endl;
    std::cout << "  <grammar_filename>  Path to the grammar file" << std::endl;
    std::cout
        << "  <text_filename>     Path to the text file to be parsed (optional)"
        << std::endl;
}

int main(int argc, char* argv[]) {
    std::string grammar_filename, text_filename;
    bool        verbose_mode = false;

    int opt;
    while ((opt = getopt(argc, argv, "hv")) != -1) {
        switch (opt) {
        case 'h':
            show_usage(argv[0]);
            return 0;
        case 'v':
            verbose_mode = true;
            break;
        default:
            std::cerr
                << "ll1: Invalid option. Use 'll1 -h' for usage information."
                << std::endl;
            return 1;
        }
    }

    if (argc - optind < 1 || argc - optind > 2) {
        std::cerr << "ll1: Incorrect number of arguments. Use 'll1 -h' for "
                     "usage information."
                  << std::endl;
        return 1;
    }

    grammar_filename = argv[optind];
    if (optind + 1 < argc) {
        text_filename = argv[optind + 1];
    }

    std::ifstream grammar_file_check(grammar_filename);
    if (!grammar_file_check) {
        std::cerr << "ll1: Grammar file '" << grammar_filename
                  << "' does not exist or cannot be opened." << std::endl;
        return 1;
    }

    try {
        LL1Parser ll1_p{grammar_filename, text_filename};
        std::cout << "Grammar is LL(1)" << std::endl;

        if (verbose_mode) {
            std::cout << "-----------------------------------------------\n";
            std::cout << "LL1 Table (Verbose Mode):" << std::endl;
            ll1_p.print_table();
            std::cout << "-----------------------------------------------\n";
            if (!text_filename.empty()) {
                std::cout << "Input (Verbose Mode):" << std::endl;
                if (print_file_to_stdout(text_filename)) {
                    std::cerr << "Error: File does not exist." << std::endl;
                    return 1;
                }
                std::cout
                    << "-----------------------------------------------\n";
            }
        }

        if (!text_filename.empty()) {
            std::ifstream file(text_filename);
            if (!file) {
                std::cerr << "ll1: File does not exist." << std::endl;
                return 1;
            }
            if (file.peek() == std::ifstream::traits_type::eof()) {
                std::cerr << "ll1: File is empty." << std::endl;
                return 1;
            }
            if (ll1_p.parse()) {
                std::cout << "Parsing was successful." << std::endl;
                if (verbose_mode) {
                    ll1_p.print_stack_trace();
                }
            } else {
                std::cerr << "Parsing encountered an error." << std::endl;
                ll1_p.print_stack_trace();
                ll1_p.print_symbol_hist();
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "ll1: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
