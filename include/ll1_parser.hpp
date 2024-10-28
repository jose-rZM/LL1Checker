#pragma once
#include "grammar.hpp"
#include <queue>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class LL1Parser {
    using ll1_table = std::unordered_map<
        std::string, std::unordered_map<std::string, std::vector<production>>>;

  public:
    /**
     * @brief Constructs an LL1Parser with a grammar object and an input file.
     *
     * @param gr Grammar object to parse with.
     * @param text_file Name of the file containing input to parse.
     */
    LL1Parser(grammar gr, std::string text_file);

    /**
     * @brief Constructs an LL1Parser with a grammar file and an input file.
     *
     * @param grammar_file Path to the grammar file.
     * @param text_file Name of the file containing input to parse.
     */
    LL1Parser(const std::string& grammar_file, std::string text_file);

    /**
     * @brief Constructs an LL1Parser with a grammar file.
     *
     * @param grammar_file Path to the grammar file.
     */
    explicit LL1Parser(const std::string& grammar_file);

    /**
     * @brief Parses the input file using the LL(1) algorithm.
     *
     * @return true if parsing completes successfully, otherwise false.
     */
    bool parse();

    /**
     * @brief Prints the LL(1) parsing table to standard output.
     *
     * Displays the LL(1) table for debugging and analysis.
     */
    void print_table();

    /**
     * @brief Prints the parsing stack trace to standard output.
     *
     * Shows the sequence of parsing steps, useful for tracing errors.
     */
    void print_stack_trace();

    /**
     * @brief Prints the last TRACE_SIZE symbols processed.
     *
     * Primarily used to identify the most recent tokens processed in case
     * of parsing errors.
     */
    void print_symbol_hist();

  private:
    /**
     * @brief Computes the FIRST set for a given production rule.
     *
     * @param rule Vector of strings representing a production rule.
     * @return A set of symbols that form the FIRST set for the rule.
     */
    std::unordered_set<std::string> first(const std::vector<std::string>& rule);

    /**
     * @brief Computes the FIRST sets for all non-terminal symbols in the
     * grammar.
     *
     * Calculates FIRST sets for each non-terminal in the grammar, aiding in
     * table construction and validation of LL(1) compatibility.
     */
    void compute_first_sets();

    /**
     * @brief Computes the FOLLOW set for a given non-terminal symbol.
     *
     * @param arg Non-terminal symbol for which to compute the FOLLOW set.
     * @return A set of symbols forming the FOLLOW set for the symbol.
     */
    std::unordered_set<std::string> follow(const std::string& arg);

    /**
     * @brief Determines the director symbols for a given rule.
     *
     * @param antecedent The left-hand side symbol of the rule.
     * @param consequent The right-hand side symbols of the rule.
     * @return A set of director symbols for the specified rule.
     */
    std::unordered_set<std::string>
    director_symbols(const std::string&              antecedent,
                     const std::vector<std::string>& consequent);

    /**
     * @brief Utility function for computing FOLLOW sets.
     *
     * @param arg Symbol to compute FOLLOW set for.
     * @param visited A set to track visited symbols and avoid recursion loops.
     * @param next_symbols Accumulated symbols forming the FOLLOW set.
     */
    void follow_util(const std::string&               arg,
                     std::unordered_set<std::string>& visited,
                     std::unordered_set<std::string>& next_symbols);

    /**
     * @brief Creates the LL(1) parsing table.
     *
     * @return true if the table is created successfully, indicating an LL(1)
     *         compatible grammar, otherwise false.
     */
    bool create_ll1_table();

    /// @brief Size limit for symbol history trace, defaults to 5.
    const size_t TRACE_SIZE{5};

    /// @brief The LL(1) parsing table, mapping non-terminals and terminals to
    /// productions.
    ll1_table ll1_t_;

    /// @brief Grammar object associated with this parser.
    grammar gr_;

    /// @brief FIRST sets for each non-terminal in the grammar.
    std::unordered_map<std::string, std::unordered_set<std::string>> first_sets;

    /// @brief Stack for managing parsing symbols.
    std::stack<std::string> symbol_stack_;

    /// @brief Queue for tracking the most recent TRACE_SIZE symbols parsed.
    std::queue<std::string> trace_;

    /// @brief Path to the grammar file used in this parser.
    std::string grammar_file_;

    /// @brief Path to the input text file to be parsed.
    std::string text_file_;
};
