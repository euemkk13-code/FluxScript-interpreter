
#pragma once
#include "ast.h"
class Parser {
    std::vector<Token> tokens;
    size_t current = 0;
    int functionDepth = 0;
    const Token& peek() const {
        return tokens[current];
    }
    const Token& previous() const {
        return tokens[current - 1];
    }
    bool atEnd() const {
        return peek().type == TokenType::END_OF_FILE;
    }
    bool check(TokenType t) const {
        return peek().type == t;
    }
    bool match(TokenType t) {
        if (check(t)) {
            ++current;
            return true;
        }
        return false;
    }
    Token consume(TokenType t, const std::string& message);
    void newlines() {
        while (match(TokenType::NEWLINE)) {}
    }
    void endStatement();
    StmtPtr statement();
    StmtPtr block();
    StmtPtr func();
    StmtPtr ifStmt();
    StmtPtr whileStmt();
    StmtPtr forStmt();
    ExprPtr expression();
    ExprPtr logicalOr();
    ExprPtr logicalAnd();
    ExprPtr equality();
    ExprPtr comparison();
    ExprPtr addition();
    ExprPtr multiplication();
    ExprPtr unary();
    ExprPtr power();
    ExprPtr postfix();
    ExprPtr primary();
public:
    explicit Parser(std::vector<Token> t)
        : tokens(std::move(t)) {}
    std::vector<StmtPtr> parse();
};
