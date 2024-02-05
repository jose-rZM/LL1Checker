#pragma once
#include <regex>
#include <string>
#include <map>
#include <utility>
#include <vector>

enum symbol_type { NO_TERMINAL, TERMINAL };

struct symbol_table {
  static std::map<std::string, std::pair<symbol_type, std::string>>
      st;
  static std::map<std::string, std::string> rst;
  const static std::string EOL;
  const static std::string EPSILON;
  static void put_symbol(std::string identifier, symbol_type type,
                         std::string regex);
  static void put_symbol(std::string identifier, symbol_type type);
  static bool in(std::string s);
  static bool has_value(std::string s);
  static bool is_terminal(std::string s);
  static std::string get_value(std::string no_terminal);
  static void debug();
};