#include <algorithm>
#include <cstddef>
#include <iostream>
#include <ranges>
#include <span>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "../include/grammar.hpp"
#include "../include/grammar_error.hpp"
#include "../include/lexer.hpp"
#include "../include/ll1_parser.hpp"
#include "../include/symbol_table.hpp"
#include "../include/tabulate.hpp"

LL1Parser::LL1Parser(Grammar gr, std::string text_file, bool table_format)
    : gr_(std::move(gr)), text_file_(std::move(text_file)),
      print_table_format_(table_format) {
    if (!CreateLL1Table()) {
        gr_.Debug();
        PrintTable();
        throw GrammarError("Grammar provided is not LL1.");
    }
}

LL1Parser::LL1Parser(const std::string& grammar_file, std::string text_file,
                     bool table_format)
    : gr_(grammar_file), text_file_(std::move(text_file)),
      print_table_format_(table_format) {
    if (!CreateLL1Table()) {
        gr_.Debug();
        PrintTable();
        throw GrammarError("Grammar provided is not LL1.");
    }
}

LL1Parser::LL1Parser(const std::string& grammar_file, bool table_format)
    : gr_(grammar_file), print_table_format_(table_format) {
    if (!CreateLL1Table()) {
        gr_.Debug();
        PrintTable();
        throw GrammarError("Grammar provided is not LL1.");
    }
}

bool LL1Parser::CreateLL1Table() {
    ComputFirstSets();
    size_t nrows{gr_.g_.size()};
    ll1_t_.reserve(nrows);
    bool has_conflict{false};
    for (const auto& rule : gr_.g_) {
        std::unordered_map<std::string, std::vector<production>> column;
        for (const production& p : rule.second) {
            std::unordered_set<std::string> ds =
                PredictionSymbols(rule.first, p);
            column.reserve(ds.size());
            for (const std::string& symbol : ds) {
                auto& cell = column[symbol];
                if (!cell.empty()) {
                    has_conflict = true;
                }
                cell.push_back(p);
            }
        }
        ll1_t_.insert({rule.first, column});
    }
    return !has_conflict;
}

void LL1Parser::PrintStackTrace() {
    std::cout << "Parser stack trace : [ ";
    while (!symbol_stack_.empty()) {
        std::cout << symbol_stack_.top() << " ";
        symbol_stack_.pop();
    }
    std::cout << "]\n";
}

void LL1Parser::PrintSymbolHist() {
    std::cout << "Last 5 processed symbols : [ ";
    while (!trace_.empty()) {
        std::cout << trace_.front() << " ";
        trace_.pop_front();
    }
    std::cout << "]\n";
}

bool LL1Parser::Parse() {
    Lex lex(text_file_);
    symbol_stack_.push(gr_.axiom_);
    std::string current_symbol = lex.Next();
    while (!current_symbol.empty() && !symbol_stack_.empty()) {
        if (symbol_stack_.top() == symbol_table::EPSILON_) {
            symbol_stack_.pop();
            continue;
        }
        const std::string& top_symbol = symbol_stack_.top();
        symbol_stack_.pop();
        if (!symbol_table::IsTerminal(top_symbol)) {
            if (ll1_t_.find(top_symbol) != ll1_t_.end() &&
                ll1_t_[top_symbol].find(current_symbol) !=
                    ll1_t_[top_symbol].end()) {
                const production& d_symbols =
                    ll1_t_.at(top_symbol).at(current_symbol)[0];
                for (auto& d : std::ranges::reverse_view(d_symbols)) {
                    symbol_stack_.push(d);
                }
            } else {
                if (gr_.HasEmptyProduction(top_symbol)) {
                    continue;
                }
                return false;
            }
        } else {
            trace_.push_back(current_symbol);
            if (trace_.size() > kTraceSize) {
                trace_.pop_front();
            }
            if (top_symbol != current_symbol) {
                return false;
            }
            current_symbol = lex.Next();
        }
    }
    return true;
}

void LL1Parser::First(std::span<const std::string>     rule,
                      std::unordered_set<std::string>& result) {
    if (rule.size() == 1 && rule[0] == symbol_table::EPSILON_) {
        result.insert(symbol_table::EPSILON_);
    }
    std::unordered_set<std::string> ret;
    size_t                          i{0};
    for (const std::string& symbol : rule) {
        if (symbol_table::IsTerminal(symbol)) {
            result.insert(symbol);
            break;
        } else {
            const std::unordered_set<std::string>& fi = first_sets[symbol];
            result.insert(fi.begin(), fi.end());
            result.erase(symbol_table::EPSILON_);
            if (fi.find(symbol_table::EPSILON_) == fi.cend()) {
                break;
            }
            ++i;
        }
    }

    if (i == rule.size()) {
        result.insert(symbol_table::EPSILON_);
    }
}

