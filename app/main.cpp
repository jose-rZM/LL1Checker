#include <cxxopts.hpp>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

#include "ll1_parser.hpp"

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
    std::string text_input;
    bool        text_is_raw = false;
    bool        verbose_mode = false;
    std::string table_format = "new";
    std::string export_tree_file;

    try {
        cxxopts::Options options(argv[0], "LL1Checker");

        options.positional_help("grammar [input-file]").show_positional_help();

        options.add_options()("h,help", "Show help message")(
            "v,verbose", "Enable verbose mode with new table format",
            cxxopts::value<bool>(verbose_mode)->default_value("false"))(
            "format", "Set table format (old/new), implies verbose mode",
            cxxopts::value<std::string>())(
            "grammar", "Grammar file",
            cxxopts::value<std::string>(grammar_filename))(
            "input-file", "Text file to parse",
            cxxopts::value<std::string>(text_filename)->default_value(""))(
            "text", "Text to parse directly",
            cxxopts::value<std::string>(text_input))(
            "export-tree",
            "Export parse tree to dot file. Filename is mandatory",
            cxxopts::value<std::string>(export_tree_file));

        options.parse_positional({"grammar", "input-file"});
        auto result = options.parse(argc, argv);

        if (result.contains("help")) {
            ShowUsage(argv[0], options);
            return 0;
        }

        if (!result.contains("grammar")) {
            throw std::runtime_error("Required option: grammar");
        }

        if (result.contains("text")) {
            text_input   = result["text"].as<std::string>();
            text_is_raw  = true;
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
        LL1Parser parser{grammar_filename,
                         text_is_raw ? text_input : text_filename,
                         table_format == "new", text_is_raw};

        std::cout << "Grammar is LL(1)\n";

        if (verbose_mode) {
            std::cout << "\n--------------------------------\n"
                      << "LL1 Table (" << table_format << " format):\n";
            parser.PrintTable();

            if (text_is_raw) {
                std::cout << "\n--------------------------------\n"
                          << "Input content:\n"
                          << text_input << "\n";
            } else if (!text_filename.empty()) {
                std::cout << "\n--------------------------------\n"
                          << "Input content:\n";
                if (PrintFileToStdout(text_filename)) {
                    throw std::runtime_error("Text file not found");
                }
            }
            std::cout << "--------------------------------\n\n";
        }

        if (text_is_raw || !text_filename.empty()) {
            if (!text_is_raw) {
                std::ifstream file(text_filename);
                if (!file)
                    throw std::runtime_error("Text file not found");
                if (file.peek() == EOF)
                    throw std::runtime_error("Text file is empty");
            }

            if (!export_tree_file.empty()) {
                LL1Parser::ParseTree tree =
                    parser.ParseWithTree(text_is_raw ? text_input : text_filename);
                if (!tree) {
                    return 1;
                }
                std::cout <<
                    "Parsing successful.\nParse tree exported! Run "
                    "'dot -Tpng " + export_tree_file +
                    " -o output.png' to generate an image\n";
                parser.ExportTreeAsDot(tree, export_tree_file);
            } else {
                if (!parser.Parse()) {
                    return 1;
                }
                std::cout << "Parsing successful\n";
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}