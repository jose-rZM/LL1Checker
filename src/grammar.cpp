#include "grammar.hpp"
#include "symbol_table.hpp"
#include <iostream>
#include <unordered_map>
#include <vector>

std::vector<std::string> grammar::split(const std::string &s) {
  if (s == symbol_table::EPSILON) {
    return {symbol_table::EPSILON};
  }
  std::vector<std::string> splitted;
  std::string str;
  unsigned start = 0;
  unsigned end = 1;
  while (end <= s.size()) {
    str = s.substr(start, end - start);

    if (symbol_table::in(str)) {
      unsigned lookahead = end + 1;
      while (lookahead <= s.size()) {
        std::string extended = s.substr(start, lookahead - start);
        if (symbol_table::in(extended)) {
          end = lookahead;
        }
        ++lookahead;
      }
      splitted.push_back(s.substr(start, end - start));
      start = end;
      end = start + 1;
    } else {
      ++end;
    }
  }

  return splitted;
}

void grammar::add_rule(const std::string &antecedent,
                       const std::string &consequent) {
  if (g.find(antecedent) == g.end()) {
    g[antecedent] = {split(consequent)};
  } else {
    g[antecedent].push_back(split(consequent));
  }
}

void grammar::set_axiom(const std::string &axiom) { AXIOM = axiom; }

void grammar::debug() {
  for (const auto &entry : g) {
    std::cout << entry.first << " -> ";
    for (const std::vector<std::string> &production : entry.second) {
      for (const std::string &s : production) {
        std::cout << s << " ";
      }
      std::cout << "| ";
    }
    std::cout << std::endl;
  }
}