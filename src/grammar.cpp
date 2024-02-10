#include "grammar.hpp"
#include "symbol_table.hpp"
#include <fstream>
#include <iostream>
#include <regex>
#include <unordered_map>
#include <vector>

grammar::grammar(const std::string &filename) : filename_(filename) {
    read_from_file();
}

void grammar::read_from_file() {
    std::ifstream file;
    file.open(filename_, std::ios::in);
    if (file.is_open()) {
        std::string input;
        std::smatch match;
        while (getline(file, input) && input != ";") {
            std::string id;
            std::string value;
            std::regex rx_no_terminal{
                "no\\s+terminal\\s+([a-zA-Z_\\'][a-zA-Z_0-9\\']*);"};
            std::regex rx_terminal{
                "terminal\\s+([a-zA-Z_\\'][a-zA-Z_0-9\\']*)\\s+([^]*);"};
            std::regex rx_axiom{
                "start\\s+with\\s+([a-zA-Z_\\'][a-zA-Z_0-9\\']*);"};
            if (std::regex_match(input, match, rx_no_terminal)) {
                symbol_table::put_symbol(match[1], NO_TERMINAL);
            } else if (std::regex_match(input, match, rx_terminal)) {
                symbol_table::put_symbol(match[1], TERMINAL, match[2]);
            } else if (std::regex_match(input, match, rx_axiom)) {
                set_axiom(match[1]);
            } else {
                std::cout << "Error while reading tokens: " << input << "\n";
                exit(-1);
            }
        }

        while (getline(file, input) && input != ";") {
            std::regex rx_empty_production{
                "([a-zA-Z_\\'][a-zA-Z_0-9\\']*)\\s*->;"};
            std::regex rx_production{
                "([a-zA-Z_\\'][a-zA-Z_0-9\\']*)\\s*->\\s*([a-zA-"
                "Z_\\'][a-zA-Z_0-9\\s$\\']*);"};
            if (std::regex_match(input, match, rx_production)) {
                std::string s = match[2];
                s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
                add_rule(match[1], s);
            } else if (std::regex_match(input, match, rx_empty_production)) {
                add_rule(match[1], symbol_table::EPSILON);
            } else {
                std::cout << "Error while reading grammar: " << input << "\n";
                exit(-1);
            }
        }
    }

    file.close();
}

std::vector<std::string> grammar::split(const std::string &s) {
    if (s == symbol_table::EPSILON) {
        return {symbol_table::EPSILON};
    }
    std::vector<std::string> splitted;
    std::string str;
    unsigned start = 0;
    unsigned end = 1;
    while (end <= s.size()) {
        str = s.substr(start, end - start);

        if (symbol_table::in(str)) {
            unsigned lookahead = end + 1;
            while (lookahead <= s.size()) {
                std::string extended = s.substr(start, lookahead - start);
                if (symbol_table::in(extended)) {
                    end = lookahead;
                }
                ++lookahead;
            }
            splitted.push_back(s.substr(start, end - start));
            start = end;
            end = start + 1;
        } else {
            ++end;
        }
    }

    return splitted;
}

void grammar::add_rule(const std::string &antecedent,
                       const std::string &consequent) {
    if (g_.find(antecedent) == g_.end()) {
        g_[antecedent] = {split(consequent)};
    } else {
        g_[antecedent].push_back(split(consequent));
    }
}

void grammar::set_axiom(const std::string &axiom) { AXIOM_ = axiom; }

bool grammar::has_empty_production(const std::string &antecedent) {
    auto rules{g_.at(antecedent)};
    return std::find_if(rules.cbegin(), rules.cend(), [](const auto &rule) {
               return rule[0] == "EPSILON";
           }) != rules.cend();
}

void grammar::debug() {
    std::cout << "Grammar:\n";
    for (const auto &entry : g_) {
        std::cout << entry.first << " -> ";
        for (const std::vector<std::string> &production : entry.second) {
            for (const std::string &s : production) {
                std::cout << s << " ";
            }
            std::cout << "| ";
        }
        std::cout << std::endl;
    }
}