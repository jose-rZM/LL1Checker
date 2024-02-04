#include "symbol_table.hpp"
#include <algorithm>
#include <cstdio>
#include <vector>

const std::string symbol_table::EOL{"\\$"};
const std::string symbol_table::EPSILON{"EPSILON"};

std::unordered_map<std::string, std::pair<symbol_type, std::string>>
    symbol_table::st = {{EOL, {TERMINAL, EOL}}, {EPSILON, {TERMINAL, EPSILON}}};
std::unordered_map<std::string, std::string> symbol_table::rst = {
    {EOL, EOL}, {EPSILON, EPSILON}};

void symbol_table::put_symbol(std::string identifier, symbol_type type,
                              std::string regex) {
  st[identifier] = {type, regex};
  rst[regex] = identifier;
}

void symbol_table::put_symbol(std::string identifier, symbol_type type) {
  st[identifier] = {type, ""};
}

std::string symbol_table::get_value(std::string no_terminal) {
  return st[no_terminal].second;
}

bool symbol_table::has_value(std::string s) {
  return rst.find(s) != rst.cend();
}

void symbol_table::debug() {
  printf(" %-15s %-15s %-15s\n", "Identifier", "Type", "Regex");
  for (const auto &entry : st) {
    printf(" %-15s %-15u %-15s\n", entry.first.c_str(), entry.second.first,
           entry.second.second.c_str());
  }
}

bool symbol_table::in(std::string s) { return st.find(s) != st.cend(); }

bool symbol_table::is_terminal(std::string s) {
  return st[s].first == TERMINAL;
}