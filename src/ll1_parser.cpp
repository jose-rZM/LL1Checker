#include "ll1_parser.hpp"
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
        std::cerr << "Grammar provided is not LL1. Aborting...\n";
        gr_.debug();
        exit(-1);
    }
    std::cout << "Grammar is LL1\n";
}

LL1Parser::LL1Parser(const std::string& grammar_file,
                     std::string text_file)
    : gr_(grammar_file), text_file_(std::move(text_file)) {
    if (!create_ll1_table()) {
        std::cerr << "Grammar provided is not LL1. Aborting...\n";
        gr_.debug();
        exit(-1);
    }
    std::cout << "Grammar is LL1\n";
}

LL1Parser::LL1Parser(const std::string& grammar_file) : gr_(grammar_file) {
    if (!create_ll1_table()) {
        std::cerr << "Grammar provided is not LL1. Aborting...\n";
        gr_.debug();
        exit(-1);
    }
    std::cout << "Grammar is LL1\n";
}

/**
 *
 * @return true if the ll1 table could be created, that is, the grammar is LL1
 */
bool LL1Parser::create_ll1_table() {
    for (const std::pair<const std::string, std::vector<production>>& rule : gr_.g_) {
        std::unordered_map<std::string, std::vector<std::string>> entry;
        for (const production& p : rule.second) {
            std::unordered_set<std::string> ds =
                director_symbols(rule.first, p);
            for (const std::string &str : ds) {
                if (!entry.insert({str, p}).second) {
                    return false;
                }
            }
        }
        ll1_t_.insert({rule.first, entry});
    }
    return true;
}

/**
 *
 * @return true if the parsing is successfully completed
 * Parses an input file using LL1 algorithm
 */
bool LL1Parser::parse() {
    lexer lex(text_file_);
    std::stack<std::string> st;
    st.push(gr_.AXIOM_);
    std::string l = lex.next();
    while (!l.empty() && !st.empty()) {
        if (st.top() == symbol_table::EPSILON) {
            st.pop();
            continue;
        }
        std::vector<std::string> ds;
        std::string s = st.top();
        st.pop();
        if (!symbol_table::is_terminal(s)) {
            try {
                ds = ll1_t_.at(s).at(l);
            } catch (const std::out_of_range &exc) {
                // check if this rule has an empty production
                if (gr_.has_empty_production(s)) {
                    continue;
                }
                return false;
            }
            for (auto &d : std::ranges::reverse_view(ds)) {
                st.push(d);
            }
        } else {
            if (s != l) {
                return false;
            }
            l = lex.next();
        }
    }
    return true;
}

/**
 *
 * @param rule
 * @return set header symbols for the given rule
 */
std::unordered_set<std::string>
LL1Parser::header(const std::vector<std::string> &rule) {
    std::unordered_set<std::string> hd;
    std::stack<std::vector<std::string>> st;
    st.push(rule);

    while (!st.empty()) {
        std::vector<std::string> current = st.top();
        st.pop();
        if (current[0] == symbol_table::EPSILON) {
            current.erase(current.begin());
        }
        if (current.empty()) {
            hd.insert(symbol_table::EPSILON);
        } else if (symbol_table::is_terminal(current[0])) {
            hd.insert(current[0]);
        } else {
            for (const std::vector<std::string> &prod : gr_.g_.at(current[0])) {
                std::vector<std::string> production;
                for (const std::string &sy : prod) {
                    production.push_back(sy);
                }
                for (unsigned i = 1; i < current.size(); ++i) {
                    production.push_back(current[i]);
                }
                st.push(production);
            }
        }
    }

    return hd;
}

/**
 *
 * @param arg symbol to calculate next symbols for
 * @return Set of next symbols for the given arg
 */
std::unordered_set<std::string> LL1Parser::next(const std::string &arg) {
    std::unordered_set<std::string> next_symbols;
    std::unordered_set<std::string> visited;
    next_util(arg, visited, next_symbols);
    if (next_symbols.find(symbol_table::EPSILON) != next_symbols.end()) {
        next_symbols.erase(symbol_table::EPSILON);
    }
    return next_symbols;
}

/**
 *
 * @param antecedent of a rule
 * @param consequent of a rule
 * @return set of director symbols for the given rule
 */
std::unordered_set<std::string>
LL1Parser::director_symbols(const std::string &antecedent,
                            const std::vector<std::string> &consequent) {
    std::unordered_set<std::string> hd{header({consequent})};
    if (hd.find(symbol_table::EPSILON) == hd.end()) {
        return hd;
    } else {
        hd.erase(symbol_table::EPSILON);
        hd.merge(next(antecedent));
        return hd;
    }
}

/**
 *
 * @param arg
 * @param visited symbols (avoid infinite recursion)
 * @param next_symbols next symbols accumulated
 */
void LL1Parser::next_util(const std::string &arg,
                          std::unordered_set<std::string> &visited,
                          std::unordered_set<std::string> &next_symbols) {
    if (visited.find(arg) != visited.cend()) {
        return;
    }
    visited.insert(arg);
    std::vector<std::pair<const std::string, production>> rules {
        gr_.filterRulesByConsequent(arg) };

    for (const std::pair<const std::string, production> &rule : rules) {
        auto it = rule.second.cbegin();
        while ((it = std::find(it, rule.second.cend(), arg)) !=
               rule.second.cend()) {
            auto nit = std::next(it);
            if (nit == rule.second.cend()) {
                next_util(rule.first, visited, next_symbols);
            } else {
                next_symbols.merge(header({*nit}));
            }
            it = std::next(it);
        }
    }
}

