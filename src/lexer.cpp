#include "lexer.hpp"
#include "symbol_table.hpp"
#include <cctype>
#include <dlfcn.h>
#include <fstream>
#include <ios>
#include <iostream>
#include <istream>
#include <iterator>
#include <regex>
#include <stdexcept>
#include <string>

lexer::lexer(const std::string &filename)
    : filename(filename), tokens(), current() {
  tokenize();
}

void lexer::tokenize() {
  std::ofstream l{"lex.l"};
  l << "%{\n\t#include<stdio.h>\n%}\n";
  l << "%%\n";

  for (int i = 0; i < symbol_table::order.size(); ++i) {
    std::string token_type { symbol_table::token_types_r.at(symbol_table::order[i]) };
    if (token_type == "EPSILON") {
      continue;
    } else if (token_type == "$") {
    l << "\\$" << "\t{ return " << symbol_table::order[i] << "; }\n"; 
    } else {

    
    std::string regex { symbol_table::st.at(token_type).second };
    l << regex << "\t{ return " << symbol_table::order[i] << "; }\n"; 
    }
  }

  l << "[ \\t\\r\\n]\t{}\n";
  l << ".\t{ return -1; }\n"; // throw lexical error
  l << "%%\n";
  l << "int set_yyin(const char* filename) {\n"
    << "\tFILE* input = fopen(filename, \"r\");\n"
    << "\tyyrestart(input);\n"
    << "\treturn 1;\n}\n";
  l << "int yywrap() {\n\treturn 1;\n}\n";

  l.close();
  system("lex lex.l");
  system("gcc lex.yy.c -o lex.yy.so -O2 -shared -fPIC");
  void *dynlib = dlopen("./lex.yy.so", RTLD_LAZY);
  if (!dynlib) {
    std::cerr << "error loading\n";
    dlclose(dynlib);
    exit(-1);
  }

  typedef int (*set_yyin)(const char *);
  set_yyin set = reinterpret_cast<set_yyin>(dlsym(dynlib, "set_yyin"));
  if (!set) {
    std::cerr << "Error al obtener el símbolo set_yyin" << std::endl;
    dlclose(dynlib);
    exit(-1);
  }

  if (set("text.txt") != 1) {
    std::cerr << "Error al establecer el archivo de entrada" << std::endl;
    dlclose(dynlib);
    exit(-1);
  }

  typedef int (*yylex)();
  yylex create = (yylex)(dlsym(dynlib, "yylex"));
  if (!create) {
    std::cerr << "Error al obtener el símbolo yylex" << std::endl;
    dlclose(dynlib);
    exit(-1);
  }

  int token = create();
  while (token != 1) {
    if (token == -1) {
      std::cerr << "Lexical error\n";
      dlclose(dynlib);
      exit(-1);
    }
    tokens.push_back(symbol_table::token_types_r[token]);
    token = create();
  }
  tokens.push_back("$");

  dlclose(dynlib);
}

std::string lexer::next() {
  return current >= tokens.size() ? "" : tokens[current++];
}
