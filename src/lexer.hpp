#include <fstream>
#include <iterator>
#include <vector>
class lexer {
    std::string filename_;
    std::vector<std::string> tokens_;
    unsigned current_;

  public:
    /**
     *
     * @param filename of the input file (where the string to validate is
     * placed).
     * The constructor creates the lexer, compile it and tokenize the input.
     * The program assumes that all the requirements listed in "README.md" are
     * met. If any errors occur the program aborts.
     */
    explicit lexer(std::string filename);
    /**
     *
     * @return next token of the vector, empty string if end of line is reached.
     */
    std::string next();

  private:
    /**
     * Open the dynamically generated library and load: "set_yyin", "yylex" and
     * "yylex_destroy". The program aborts if any of these loads fail.
     * If everything goes well, tokenize the input using yylex. Each token is
     * stored in the "tokens" vector. Once the EOL character is reached (with
     * value 1), all resources are freed. This function is called only once.
     */
    void tokenize();
    /**
     * Generates a lexer file using the symbol table. The symbols are placed in
     * order. It also generates a custom function "set_yyin" for changing the
     * input of yylex.
     */
    void make_lexer();
    /**
     * Compile lexer file named "lexer.l" into lex.yy.c. After that, it
     * generates a dynamic library using the compiled lexer "lex.yy.c".
     */
    static void compile();
    const std::string LEXER_FILENAME{"lex.l"};
    const std::string SRC_PATH{"./src"};
    const std::string LIB_PATH{"./lib"};
};
