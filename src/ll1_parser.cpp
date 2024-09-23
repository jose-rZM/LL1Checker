#include "ll1_parser.hpp"
#include "errors/grammar_error.hpp"
#include "grammar.hpp"
#include "lexer.hpp"
#include "symbol_table.hpp"
#include <algorithm>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

LL1Parser::LL1Parser(grammar gr, std::string text_file)
    : gr_(std::move(gr)), text_file_(std::move(text_file)) {
        gr_.debug();
    if (!create_ll1_table()) {
        gr_.debug();
        throw GrammarError("Grammar provided is not LL1.");
    }
}

LL1Parser::LL1Parser(const std::string &grammar_file, std::string text_file)
    : gr_(grammar_file), text_file_(std::move(text_file)) {
    if (!create_ll1_table()) {
        gr_.debug();
        throw GrammarError("Grammar provided is not LL1.");
    }
}

LL1Parser::LL1Parser(const std::string &grammar_file) : gr_(grammar_file) {
    if (!create_ll1_table()) {
        gr_.debug();
        throw GrammarError("Grammar provided is not LL1.");
    }
}

bool LL1Parser::create_ll1_table() {
    for (const auto &rule : gr_.g_) {
        std::unordered_map<int, std::vector<int>> column;
        for (const production &p : rule.second) {
            std::unordered_set<int> ds =
                director_symbols(rule.first, p);
            for (int symbol : ds) {
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

    std::stack<int> symbol_stack;
    symbol_stack.push(symbol_table::token_types_[gr_.AXIOM_]);
    int current_symbol = lex.next();
    while (current_symbol != 0 && !symbol_stack.empty()) {
        if (symbol_stack.top() == 2) {
            symbol_stack.pop();
            continue;
        }
        std::vector<int> d_symbols;
        int top_symbol = symbol_stack.top();
        symbol_stack.pop();
        if (!symbol_table::is_terminal(top_symbol)) {
            try {
                d_symbols = ll1_t_.at(top_symbol).at(current_symbol);
            } catch (const std::out_of_range &exc) {
                // check if this rule has an empty production
                if (gr_.has_empty_production(top_symbol)) {
                    continue;
                }
                return false;
            }
            for (auto it = d_symbols.rbegin(); it != d_symbols.rend(); ++it) {
                symbol_stack.push(*it);
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

std::unordered_set<int>
LL1Parser::header(const std::vector<int> &rule) {
    std::unordered_set<int> current_header;
    std::stack<std::vector<int>> symbol_stack;
    symbol_stack.push(rule);

    while (!symbol_stack.empty()) {
        std::vector<int> current = symbol_stack.top();
        symbol_stack.pop();
        if (current[0] == 2) {
            current.erase(current.begin());
        }
        if (current.empty()) {
            current_header.insert(2);
        } else if (symbol_table::is_terminal(current[0])) {
            current_header.insert(current[0]);
        } else {
            for (const std::vector<int> &prod : gr_.g_.at(current[0])) {
                std::vector<int> production;
                for (int sy : prod) {
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

    return current_header;
}

std::unordered_set<int> LL1Parser::next(int arg) {
    std::unordered_set<int> next_symbols;
    std::unordered_set<int> visited;
    next_util(arg, visited, next_symbols);
    if (next_symbols.find(2) != next_symbols.end()) {
        next_symbols.erase(2);
    }
    return next_symbols;
}

std::unordered_set<int>
LL1Parser::director_symbols(int antecedent,
                            const std::vector<int> &consequent) {
    std::unordered_set<int> hd{header({consequent})};
    if (hd.find(2) == hd.end()) {
        return hd;
    } else {
        hd.erase(2);
        hd.merge(next(antecedent));
        return hd;
    }
}

void LL1Parser::next_util(int arg,
                          std::unordered_set<int> &visited,
                          std::unordered_set<int> &next_symbols) {
    if (visited.find(arg) != visited.cend()) {
        return;
    }
    visited.insert(arg);
    std::vector<std::pair<int, production>> rules{
        gr_.filter_rules_by_consequent(arg)};

    for (const auto &rule : rules) {
        // Next must be applied to all Arg symbols, for example
        // if arg: B; A -> BbBCB, next is applied three times
        auto it = rule.second.cbegin();
        while ((it = std::find(it, rule.second.cend(), arg)) !=
               rule.second.cend()) {
            auto next_it = std::next(it);
            if (next_it == rule.second.cend()) {
                next_util(rule.first, visited, next_symbols);
            } else {
                next_symbols.merge(header({*next_it}));
            }
            it = std::next(it);
        }
    }
}
