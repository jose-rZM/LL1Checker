#include "grammar.hpp"
#include "lexer.hpp"
#include "symbol_table.hpp"
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <ios>
#include <iostream>
#include <istream>
#include <iterator>
#include <map>
#include <ostream>
#include <regex>
#include <stack>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <dlfcn.h>

using ll1_table = std::unordered_map<
    std::string, std::unordered_map<std::string, std::vector<std::string>>>;

std::unordered_set<std::string> header(const grammar &gr,
                                       std::vector<std::string> rule) {
  std::unordered_set<std::string> hd;
  std::stack<std::vector<std::string>> st;
  st.push(rule);

  while (!st.empty()) {
    std::vector<std::string> current = st.top();
    st.pop();
    if (current[0] == symbol_table::EPSILON) {
      current.erase(current.begin());
    }
    if (current.size() == 0) {
      hd.insert(symbol_table::EPSILON);
    } else if (symbol_table::is_terminal(current[0])) {
      hd.insert(current[0]);
    } else {
      for (const std::vector<std::string> &prod : gr.g.at(current[0])) {
        std::vector<std::string> production;
        for (const std::string &sy : prod) {
          production.push_back(sy);
        }
        for (unsigned i = 1; i < current.size(); ++i) {
          production.push_back(current[i]);
        }
        st.push(production);
      }
    }
  }

  return hd;
}

void nextUtil(const grammar &gr, const std::string &arg,
              std::unordered_set<std::string> &visited,
              std::unordered_set<std::string> &next_symbols) {
  if (visited.find(arg) != visited.cend()) {
    return;
  }
  visited.insert(arg);
  std::vector<std::pair<const std::string, production>> rules;
  // find rules with arg as part of consequence
  for (std::pair<const std::string, std::vector<production>> rule : gr.g) {
    for (production prod : rule.second) {
      if (std::find(prod.cbegin(), prod.cend(), arg) != prod.cend()) {
        rules.push_back({rule.first, prod});
      }
    }
  }

  for (std::pair<const std::string, production> rule : rules) {
    production::const_iterator it = rule.second.begin();
    while ((it = std::find(it, rule.second.cend(), arg)) != rule.second.cend()) {
      production::const_iterator nit = std::next(it);
      if (nit == rule.second.cend()) {
        nextUtil(gr, rule.first, visited, next_symbols);
      } else {
        next_symbols.merge(header(gr, {*nit}));
      }
      it = std::next(it);
    }
  }
}

std::unordered_set<std::string> next(const grammar &gr, std::string arg) {
  std::unordered_set<std::string> next_symbols;
  std::unordered_set<std::string> visited;
  nextUtil(gr, arg, visited, next_symbols);
  if (next_symbols.find(symbol_table::EPSILON) != next_symbols.end()) {
    next_symbols.erase(symbol_table::EPSILON);
  }
  //next_symbols.insert(symbol_table::EOL);
  return next_symbols;
}

std::unordered_set<std::string>
director_symbol(const grammar &gr, const std::string &antecedent,
                const std::vector<std::string> &consequent) {
  std::unordered_set<std::string> hd{header(gr, consequent)};
  if (hd.find(symbol_table::EPSILON) == hd.end()) {
    return hd;
  } else {
    hd.erase(symbol_table::EPSILON);
    hd.merge(next(gr, antecedent));
    return hd;
  }
}

