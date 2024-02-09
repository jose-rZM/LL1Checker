#pragma once
#include <string>
#include <unordered_map>
#include <vector>

using production = std::vector<std::string>;

struct grammar {
  std::unordered_map<std::string, std::vector<production>> g;
  std::string AXIOM;
  void add_rule(const std::string &antecedent, const std::string &consequent);
  void set_axiom(const std::string &axiom);
  void debug();
  static std::vector<std::string> split(const std::string &s);
};