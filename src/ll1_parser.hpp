
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
    LL1Parser(const std::string& grammar_file, std::string text_file);
    LL1Parser(const std::string& grammar_file);
    bool parse();

  private:
    std::unordered_set<std::string>
    header(const std::vector<std::string> &rule);
    std::unordered_set<std::string> next(const std::string &arg);
    std::unordered_set<std::string>
    director_symbols(const std::string &antecedent,
                     const std::vector<std::string> &consequent);
    void next_util(const std::string &arg,
                   std::unordered_set<std::string> &visited,
                   std::unordered_set<std::string> &next_symbols);
    bool create_ll1_table();

    ll1_table ll1_t_;
    grammar gr_;
    std::string grammar_file_;
    std::string text_file_;
    std::vector<const std::string, production>
    filterRulesByConsequent(const std::string &arg);
};