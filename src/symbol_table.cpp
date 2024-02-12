#include "symbol_table.hpp"
#include <algorithm>
#include <cstdio>
#include <map>
#include <unordered_map>
#include <vector>

/**
 *
 * @param identifier of the terminal symbol
 * @param regex  of the terminal symbol
 * Stores the terminal symbol alongside its regex.
 * Also, it updates the token types, it also keeps track of the insertion order.
 */
void symbol_table::put_symbol(const std::string &identifier,
                              const std::string &regex) {
    st_[identifier] = {TERMINAL, regex};
    token_types_[identifier] = i_;
    order_.push_back(i_);
    token_types_r_[i_++] = identifier;
}

/**
 *
 * @param identifier of the no terminal symbol
 * Stores the no terminal symbol in the symbol table.
 */
void symbol_table::put_symbol(const std::string &identifier) {
    st_[identifier] = {NO_TERMINAL, ""};
}

/**
 *
 * @param terminal symbol to retrieve the regex from
 * @return regex of the symbol
 */
std::string symbol_table::get_value(const std::string &terminal) {
    return st_.at(terminal).second;
}

/**
 * Print all symbols in symbol table
 */
void symbol_table::debug() {
    printf(" %-15s %-15s %-15s\n", "Identifier", "Type", "Regex");
    for (const auto &entry : st_) {
        printf(" %-15s %-15u %-15s\n", entry.first.c_str(), entry.second.first,
               entry.second.second.c_str());
    }
}

/**
 *
 * @param s identifier
 * @return true if s is in symbol table
 */
bool symbol_table::in(const std::string &s) {
    return st_.find(s) != st_.cend();
}

/**
 *
 * @param s identifier
 * @return true if s is terminal symbol
 */
bool symbol_table::is_terminal(const std::string &s) {
    return st_.at(s).first == TERMINAL;
}

void symbol_table::set_eol(const std::string &eol) {
    EOL_ = eol;
    st_[EOL_] = {TERMINAL, EOL_};
    token_types_[EOL_] = 1;
    token_types_r_[1] = EOL_;
}
