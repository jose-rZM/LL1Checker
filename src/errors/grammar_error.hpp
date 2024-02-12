#pragma once
#include <stdexcept>
#include <utility>

class GrammarError : public std::exception {
  public:
    explicit GrammarError(std::string msg) : msg_(std::move(msg)) {}
    [[nodiscard]] const char *what() const noexcept override {
        return msg_.c_str();
    }

  private:
    std::string msg_;
};