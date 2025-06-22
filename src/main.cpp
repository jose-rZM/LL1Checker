#include <cxxopts.hpp>
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

void ShowUsage(const char* program_name, const cxxopts::Options& options) {
    std::cout << options.help({""}) << std::endl;
}

int main(int argc, char* argv[]) {
    std::string grammar_filename;
    std::string text_filename;
    bool        verbose_mode = false;
    std::string table_format = "new";

    try {
        cxxopts::Options options(argv[0], "LL1Checker");

        options.positional_help("grammar text").show_positional_help();

        options.add_options()("h,help", "Show help message")(
            "v,verbose", "Enable verbose mode with new table format",
            cxxopts::value<bool>(verbose_mode)->default_value("false"))(
            "format", "Set table format (old/new), implies verbose mode",
            cxxopts::value<std::string>())(
            "grammar", "Grammar file",
            cxxopts::value<std::string>(grammar_filename))(
            "text", "Text file to parse",
            cxxopts::value<std::string>(text_filename)->default_value(""));

        options.parse_positional({"grammar", "text"});
        auto result = options.parse(argc, argv);

        if (result.contains("help")) {
            ShowUsage(argv[0], options);
            return 0;
        }

        if (!result.contains("grammar")) {
            throw std::runtime_error("Required option: grammar");
        }

        if (result.contains("format")) {
            verbose_mode = true;
            table_format = result["format"].as<std::string>();
            if (table_format != "old" && table_format != "new") {
                throw std::runtime_error(
                    "Invalid format - must be 'old' or 'new'");
            }

        } else if (result.contains("verbose")) {
            if (table_format.empty()) {
                table_format = "new";
            }
        }
    } catch (const cxxopts::exceptions::specification& e) {
        std::cerr << "Error parsing options: " << e.what() << "\n\n";
        std::cerr << "Use --help to view the available arguments.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n\n";
    }

    if (!std::ifstream(grammar_filename)) {
        std::cerr << "Error: Grammar file '" << grammar_filename
                  << "' not found\n";
        return 1;
    }

    try {
        LL1Parser parser{grammar_filename, text_filename,
                         table_format == "new"};

        std::cout << "Grammar is LL(1)\n";

        if (verbose_mode) {
            std::cout << "\n--------------------------------\n"
                      << "LL1 Table (" << table_format << " format):\n";
            parser.PrintTable();

            if (!text_filename.empty()) {
                std::cout << "\n--------------------------------\n"
                          << "Input content:\n";
                if (PrintFileToStdout(text_filename)) {
                    throw std::runtime_error("Text file not found");
                }
            }
            std::cout << "--------------------------------\n\n";
        }

        if (!text_filename.empty()) {
            std::ifstream file(text_filename);
            if (!file)
                throw std::runtime_error("Text file not found");
            if (file.peek() == EOF)
                throw std::runtime_error("Text file is empty");

            if (parser.Parse()) {
                std::cout << "Parsing successful\n";
                if (verbose_mode)
                    parser.PrintStackTrace();
            } else {
                std::cerr << "Parsing failed\n";
                parser.PrintStackTrace();
                parser.PrintSymbolHist();
                return 1;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}