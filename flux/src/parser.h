#pragma once
#include "ast.h"
#include "token.h"
#include <vector>
#include <memory>

class Parser {
private:
    std::vector<Token> tokens;
    int current = 0;

    bool isAtEnd();
    Token peek();
    Token previous();
    Token advance();
    bool check(TokenType type);
    bool match(std::vector<TokenType> types);

    std::unique_ptr<Stmt> statement();
    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();

public:
    Parser(std::vector<Token> t) : tokens(t) {}
    std::vector<std::unique_ptr<Stmt>> parse();
};