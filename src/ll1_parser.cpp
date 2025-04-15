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
    ComputeFirstSets();
    ComputeFollowSets();

    size_t nrows{gr_.g_.size()};
    ll1_t_.reserve(nrows);
    bool has_conflict{false};
    for (const auto& [lhs, productions] : gr_.g_) {
        std::unordered_map<std::string, std::vector<production>> column;
        for (const production& p : productions) {
            std::unordered_set<std::string> ds =
                PredictionSymbols(lhs, p);
            column.reserve(ds.size());
            for (const std::string& symbol : ds) {
                auto& cell = column[symbol];
                if (!cell.empty()) {
                    has_conflict = true;
                }
                cell.push_back(p);
            }
        }
        ll1_t_.emplace(lhs, std::move(column));
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

bool LL1Parser::MatchTerminal(const std::string& top_symbol,
                              const std::string& current_symbol) {
    trace_.push_back(current_symbol);
    if (trace_.size() > kTraceSize) {
        trace_.pop_front();
    }

    return top_symbol == current_symbol;
}

bool LL1Parser::ProcessNonTerminal(const std::string& top_symbol,
                                   const std::string& current_symbol) {
    auto it = ll1_t_.find(top_symbol);
    if (it != ll1_t_.end()) {
        auto prod_it = it->second.find(current_symbol);
        if (prod_it != it->second.end()) {
            const production& d_symbols = prod_it->second[0];
            for (auto& d : std::ranges::reverse_view(d_symbols)) {
                symbol_stack_.push(d);
            }
            return true;
        }
    }
    return gr_.HasEmptyProduction(top_symbol);
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
        if (symbol_table::IsTerminal(top_symbol)) {
            if (!MatchTerminal(top_symbol, current_symbol))
                return false;
            current_symbol = lex.Next();

        } else {
            if (!ProcessNonTerminal(top_symbol, current_symbol))
                return false;
        }
    }
    return true;
}

void LL1Parser::First(std::span<const std::string>     rule,
                      std::unordered_set<std::string>& result) {
    if (rule.empty() ||
        (rule.size() == 1 && rule[0] == symbol_table::EPSILON_)) {
        result.insert(symbol_table::EPSILON_);
        return;
    }

    bool allEpsilon = true;

    if (symbol_table::IsTerminal(rule[0])) {
        result.insert(rule[0]);
        return;
    }

    const std::unordered_set<std::string>& fii = first_sets_[rule[0]];
    for (const auto& s : fii) {
        if (s != symbol_table::EPSILON_) {
            result.insert(s);
        }
    }
    if (fii.find(symbol_table::EPSILON_) == fii.cend()) {
        return;
    }
    First(std::span<const std::string>(rule.begin() + 1, rule.end()), result);
}

void LL1Parser::ComputeFirstSets() {
    // Initialize FIRST sets for each non-terminal
    for (const auto& [nonTerminal, _] : gr_.g_) {
        first_sets_[nonTerminal] = {};
    }

    bool changed;
    do {
        auto old_first_sets = first_sets_; // Copy current state

        for (const auto& [nonTerminal, productions] : gr_.g_) {
            for (const auto& prod : productions) {
                std::unordered_set<std::string> tempFirst;
                First(prod, tempFirst);

                if (tempFirst.find(symbol_table::EOL_) != tempFirst.end()) {
                    tempFirst.erase(symbol_table::EOL_);
                    tempFirst.insert(symbol_table::EPSILON_);
                }
                // Insert the computed FIRST into the non-terminal's set
                auto& current_set = first_sets_[nonTerminal];
                current_set.insert(tempFirst.begin(), tempFirst.end());
            }
        }

        // Check if any changes occurred
        changed = (old_first_sets != first_sets_);

    } while (changed);
}

void LL1Parser::ComputeFollowSets() {
    for (const auto& [nt, _] : gr_.g_) {
        follow_sets_[nt] = {};
    }
    follow_sets_[gr_.axiom_].insert(symbol_table::EOL_);

    bool changed;
    do {
        changed = false;
        for (const auto& rule : gr_.g_) {
            const std::string& lhs = rule.first;
            for (const production& rhs : rule.second) {
                for (size_t i = 0; i < rhs.size(); ++i) {
                    const std::string& symbol = rhs[i];
                    if (!symbol_table::IsTerminal(symbol)) {
                        changed |= UpdateFollow(symbol, lhs, rhs, i);
                    }
                }
            }
        }
    } while (changed);
}

bool LL1Parser::UpdateFollow(const std::string& symbol, const std::string& lhs,
                             const production& rhs, size_t i) {
    bool changed = false;

    std::unordered_set<std::string> first_remaining;
    if (i + 1 < rhs.size()) {
        First(std::span<const std::string>(rhs.begin() + i + 1, rhs.end()),
              first_remaining);
    } else {
        first_remaining.insert(symbol_table::EPSILON_);
    }

    // Add FIRST(β) \ {ε}
    for (const auto& terminal : first_remaining) {
        if (terminal != symbol_table::EPSILON_) {
            changed |= follow_sets_[symbol].insert(terminal).second;
        }
    }

    // If FIRST(β) contains ε, add FOLLOW(lhs)
    if (first_remaining.contains(symbol_table::EPSILON_)) {
        for (const auto& terminal : follow_sets_[lhs]) {
            changed |= follow_sets_[symbol].insert(terminal).second;
        }
    }

    return changed;
}

std::unordered_set<std::string> LL1Parser::Follow(const std::string& arg) {
    auto it = follow_sets_.find(arg);
    if (it != follow_sets_.end()) {
        return it->second;
    }
    return {};
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

    std::vector<std::string> non_terminals;
    for (const auto& outerPair : ll1_t_) {
        non_terminals.push_back(outerPair.first);
    }

    std::sort(non_terminals.begin(), non_terminals.end(),
              [this](const std::string& a, const std::string& b) {
                  if (a == gr_.axiom_)
                      return true; // Axiom comes first
                  if (b == gr_.axiom_)
                      return false; // Axiom comes first
                  return a < b;     // Sort the rest alphabetically
              });

    for (const std::string& nonTerminal : non_terminals) {
        Table::Row_t row_data = {nonTerminal};

        for (const auto& col : columns) {
            auto innerIt = ll1_t_.at(nonTerminal).find(col.first);
            if (innerIt != ll1_t_.at(nonTerminal).end()) {
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

        table.add_row(row_data);
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

    // Print the table
    std::cout << table << std::endl;
}