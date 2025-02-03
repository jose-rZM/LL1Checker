#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

#include "../include/ll1_parser.hpp"
namespace po = boost::program_options;

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

void ShowUsage(const char* program_name, const po::options_description& desc) {
    std::cout << "Usage: " << program_name
              << " <grammar_filename> [<text_filename>] [options]\n"
              << desc;
}

int main(int argc, char* argv[]) {
    std::string grammar_filename, text_filename;
    bool        verbose_mode = false;
    std::string table_format = "new";

    po::options_description desc("Options");
    desc.add_options()("help,h", "Show help message")(
        "verbose,v", po::bool_switch(&verbose_mode),
        "Enable verbose mode with new table format")(
        "format", po::value<std::string>(),
        "Set table format (old/new), implies verbose mode")(
        "grammar", po::value<std::string>(&grammar_filename)->required(),
        "Grammar file")("text", po::value<std::string>(&text_filename),
                        "Text file to parse");

    po::positional_options_description pos;
    pos.add("grammar", 1);
    pos.add("text", 1);

    try {
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv)
                      .options(desc)
                      .positional(pos)
                      .run(),
                  vm);

        if (vm.count("help")) {
            ShowUsage(argv[0], desc);
            return 0;
        }

        po::notify(vm);
        if (vm.count("format")) {
            verbose_mode = true;
            table_format = vm["format"].as<std::string>();
            if (table_format != "old" && table_format != "new") {
                throw std::runtime_error(
                    "Invalid format - must be 'old' or 'new'");
            }
        } else if (vm.count("verbose")) {
            if (table_format.empty())
                table_format = "new";
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n\n";
        ShowUsage(argv[0], desc);
        return 1;
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
