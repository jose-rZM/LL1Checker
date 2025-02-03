#ifndef _WIN32
#include <getopt.h>
#endif
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

#include "../include/ll1_parser.hpp"

int PrintFileToStdout(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        return 1;
    }
    std::string line;
    while (getline(file, line)) {
        std::cout << line << "\n";
    }
    return 0;
}

void ShowUsage(const char* program_name) {
    std::cout << "Usage: " << program_name
              << " <grammar_filename> [<text_filename>] [-v] [-h]\n";
    std::cout << "Options:\n";
    std::cout << "  -h             Show this help message\n";
    std::cout << "  -v             Enable verbose mode: print ll1 table, input "
                 "file and parser stack trace\n";
    std::cout << "  <grammar_filename>  Path to the grammar file\n";
    std::cout << "  <text_filename>     Path to the text file to be parsed "
                 "(optional)\n";
}

int main(int argc, char* argv[]) {
    std::string grammar_filename, text_filename;
    bool        verbose_mode = false;

#ifndef _WIN32
    int opt;
    while ((opt = getopt(argc, argv, "hv")) != -1) {
        switch (opt) {
        case 'h':
            ShowUsage(argv[0]);
            return 0;
        case 'v':
            verbose_mode = true;
            break;
        default:
            std::cerr
                << "ll1: Invalid option. Use 'll1 -h' for usage information.\n";
            return 1;
        }
    }

    if (argc - optind < 1 || argc - optind > 2) {
        std::cerr << "ll1: Incorrect number of arguments. Use 'll1 -h' for "
                     "usage information.\n";
        return 1;
    }

    grammar_filename = argv[optind];
    if (optind + 1 < argc) {
        text_filename = argv[optind + 1];
    }
#else
    if (argc < 2) {
        show_usage(argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h") {
            show_usage(argv[0]);
            return 0;
        } else if (arg == "-v") {
            verbose_mode = true;
        } else if (grammar_filename.empty()) {
            grammar_filename = arg;
        } else {
            text_filename = arg;
        }
    }
#endif
    std::ifstream grammar_file_check(grammar_filename);
    if (!grammar_file_check) {
        std::cerr << "ll1: Grammar file '" << grammar_filename
                  << "' does not exist or cannot be opened.\n";
        return 1;
    }

    try {
        LL1Parser ll1_p{grammar_filename, text_filename};
        std::cout << "Grammar is LL(1)\n";

        if (verbose_mode) {
            std::cout << "-----------------------------------------------\n";
            std::cout << "LL1 Table (Verbose Mode):\n";
            ll1_p.PrintTable(true);
            std::cout << "-----------------------------------------------\n";
            if (!text_filename.empty()) {
                std::cout << "Input (Verbose Mode):\n";
                if (PrintFileToStdout(text_filename)) {
                    std::cerr << "Error: File does not exist.\n";
                    return 1;
                }
                std::cout
                    << "-----------------------------------------------\n";
            }
        }

        if (!text_filename.empty()) {
            std::ifstream file(text_filename);
            if (!file) {
                std::cerr << "ll1: File does not exist.\n";
                return 1;
            }
            if (file.peek() == std::ifstream::traits_type::eof()) {
                std::cerr << "ll1: File is empty.\n";
                return 1;
            }
            if (ll1_p.Parse()) {
                std::cout << "Parsing was successful.\n";
                if (verbose_mode) {
                    ll1_p.PrintStackTrace();
                }
            } else {
                std::cerr << "Parsing encountered an error.\n";
                ll1_p.PrintStackTrace();
                ll1_p.PrintSymbolHist();
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "ll1: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
