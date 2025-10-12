#include "lexer.hpp"
#include "lexer_error.hpp"
#include "symbol_table.hpp"
#include <algorithm>
#include <boost/spirit/include/lex_lexertl.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
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
    // Pre-compute line boundaries without copying whole lines.
    std::vector<size_t> line_starts;
    std::vector<size_t> line_lengths;
    line_starts.reserve(256);
    line_lengths.reserve(256);

    line_starts.push_back(0);
    size_t current_start = 0;
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '\n') {
            line_lengths.push_back(i - current_start);
            current_start = i + 1;
            line_starts.push_back(current_start);
        }
    }
    if (current_start <= input.size()) {
        line_lengths.push_back(input.size() - current_start);
    }

    if (line_lengths.empty()) {
        line_lengths.push_back(0);
        line_starts[0] = 0;
    }

    size_t line_count = line_lengths.size();
    if (line_starts.size() > line_count) {
        line_starts.resize(line_count);
    }

    size_t clamped_pos =
        input.empty() ? 0 : std::min(pos, static_cast<size_t>(input.size()));

    auto upper =
        std::upper_bound(line_starts.begin(), line_starts.end(), clamped_pos);
    size_t err_line_index =
        upper == line_starts.begin()
            ? 0
            : static_cast<size_t>(std::distance(line_starts.begin(), upper) -
                                  1);

    out_err_line = err_line_index;
    out_err_col  = clamped_pos - line_starts[err_line_index];

    size_t start_line =
        (err_line_index < context_lines ? 0 : err_line_index - context_lines);
    size_t end_line = std::min(line_count - 1, err_line_index + context_lines);

    size_t line_no_width =
        std::max<size_t>(4, std::to_string(line_count).size());

    std::ostringstream out;
    if (start_line > 0)
        out << "   ...\n";

    for (size_t i = start_line; i <= end_line; ++i) {
        std::string_view line_view(input.data() + line_starts[i],
                                   line_lengths[i]);
        out << std::setw(line_no_width) << (i + 1) << " | ";

        if (i == err_line_index) {
            size_t win_start =
                (out_err_col > window_width ? out_err_col - window_width : 0);
            size_t win_end =
                std::min(line_view.size(), out_err_col + window_width);
            bool left_trunc  = win_start > 0 && win_start < line_view.size();
            bool right_trunc = win_end < line_view.size();

            if (left_trunc)
                out << "...";
            out << line_view.substr(win_start, win_end - win_start);
            if (right_trunc)
                out << "...";
            out << "\n";

            out << std::string(line_no_width, ' ') << " | ";
            size_t caret_pos = out_err_col - win_start;
            if (left_trunc)
                caret_pos += 3;
            out << std::string(caret_pos, ' ') << "^\n";
        } else {
            if (line_view.size() > max_line_width) {
                out << line_view.substr(0, max_line_width) << "...\n";
            } else {
                out << line_view << "\n";
            }
        }
    }

    if (end_line + 1 < line_count)
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
