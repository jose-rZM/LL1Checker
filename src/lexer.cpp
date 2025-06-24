#include "lexer.hpp"
#include "lexer_error.hpp"
#include "symbol_table.hpp"
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <regex>
#include <iomanip>
#include <string_view>

namespace {
struct Pattern {
    std::string type;
    std::regex  regex;
};
} // namespace

Lex::Lex(std::string filename) : filename_(std::move(filename)), current_() {
    Tokenize();
}

static std::string format_error_window(const std::string& input,
                                       size_t pos,
                                       size_t& out_err_line,
                                       size_t& out_err_col,
                                       size_t context_lines   = 2,
                                       size_t max_line_width  = 40,
                                       size_t window_width    = 20)
{
    // 1. Split in lines
    std::vector<std::string> lines;
    {
        std::istringstream ss(input);
        std::string line;
        while (std::getline(ss, line)) {
            lines.push_back(line);
        }
        // end of file = \n
        if (!input.empty() && input.back() == '\n' && (lines.empty() || lines.back() != ""))
            lines.push_back("");
    }

    // 2. Error line and column
    size_t running = 0;
    out_err_line = out_err_col = 0;
    for (size_t i = 0; i < lines.size(); ++i) {
        size_t line_len = lines[i].size() + 1;
        if (pos < running + line_len) {
            out_err_line = i;
            out_err_col  = pos - running;
            break;
        }
        running += line_len;
    }

    // 3. Context window
    size_t start = (out_err_line < context_lines ? 0 : out_err_line - context_lines);
    size_t end   = std::min(lines.size() - 1, out_err_line + context_lines);

    std::ostringstream out;
    if (start > 0) out << "   ...\n";

    // 4. Print each line
    for (size_t i = start; i <= end; ++i) {
        out << std::setw(4) << (i + 1) << " | ";
        const std::string& L = lines[i];

        if (i == out_err_line) {
            size_t win_start = (out_err_col > window_width ? out_err_col - window_width : 0);
            size_t win_end   = std::min(L.size(), out_err_col + window_width);
            if (win_start > 0) out << "...";
            out << L.substr(win_start, win_end - win_start);
            if (win_end < L.size()) out << "...";
            out << "\n";

            out << "     | ";
            size_t caret_pos = std::min(window_width, out_err_col);
            if (win_start > 0) caret_pos += 3;
            out << std::string(caret_pos, ' ') << "^\n";
        }
        else {
            if (L.size() > max_line_width) {
                out << L.substr(0, max_line_width) << "...\n";
            } else {
                out << L << "\n";
            }
        }
    }

    if (end + 1 < lines.size()) out << "   ...\n";
    return out.str();
}



void Lex::Tokenize() {
    std::ifstream file(filename_);
    if (!file) {
        throw LexerError("Cannot open file: " + filename_);
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::string input = buffer.str();
    size_t      pos   = 0;

    std::vector<Pattern> patterns;
    patterns.reserve(symbol_table::order_.size());

    for (size_t j = 1; j < symbol_table::order_.size(); ++j) {
        unsigned long id         = symbol_table::order_[j];
        std::string   token_type = symbol_table::token_types_r_.at(id);
        std::string   regex_str  = symbol_table::st_.at(token_type).second;
        patterns.emplace_back(token_type, std::regex(regex_str));
    }

    std::regex ws{R"([ \t\n]+)"};
    while (pos < input.size()) {
        std::string_view remaining{input.data() + pos, input.size() - pos};
        std::cmatch      m;
        if (std::regex_search(remaining.begin(), remaining.end(), m, ws,
                              std::regex_constants::match_continuous)) {
            pos += m.length();
            continue;
        }
        std::ptrdiff_t best_len = 0;
        std::string    best_tok;
        for (const auto& p : patterns) {
            if (std::regex_search(remaining.begin(), remaining.end(), m,
                                  p.regex,
                                  std::regex_constants::match_continuous)) {
                if (m.length() > best_len) {
                    best_len = m.length();
                    best_tok = p.type;
                }
            }
        }
        if (best_len == 0) {
            size_t err_line, err_col;
            std::string snippet = format_error_window(input, pos, err_line, err_col);
            throw LexerError(
                "Lexical error at line " + std::to_string(err_line + 1) +
                ", column " + std::to_string(err_col + 1) + ":\n\n" +
                snippet
            );
        }
        tokens_.push_back(best_tok);
        pos += best_len;
    }
}

std::string Lex::Next() {
    return current_ >= tokens_.size() ? symbol_table::EOF_ : tokens_[current_++];
}