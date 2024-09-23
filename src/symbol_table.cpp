#include "symbol_table.hpp"
#include <cstdio>
#include <unordered_map>
#include <vector>

void symbol_table::put_symbol(const std::string &identifier,
                              const std::string &regex) {
    if (in(identifier))
    {
        return;
    }
    int id = i_++;
    st_[id] = {TERMINAL, regex};
    token_types_[identifier] = id;
    order_.push_back(id);
    token_types_r_[id] = identifier;
}

void symbol_table::put_symbol(const std::string &identifier) {
    if (in(identifier))
        return;
    int id = i_++;
    st_.insert({id, {NO_TERMINAL, ""}});
    token_types_[identifier] = id;
    token_types_r_[id] = identifier;

}

std::string symbol_table::get_value(int id) {
    return st_.find(id) != st_.end() ? st_[id].second : "";
}

void symbol_table::debug() {
    printf(" %-10s %-15s %-15s %-15s\n", "ID", "Identifier", "Type", "Regex");
    for (const auto &entry : st_) {
        const std::string &identifier = token_types_r_[entry.first];
        const char *type = (entry.second.first == TERMINAL) ? "TERMINAL" : "NO_TERMINAL";
        const std::string &regex = entry.second.second;
        printf(" %-10d %-15s %-15s %-15s\n", entry.first, identifier.c_str(), type, regex.c_str());
    }
}


bool symbol_table::in(const std::string &s) {
    return token_types_.find(s) != token_types_.end();
}

bool symbol_table::is_terminal(int id) {
    return st_.find(id) != st_.end() && st_[id].first == TERMINAL;
}

void symbol_table::set_eol(const std::string &eol) {
    EOL_ = eol;
    st_[1] = {TERMINAL, EOL_};
    token_types_[EOL_] = 1;
    token_types_r_[1] = EOL_;
}
