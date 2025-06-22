#include <string>
#include <vector>

class Lex {
    std::string              filename_;
    std::vector<std::string> tokens_;
    unsigned                 current_{};

  public:
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
    std::string Next();

  private:
    /**
     * @brief Tokenizes the input file using Boost Spirit Lex.
     *
     * This function reads the content of the file specified by `filename_`,
     * tokenizes it using Boost Spirit Lex, and stores the resulting tokens in
     * the `tokens_` member variable. If the tokenization process encounters an
     * invalid token, a `LexerError` is thrown with an error message indicating
     * the invalid token.
     *
     * @throws LexerError If an invalid token is encountered during
     * tokenization.
     *
     * @details The function performs the following steps:
     * 1. Opens the file specified by `filename_` and reads its content into a
     * string.
     * 2. Converts the string into a C-style string (char array) for processing.
     * 3. If tokenization is successful, the tokens are stored in the `tokens_`
     * member variable.
     * 4. If tokenization fails (e.g., due to an invalid token), a `LexerError`
     * is thrown.
     *
     * @see LexerError
     * @see tokens_
     * @see filename_
     */
    void Tokenize();
};
