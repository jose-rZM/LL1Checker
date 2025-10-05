#pragma once
#include "symbol_table.hpp"
#include <regex>
#include <string>
#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <vector>

using BaseLexer = boost::spirit::lex::lexertl::lexer<>;
using Iterator = typename BaseLexer::iterator_type;

/// @brief A simple lexer for tokenizing an input string and reporting errors.
class Lex {
    std::string filename_; ///< Path of the file being lexed.
    std::string input_;    ///< Complete contents of the input file.
    Iterator iter_, end_;
    size_t current_{0};
public:
    /// @brief Represents a single token produced by the lexer.
    struct Token {
        symbol_table::TokenID type; ///< Token ID
        size_t pos; ///< Byte-offset in the input where this token starts.
    };


    /// Construct a lexer from a raw string instead of a file.
    /// The boolean argument is only used to differentiate the constructor
    /// signature.
    Lex(std::string input, bool from_string);

    void Tokenize();

    /**
     * @brief Retrieves the next token from input string in a lazy way.
     *
     * @return Token The next token in the sequence; EOF if there is no more
     * symbols.
     *
     */
    Token Next();

    /**
     * @brief Access the raw input buffer.
     *
     * @return std::string& A mutable reference to the entire input string.
     */
    std::string& input();

    /**
     * @brief Generate a formatted snippet around an error location.
     *
     * Given an @p input string and a byte-offset @p pos, extracts a few lines
     * of context and places a caret (^) under the error column. Also computes
     * the zero-based line (@p out_err_line) and column (@p out_err_col).
     *
     * @param input             The full text being lexed.
     * @param pos               Byte-offset in @p input where the error
     * occurred.
     * @param[out] out_err_line Zero-based index of the line containing the
     * error.
     * @param[out] out_err_col  Zero-based index of the column within that line.
     * @param context_lines     Number of lines of context before/after (default
     * 2).
     * @param max_line_width    Maximum characters to show per non-error line
     * (40).
     * @param window_width      Characters shown on either side of the error
     * (20).
     * @return std::string      A multi-line string highlighting the error
     * window.
     */
    static std::string format_error_window(const std::string& input, size_t pos,
                                           size_t& out_err_line,
                                           size_t& out_err_col,
                                           size_t  context_lines  = 2,
                                           size_t  max_line_width = 40,
                                           size_t  window_width   = 20);

private:

    /// @brief Internal helper: a regex pattern and its associated token type.
    struct Pattern {
        symbol_table::TokenID type; ///< Token ID for this pattern.
        std::string regex; ///< The regex used to match and extract the token.
 
    };

    std::vector<Token> tokens_;
};
