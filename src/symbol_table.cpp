#include "symbol_table.hpp"
#include "grammar_error.hpp"
#include <cstdio>
#include <unordered_map>
#include <vector>

void symbol_table::PutSymbol(const std::string& identifier,
                             const std::string& regex) {
    if (lookup_.contains(identifier))
        throw GrammarError("Duplicate identifier detected: " + identifier);

    TokenID id          = next_id_++;
    lookup_[identifier] = id;
    names_.push_back(identifier);
    st_.emplace_back(TERMINAL, regex);
    order_.push_back(id);
}

void symbol_table::PutSymbol(const std::string& identifier) {
    if (lookup_.contains(identifier))
        throw GrammarError("Duplicate identifier detected: " + identifier);

    TokenID id          = next_id_++;
    lookup_[identifier] = id;
    names_.push_back(identifier);
    st_.emplace_back(NO_TERMINAL, "");
}

std::string symbol_table::GetValue(TokenID id) {
    if (!In(id))
        return "";
    return st_[id].second;
}

void symbol_table::Debug() {
    printf(" %-5s | %-15s | %-10s | %-s\n", "ID", "Identifier", "Type",
           "Regex");
    for (TokenID id = 1; id < st_.size(); ++id) {
        const char* type = st_[id].first == TERMINAL      ? "TERMINAL"
                           : st_[id].first == NO_TERMINAL ? "NON_TERMINAL"
                                                          : "META";
        printf(" %-5lu | %-15s | %-10s | %-s\n", id, names_[id].c_str(), type,
               st_[id].second.c_str());
    }
}

bool symbol_table::In(TokenID id) {
    return id < st_.size() && id != INVALID_TOKEN;
}

bool symbol_table::In(const std::string& identifier) {
    return lookup_.contains(identifier);
}

const std::string& symbol_table::ToString(TokenID id) {
    static std::string empty;
    if (!In(id))
        return empty;
    return names_[id];
}

symbol_table::TokenID symbol_table::ToID(const std::string& identifier) {
    auto it = lookup_.find(identifier);
    if (it == lookup_.end())
        return INVALID_TOKEN;
    return it->second;
}

bool symbol_table::IsTerminal(TokenID id) {
    return In(id) && st_[id].first == TERMINAL;
}
