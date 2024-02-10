#pragma once
#include <map>
#include <regex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

enum symbol_type { NO_TERMINAL, TERMINAL };

struct symbol_table {
    static std::map<std::string, std::pair<symbol_type, std::string>> st;
    static std::unordered_map<std::string, int> token_types;
    static std::unordered_map<int, std::string> token_types_r;
    static std::vector<int> order;

    static int i;
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