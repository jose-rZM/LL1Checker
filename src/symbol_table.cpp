#include "../include/symbol_table.hpp"
#include <cstdio>
#include <unordered_map>
#include <vector>

void symbol_table::PutSymbol(const std::string& identifier,
                             const std::string& regex) {
    st_[identifier]          = {TERMINAL, regex};
    token_types_[identifier] = i_;
    order_.push_back(i_);
    token_types_r_[i_++] = identifier;
}

void symbol_table::PutSymbol(const std::string& identifier) {
    st_.insert({identifier, {NO_TERMINAL, ""}});
}

std::string symbol_table::GetValue(const std::string& terminal) {
    return st_.at(terminal).second;
}

void symbol_table::Debug() {
    printf(" %-15s %-15s %-15s\n", "Identifier", "Type", "Regex");
    for (const auto& entry : st_) {
        printf(" %-15s %-15u %-15s\n", entry.first.c_str(), entry.second.first,
               entry.second.second.c_str());
    }
}

bool symbol_table::In(const std::string& s) {
    return st_.find(s) != st_.cend();
}

bool symbol_table::IsTerminal(const std::string& s) {
    return st_.at(s).first == TERMINAL;
}

void symbol_table::SetEol(const std::string& eol) {
    EOL_               = eol;
    st_[EOL_]          = {TERMINAL, EOL_};
    token_types_[EOL_] = 1;
    token_types_r_[1]  = EOL_;
}
