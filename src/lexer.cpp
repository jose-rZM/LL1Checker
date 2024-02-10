#include "lexer.hpp"
#include "symbol_table.hpp"
#include <dlfcn.h>
#include <fstream>
#include <iostream>
#include <istream>
#include <string>

lexer::lexer(const std::string &filename)
    : filename_(filename), tokens_(), current_() {
    tokenize();
}

void lexer::tokenize() {
    std::ofstream l{"./src/lex.l"};
    l << "%{\n\t#include<stdio.h>\n%}\n";
    l << "%%\n";

    for (int i : symbol_table::order_) {
        std::string token_type{
            symbol_table::token_types_r_.at(i)};
        if (token_type == symbol_table::EPSILON) {
            continue;
        } else if (token_type == symbol_table::EOL) {
            l << "\"" << symbol_table::EOL << "\""
              << "\t{ return " << i << "; }\n";
        } else {
            std::string regex{symbol_table::st_.at(token_type).second};
            l << regex << "\t{ return " << i << "; }\n";
        }
    }

    l << "[ \\t\\r\\n]+\t{}\n";
    l << ".\t{ return -1; }\n"; // throw lexical error
    l << "%%\n";
    l << "int set_yyin(FILE* file) {\n"
      << "\tyyrestart(file);\n"
      << "\treturn 1;\n}\n";
    l << "int yywrap() {\n\treturn 1;\n}\n";

    l.close();
    int ret = system("flex -t src/lex.l > src/lex.yy.c");
    if (ret != 0) {
        std::cerr << "Error while compiling lexer. Check if you have flex installed\n";
        exit(-1);
    }

    ret = system("gcc -c src/lex.yy.c -o out/lex.yy.o -fPIC");
    if (ret != 0) {
        std::cerr << "Error while compiling lex.yy.c\n";
        exit(-1);
    }

    ret = system("gcc -shared -o lib/lex.yy.so out/lex.yy.o");
    if (ret != 0) {
        std::cerr << "Error while creating dynamic library\n";
        exit(-1);
    }
    void *dynlib = dlopen("./lib/lex.yy.so", RTLD_LAZY);
    if (!dynlib) {
        std::cerr << "Error loading dynamic library lex.yy.so\n";
        exit(-1);
    }

    using set_yyin = int (*)(FILE*);
    set_yyin set = reinterpret_cast<set_yyin>(dlsym(dynlib, "set_yyin"));
    if (!set) {
        std::cerr << "Error while obtaining set_yyin symbol" << std::endl;
        dlclose(dynlib);
        exit(-1);
    }

    using yylex_symbol = int (*)();
    yylex_symbol yylex =
        reinterpret_cast<yylex_symbol>((dlsym(dynlib, "yylex")));
    if (!yylex) {
        std::cerr << "Error while obtaining yylex symbol" << std::endl;
        dlclose(dynlib);
        exit(-1);
    }

    // Without this, the program would have memory leaks!
    using yylex_destroy = int (*)();
    yylex_destroy destroy = reinterpret_cast<yylex_destroy>(dlsym(dynlib, "yylex_destroy"));
    if (!destroy) {
        std::cerr << "Error while obtaining yylex_destroy symbol" << std::endl;
        dlclose(dynlib);
        exit(-1);
    }

    FILE *file = fopen(filename_.c_str(), "r");
    if (!file) {
        std::cerr << "Error opening the file" << filename_ << "\n";
        dlclose(dynlib);
        exit(-1);
    }

    if (set(file) != 1) {
        std::cerr << "Error while establishing the input file" << std::endl;
        dlclose(dynlib);
        exit(-1);
    }

    int token = yylex();
    while (token != 1) {
        if (token == -1) {
            std::cerr << "Lexical error\n";
            dlclose(dynlib);
            exit(-1);
        }
        tokens_.push_back(symbol_table::token_types_r_[token]);
        token = yylex();
    }
    tokens_.push_back(symbol_table::EOL);

    destroy();
    fclose(file);
    dlclose(dynlib);
}

std::string lexer::next() {
    return current_ >= tokens_.size() ? "" : tokens_[current_++];
}