void read_from_file(grammar &g) {
  const std::string filename{"input.txt"};
  std::ifstream file;
  file.open(filename, std::ios::in);
  if (file.is_open()) {
    std::string input;
    std::smatch match;
    while (getline(file, input) && input != ";") {
      std::string id;
      std::string value;
      std::regex rx_no_terminal{
          "no\\s+terminal\\s+([a-zA-Z_\\'][a-zA-Z_0-9\\']*);"};
      std::regex rx_terminal{
          "terminal\\s+([a-zA-Z_\\'][a-zA-Z_0-9\\']*)\\s+([^]*);"};
      std::regex rx_axiom{"start\\s+with\\s+([a-zA-Z_\\'][a-zA-Z_0-9\\']*);"};
      if (std::regex_match(input, match, rx_no_terminal)) {
        symbol_table::put_symbol(match[1], NO_TERMINAL);
      } else if (std::regex_match(input, match, rx_terminal)) {
        symbol_table::put_symbol(match[1], TERMINAL, match[2]);
      } else if (std::regex_match(input, match, rx_axiom)) {
        g.set_axiom(match[1]);
      } else {
        std::cout << "Error while reading tokens: " << input << "\n";
        exit(-1);
      }
    }

    while (getline(file, input) && input != ";") {
      std::regex rx_empty_production{"([a-zA-Z_\\'][a-zA-Z_0-9\\']*)\\s*->;"};
      std::regex rx_production{"([a-zA-Z_\\'][a-zA-Z_0-9\\']*)\\s*->\\s*([a-zA-"
                               "Z_\\'][a-zA-Z_0-9\\s$\\']*);"};
      if (std::regex_match(input, match, rx_production)) {
        std::string s = match[2];
        s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
        g.add_rule(match[1], s);
      } else if (std::regex_match(input, match, rx_empty_production)) {
        g.add_rule(match[1], symbol_table::EPSILON);
      } else {
        std::cout << "Error while reading grammar: " << input << "\n";
        exit(-1);
      }
    }
  }

  file.close();
}

void print_director_symbols(const std::string &antecedent, const production &p,
                            const std::unordered_set<std::string> ds) {
  std::cout << "SD(" << antecedent << " -> ";
  for (unsigned i = 0; i < p.size() - 1; ++i) {
    std::cout << p[i] << " ";
  }

  std::cout << p.back() << ") = {";
  for (const std::string &str : ds) {
    std::cout << str << " ";
  }
  std::cout << "}" << std::endl;
}

void create_ll1_table(const grammar &gr, ll1_table &ll1_t) {
  for (std::pair<const std::string, std::vector<production>> rule : gr.g) {
    std::unordered_map<std::string, std::vector<std::string>> entry;
    for (production p : rule.second) {
      std::unordered_set<std::string> ds = director_symbol(gr, rule.first, p);
      for (const std::string &str : ds) {
        if (!entry.insert({str, p}).second) {
          std::cout << "Grammar is not LL1!" << std::endl;
          exit(-1);
        }
      }
    }
    ll1_t.insert({rule.first, entry});
  }
  std::cout << "Grammar is LL1." << std::endl;
}

bool ll1_parser(const grammar &gr, const std::string &filename) {
  ll1_table ll1;
  create_ll1_table(gr, ll1);
  lexer lex(filename);
  std::stack<std::string> st;
  st.push(gr.AXIOM);
  std::string l = lex.next();
  while (l != "$" && !st.empty()) {
    if (st.top() == symbol_table::EPSILON) {
      st.pop();
      continue;
    }
    std::vector<std::string> ds;
    std::string s = st.top();
    st.pop();
    if (!symbol_table::is_terminal(s)) {
      try {
        ds = ll1.at(s).at(l);
      } catch (const std::out_of_range &s) {
        return false;
      }
      for (std::vector<std::string>::reverse_iterator it = ds.rbegin();
           it != ds.rend(); it++) {
        st.push(*it);
      }
    } else {
      if (s != l) {
        return false;
      }
      l = lex.next();
    }
  }
  return true;
}

int main() {
  grammar gr;
  read_from_file(gr);

  std::cout << "Grammar:\n";
  gr.debug();
  std::cout << "\nInput:" << std::endl;
  std::ifstream file;
  file.open("text.txt");
  std::string line;
  while (getline(file, line)) {
    std::cout << line << std::endl;
  }
  file.close();
  

  if (ll1_parser(gr, "text.txt")) {
    std::cout << "Parsing was successful";
  } else {
    std::cerr << "Parsing encountered an error.";
  }

  std::cout << std::endl;
}
