#include "../include/lexer.hpp"
#include "../include/lexer_error.hpp"
#include "../include/symbol_table.hpp"
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <regex>
#include <string_view>

namespace {
std::string EscapeRegex(const std::string& str) {
    static const std::regex special{R"([.^$|()\[\]{}*+?\\])"};
    return std::regex_replace(str, special, R"(\$&)");
}

struct Pattern {
    std::string type;
    std::regex  regex;
};
} // namespace

Lex::Lex(std::string filename) : filename_(std::move(filename)), current_() {
    Tokenize();
}

void Lex::Tokenize() {
    std::ifstream file(filename_);
    if (!file) {
        throw LexerError("Cannot open " + filename_);
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::string input = buffer.str();
    size_t      pos   = 0;

    std::vector<Pattern> patterns;
    patterns.reserve(symbol_table::order_.size());

    patterns.push_back({symbol_table::token_types_r_.at(1),
                        std::regex(EscapeRegex(symbol_table::EOL_))});

    for (size_t j = 1; j < symbol_table::order_.size(); ++j) {
        unsigned long id         = symbol_table::order_[j];
        std::string   token_type = symbol_table::token_types_r_.at(id);
        std::string   regex_str  = symbol_table::st_.at(token_type).second;
        patterns.emplace_back(token_type, std::regex(regex_str));
    }

    std::regex ws{R"([ \t\n]+)"};
    while (pos < input.size()) {
        std::string_view remaining{input.data() + pos, input.size() - pos};
        std::cmatch      m;
        if (std::regex_search(remaining.begin(), remaining.end(), m, ws,
                              std::regex_constants::match_continuous)) {
            pos += m.length();
            continue;
        }
        size_t      best_len = 0;
        std::string best_tok;
        for (const auto& p : patterns) {
            if (std::regex_search(remaining.begin(), remaining.end(), m,
                                  p.regex,
                                  std::regex_constants::match_continuous)) {
                if (m.length() > best_len) {
                    best_len = m.length();
                    best_tok = p.type;
                }
            }
        }
        if (best_len == 0) {
            std::string rest(remaining.begin(), remaining.end());
            throw LexerError("Lexical error: encountered an invalid token:\n" +
                             rest);
        }
        tokens_.push_back(best_tok);
        pos += best_len;
    }
}

std::string Lex::Next() {
    return current_ >= tokens_.size() ? std::string() : tokens_[current_++];
}