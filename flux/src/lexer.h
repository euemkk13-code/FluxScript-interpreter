#pragma once
#include "token.h"
#include <vector>
class Lexer {
    std::string source;
    std::vector<Token> tokens;
    size_t current = 0;
    int line = 1;
    bool end() const {
        return current >= source.size();
    }
    char peek(size_t ahead = 0) const {
        return current + ahead < source.size()
            ? source[current + ahead]
            : '\0';
    }
    char advance() {
        return source[current++];
    }
    void add(TokenType t, std::string s) {
        tokens.emplace_back(t, std::move(s), line);
    }
    void scan();
public:
    explicit Lexer(std::string s)
        : source(std::move(s)) {}
    std::vector<Token> scanTokens();
};