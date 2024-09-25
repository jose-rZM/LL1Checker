
#include "grammar.hpp"
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class LL1Parser {
    using ll1_table = std::unordered_map<
        int, std::unordered_map<int, std::vector<int>>>;

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
    std::unordered_set<int>
    header(const std::vector<int> &rule);
    /**
     *
     * @param arg symbol to calculate next symbols for
     * @return Set of next symbols for the given arg
     */
    std::unordered_set<int> next(int arg);
    /**
     *
     * @param antecedent of a rule
     * @param consequent of a rule
     * @return set of director symbols for the given rule
     */
    std::unordered_set<int>
    director_symbols(int antecedent,
                     const std::vector<int> &consequent);
    /**
     *
     * @param arg
     * @param visited symbols (avoid infinite recursion)
     * @param next_symbols next symbols accumulated
     */
    void next_util(int arg,
                   std::unordered_set<int> &visited,
                   std::unordered_set<int> &next_symbols);
    /**
     *
     * @return true if the ll1 table could be created, that is, the grammar is
     * LL1
     */
    bool create_ll1_table();
    
    /**
     * print the current stack when parsing error
     */
    void print_stack_trace(std::stack<int>& stack) const;

    ll1_table ll1_t_;
    grammar gr_;
    std::string grammar_file_;
    std::string text_file_;
};
