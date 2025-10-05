#include "lexer.hpp"
#include "lexer_error.hpp"
#include "symbol_table.hpp"
#include <boost/spirit/include/lex_lexertl.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string_view>

Lex::Lex(std::string input, bool from_string) {
    if (!from_string) {
        std::ifstream file(input);
        if (!file)
            throw LexerError("Cannot open file: " + input);
        std::ostringstream buf;
        buf << file.rdbuf();
        input_ = buf.str();
    } else
        input_ = std::move(input);
    Tokenize();
}

namespace {
struct DynamicLexer
    : boost::spirit::lex::lexer<boost::spirit::lex::lexertl::lexer<>> {
    DynamicLexer() {
        for (size_t j = 1; j < symbol_table::order_.size(); ++j) {
            auto id        = symbol_table::order_[j];
            auto regex_str = symbol_table::GetValue(id);
            this->self.add(regex_str, j);
        }
        this->self.add("[ \t\n]+", symbol_table::order_.size());
    }
};
}  // namespace

void Lex::Tokenize() {
    using namespace boost::spirit::lex;
    using iterator_type = const char*;
    DynamicLexer  lexer;
    iterator_type first     = input_.c_str();
    iterator_type end       = first + input_.size();
    bool          completed = tokenize(first, end, lexer, [&](auto const& t) {
        if (static_cast<unsigned long>(t.id()) == symbol_table::order_.size()) {
            // Whitespace, skip
            return true;
        }
        size_t pos = t.value().begin() - input_.c_str();
        tokens_.emplace_back(symbol_table::order_.at(t.id()), pos);
        return true;
    });
    if (!completed) {
        size_t      error_pos = first - input_.c_str();
        size_t      out_err_line, out_err_col;
        std::string window =
            format_error_window(input_, error_pos, out_err_line, out_err_col);
        throw LexerError("Lexing error at position Ln " +
                         std::to_string(out_err_line + 1) + ", Col " +
                         std::to_string(out_err_col + 1) + "\n" + window);
    }
}

std::string Lex::format_error_window(const std::string& input, size_t pos,
                                     size_t& out_err_line, size_t& out_err_col,
                                     size_t context_lines,
                                     size_t max_line_width,
                                     size_t window_width) {
    // 1. Split in lines
    std::vector<std::string> lines;
    {
        std::istringstream ss(input);
        std::string        line;
        while (std::getline(ss, line)) {
            lines.push_back(line);
        }
        // end of file = \n
        if (!input.empty() && input.back() == '\n' &&
            (lines.empty() || lines.back() != ""))
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
    size_t start =
        (out_err_line < context_lines ? 0 : out_err_line - context_lines);
    size_t end = std::min(lines.size() - 1, out_err_line + context_lines);

    std::ostringstream out;
    if (start > 0)
        out << "   ...\n";

    // 4. Print each line
    for (size_t i = start; i <= end; ++i) {
        out << std::setw(4) << (i + 1) << " | ";
        const std::string& L = lines[i];

        if (i == out_err_line) {
            size_t win_start =
                (out_err_col > window_width ? out_err_col - window_width : 0);
            size_t win_end = std::min(L.size(), out_err_col + window_width);
            if (win_start > 0)
                out << "...";
            out << L.substr(win_start, win_end - win_start);
            if (win_end < L.size())
                out << "...";
            out << "\n";

            out << "     | ";
            size_t caret_pos = std::min(window_width, out_err_col);
            if (win_start > 0)
                caret_pos += 3;
            out << std::string(caret_pos, ' ') << "^\n";
        } else {
            if (L.size() > max_line_width) {
                out << L.substr(0, max_line_width) << "...\n";
            } else {
                out << L << "\n";
            }
        }
    }

    if (end + 1 < lines.size())
        out << "   ...\n";
    return out.str();
}

std::string& Lex::input() {
    return input_;
}

Lex::Token Lex::Next() {
    if (current_ >= tokens_.size()) {
        return {symbol_table::EOF_ID, input_.size()};
    } else {
        return tokens_.at(current_++);
    }
}
