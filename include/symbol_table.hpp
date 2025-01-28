#pragma once
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

enum symbol_type { NO_TERMINAL, TERMINAL };

struct symbol_table {
    /// @brief End-of-line symbol used in parsing, initialized as "$".
    inline static std::string EOL_{"$"};

    /// @brief Epsilon symbol, representing empty transitions, initialized as
    /// "EPSILON".
    inline static std::string EPSILON_{"EPSILON"};

    /// @brief Main symbol table, mapping identifiers to a pair of symbol type
    /// and its regex.
    inline static std::unordered_map<std::string,
                                     std::pair<symbol_type, std::string>>
        st_{{EOL_, {TERMINAL, EOL_}}, {EPSILON_, {TERMINAL, EPSILON_}}};

    /// @brief Token types, mapping each symbol to a unique integer ID.
    inline static std::unordered_map<std::string, unsigned long> token_types_{
        {EOL_, 1}};

    /// @brief Reverse mapping from integer token IDs back to symbols.
    inline static std::unordered_map<unsigned long, std::string> token_types_r_{
        {1, EOL_}};

    /// @brief Tracks insertion order of token types.
    inline static std::vector<unsigned long> order_{1};

    /// @brief Current index for assigning new token IDs, starting from 2.
    inline static unsigned long i_{2};

    /**
     * @brief Adds a terminal symbol with its associated regex to the symbol
     * table.
     *
     * Updates the token type mappings and tracks insertion order.
     *
     * @param identifier Name of the terminal symbol.
     * @param regex Regular expression representing the terminal symbol.
     */
    static void put_symbol(const std::string& identifier,
                           const std::string& regex);

    /**
     * @brief Adds a non-terminal symbol to the symbol table.
     *
     * @param identifier Name of the non-terminal symbol.
     */
    static void put_symbol(const std::string& identifier);

    /**
     * @brief Checks if a symbol exists in the symbol table.
     *
     * @param s Symbol identifier to search.
     * @return true if the symbol is present, otherwise false.
     */
    static bool in(const std::string& s);

    /**
     * @brief Checks if a symbol is a terminal.
     *
     * @param s Symbol identifier to check.
     * @return true if the symbol is terminal, otherwise false.
     */
    static bool is_terminal(const std::string& s);

    /**
     * @brief Retrieves the regex pattern for a terminal symbol.
     *
     * @param terminal Terminal symbol identifier.
     * @return Regex pattern associated with the terminal symbol.
     */
    static std::string get_value(const std::string& terminal);

    /**
     * @brief Prints all symbols and their properties in the symbol table.
     *
     * Outputs the symbol table for debugging purposes.
     */
    static void debug();

    /**
     * @brief Sets the end-of-line symbol.
     *
     * @param eol String to use as the new end-of-line symbol.
     */
    static void set_eol(const std::string& eol);
};
