#pragma once
#include <map>
#include <regex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

enum symbol_type { NO_TERMINAL, TERMINAL };

struct symbol_table {
    inline const static std::string EOL{"$"};
    inline const static std::string EPSILON{"EPSILON"};
    inline static std::unordered_map<std::string,
                                     std::pair<symbol_type, std::string>>
        st_{{EOL, {TERMINAL, EOL}}, {EPSILON, {TERMINAL, EPSILON}}};
    inline static std::unordered_map<std::string, int> token_types_{{EOL, 1}};
    inline static std::unordered_map<int, std::string> token_types_r_{{1, EOL}};
    inline static std::vector<int> order_{1};
    inline static int i_{2};

    static void put_symbol(const std::string &identifier,
                           const std::string &regex);
    static void put_symbol(const std::string &identifier);
    static bool in(const std::string &s);
    static bool is_terminal(const std::string &s);
    static std::string get_value(const std::string &terminal);
    static void debug();
};