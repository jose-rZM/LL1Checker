#include "grammar.hpp"
#include "grammar_error.hpp"
#include "symbol_table.hpp"
#include <fstream>
#include <iostream>
#include <regex>
#include <unordered_map>
#include <utility>
#include <vector>

Grammar::Grammar(std::string filename) : kFilename(std::move(filename)) {
    ReadFromFile();
}

void Grammar::ReadFromFile() {
    std::ifstream file;
    file.open(kFilename, std::ios::in);

    if (!file.is_open()) {
        throw std::runtime_error("Error opening file: " + kFilename);
    }

    std::unordered_map<std::string, std::vector<std::string>> p_grammar;
    std::regex                                                rx_terminal{
        R"(terminal\s+([a-zA-Z_\'][a-zA-Z_0-9\']*)\s+([^]*);\s*)"};
    std::regex rx_axiom{R"(start\s+with\s+([a-zA-Z_\'][a-zA-Z_0-9\']*);\s*)"};
    std::regex rx_empty_production{R"(([a-zA-Z_\'][a-zA-Z_0-9\']*)\s*->;\s*)"};
    std::regex rx_production{
        R"(([a-zA-Z_\'][a-zA-Z_0-9\']*)\s*->\s*([a-zA-Z_\'][a-zA-Z_0-9\s\']*);\s*)"};

    std::string              axiom_name;
    std::vector<std::string> nt_order_names;
    std::string              input;
    std::smatch              match;

    if (file.peek() == std::ifstream::traits_type::eof()) {
        throw std::runtime_error("File is empty");
    }
    try {
        while (getline(file, input) && input != ";") {
            std::string id;
            std::string value;

            if (std::regex_match(input, match, rx_terminal)) {
                if (match[1] == symbol_table::EOF_ ||
                    match[1] == symbol_table::EPSILON_) {
                    throw GrammarError("Reserved token name: " +
                                       match[1].str());
                }
                symbol_table::PutSymbol(match[1], match[2]);
            } else if (std::regex_match(input, match, rx_axiom)) {
                axiom_name = match[1];
            } else {
                throw GrammarError("Error while reading token definitions: " +
                                   input);
            }
        }

        while (getline(file, input) && input != ";") {
            if (std::regex_match(input, match, rx_production)) {
                std::string nt = match[1];
                std::string s  = match[2];
                s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
                if (!p_grammar.contains(nt)) {
                    nt_order_names.push_back(nt);
                }
                p_grammar[nt].push_back(s);
            } else if (std::regex_match(input, match, rx_empty_production)) {
                std::string nt = match[1];
                if (!p_grammar.contains(nt)) {
                    nt_order_names.push_back(nt);
                }
                p_grammar[nt].push_back(symbol_table::EPSILON_);
            } else {
                throw GrammarError("Error while reading grammar rule: " +
                                   input);
            }
        }
    } catch (const std::exception& e) {
        if (file) {
            file.close();
        }
        throw;
    }
    file.close();

    // Add non terminal symbols
    for (const auto& entry : p_grammar) {
        symbol_table::PutSymbol(entry.first);
    }

    for (const std::string& nt : nt_order_names) {
        nt_order_.push_back(symbol_table::ToID(nt));
    }

    if (!axiom_name.empty()) {
        SetAxiom(symbol_table::ToID(axiom_name));
    }

    // Add all rules
    for (const auto& entry : p_grammar) {
        for (const auto& prod : entry.second) {
            production p = Split(prod);
            AddRule(symbol_table::ToID(entry.first), p);
        }
    }

    if (symbol_table::IsTerminal(axiom_)) {
        throw GrammarError("Axiom cannot be a terminal symbol");
    }

    const std::string aug =
        GenerateNewNonTerminal(symbol_table::ToString(axiom_));
    symbol_table::PutSymbol(aug);
    production aug_prod = {axiom_, symbol_table::EOF_ID};
    AddRule(symbol_table::ToID(aug), aug_prod);
    axiom_ = symbol_table::ToID(aug);
    nt_order_.insert(nt_order_.begin(), axiom_);
}

std::vector<symbol_table::TokenID> Grammar::Split(const std::string& s) {
    if (s == symbol_table::EPSILON_) {
        return {symbol_table::EPSILON_ID};
    }
    std::vector<symbol_table::TokenID> splitted{};
    std::string                        str;
    unsigned                           start{0};
    unsigned                           end{1};
    while (end <= s.size()) {
        str = s.substr(start, end - start);

        if (symbol_table::In(str)) {
            unsigned lookahead = end + 1;
            while (lookahead <= s.size()) {
                std::string extended = s.substr(start, lookahead - start);
                if (symbol_table::In(extended)) {
                    end = lookahead;
                }
                ++lookahead;
            }
            splitted.push_back(
                symbol_table::ToID(s.substr(start, end - start)));
            start = end;
            end   = start + 1;
        } else {
            ++end;
        }
    }

    // If start < end - 1 there is at least one symbol not recognized
    if (start < end - 1) {
        throw GrammarError("Error processing line: " + s.substr(start, end));
    }

    return splitted;
}

std::string Grammar::GenerateNewNonTerminal(const std::string& base) {
    std::string newNt = base;
    do {
        newNt.append("'");
    } while (symbol_table::In(newNt));
    return newNt;
}

void Grammar::AddRule(symbol_table::TokenID antecedent,
                      const production&     consequent) {
    g_[antecedent].push_back(consequent);
}

void Grammar::SetAxiom(symbol_table::TokenID axiom) {
    axiom_ = axiom;
}

std::vector<std::pair<symbol_table::TokenID, production>>
Grammar::FilterRulesByConsequent(symbol_table::TokenID arg) {
    std::vector<std::pair<symbol_table::TokenID, production>> rules;
    for (const auto& rule : g_) {
        for (const production& prod : rule.second) {
            if (std::find(prod.cbegin(), prod.cend(), arg) != prod.cend()) {
                rules.emplace_back(rule.first, prod);
            }
        }
    }
    return rules;
}

void Grammar::Debug() {
    std::cout << "Grammar:\n";

    for (symbol_table::TokenID nt : nt_order_) {
        std::cout << symbol_table::ToString(nt) + " -> ";
        const auto& productions = g_.at(nt);
        for (size_t i = 0; i < productions.size(); ++i) {
            for (symbol_table::TokenID symbol : productions[i]) {
                std::cout << symbol_table::ToString(symbol) << " ";
            }
            if (i < productions.size() - 1) {
                std::cout << "| ";
            }
        }
        std::cout << "\n";
    }
}
