#pragma once
#include <string>
#include <unordered_map>
#include <vector>

using production = std::vector<std::string>;

struct grammar {

    explicit grammar(std::string filename);
    /**
     * @throws GrammarError if something went wrong while reading the symbols,
     * grammar or while splitting the rules Read the grammar from a file. The
     * structure of grammar.txt is explained in README.md
     */
    void read_from_file();
    /**
     *
     * @param antecedent of the rule
     * @param consequent of the rule
     */
    void add_rule(const std::string &antecedent, const std::string &consequent);
    /**
     *
     * @param axiom of the grammar
     * Sets the axiom (entry point) of the grammar
     */
    void set_axiom(const std::string &axiom);
    /**
     *
     * @param antecedent of rule
     * @return true if there exists an empty rule, that is, <antecedent> ->;"
     */
    bool has_empty_production(const std::string &antecedent);
    /**
     *
     * @param arg token to be searched in grammar rules
     * @return vector of rules with args as part of the consequent
     */
    std::vector<std::pair<const std::string, production>>
    filter_rules_by_consequent(const std::string &arg);
    /**
     * Prints the grammar
     */
    void debug();

    /**
     *
     * @param s production to be splitted
     * @return vector of tokens
     * Splits the production into tokens. For example, if symbol table contains:
     * {(A, NON TERMINAL), (B, NON TERMINAL), (PLUS, TERMINAL)} and s = APLUSB,
     * the method would return {A, PLUS, B}.
     */
    static std::vector<std::string> split(const std::string &s);


    /**
     *
     * @param antecedent of the rule
     * @param consequent vector of tokens
     * @return true if grammar has left recursion, for example: A -> A + A
     */
    static bool has_left_recursion(const std::string &antecedent,
                                   const std::vector<std::string> &consequent);
    std::unordered_map<std::string, std::vector<production>> g_;
    std::string AXIOM_;
    const std::string filename_;
};