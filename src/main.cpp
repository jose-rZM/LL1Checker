#include "grammar.hpp"
#include "lexer.hpp"
#include "symbol_table.hpp"
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <ios>
#include <iostream>
#include <istream>
#include <iterator>
#include <map>
#include <ostream>
#include <regex>
#include <stack>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <dlfcn.h>
#include "ll1_parser.hpp"


void read_from_file(grammar &g) {
  const std::string filename{"input.txt"};
  std::ifstream file;
  file.open(filename, std::ios::in);
  if (file.is_open()) {
    std::string input;
    std::smatch match;
    while (getline(file, input) && input != ";") {
      std::string id;
      std::string value;
      std::regex rx_no_terminal{
          "no\\s+terminal\\s+([a-zA-Z_\\'][a-zA-Z_0-9\\']*);"};
      std::regex rx_terminal{
          "terminal\\s+([a-zA-Z_\\'][a-zA-Z_0-9\\']*)\\s+([^]*);"};
      std::regex rx_axiom{"start\\s+with\\s+([a-zA-Z_\\'][a-zA-Z_0-9\\']*);"};
      if (std::regex_match(input, match, rx_no_terminal)) {
        symbol_table::put_symbol(match[1], NO_TERMINAL);
      } else if (std::regex_match(input, match, rx_terminal)) {
        symbol_table::put_symbol(match[1], TERMINAL, match[2]);
      } else if (std::regex_match(input, match, rx_axiom)) {
        g.set_axiom(match[1]);
      } else {
        std::cout << "Error while reading tokens: " << input << "\n";
        exit(-1);
      }
    }

    while (getline(file, input) && input != ";") {
      std::regex rx_empty_production{"([a-zA-Z_\\'][a-zA-Z_0-9\\']*)\\s*->;"};
      std::regex rx_production{"([a-zA-Z_\\'][a-zA-Z_0-9\\']*)\\s*->\\s*([a-zA-"
                               "Z_\\'][a-zA-Z_0-9\\s$\\']*);"};
      if (std::regex_match(input, match, rx_production)) {
        std::string s = match[2];
        s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
        g.add_rule(match[1], s);
      } else if (std::regex_match(input, match, rx_empty_production)) {
        g.add_rule(match[1], symbol_table::EPSILON);
      } else {
        std::cout << "Error while reading grammar: " << input << "\n";
        exit(-1);
      }
    }
  }

  file.close();
}

int main() {
  grammar gr;
  read_from_file(gr);

  std::cout << "Grammar:\n";
  gr.debug();
  std::cout << "\nInput:" << std::endl;
  std::ifstream file;
  file.open("text.txt");
  std::string line;
  while (getline(file, line)) {
    std::cout << line << std::endl;
  }
  file.close();
  
  LL1Parser parser(gr, "input.txt", "text.txt");

  if (parser.parse()) {
    std::cout << "Parsing was successful";
  } else {
    std::cerr << "Parsing encountered an error.";
  }

  std::cout << std::endl;
}
