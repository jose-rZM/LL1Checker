#include "../include/lexer.hpp"
#include "../include/lexer_error.hpp"
#include "../include/symbol_table.hpp"
#include <boost/bind/bind.hpp>
#include <boost/ref.hpp>
#include <boost/spirit/include/lex_lexertl.hpp>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

Lex::Lex(std::string filename) : filename_(std::move(filename)), current_() {
    Tokenize();
}

template <typename Lexer> Lex::ParseInput<Lexer>::ParseInput() {
    unsigned long i{1};
    std::string   token_type{symbol_table::token_types_r_.at(i)};
    this->self.add("\\" + symbol_table::EOL_, i);
    ++i;
    for (size_t j = 1; j < symbol_table::order_.size(); ++j) {
        token_type = symbol_table::token_types_r_.at(i);
        std::string regex{symbol_table::st_.at(token_type).second};
        this->self.add(regex, i);
        ++i;
    }
    this->self.add("[ \\t\\n]+", i);
}

template <typename Token>
bool Lex::Add::operator()(Token const& t, std::vector<std::string>& tks) const {
    if (t.id() == symbol_table::i_) {
        return true;
    }
    tks.push_back(symbol_table::token_types_r_.at(t.id()));
    return true;
}

void Lex::Tokenize() {
    ParseInput<boost::spirit::lex::lexertl::lexer<>> functor;
    using boost::placeholders::_1;
    std::ifstream      file(filename_);
    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::string input     = buffer.str();
    char const* first     = input.c_str();
    char const* end       = &first[input.size()];
    bool        completed = boost::spirit::lex::tokenize(
        first, end, functor, boost::bind(Add(), _1, boost::ref(tokens_)));
    if (!completed) {
        std::string rest(first, end);
        throw LexerError("Lexical error: encountered an invalid token: " +
                         rest);
    }
}

std::string Lex::Next() {
    return current_ >= tokens_.size() ? "" : tokens_[current_++];
}