#pragma once
#include "grammar.hpp"
#include <deque>
#include <queue>
#include <span>
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
    LL1Parser(Grammar gr, std::string text_file, bool table_format = true);

    /**
     * @brief Constructs an LL1Parser with a grammar file and an input file.
     *
     * @param grammar_file Path to the grammar file.
     * @param text_file Name of the file containing input to parse.
     */
    LL1Parser(const std::string& grammar_file, std::string text_file,
              bool table_format = true);

    /**
     * @brief Constructs an LL1Parser with a grammar file.
     *
     * @param grammar_file Path to the grammar file.
     */
    explicit LL1Parser(const std::string& grammar_file,
                       bool               table_format = true);

    /**
     * @brief Parses an input string or file using the LL(1) parsing algorithm.
     *
     * This function performs syntactic analysis on the input based on the LL(1)
     * parsing table, working to validate whether the input string conforms to
     * the grammar. The parsing process involves a stack-based approach, where:
     *
     * - The function initializes a stack with the starting symbol of the
     * grammar.
     * - For each symbol in the input, it matches and expands according to the
     *   entries in the LL(1) parsing table.
     * - If a match is found for the current input symbol and top of the stack,
     *   the function advances in the input and continues parsing.
     * - If a production rule applies, it expands the non-terminal on the stack
     *   using the rule.
     * - If an unexpected symbol or parsing conflict arises, the function
     * returns `false`, indicating that the input does not conform to the
     * grammar.
     *
     * The function returns `true` if parsing completes successfully, reaching
     * the end of the input and stack without errors. Otherwise, it returns
     * `false`.
     *
     * @return `true` if the input is parsed successfully, meaning it conforms
     * to the LL(1) grammar; `false` if parsing fails due to a mismatch,
     * conflict, or unexpected input symbol.
     */
    bool Parse();

    /**
     * @brief Matches a terminal symbol from the stack with the current input
     * symbol.
     *
     * This function compares the terminal symbol at the top of the stack with
     * the current input symbol. If they match, it updates the parsing trace
     * (`trace_`), consumes the current input symbol, and retrieves the next
     * input symbol using the lexical analyzer (`lex`).
     *
     * @param top_symbol The terminal symbol popped from the top of the stack.
     * @param current_symbol The current input symbol to be matched.
     * @return true if the terminal symbol matches the current input symbol,
     * false otherwise.
     */
    bool MatchTerminal(const std::string& top_symbol,
                       const std::string& current_symbol);

    /**
     * @brief Processes a non-terminal symbol by expanding it according to the
     * LL(1) parsing table.
     *
     * This function looks up the production rule in the LL(1) parsing table
     * (`ll1_t_`) for the given non-terminal symbol and current input symbol. If
     * a matching production is found, the production's symbols are pushed onto
     * the stack in reverse order.
     *
     * If no matching production is found, it checks whether the grammar allows
     * an empty production for the non-terminal.
     *
     * @param top_symbol The non-terminal symbol popped from the top of the
     * stack.
     * @param current_symbol The current input symbol used to select a
     * production.
     *
     * @return true if a production was successfully applied or an empty
     * production exists, false if no valid production exists for the current
     * input.
     */
    bool ProcessNonTerminal(const std::string& top_symbol,
                            const std::string& current_symbol);

    /**
     * @brief Print the LL(1) parsing table to standard output.
     *
     * Displays the LL(1) table for debugging and analysis. The output format
     * depends on the value of `print_table_format_`:
     * - If `print_table_format_` is `true`, the table is printed using the
     *   `tabulate` library for a structured and visually enhanced output.
     * - If `print_table_format_` is `false`, the table is printed in a simpler
     *   text format.
     */
    void PrintTable();

    /**
     * @brief Prints the remaining symbols in the parsing stack after the
     * parsing process.
     *
     * This function outputs the contents of the parsing stack to standard
     * output after the parsing attempt completes, showing any symbols left
     * unresolved. It is useful for debugging and tracing parsing issues, as it
     * provides insight into where the parsing process may have diverged from
     * expected behavior.
     */
    void PrintStackTrace();

    /**
     * @brief Prints the last kTraceSize symbols processed.
     *
     * Primarily used to identify the most recent tokens processed in case
     * of parsing errors.
     */
    void PrintSymbolHist();

  private:
    /**
     * @brief Calculates the FIRST set for a given production rule in a grammar.
     *
     * The FIRST set of a production rule contains all terminal symbols that can
     * appear at the beginning of any string derived from that rule. If the rule
     * can derive the empty string (epsilon), epsilon is included in the FIRST
     * set.
     *
     * This function computes the FIRST set by examining each symbol in the
     * production rule:
     * - If a terminal symbol is encountered, it is added directly to the FIRST
     * set, as it is the starting symbol of some derivation.
     * - If a non-terminal symbol is encountered, its FIRST set is recursively
     * computed and added to the result, excluding epsilon unless it is followed
     * by another symbol that could also lead to epsilon.
     * - If the entire rule could derive epsilon (i.e., each symbol in the rule
     * can derive epsilon), then epsilon is added to the FIRST set.
     *
     * @param rule A span of strings representing the production rule for which
     * to compute the FIRST set. Each string in the span is a symbol (either
     * terminal or non-terminal).
     * @param result A reference to an unordered set of strings where the
     * computed FIRST set will be stored. The set will contain all terminal
     * symbols that can start derivations of the rule, and possibly epsilon if
     * the rule can derive an empty string.
     */
    void First(std::span<const std::string>     rule,
               std::unordered_set<std::string>& result);

    /**
     * @brief Computes the FIRST sets for all non-terminal symbols in the
     * grammar.
     *
     * This function calculates the FIRST set for each non-terminal symbol in
     * the grammar by iteratively applying a least fixed-point algorithm. This
     * approach ensures that the FIRST sets are fully populated by repeatedly
     * expanding and updating the sets until no further changes occur (i.e., a
     * fixed-point is reached).
     */
    void ComputeFirstSets();

    /**
     * @brief Computes the FOLLOW sets for all non-terminal symbols in the
     * grammar.
     *
     * The FOLLOW set of a non-terminal symbol A contains all terminal symbols
     * that can appear immediately after A in any sentential form derived from
     * the grammar's start symbol. Additionally, if A can be the last symbol in
     * a derivation, the end-of-input marker (`$`) is included in its FOLLOW
     * set.
     *
     * This function computes the FOLLOW sets using the following rules:
     * 1. Initialize FOLLOW(S) = { $ }, where S is the start symbol.
     * 2. For each production rule of the form A → αBβ:
     *    - Add FIRST(β) (excluding ε) to FOLLOW(B).
     *    - If ε ∈ FIRST(β), add FOLLOW(A) to FOLLOW(B).
     * 3. Repeat step 2 until no changes occur in any FOLLOW set.
     *
     * The computed FOLLOW sets are cached in the `follow_sets_` member variable
     * for later use by the parser.
     *
     * @note This function assumes that the FIRST sets for all symbols have
     * already been computed and are available in the `first_sets_` member
     * variable.
     *
     * @see First
     * @see follow_sets_
     */
    void ComputeFollowSets();

    /**
     * @brief Updates the FOLLOW set for a non-terminal based on a production.
     *
     * This method updates the FOLLOW set of a given non-terminal symbol based
     * on its position within a production. It considers:
     * - The FIRST set of the remaining symbols after the current non-terminal.
     * - The FOLLOW set of the left-hand side non-terminal, if the remaining
     * symbols can derive ε.
     *
     * @param symbol The non-terminal symbol whose FOLLOW set is being updated.
     * @param lhs The left-hand side non-terminal of the current production.
     * @param rhs The production (right-hand side) containing the symbol.
     * @param i The position of the symbol within the production.
     *
     * @return true if the FOLLOW set was modified (new elements were added),
     * false otherwise.
     */
    bool UpdateFollow(const std::string& symbol, const std::string& lhs,
                      const production& rhs, size_t i);

    /**
     * @brief Computes the FOLLOW set for a given non-terminal symbol in the
     * grammar.
     *
     * The FOLLOW set for a non-terminal symbol includes all symbols that can
     * appear immediately to the right of that symbol in any derivation, as well
     * as any end-of-input markers if the symbol can appear at the end of
     * derivations. FOLLOW sets are used in LL(1) parsing table construction to
     * determine possible continuations after a non-terminal.
     *
     * @note This function assumes that the follow sets for all symbols have
     * already been computed by using ComputeFollowSets function.
     *
     * @param arg Non-terminal symbol for which to compute the FOLLOW set.
     * @return An unordered set of strings containing symbols that form the
     * FOLLOW set for `arg`.
     */
    std::unordered_set<std::string> Follow(const std::string& arg);

    /**
     * @brief Computes the prediction symbols for a given
     * production rule.
     *
     * The prediction symbols for a rule,
     * determine the set of input symbols that can trigger this rule in the
     * parsing table. This function calculates the prediction symbols based on
     * the FIRST set of the consequent and, if epsilon (the empty symbol) is in
     * the FIRST set, also includes the FOLLOW set of the antecedent.
     *
     * - If the FIRST set of the consequent does not contain epsilon, the
     * prediction symbols are simply the FIRST symbols of the consequent.
     * - If the FIRST set of the consequent contains epsilon, the prediction
     * symbols are computed as (FIRST(consequent) - {epsilon}) ∪
     * FOLLOW(antecedent).
     *
     * @param antecedent The left-hand side non-terminal symbol of the rule.
     * @param consequent A vector of symbols on the right-hand side of the rule
     * (production body).
     * @return An unordered set of strings containing the prediction symbols for
     * the specified rule.
     */
    std::unordered_set<std::string>
    PredictionSymbols(const std::string&              antecedent,
                      const std::vector<std::string>& consequent);

    /**
     * @brief Creates the LL(1) parsing table for the grammar.
     *
     * This function constructs the LL(1) parsing table by iterating over each
     * production in the grammar and determining the appropriate cells for each
     * non-terminal and director symbol (prediction symbol) combination. If the
     * grammar is LL(1) compatible, each cell will contain at most one
     * production, indicating no conflicts. If conflicts are found, the function
     * will return `false`, signaling that the grammar is not LL(1).
     *
     * - For each production rule `A -> α`, the function calculates the director
     * symbols using the `director_symbols` function.
     * - It then fills the parsing table at the cell corresponding to the
     * non-terminal `A` and each director symbol in the set.
     * - If a cell already contains a production, this indicates a conflict,
     * meaning the grammar is not LL(1).
     *
     * @return `true` if the table is created successfully, indicating the
     * grammar is LL(1) compatible; `false` if any conflicts are detected,
     * showing that the grammar does not meet LL(1) requirements.
     */
    bool CreateLL1Table();

    /**
     * @brief Print the LL(1) parsing table using the tabulate library.
     *
     * This function generates a structured LL(1) table using the `tabulate`
     * library, displaying non-terminals as row headers and terminal symbols as
     * columns. Productions are formatted within the table cells. The table
     * uses:
     * - Bold, centered headers with yellow font color.
     * - Cyan font color for the first column (non-terminals).
     * - Red font color for cells containing multiple productions (conflicts).
     * - A visually structured alignment for improved readability.
     */
    void PrintTableUsingTabulate();

    /// @brief Size limit for symbol history trace, defaults to 5.
    const size_t kTraceSize{5};

    /// @brief The LL(1) parsing table, mapping non-terminals and terminals to
    /// productions.
    ll1_table ll1_t_;

    /// @brief Grammar object associated with this parser.
    Grammar gr_;

    /// @brief FIRST sets for each non-terminal in the grammar.
    std::unordered_map<std::string, std::unordered_set<std::string>>
        first_sets_;

    /// @brief FOLLOW sets for each non-terminal in the grammar.
    std::unordered_map<std::string, std::unordered_set<std::string>>
        follow_sets_;

    /// @brief Stack for managing parsing symbols.
    std::stack<std::string> symbol_stack_;

    /// @brief Deque for tracking the most recent kTraceSize symbols parsed.
    std::deque<std::string> trace_;

    /// @brief Path to the grammar file used in this parser.
    std::string grammar_file_;

    /// @brief Path to the input text file to be parsed.
    std::string text_file_;

    /// @brief True if new format is used when printing the table
    bool print_table_format_{true};
};
