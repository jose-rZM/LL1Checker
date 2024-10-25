#include "ll1_parser.hpp"
#include "errors/grammar_error.hpp"
#include "grammar.hpp"
#include "lexer.hpp"
#include "symbol_table.hpp"
#include <iostream>
#include <ranges>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

LL1Parser::LL1Parser(grammar gr, std::string text_file)
    : gr_(std::move(gr)), text_file_(std::move(text_file)) {
    if (!create_ll1_table()) {
        gr_.debug();
        throw GrammarError("Grammar provided is not LL1.");
    }
}

LL1Parser::LL1Parser(const std::string& grammar_file, std::string text_file)
    : gr_(grammar_file), text_file_(std::move(text_file)) {
    if (!create_ll1_table()) {
        gr_.debug();
        throw GrammarError("Grammar provided is not LL1.");
    }
}

LL1Parser::LL1Parser(const std::string& grammar_file) : gr_(grammar_file) {
    if (!create_ll1_table()) {
        gr_.debug();
        throw GrammarError("Grammar provided is not LL1.");
    }
}

bool LL1Parser::create_ll1_table() {
    for (const std::pair<const std::string, std::vector<production>>& rule :
         gr_.g_) {
        std::unordered_map<std::string, std::vector<std::string>> column;
        for (const production& p : rule.second) {
            std::unordered_set<std::string> ds =
                director_symbols(rule.first, p);
            for (const std::string& symbol : ds) {
                if (!column.insert({symbol, p}).second) {
                    return false;
                }
            }
        }
        ll1_t_.insert({rule.first, column});
    }
    return true;
}

bool LL1Parser::parse() {
    lexer lex(text_file_);

    std::stack<std::string> symbol_stack;
    symbol_stack.push(gr_.AXIOM_);
    std::string current_symbol = lex.next();
    while (!current_symbol.empty() && !symbol_stack.empty()) {
        if (symbol_stack.top() == symbol_table::EPSILON_) {
            symbol_stack.pop();
            continue;
        }
        std::vector<std::string> d_symbols;
        std::string              top_symbol = symbol_stack.top();
        symbol_stack.pop();
        if (!symbol_table::is_terminal(top_symbol)) {
            try {
                d_symbols = ll1_t_.at(top_symbol).at(current_symbol);
            } catch (const std::out_of_range& exc) {
                // check if this rule has an empty production
                if (gr_.has_empty_production(top_symbol)) {
                    continue;
                }
                return false;
            }
            for (auto& d : std::ranges::reverse_view(d_symbols)) {
                symbol_stack.push(d);
            }
        } else {
            if (top_symbol != current_symbol) {
                return false;
            }
            current_symbol = lex.next();
        }
    }

    return true;
}

std::unordered_set<std::string>
LL1Parser::header(const std::vector<std::string>& rule) {
    std::unordered_set<std::string>      current_header;
    std::stack<std::vector<std::string>> symbol_stack;
    std::unordered_set<std::string>      visited;
    symbol_stack.push(rule);

    while (!symbol_stack.empty()) {
        std::vector<std::string> current = symbol_stack.top();
        symbol_stack.pop();
        if (current[0] == symbol_table::EPSILON_) {
            current.erase(current.begin());
        }
        if (current.empty()) {
            current_header.insert(symbol_table::EPSILON_);
        } else if (symbol_table::is_terminal(current[0])) {
            current_header.insert(current[0]);
        } else {
            if (visited.find(current[0]) == visited.end()) {
                visited.insert(current[0]);

                for (const std::vector<std::string>& prod :
                     gr_.g_.at(current[0])) {
                    std::vector<std::string> production;
                    for (const std::string& sy : prod) {
                        production.push_back(sy);
                    }
                    // Add remaining symbols
                    for (unsigned i = 1; i < current.size(); ++i) {
                        production.push_back(current[i]);
                    }
                    symbol_stack.push(production);
                }
            }
        }
    }

    return current_header;
}

std::unordered_set<std::string> LL1Parser::next(const std::string& arg) {
    std::unordered_set<std::string> next_symbols;
    std::unordered_set<std::string> visited;
    next_util(arg, visited, next_symbols);
    if (next_symbols.find(symbol_table::EPSILON_) != next_symbols.end()) {
        next_symbols.erase(symbol_table::EPSILON_);
    }
    return next_symbols;
}

std::unordered_set<std::string>
LL1Parser::director_symbols(const std::string&              antecedent,
                            const std::vector<std::string>& consequent) {
    std::unordered_set<std::string> hd{header({consequent})};
    if (hd.find(symbol_table::EPSILON_) == hd.end()) {
        return hd;
    } else {
        hd.erase(symbol_table::EPSILON_);
        hd.merge(next(antecedent));
        return hd;
    }
}

void LL1Parser::print_table() {
    for (const auto& outerPair : ll1_t_) {
        const std::string& nonTerminal = outerPair.first;
        std::cout << "Non-terminal: " << nonTerminal << "\n";
        for (const auto& innerPair : outerPair.second) {
            const std::string& symbol = innerPair.first;
            const production&  prod   = innerPair.second;
            std::cout << "\tSymbol: " << symbol << " -> { ";
            for (const std::string& elem : prod) {
                std::cout << elem << " ";
            }
            std::cout << "}\n";
        }
        std::cout << std::endl;
    }
}

void LL1Parser::next_util(const std::string&               arg,
                          std::unordered_set<std::string>& visited,
                          std::unordered_set<std::string>& next_symbols) {
    if (visited.find(arg) != visited.cend()) {
        return;
    }
    visited.insert(arg);
    std::vector<std::pair<const std::string, production>> rules{
        gr_.filter_rules_by_consequent(arg)};

    for (const std::pair<const std::string, production>& rule : rules) {
        // Next must be applied to all Arg symbols, for example
        // if arg: B; A -> BbBCB, next is applied three times
        auto it = rule.second.cbegin();
        while ((it = std::find(it, rule.second.cend(), arg)) !=
               rule.second.cend()) {
            auto next_it = std::next(it);
            if (next_it == rule.second.cend()) {
                next_util(rule.first, visited, next_symbols);
            } else {
                next_symbols.merge(header(
                    std::vector<std::string>(next_it, rule.second.cend())));
                if (next_symbols.find(symbol_table::EPSILON_) !=
                    next_symbols.end()) {
                    next_symbols.erase(symbol_table::EPSILON_);
                    next_util(rule.first, visited, next_symbols);
                }
            }
            it = std::next(it);
        }
    }
}
