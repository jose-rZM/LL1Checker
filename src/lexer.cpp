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

    for (int i = 0; i < symbol_table::order_.size(); ++i) {
        std::string token_type{
            symbol_table::token_types_r_.at(symbol_table::order_[i])};
        if (token_type == "EPSILON") {
            continue;
        } else if (token_type == "$") {
            l << "\\$"
              << "\t{ return " << symbol_table::order_[i] << "; }\n";
        } else {
            std::string regex{symbol_table::st_.at(token_type).second};
            l << regex << "\t{ return " << symbol_table::order_[i] << "; }\n";
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
    int ret = system("lex -t src/lex.l > src/lex.yy.c");
    if (ret != 0) {
        std::cerr << "Error while compiling lexer\n";
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
        std::cerr << "error loading\n";
        exit(-1);
    }

    using set_yyin = int (*)(FILE*);
    set_yyin set = reinterpret_cast<set_yyin>(dlsym(dynlib, "set_yyin"));
    if (!set) {
        std::cerr << "Error al obtener el símbolo set_yyin" << std::endl;
        dlclose(dynlib);
        exit(-1);
    }

    using yylex_symbol = int (*)();
    yylex_symbol yylex =
        reinterpret_cast<yylex_symbol>((dlsym(dynlib, "yylex")));
    if (!yylex) {
        std::cerr << "Error al obtener el símbolo yylex" << std::endl;
        dlclose(dynlib);
        exit(-1);
    }

    // Without this, the program would have memory leaks!
    using yylex_destroy = int (*)();
    yylex_destroy destroy = reinterpret_cast<yylex_destroy>(dlsym(dynlib, "yylex_destroy"));
    if (!destroy) {
        std::cerr << "Error al obtener el simbolo yylex_destroy" << std::endl;
        dlclose(dynlib);
        exit(-1);
    }

    FILE *file = fopen(filename_.c_str(), "r");
    if (!file) {
        std::cerr << "Error al abrir fichero " << filename_ << "\n";
        dlclose(dynlib);
        exit(-1);
    }

    if (set(file) != 1) {
        std::cerr << "Error al establecer el archivo de entrada" << std::endl;
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
    tokens_.push_back("$");

    destroy();
    fclose(file);
    dlclose(dynlib);
}

std::string lexer::next() {
    return current_ >= tokens_.size() ? "" : tokens_[current_++];
}
