#include "symbol_table.hpp"
#include <cstdio>
#include <unordered_map>
#include <vector>

void symbol_table::PutSymbol(const std::string& identifier,
                             const std::string& regex) {
    auto it = lookup_.find(identifier);
    if (it != lookup_.end()) return;
    TokenID id = next_id_++;
    lookup_[identifier] = id;
    st_.emplace_back(TERMINAL, regex);
}

void symbol_table::PutSymbol(const std::string& identifier) {
    auto it = lookup_.find(identifier);
    if (it != lookup_.end()) return;
    TokenID id = next_id_++;
    lookup_[identifier] = id;
    st_.emplace_back(NO_TERMINAL, "");
}

std::string symbol_table::GetValue(TokenID id) {
    return st_.at(id).second;
}

void symbol_table::Debug() {
    ;
}

bool symbol_table::In(TokenID id) {
    return false;
}

bool symbol_table::In(const std::string &identifier) {
    return lookup_.contains(identifier);
}


bool symbol_table::IsTerminal(TokenID id) {
    return In(id) && st_[id].first == TERMINAL;
}