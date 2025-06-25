#pragma once
#include <regex>
#include <string>
#include <vector>

class Lex {
    std::string filename_;
    std::string input_;
    size_t      pos_{0};

  public:
    struct Token {
        std::string type;
        size_t      pos;
    };

    /**
     * @brief Constructs a lexer and tokenizes the specified input file.
     *
     * @param filename Path to the input file containing the string to be
     * validated.
     *
     * @note The program aborts if any errors occur during lexer creation or
     * tokenization.
     */
    explicit Lex(std::string filename);

    /**
     * @brief Retrieves the next token from the token vector.
     *
     * @return std::string The next token in the sequence; returns an empty
     * string if the end of the line (EOL) is reached.
     *
     * This function allows sequential access to tokens processed by the lexer.
     */
    Token Next();

    std::string& input();

    static std::string format_error_window(const std::string& input, size_t pos,
                                           size_t& out_err_line,
                                           size_t& out_err_col,
                                           size_t  context_lines  = 2,
                                           size_t  max_line_width = 40,
                                           size_t  window_width   = 20);

  private:
    void skip_ws();

    struct Pattern {
        std::string type;
        std::regex  regex;
    };

    std::vector<Pattern> patterns_;
};
