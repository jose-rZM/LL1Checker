#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <ranges>
#include <span>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "grammar.hpp"
#include "grammar_error.hpp"
#include "lexer.hpp"
#include "ll1_parser.hpp"
#include "symbol_table.hpp"
#include "tabulate.hpp"

LL1Parser::LL1Parser(Grammar gr, std::string text_file, bool table_format,
                     bool text_is_raw)
    : gr_(std::move(gr)), text_file_(std::move(text_file)),
      text_is_raw_(text_is_raw), print_table_format_(table_format) {
    if (!CreateLL1Table()) {
        gr_.Debug();
        PrintTable();
        throw GrammarError("Provided grammar is not LL(1).");
    }
}

LL1Parser::LL1Parser(const std::string& grammar_file, std::string text_file,
                     bool table_format, bool text_is_raw)
    : gr_(grammar_file), text_file_(std::move(text_file)),
      text_is_raw_(text_is_raw), print_table_format_(table_format) {
    if (!CreateLL1Table()) {
        gr_.Debug();
        PrintTable();
        throw GrammarError("Provided grammar is not LL(1).");
    }
}

bool LL1Parser::CreateLL1Table() {
    ComputeFirstSets();
    ComputeFollowSets();

    size_t nrows{gr_.g_.size()};
    ll1_t_.reserve(nrows);
    bool has_conflict{false};
    for (const auto& rule : gr_.g_) {
        std::unordered_map<symbol_table::TokenID, std::vector<production>>
            column;
        for (const production& p : rule.second) {
            std::unordered_set<symbol_table::TokenID> ds =
                PredictionSymbols(rule.first, p);
            column.reserve(ds.size());
            for (symbol_table::TokenID symbol : ds) {
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

bool LL1Parser::MatchTerminal(symbol_table::TokenID top_symbol,
                              symbol_table::TokenID current_symbol) {
    trace_.push_back(current_symbol);
    if (trace_.size() > kTraceSize) {
        trace_.pop_front();
    }

    return top_symbol == current_symbol;
}

bool LL1Parser::ProcessNonTerminal(symbol_table::TokenID top_symbol,
                                   symbol_table::TokenID current_symbol) {
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
    return false;
}

void LL1Parser::ReportParseError(const std::string& input, size_t err_pos,
                                 symbol_table::TokenID expected,
                                 symbol_table::TokenID found) {
    size_t      line, col;
    std::string snippet = Lex::format_error_window(input, err_pos, line, col);

    std::cerr << "Parse error at line " << (line + 1) << ", column "
              << (col + 1) << ":\n"
              << "  expected `" << symbol_table::ToString(expected)
              << "` but found `" << symbol_table::ToString(found) << "`\n\n"
              << snippet << "\n";
}

bool LL1Parser::Parse() {
    Lex lex(text_file_, text_is_raw_);
    symbol_stack_.push(gr_.axiom_);
    Lex::Token current_symbol = lex.Next();
    while (!symbol_stack_.empty()) {
        if (symbol_stack_.top() == symbol_table::EPSILON_ID) {
            symbol_stack_.pop();
            continue;
        }
        symbol_table::TokenID top_symbol = symbol_stack_.top();
        symbol_stack_.pop();
        if (symbol_table::IsTerminal(top_symbol)) {
            if (!MatchTerminal(top_symbol, current_symbol.type)) {
                ReportParseError(lex.input(), current_symbol.pos, top_symbol,
                                 current_symbol.type);
                return false;
            }
            current_symbol = lex.Next();

        } else {
            if (!ProcessNonTerminal(top_symbol, current_symbol.type)) {
                ReportParseError(lex.input(), current_symbol.pos, top_symbol,
                                 current_symbol.type);
                return false;
            }
        }
    }
    return true;
}

LL1Parser::ParseTree LL1Parser::ParseWithTree(const std::string& file) {
    Lex lex(file, text_is_raw_);
    std::stack<std::pair<symbol_table::TokenID, ParseNode*>> stack;

    ParseTree tree = std::make_unique<ParseNode>();
    tree->symbol   = symbol_table::ToString(gr_.axiom_);
    stack.push({gr_.axiom_, tree.get()});

    Lex::Token current = lex.Next();
    while (!stack.empty()) {
        auto [top_symbol, node] = stack.top();
        stack.pop();

        if (top_symbol == symbol_table::EPSILON_ID) {
            node->symbol = symbol_table::EPSILON_;
            continue;
        }

        node->symbol = symbol_table::ToString(top_symbol);

        if (symbol_table::IsTerminal(top_symbol)) {
            if (!MatchTerminal(top_symbol, current.type)) {
                ReportParseError(lex.input(), current.pos, top_symbol,
                                 current.type);
                return nullptr;
            }
            current = lex.Next();
        } else {
            auto it = ll1_t_.find(top_symbol);
            if (it == ll1_t_.end()) {
                ReportParseError(lex.input(), current.pos, top_symbol,
                                 current.type);
                return nullptr;
            }

            auto prod_it = it->second.find(current.type);
            if (prod_it == it->second.end()) {
                ReportParseError(lex.input(), current.pos, top_symbol,
                                 current.type);
                return nullptr;
            }

            const production& prod = prod_it->second[0];
            std::vector<std::pair<symbol_table::TokenID, ParseNode*>> pushes;
            pushes.reserve(prod.size());

            for (symbol_table::TokenID sym : prod) {
                auto child           = std::make_unique<ParseNode>();
                child->symbol        = symbol_table::ToString(sym);
                ParseNode* child_ptr = child.get();
                node->children.push_back(std::move(child));
                pushes.push_back({sym, child_ptr});
            }

            for (auto itp = pushes.rbegin(); itp != pushes.rend(); ++itp) {
                stack.push(*itp);
            }
        }
    }

    return tree;
}

void LL1Parser::First(std::span<const symbol_table::TokenID>     rule,
                      std::unordered_set<symbol_table::TokenID>& result) {
    if (rule.empty() ||
        (rule.size() == 1 && rule[0] == symbol_table::EPSILON_ID)) {
        result.insert(symbol_table::EPSILON_ID);
        return;
    }

    if (symbol_table::IsTerminal(rule[0])) {
        result.insert(rule[0]);
        return;
    }

    const auto& fii = first_sets_[rule[0]];
    for (const auto& s : fii) {
        if (s != symbol_table::EPSILON_ID) {
            result.insert(s);
        }
    }
    if (!fii.contains(symbol_table::EPSILON_ID)) {
        return;
    }
    First(rule.subspan(1), result);
}

void LL1Parser::ComputeFirstSets() {
    // Initialize FIRST sets for each non-terminal
    for (const auto& [nonTerminal, _] : gr_.g_) {
        first_sets_[nonTerminal] = {};
    }

    bool changed;
    do {
        auto old_first_sets = first_sets_;  // Copy current state

        for (const auto& [nonTerminal, productions] : gr_.g_) {
            for (const auto& prod : productions) {
                std::unordered_set<symbol_table::TokenID> tempFirst;
                First(prod, tempFirst);

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
    follow_sets_[gr_.axiom_].insert(symbol_table::EOF_ID);

    bool changed;
    do {
        changed = false;
        for (const auto& rule : gr_.g_) {
            symbol_table::TokenID lhs = rule.first;
            for (const production& rhs : rule.second) {
                for (size_t i = 0; i < rhs.size(); ++i) {
                    symbol_table::TokenID symbol = rhs[i];
                    if (!symbol_table::IsTerminal(symbol)) {
                        changed |= UpdateFollow(symbol, lhs, rhs, i);
                    }
                }
            }
        }
    } while (changed);
}

bool LL1Parser::UpdateFollow(symbol_table::TokenID symbol,
                             symbol_table::TokenID lhs, const production& rhs,
                             size_t i) {
    bool changed = false;

    std::unordered_set<symbol_table::TokenID> first_remaining;
    if (i + 1 < rhs.size()) {
        First(std::span<const symbol_table::TokenID>(rhs.begin() + i + 1,
                                                     rhs.end()),
              first_remaining);
    } else {
        first_remaining.insert(symbol_table::EPSILON_ID);
    }

    // Add FIRST(β) \ {ε}
    for (const auto& terminal : first_remaining) {
        if (terminal != symbol_table::EPSILON_ID) {
            changed |= follow_sets_[symbol].insert(terminal).second;
        }
    }

    // If FIRST(β) contains ε, add FOLLOW(lhs)
    if (first_remaining.contains(symbol_table::EPSILON_ID)) {
        for (const auto& terminal : follow_sets_[lhs]) {
            changed |= follow_sets_[symbol].insert(terminal).second;
        }
    }

    return changed;
}

std::unordered_set<symbol_table::TokenID>
LL1Parser::Follow(symbol_table::TokenID arg) {
    auto it = follow_sets_.find(arg);
    if (it != follow_sets_.end()) {
        return it->second;
    }
    return {};
}

std::unordered_set<symbol_table::TokenID>
LL1Parser::PredictionSymbols(symbol_table::TokenID antecedent,
                             const production&     consequent) {
    std::unordered_set<symbol_table::TokenID> first{};
    First({consequent}, first);
    if (!first.contains(symbol_table::EPSILON_ID)) {
        return first;
    }
    first.erase(symbol_table::EPSILON_ID);
    first.merge(Follow(antecedent));
    return first;
}

void LL1Parser::PrintTable() {
    if (print_table_format_) {
        PrintTableUsingTabulate();
        return;
    }
    for (const auto& nonTerminal : gr_.nt_order_) {
        auto it = ll1_t_.find(nonTerminal);
        if (it == ll1_t_.end())
            continue;

        const auto& row = it->second;
        std::cout << "Non-terminal: " << symbol_table::ToString(nonTerminal)
                  << "\n";

        size_t maxSymLen = 0;
        for (const auto& innerPair : row) {
            maxSymLen = std::max(
                maxSymLen, symbol_table::ToString(innerPair.first).size());
        }

        for (const auto& innerPair : row) {
            symbol_table::TokenID symbol      = innerPair.first;
            const auto&           productions = innerPair.second;

            std::cout << "\tSymbol: " << std::setw(static_cast<int>(maxSymLen))
                      << std::left << symbol_table::ToString(symbol)
                      << " -> { ";

            for (const auto& prod : productions) {
                std::cout << "[ ";
                for (auto elem : prod) {
                    std::cout << symbol_table::ToString(elem) << " ";
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

    Table::Row_t                                    headers = {"Non-terminal"};
    std::unordered_map<symbol_table::TokenID, bool> columns;

    for (const auto& outerPair : ll1_t_) {
        for (const auto& innerPair : outerPair.second) {
            columns[innerPair.first] = true;
        }
    }

    for (const auto& col : columns) {
        headers.push_back(symbol_table::ToString(col.first));
    }

    auto& header_row = table.add_row(headers);
    header_row.format()
        .font_align(FontAlign::center)
        .font_color(Color::yellow)
        .font_style({FontStyle::bold});

    std::vector<symbol_table::TokenID> non_terminals;
    for (const auto& nt : gr_.nt_order_) {
        non_terminals.push_back(nt);
    }

    for (symbol_table::TokenID nonTerminal : non_terminals) {
        Table::Row_t row_data = {symbol_table::ToString(nonTerminal)};

        for (const auto& col : columns) {
            auto innerIt = ll1_t_.at(nonTerminal).find(col.first);
            if (innerIt != ll1_t_.at(nonTerminal).end()) {
                std::string cell_content;
                for (const auto& prod : innerIt->second) {
                    cell_content += "[ ";
                    for (auto elem : prod) {
                        cell_content += symbol_table::ToString(elem) + " ";
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
    std::cout << table << "\n";
}

void LL1Parser::ExportTreeAsDot(const ParseTree&   tree,
                                const std::string& filename) {
    if (!tree)
        return;

    std::ofstream out(filename);
    out << "digraph ParseTree {\n";
    size_t id = 0;

    std::function<size_t(const ParseNode*)> dump = [&](const ParseNode* node) {
        size_t current = id++;
        out << "  node" << current << " [label=\"" << node->symbol << "\"";
        bool is_leaf = node->children.empty();
        bool is_terminal =
            symbol_table::In(node->symbol) &&
            symbol_table::IsTerminal(symbol_table::ToID(node->symbol));
        if (is_leaf && is_terminal && node->symbol != symbol_table::EPSILON_ &&
            node->symbol != symbol_table::EOF_) {
            out << ", style=filled, fillcolor=lightblue";
        }
        out << "]\n";
        for (const auto& child : node->children) {
            size_t child_id = dump(child.get());
            out << "  node" << current << " -> node" << child_id << "\n";
        }
        return current;
    };

    dump(tree.get());
    out << "}\n";
}
