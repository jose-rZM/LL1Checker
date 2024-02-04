#include "lexer.hpp"
#include "symbol_table.hpp"
#include <cctype>
#include <fstream>
#include <ios>
#include <iostream>
#include <istream>
#include <iterator>
#include <regex>
#include <stdexcept>
#include <string>

lexer::lexer(const std::string &filename)
    : file(filename), tokens(), current() {
  if (!file.is_open()) {
    throw std::runtime_error("Error opening file " + filename);
  }
  tokenize();
}

lexer::~lexer() {
  if (file.is_open()) {
    file.close();
  }
  file.close();
}

void lexer::tokenize() {
  char c;
  std::string token;
  std::string id;
  std::streampos pos;
  

  while (file.get(c)) {
    // skip whitespaces (delimiter between tokens)
    bool match_found = false;
    if (std::isspace(static_cast<unsigned char>(c))) {
      continue;
    }
    token += c;

    for (const auto &entry : symbol_table::rst) {
      std::regex r{entry.first};
      if (std::regex_match(token, r)) {
        match_found = true;
        std::string lookahead{token};
        id = entry.second;
        // advance
        pos = file.tellg(); // store current position
        while (file.get(c) && !std::isspace(static_cast<unsigned char>(c))) {
          lookahead += c;
          for (const auto &regex : symbol_table::rst) {
            r = regex.first;
            if (std::regex_match(lookahead, r)) {
              pos = file.tellg(); // update current position
              token = lookahead;
              id = entry.second;
              break;
            }
          }
        }
        file.seekg(pos); // undo lookahead
      }
    }
    if (!match_found) {
      std::cerr << "Lexical error: " << token << std::endl;
      file.close();
      exit(-1);
    }
    tokens.push_back(id);
    token.clear();
    id.clear();
    match_found = false;
  }
  file.close();
}

std::string lexer::next() {
  return current >= tokens.size() ? "" : tokens[current++];
}
