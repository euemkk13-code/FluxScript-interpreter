#pragma once
#include "token.h"
#include <vector>
#include <string>

class Lexer {
private:
    std::string source;
    std::vector<Token> tokens;
    int start = 0;
    int current = 0;
    int line = 1;

    bool isAtEnd();
    char advance();
    char peek();
    char peekNext();
    bool match(char expected);
    void addToken(TokenType type, std::string lexeme);

    void scanToken();
    void number();
    void identifier();
    void string(char quote);
    void lineComment();   // //
    void blockComment();  // --[[ ]]

    bool isDigit(char c);
    bool isAlpha(char c);
    bool isAlphaNumeric(char c);

public:
    Lexer(std::string src) : source(src) {}
    std::vector<Token> scanTokens();
};