void LL1Parser::ComputFirstSets() {
    for (const auto& rule : gr_.g_) {
        first_sets[rule.first] = {};
    }
    bool changed{true};
    while (changed) {
        changed = false;
        for (const auto& rule : gr_.g_) {
            const std::string& nonTerminal = rule.first;
            std::size_t        beforeSize  = first_sets[nonTerminal].size();
            for (const auto& prod : rule.second) {
                First(prod, first_sets[nonTerminal]);
            }
            if (first_sets[nonTerminal].size() > beforeSize) {
                changed = true;
            }
        }
    }
    first_sets[gr_.axiom_].erase(symbol_table::EOL_);
}

std::unordered_set<std::string> LL1Parser::Follow(const std::string& arg) {
    std::unordered_set<std::string> next_symbols;
    std::unordered_set<std::string> visited;
    if (arg == gr_.axiom_) {
        return {symbol_table::EOL_};
    }
    FollowUtil(arg, visited, next_symbols);
    if (next_symbols.find(symbol_table::EPSILON_) != next_symbols.end()) {
        next_symbols.erase(symbol_table::EPSILON_);
    }
    return next_symbols;
}

std::unordered_set<std::string>
LL1Parser::PredictionSymbols(const std::string&              antecedent,
                             const std::vector<std::string>& consequent) {
    std::unordered_set<std::string> hd{};
    First({consequent}, hd);
    if (hd.find(symbol_table::EPSILON_) == hd.end()) {
        return hd;
    }
    hd.erase(symbol_table::EPSILON_);
    hd.merge(Follow(antecedent));
    return hd;
}

void LL1Parser::PrintTable() {
    if (print_table_format_) {
        PrintTableUsingTabulate();
        return;
    }
    for (const auto& outerPair : ll1_t_) {
        const std::string& nonTerminal = outerPair.first;
        std::cout << "Non-terminal: " << nonTerminal << "\n";

        for (const auto& innerPair : outerPair.second) {
            const std::string& symbol      = innerPair.first;
            const auto&        productions = innerPair.second;

            std::cout << "\tSymbol: " << symbol << " -> { ";
            for (const auto& prod : productions) {
                std::cout << "[ ";
                for (const std::string& elem : prod) {
                    std::cout << elem << " ";
                }
                std::cout << "] ";
            }
            std::cout << "}\n";
        }
        std::cout << "\n";
    }
}

void LL1Parser::PrintTableUsingTabulate() {
    using namespace tabulate;
    Table table;

    Table::Row_t                          headers = {"Non-terminal"};
    std::unordered_map<std::string, bool> columns;

    for (const auto& outerPair : ll1_t_) {
        for (const auto& innerPair : outerPair.second) {
            columns[innerPair.first] = true;
        }
    }

    for (const auto& col : columns) {
        headers.push_back(col.first);
    }

    auto& header_row = table.add_row(headers);
    header_row.format()
        .font_align(FontAlign::center)
        .font_color(Color::yellow)
        .font_style({FontStyle::bold});

    for (const auto& outerPair : ll1_t_) {
        const std::string& nonTerminal = outerPair.first;
        Table::Row_t       row_data    = {nonTerminal};

        for (const auto& col : columns) {
            auto innerIt = outerPair.second.find(col.first);
            if (innerIt != outerPair.second.end()) {
                std::string cell_content;
                for (const auto& prod : innerIt->second) {
                    cell_content += "[ ";
                    for (const std::string& elem : prod) {
                        cell_content += elem + " ";
                    }
                    cell_content += "] ";
                }
                row_data.push_back(cell_content);
            } else {
                row_data.push_back("-");
            }
        }
        auto& tb = table.add_row(row_data);
    }

    table[0].format().font_color(Color::cyan).font_style({FontStyle::bold});
    for (size_t i = 1; i < table.size(); ++i) {
        for (size_t j = 1; j < table[i].size(); ++j) {
            if (table[i][j].get_text().find("] [") != std::string::npos) {
                table[i][j].format().font_color(Color::red);
            }
        }
    }
    table.format().font_align(FontAlign::center);
    table.column(0).format().font_color(Color::cyan);
    std::cout << table << std::endl;
}

void LL1Parser::FollowUtil(const std::string&               arg,
                           std::unordered_set<std::string>& visited,
                           std::unordered_set<std::string>& next_symbols) {
    if (visited.find(arg) != visited.cend()) {
        return;
    }
    visited.insert(arg);
    std::vector<std::pair<const std::string, production>> rules{
        gr_.FilterRulesByConsequent(arg)};
    for (const std::pair<const std::string, production>& rule : rules) {
        // Next must be applied to all Arg symbols, for example
        // if arg: B; A -> BbBCB, next is applied three times
        auto it = rule.second.cbegin();
        while ((it = std::find(it, rule.second.cend(), arg)) !=
               rule.second.cend()) {
            auto next_it = std::next(it);
            if (next_it == rule.second.cend()) {
                FollowUtil(rule.first, visited, next_symbols);
            } else {
                First(std::span<const std::string>(next_it, rule.second.cend()),
                      next_symbols);
                if (next_symbols.find(symbol_table::EPSILON_) !=
                    next_symbols.end()) {
                    next_symbols.erase(symbol_table::EPSILON_);
                    FollowUtil(rule.first, visited, next_symbols);
                }
            }
            it = std::next(it);
        }
    }
}
