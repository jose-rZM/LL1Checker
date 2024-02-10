#include <fstream>
#include <iterator>
#include <vector>
struct lexer {
    std::string filename_;
    std::vector<std::string> tokens_;
    unsigned current_;
    lexer(const std::string &filename);
    std::string next();
    void tokenize();
};
