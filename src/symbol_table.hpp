#pragma once
#include <map>
#include <regex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

enum symbol_type { NO_TERMINAL, TERMINAL };

struct symbol_table {
    static std::unordered_map<std::string, std::pair<symbol_type, std::string>> st_;
    static std::unordered_map<std::string, int> token_types_;
    static std::unordered_map<int, std::string> token_types_r_;
    static std::vector<int> order_;
    static int i_;

    const static std::string EOL;
    const static std::string EPSILON;
    static void put_symbol(const std::string& identifier,
                           const std::string& regex);
    static void put_symbol(const std::string& identifier);
    static bool in(const std::string& s);
    static bool is_terminal(const std::string& s);
    static std::string get_value(const std::string& terminal);
    static void debug();
};