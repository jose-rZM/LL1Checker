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




int main() {
  

  std::cout << "Grammar:\n";
  
  std::cout << "\nInput:" << std::endl;
  std::ifstream file;
  file.open("text.txt");
  std::string line;
  while (getline(file, line)) {
    std::cout << line << std::endl;
  }
  file.close();
  
  LL1Parser parser("input.txt", "text.txt");

  if (parser.parse()) {
    std::cout << "Parsing was successful";
  } else {
    std::cerr << "Parsing encountered an error.";
  }

  std::cout << std::endl;
}
