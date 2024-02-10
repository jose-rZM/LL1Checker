#include <fstream>
#include <iterator>
#include <vector>
struct lexer {
    std::string filename;
    std::vector<std::string> tokens;
    unsigned current;
    lexer(const std::string &filename);
    std::string next();
    void tokenize();
};
