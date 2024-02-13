#pragma once
#include <map>
#include <regex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

enum symbol_type { NO_TERMINAL, TERMINAL };

struct symbol_table {
    inline static std::string EOL_{"$"};
    inline static std::string EPSILON_{"EPSILON"};
    inline static std::unordered_map<std::string,
                                     std::pair<symbol_type, std::string>>
        st_{{EOL_, {TERMINAL, EOL_}}, {EPSILON_, {TERMINAL, EPSILON_}}};
    inline static std::unordered_map<std::string, int> token_types_{{EOL_, 1}};
    inline static std::unordered_map<int, std::string> token_types_r_{
        {1, EOL_}};
    inline static std::vector<int> order_{1};
    inline static int i_{2};

    /**
     *
     * @param identifier of the terminal symbol
     * @param regex  of the terminal symbol
     * Stores the terminal symbol alongside its regex.
     * Also, it updates the token types, it also keeps track of the insertion
     * order.
     */
    static void put_symbol(const std::string &identifier,
                           const std::string &regex);
    /**
     *
     * @param identifier of the no terminal symbol
     * Stores the no terminal symbol in the symbol table.
     */
    static void put_symbol(const std::string &identifier);
    /**
     *
     * @param s identifier
     * @return true if s is in symbol table
     */
    static bool in(const std::string &s);

    /**
     *
     * @param s identifier
     * @return true if s is terminal symbol
     */
    static bool is_terminal(const std::string &s);
    /**
     *
     * @param terminal symbol to retrieve the regex from
     * @return regex of the symbol
     */
    static std::string get_value(const std::string &terminal);

    /**
     * Print all symbols in symbol table
     */
    static void debug();

    /**
     *
     * @param eol string
     * Set the EOL string.
     */
    static void set_eol(const std::string &eol);
};