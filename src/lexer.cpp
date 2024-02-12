#include "lexer.hpp"
#include "errors/lexer_error.hpp"
#include "symbol_table.hpp"
#include <dlfcn.h>
#include <fstream>
#include <iostream>
#include <istream>
#include <string>
#include <utility>

/**
 *
 * @param filename of the input file (where the string to validate is
 * placed).
 * The constructor creates the lexer, compile it and tokenize the input.
 * The program assumes that all the requirements listed in "README.md" are met.
 * If any errors occur the program aborts.
 */
lexer::lexer(std::string filename)
    : filename_(std::move(filename)), tokens_(), current_() {
    make_lexer();
    compile();
    tokenize();
}

/**
 * Open the dynamically generated library and load: "set_yyin", "yylex" and
 * "yylex_destroy". The program aborts if any of these loads fail.
 * If everything goes well, tokenize the input using yylex. Each token is
 * stored in the "tokens" vector. Once the EOL character is reached (with value
 * 1), all resources are freed. This function is called only once.
 */
void lexer::tokenize() {

    void *dynlib = nullptr;
    FILE *file = nullptr;
    using set_yyin = int (*)(FILE *);
    using yylex_symbol = int (*)();
    using yylex_destroy = int (*)();

    yylex_destroy destroy = nullptr;

    try {
        dynlib = dlopen("./lib/lex.yy.so", RTLD_LAZY);
        if (!dynlib) {
            throw LexerError("Error loading dynamic library lex.yy.so");
        }

        // Load symbol set_yyin, customm function to make yylex read from a file
        set_yyin set = reinterpret_cast<set_yyin>(dlsym(dynlib, "set_yyin"));
        // Load symbol yylex
        yylex_symbol yylex =
            reinterpret_cast<yylex_symbol>((dlsym(dynlib, "yylex")));
        // Load symbol yylex_destroy. Without this, the program would have
        // memory leaks!
        destroy =
            reinterpret_cast<yylex_destroy>(dlsym(dynlib, "yylex_destroy"));
        if (!set || !yylex || !destroy) {
            throw LexerError("Error while obtaining one or more symbols");
        }

        file = fopen(filename_.c_str(), "r");
        if (!file) {
            throw std::runtime_error("Error opening the file " + filename_);
        }

        if (set(file) != 1) {
            throw LexerError("Error while establishing the input file");
        }

        // Read the file
        int token = yylex();
        while (token != 0) {
            if (token == -1) {
                throw LexerError("Lexical error");
            }
            tokens_.push_back(symbol_table::token_types_r_[token]);
            token = yylex();
        }
    } catch (const std::exception &e) {
        if (file) {
            fclose(file);
        }

        if (destroy) {
            destroy();
        }

        if (dynlib) {
            dlclose(dynlib);
        }
        throw;
    }

    // Freeing resources
    destroy();
    fclose(file);
    dlclose(dynlib);
}

/**
 *
 * @return next token of the vector, empty string if end of line is reached.
 */
std::string lexer::next() {
    return current_ >= tokens_.size() ? "" : tokens_[current_++];
}

/**
 * Generates a lexer file using the symbol table. The symbols are placed in
 * order. It also generates a custom function "set_yyin" for changing the input
 * of yylex.
 */
void lexer::make_lexer() {
    std::ofstream lex{SRC_PATH + "/" + LEXER_FILENAME};
    if (!lex.is_open()) {
        std::cerr << "Error opening " << SRC_PATH + "/" + LEXER_FILENAME
                  << std::endl;
        exit(-1);
    }
    lex << "%{\n\t#include<stdio.h>\n%}\n";
    lex << "%%\n";

    for (int i : symbol_table::order_) {
        std::string token_type{symbol_table::token_types_r_.at(i)};
        if (token_type == symbol_table::EPSILON_) {
            continue;
        } else if (token_type == symbol_table::EOL_) {
            lex << "\"" << symbol_table::EOL_ << "\""
                << "\t{ return " << i << "; }\n";
        } else {
            std::string regex{symbol_table::st_.at(token_type).second};
            lex << regex << "\t{ return " << i << "; }\n";
        }
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

/**
 * Compile lexer file named "lexer.l" into lex.yy.c. After that, it generates a
 * dynamic library using the compiled lexer "lex.yy.c".
 */
void lexer::compile() {
    int ret = system("flex -t src/lex.l > src/lex.yy.c");
    if (ret != 0) {
        throw LexerError(
            "Error while compiling lexer. Check if you have flex installed or "
            "if there are errors in the lexer (check the tokens).");
    }

    ret = system("gcc -c src/lex.yy.c -o out/lex.yy.o -fPIC");
    if (ret != 0) {
        throw LexerError("Error while compiling lex.yy.c.");
    }

    ret = system("gcc -shared -o lib/lex.yy.so out/lex.yy.o");
    if (ret != 0) {
        throw LexerError("Error while creating dynamic library.");
    }
}
