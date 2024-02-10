#include "lexer.hpp"
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
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
