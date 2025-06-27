#pragma once
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

enum symbol_type { NO_TERMINAL, TERMINAL, META };

struct symbol_table {
    using TokenID = unsigned long;

    inline static TokenID INVALID_TOKEN = 0;
    inline static TokenID next_id_ = 3;
 /// @brief End-of-line symbol used in parsing, initialized as "EOF".
    inline static std::string EOF_{"<<EOF>>"};
    /// @brief Epsilon symbol, representing empty transitions, initialized as
    /// "EPSILON".
    inline static std::string EPSILON_{"<<EPSILON>>"};
    inline static std::vector<std::pair<symbol_type, std::string>> st_ {
        {META, ""}, // TOKEN 0 - INVALID
        {TERMINAL, ""}, // TOKEN 1 - <<EOF>>
        {META, ""}, // TOKEN 2 - <<EPSILON>>
    };

    inline static std::unordered_map<std::string, TokenID> lookup_ {
        {EOF_, 1}, {EPSILON_, 2}
    };


    /**
     * @brief Adds a terminal symbol with its associated regex to the symbol
     * table.
     *
     * Updates the token type mappings and tracks insertion order.
     *
     * @param identifier Name of the terminal symbol.
     * @param regex Regular expression representing the terminal symbol.
     */
    static void PutSymbol(const std::string& identifier,
                          const std::string& regex);

    /**
     * @brief Adds a non-terminal symbol to the symbol table.
     *
     * @param identifier Name of the non-terminal symbol.
     */
    static void PutSymbol(const std::string& identifier);

    /**
     * @brief Checks if a symbol exists in the symbol table.
     *
     * @param s Symbol identifier to search.
     * @return true if the symbol is present, otherwise false.
     */
    static bool In(TokenID id);

        static bool In(const std::string &identifier);

    /**
     * @brief Checks if a symbol is a terminal.
     *
     * @param s Symbol identifier to check.
     * @return true if the symbol is terminal, otherwise false.
     */
    static bool IsTerminal(TokenID id);

    /**
     * @brief Retrieves the regex pattern for a terminal symbol.
     *
     * @param terminal Terminal symbol identifier.
     * @return Regex pattern associated with the terminal symbol.
     */
    static std::string GetValue(TokenID id);

    /**
     * @brief Prints all symbols and their properties in the symbol table.
     *
     * Outputs the symbol table for debugging purposes.
     */
    static void Debug();
};
