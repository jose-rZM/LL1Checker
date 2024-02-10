#include "ll1_parser.hpp"
#include "grammar.hpp"
#include "lexer.hpp"
#include "symbol_table.hpp"
#include <iostream>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>

LL1Parser::LL1Parser(const grammar &gr, const std::string &text_file)
    : gr_(gr), text_file_(text_file) {
    if (!create_ll1_table()) {
        std::cerr << "Grammar provided is not LL1. Aborting...\n";
        gr_.debug();
        exit(-1);
    }
}

LL1Parser::LL1Parser(const std::string &grammar_file,
                     const std::string &text_file)
    : gr_(grammar_file), text_file_(text_file) {
    if (!create_ll1_table()) {
        std::cerr << "Grammar provided is not LL1. Aborting...\n";
        gr_.debug();
        exit(-1);
    }
    std::cout << "Grammar is LL1\n";
}

bool LL1Parser::create_ll1_table() {
    for (std::pair<const std::string, std::vector<production>> rule : gr_.g) {
        std::unordered_map<std::string, std::vector<std::string>> entry;
        for (production p : rule.second) {
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

bool LL1Parser::parse() {
    lexer lex(text_file_);
    std::stack<std::string> st;
    st.push(gr_.AXIOM);
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
            for (std::vector<std::string>::reverse_iterator it = ds.rbegin();
                 it != ds.rend(); it++) {
                st.push(*it);
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
        if (current.size() == 0) {
            hd.insert(symbol_table::EPSILON);
        } else if (symbol_table::is_terminal(current[0])) {
            hd.insert(current[0]);
        } else {
            for (const std::vector<std::string> &prod : gr_.g.at(current[0])) {
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

std::unordered_set<std::string> LL1Parser::next(const std::string &arg) {
    std::unordered_set<std::string> next_symbols;
    std::unordered_set<std::string> visited;
    next_util(arg, visited, next_symbols);
    if (next_symbols.find(symbol_table::EPSILON) != next_symbols.end()) {
        next_symbols.erase(symbol_table::EPSILON);
    }
    return next_symbols;
}

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

void LL1Parser::next_util(const std::string &arg,
                          std::unordered_set<std::string> &visited,
                          std::unordered_set<std::string> &next_symbols) {
    if (visited.find(arg) != visited.cend()) {
        return;
    }
    visited.insert(arg);
    std::vector<std::pair<const std::string, production>> rules;
    // find rules with arg as part of consequence
    for (std::pair<const std::string, std::vector<production>> rule : gr_.g) {
        for (production prod : rule.second) {
            if (std::find(prod.cbegin(), prod.cend(), arg) != prod.cend()) {
                rules.push_back({rule.first, prod});
            }
        }
    }

    for (std::pair<const std::string, production> rule : rules) {
        production::const_iterator it = rule.second.begin();
        while ((it = std::find(it, rule.second.cend(), arg)) !=
               rule.second.cend()) {
            production::const_iterator nit = std::next(it);
            if (nit == rule.second.cend()) {
                next_util(rule.first, visited, next_symbols);
            } else {
                next_symbols.merge(header({*nit}));
            }
            it = std::next(it);
        }
    }
}
