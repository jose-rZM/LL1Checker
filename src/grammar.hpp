#pragma once
#include <string>
#include <unordered_map>
#include <vector>

using production = std::vector<std::string>;

struct grammar {

    explicit grammar(std::string filename);
    void read_from_file();
    void add_rule(const std::string &antecedent, const std::string &consequent);
    void set_axiom(const std::string &axiom);
    bool has_empty_production(const std::string &antecedent);
    std::vector<std::pair<const std::string, production>>filterRulesByConsequent(const std::string &arg);
    void debug();

    static std::vector<std::string> split(const std::string &s);

    std::unordered_map<std::string, std::vector<production>> g_;
    std::string AXIOM_;
    const std::string filename_;
};