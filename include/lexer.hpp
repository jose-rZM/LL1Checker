#pragma once
#include <regex>
#include <string>
#include <vector>

/// @brief A simple lexer for tokenizing an input string and reporting errors.
class Lex {
    std::string filename_; ///< Path of the file being lexed.
    std::string input_;    ///< Complete contents of the input file.
    size_t      pos_{0};   ///< Current byte-offset in @c input_.

public:
    /// @brief Represents a single token produced by the lexer.
    struct Token {
        std::string
               type; ///< The token’s type or category (e.g. "NUMBER", "IDENT").
        size_t pos;  ///< Byte-offset in the input where this token starts.
    };

    /**
     * @brief Constructs a lexer.
     *
     * Opens the file at @p filename, reads its entire contents into memory,
     * then configures the patterns filling the vector of patterns..
     * Aborts the program on I/O or regex errors.
     *
     * @param filename Path to the input file containing the text to lex.
     */
    explicit Lex(std::string filename);

    /**
     * @brief Retrieves the next token from input string in a lazy way.
     *
     * @return Token The next token in the sequence; EOF if there is no more symbols.
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
    /**
     * @brief Skip over whitespace in the input buffer.
     *
     * Advances @c pos_ past any spaces, tabs, newlines, etc., until the next
     * non-whitespace character or end-of-input is reached.
     */
    void skip_ws();

    /// @brief Internal helper: a regex pattern and its associated token type.
    struct Pattern {
        std::string type;  ///< The token type name for this pattern.
        std::regex  regex; ///< The regex used to match and extract the token.
    };

    std::vector<Pattern>
        patterns_; ///< All regex patterns used for tokenization.
};
