#include <fstream>
#include <iterator>
#include <vector>
struct lexer {
  std::ifstream file;
  std::vector<std::string> tokens;
  unsigned current;
  lexer(const std::string &filename);
  ~lexer();
  std::string next();
  void tokenize();
};
