#pragma once
#include <stdexcept>
class LexerError : public std::exception {
  public:
    explicit LexerError(std::string msg) : msg_(std::move(msg)) {}
    [[nodiscard]] const char *what() const noexcept override {
        return msg_.c_str();
    }

  private:
    std::string msg_;
};
