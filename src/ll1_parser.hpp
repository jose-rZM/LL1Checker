
#include "grammar.hpp"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class LL1Parser {
    using ll1_table = std::unordered_map<
        std::string, std::unordered_map<std::string, std::vector<std::string>>>;

  public:
    LL1Parser(grammar gr, std::string text_file);
    LL1Parser(const std::string &grammar_file, std::string text_file);
    explicit LL1Parser(const std::string &grammar_file);
    /**
     *
     * @return true if the parsing is successfully completed
     * Parses an input file using LL1 algorithm
     */
    bool parse();

  private:
    /**
     *
     * @param rule
     * @return set header symbols for the given rule
     * TODO: if grammar consists only in one non terminal (apart the axiom), it
     * could end with an infinite loop. For example: A -> A & A, A -> ( A ). The
     * grammar obviously is not LL1, but this will provoke an infinite loop.
     */
    std::unordered_set<std::string>
    header(const std::vector<std::string> &rule);
    /**
     *
     * @param arg symbol to calculate next symbols for
     * @return Set of next symbols for the given arg
     */
    std::unordered_set<std::string> next(const std::string &arg);
    /**
     *
     * @param antecedent of a rule
     * @param consequent of a rule
     * @return set of director symbols for the given rule
     */
    std::unordered_set<std::string>
    director_symbols(const std::string &antecedent,
                     const std::vector<std::string> &consequent);
    /**
     *
     * @param arg
     * @param visited symbols (avoid infinite recursion)
     * @param next_symbols next symbols accumulated
     */
    void next_util(const std::string &arg,
                   std::unordered_set<std::string> &visited,
                   std::unordered_set<std::string> &next_symbols);
    /**
     *
     * @return true if the ll1 table could be created, that is, the grammar is
     * LL1
     */
    bool create_ll1_table();

    ll1_table ll1_t_;
    grammar gr_;
    std::string grammar_file_;
    std::string text_file_;
};