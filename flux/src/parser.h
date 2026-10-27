#pragma once
#include "ast.h"
#include "token.h"
class Parser {
    std::vector<Token> tokens;
    size_t current = 0;
    int functionDepth = 0;
    int loopDepth = 0;
    int classDepth = 0;

    const Token& peek() const {
        return tokens[current];
    }

    const Token& previous() const {
        return tokens[current - 1];
    }

    bool atEnd() const {
        return peek().type == FluxTokenType::END_OF_FILE;
    }

    bool check(FluxTokenType t) const {
        return peek().type == t;
    }

    bool match(FluxTokenType t) {
        if (check(t)) {
            ++current;
            return true;
        }
        return false;
    }

    Token consume(FluxTokenType t, const std::string& message);

    void newlines() {
        while (match(FluxTokenType::NEWLINE)) {}
    }

    void endStatement();

    StmtPtr statement();
    StmtPtr block();
    StmtPtr func();
    StmtPtr classStmt();
    StmtPtr ifStmt();
    StmtPtr ifTail();
    StmtPtr whileStmt();
    StmtPtr forStmt();

    ExprPtr expression();
    ExprPtr logicalOr();
    ExprPtr ternary();
    ExprPtr logicalAnd();
    ExprPtr equality();
    ExprPtr logicalNot();
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