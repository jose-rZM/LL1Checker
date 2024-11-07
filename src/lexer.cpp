#include "../include/lexer.hpp"
#include "../include/lexer_error.hpp"
#include "../include/symbol_table.hpp"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <utility>

lexer::lexer(std::string filename)
    : filename_(std::move(filename)), tokens_(), current_() {
    create_temp_files();
    make_lexer();
    compile();
    tokenize();
    cleanup();
}

void lexer::create_temp_files() {
    temp_dir_ = boost::filesystem::temp_directory_path() /
                boost::filesystem::unique_path("ll1_%%%%-%%%%-%%%%-%%%%");
    boost::filesystem::create_directory(temp_dir_);
    lex_file_path_ = temp_dir_ / boost::filesystem::unique_path(
                                     "lexer_%%%%-%%%%-%%%%-%%%%.lex");
    c_file_path_ = temp_dir_ / boost::filesystem::unique_path(
                                   "lexer_%%%%-%%%%-%%%%-%%%%.c");
    o_file_path_ = temp_dir_ / boost::filesystem::unique_path(
                                   "lexer_%%%%-%%%%-%%%%-%%%%.o");
    so_file_path_ = temp_dir_ / boost::filesystem::unique_path(
                                    "lex.yy_%%%%-%%%%-%%%%-%%%%.so");
}

void lexer::tokenize() {

    void* dynlib{nullptr};
    FILE* file{nullptr};
    using set_yyin      = int (*)(FILE*);
    using yylex_symbol  = int (*)();
    using yylex_destroy = int (*)();

    yylex_destroy destroy{nullptr};

    try {
        dynlib = dlopen(so_file_path_.c_str(), RTLD_LAZY);
        if (!dynlib) {
            cleanup();
            throw LexerError("Error loading dynamic library lex.yy.so");
        }

        // Load symbol set_yyin, customm function to make yylex read from a file
        set_yyin set{reinterpret_cast<set_yyin>(dlsym(dynlib, "set_yyin"))};
        // Load symbol yylex
        yylex_symbol yylex{
            reinterpret_cast<yylex_symbol>((dlsym(dynlib, "yylex")))};
        // Load symbol yylex_destroy. Without this, the program would have
        // memory leaks!
        destroy =
            reinterpret_cast<yylex_destroy>(dlsym(dynlib, "yylex_destroy"));

        if (!set || !yylex || !destroy) {
            cleanup();
            throw LexerError(
                "Initialization error: failed to retrieve necessary lexer "
                "symbols (set, yylex, or destroy)");
        }

        file = fopen(filename_.c_str(), "r");
        if (!file) {
            cleanup();
            throw std::runtime_error(
                "File error: unable to open the specified file '" + filename_ +
                "'. Please check the file path and permissions");
        }

        if (set(file) != 1) {
            cleanup();
            throw LexerError(
                "Configuration error: failed to set the input file for lexing. "
                "Verify that the filename is correct");
        }

        // Read the file
        int token{yylex()};
        while (token != 0) {
            if (token == -1) {
                cleanup();
                throw LexerError(
                    "Lexical error: encountered an invalid token in the input");
            }
            tokens_.push_back(symbol_table::token_types_r_[token]);
            token = yylex();
        }
    } catch (const std::exception& e) {
        if (file) {
            fclose(file);
        }

        if (destroy) {
            destroy();
        }

        if (dynlib) {
            dlclose(dynlib);
        }
        cleanup();
        throw;
    }

    // Freeing resources
    destroy();
    fclose(file);
    dlclose(dynlib);
}

std::string lexer::next() {
    return current_ >= tokens_.size() ? "" : tokens_[current_++];
}

void lexer::make_lexer() {
    boost::filesystem::ofstream lex(lex_file_path_);
    if (!lex.is_open()) {
        cleanup();
        throw LexerError("Error opening temporary lexer file.");
    }
    lex << "%{\n\t#include<stdio.h>\n%}\n";
    lex << "%%\n";

    int         i{1};
    std::string token_type{symbol_table::token_types_r_.at(i)};
    lex << "\"" << symbol_table::EOL_ << "\""
        << "\t{ return " << i << "; }\n";
    ++i;
    for (size_t j = 1; j < symbol_table::order_.size(); ++j) {
        token_type = symbol_table::token_types_r_.at(i);
        std::string regex{symbol_table::st_.at(token_type).second};
        lex << regex << "\t{ return " << i << "; }\n";
        ++i;
    }

    lex << "[ \\t\\r\\n]+\t{}\n";
    lex << ".\t{ return -1; }\n"; // throw lexical error
    lex << "%%\n";
    lex << "int set_yyin(FILE* file) {\n"
        << "\tyyrestart(file);\n"
        << "\treturn 1;\n}\n";
    lex << "int yywrap() {\n\treturn 1;\n}\n";

    lex.close();
}

void lexer::compile() {
    std::string flex_cmd =
        "flex -t " + lex_file_path_.string() + " > " + c_file_path_.string();
    int ret = system(flex_cmd.c_str());
    if (ret != 0) {
        cleanup();
        throw LexerError(
            "Error while compiling lexer. Check if you have flex installed or "
            "if there are errors in the lexer (check the tokens).");
    }
    std::string gcc_cmd = "gcc -c " + c_file_path_.string() + " -o " +
                          o_file_path_.string() + " -fPIC";
    ret = system(gcc_cmd.c_str());
    if (ret != 0) {
        cleanup();
        throw LexerError("Error while compiling lex.yy.c.");
    }

    gcc_cmd = "gcc -shared -o " + so_file_path_.string() + " " +
              o_file_path_.string();
    ret = system(gcc_cmd.c_str());
    if (ret != 0) {
        cleanup();
        throw LexerError("Error while creating dynamic library.");
    }
}

void lexer::cleanup() {
    boost::filesystem::remove(lex_file_path_);
    boost::filesystem::remove(c_file_path_);
    boost::filesystem::remove(o_file_path_);
    boost::filesystem::remove(so_file_path_);
    boost::filesystem::remove_all(temp_dir_);
}
