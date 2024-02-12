#include "grammar.hpp"
#include "errors/grammar_error.hpp"
#include "symbol_table.hpp"
#include <fstream>
#include <iostream>
#include <regex>
#include <unordered_map>
#include <utility>
#include <vector>

grammar::grammar(std::string filename) : filename_(std::move(filename)) {
    read_from_file();
}

/**
 * @throws GrammarError if something went wrong while reading the symbols,
 * grammar or while splitting the rules Read the grammar from a file. The
 * structure of grammar.txt is explained in README.md
 */
void grammar::read_from_file() {
    std::ifstream file;
    file.open(filename_, std::ios::in);

    if (!file.is_open()) {
        throw std::runtime_error("Error opening " + filename_);
    }

    std::regex rx_no_terminal{
        R"(no\s+terminal\s+([a-zA-Z_\'][a-zA-Z_0-9\']*);\s*)"};
    std::regex rx_terminal{
        R"(terminal\s+([a-zA-Z_\'][a-zA-Z_0-9\']*)\s+([^]*);\s*)"};
    std::regex rx_eol{R"(set\s+EOL\s+char\s+([^]*);\s*)"};
    std::regex rx_axiom{R"(start\s+with\s+([a-zA-Z_\'][a-zA-Z_0-9\']*);\s*)"};
    std::regex rx_empty_production{R"(([a-zA-Z_\'][a-zA-Z_0-9\']*)\s*->;\s*)"};
    std::regex rx_production{"([a-zA-Z_\\'][a-zA-Z_0-9\\']*)\\s*->\\s*([a-zA-"
                             "Z_\\'][a-zA-Z_0-9\\s$\\']*);"};

    std::string input;
    std::smatch match;
    try {
        while (getline(file, input) && input != ";") {
            std::string id;
            std::string value;

            if (std::regex_match(input, match, rx_no_terminal)) {
                symbol_table::put_symbol(match[1]);
            } else if (std::regex_match(input, match, rx_terminal)) {
                symbol_table::put_symbol(match[1], match[2]);
            } else if (std::regex_match(input, match, rx_axiom)) {
                set_axiom(match[1]);
            } else if (std::regex_match(input, match, rx_eol)) {
                symbol_table::set_eol(match[1]);
            } else {
                throw GrammarError("Error while reading tokens " + input);
            }
        }

        while (getline(file, input) && input != ";") {
            if (std::regex_match(input, match, rx_production)) {
                std::string s = match[2];
                s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
                add_rule(match[1], s);
            } else if (std::regex_match(input, match, rx_empty_production)) {
                add_rule(match[1], symbol_table::EPSILON_);
            } else {
                throw GrammarError("Error while reading grammar " + input);
            }
        }
    } catch (const std::exception &e) {
        if (file) {
            file.close();
        }
        std::cerr << e.what() << std::endl;
    }

    file.close();
}

/**
 *
 * @param s production to be splitted
 * @return vector of tokens
 * Splits the production into tokens. For example, if symbol table contains:
 * {(A, NON TERMINAL), (B, NON TERMINAL), (PLUS, TERMINAL)} and s = APLUSB, the
 * method would return {A, PLUS, B}.
 */
std::vector<std::string> grammar::split(const std::string &s) {
    if (s == symbol_table::EPSILON_) {
        return {symbol_table::EPSILON_};
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

    // If start < end - 1 there is at least one symbol not recognized
    if (start < end - 1) {
        throw GrammarError("Error processing the line " + s.substr(start, end));
    }

    return splitted;
}

/**
 *
 * @param antecedent of the rule
 * @param consequent of the rule
 */
void grammar::add_rule(const std::string &antecedent,
                       const std::string &consequent) {
    if (g_.find(antecedent) == g_.end()) {
        g_[antecedent] = {split(consequent)};
    } else {
        g_[antecedent].push_back(split(consequent));
    }
}

/**
 *
 * @param axiom of the grammar
 * Sets the axiom (entry point) of the grammar
 */
void grammar::set_axiom(const std::string &axiom) { AXIOM_ = axiom; }

/**
 *
 * @param antecedent of rule
 * @return true if there exists an empty rule, that is, <antecedent> ->;"
 */
bool grammar::has_empty_production(const std::string &antecedent) {
    auto rules{g_.at(antecedent)};
    return std::find_if(rules.cbegin(), rules.cend(), [](const auto &rule) {
               return rule[0] == symbol_table::EPSILON_;
           }) != rules.cend();
}

/**
 *
 * @param arg token to be searched in grammar rules
 * @return vector of rules with args as part of the consequent
 */
std::vector<std::pair<const std::string, production>>
grammar::filterRulesByConsequent(const std::string &arg) {
    std::vector<std::pair<const std::string, production>> rules;
    for (const std::pair<const std::string, std::vector<production>> &rule :
         g_) {
        for (const production &prod : rule.second) {
            if (std::find(prod.cbegin(), prod.cend(), arg) != prod.cend()) {
                rules.emplace_back(rule.first, prod);
            }
        }
    }
    return rules;
}

/**
 * Prints the grammar
 */
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