#include <boost/filesystem.hpp>
#include <vector>
class lexer {
    std::string              filename_;
    std::vector<std::string> tokens_;
    unsigned                 current_;

  public:
    /**
     * @brief Constructs a lexer and tokenizes the specified input file.
     *
     * @param filename Path to the input file containing the string to be
     * validated.
     *
     * The constructor initializes the lexer, compiles it, and tokenizes the
     * input file's contents. Assumes that all requirements outlined in
     * "README.md" are met.
     *
     * @note The program aborts if any errors occur during lexer creation or
     * tokenization.
     */
    explicit lexer(std::string filename);

    /**
     * @brief Retrieves the next token from the token vector.
     *
     * @return std::string The next token in the sequence; returns an empty
     * string if the end of the line (EOL) is reached.
     *
     * This function allows sequential access to tokens processed by the lexer.
     */
    std::string next();

  private:
    /**
     * @brief Loads and initializes necessary lexer symbols, then tokenizes
     * input.
     *
     * This function dynamically loads the symbols "set_yyin", "yylex", and
     * "yylex_destroy". If any loading operation fails, the program aborts. Once
     * symbols are loaded successfully, it tokenizes the input file using
     * `yylex`. All tokens are stored in the `tokens_` vector.
     *
     * @note The function frees resources upon reaching the EOL (character value
     * 1). This function is called only once for initial setup and tokenization.
     */
    void tokenize();
    /**
     * @brief Generates the lexer file based on the symbol table.
     *
     * This function generates a lexer file with tokens defined in the symbol
     * table, placing them in the appropriate order. Additionally, it creates a
     * custom function "set_yyin" to enable input redirection for `yylex`.
     */
    void make_lexer();
    /**
     * @brief Compiles the lexer file and creates a dynamic library.
     *
     * Compiles the lexer source file "lexer.l" into an intermediate file
     * "lex.yy.c", and then generates a dynamic library from "lex.yy.c" for
     * runtime lexing.
     *
     * @throws std::runtime_error Throws if compilation fails at any step.
     */
    void compile();

    /**
     * @brief Creates temporary files required for lexer processing.
     *
     * This function sets up temporary files that are used during the lexing
     * process. It ensures that all necessary files are created before any
     * tokens are processed.
     *
     * @note Temporary files created by this function should be removed after
     * processing by calling the cleanup() function.
     *
     * @exception std::runtime_error Throws if a temporary file cannot be
     * created.
     */
    void create_temp_files();

    /**
     * @brief Cleans up resources and removes temporary files.
     *
     * This function deletes all temporary files created by create_temp_files()
     * to free up resources and prevent file clutter. It should be called at the
     * end of the lexing process or in case of errors to ensure proper cleanup.
     *
     * @note This function is safe to call even if no temporary files were
     * created, as it will silently ignore missing files.
     */
    void cleanup();

    boost::filesystem::path temp_dir_;
    boost::filesystem::path lex_file_path_;
    boost::filesystem::path c_file_path_;
    boost::filesystem::path o_file_path_;
    boost::filesystem::path so_file_path_;
    const std::string       LEXER_FILENAME{"lex.l"};
    const std::string       SRC_PATH{"./src"};
    const std::string       LIB_PATH{"./lib"};
};
