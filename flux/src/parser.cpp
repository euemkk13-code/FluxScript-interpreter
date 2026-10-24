#include "parser.h"

bool Parser::isAtEnd() { return peek().type == TokenType::END_OF_FILE; }
Token Parser::peek() { return tokens[current]; }
Token Parser::previous() { return tokens[current - 1]; }
Token Parser::advance() { if (!isAtEnd()) current++; return previous(); }
bool Parser::check(TokenType type) { if (isAtEnd()) return false; return peek().type == type; }
bool Parser::match(std::vector<TokenType> types) {
    for (auto type : types) {
        if (check(type)) { advance(); return true; }
    }
    return false;
}

std::unique_ptr<Expr> Parser::factor() {
    if (match({TokenType::NUMBER, TokenType::STRING, TokenType::IDENTIFIER})) {
        return std::make_unique<LiteralExpr>(previous());
    }
    if (match({TokenType::LPAREN})) {
        auto expr = expression();
        advance(); // consome )
        return std::make_unique<GroupingExpr>(std::move(expr));
    }
    return nullptr;
}

std::unique_ptr<Expr> Parser::term() {
    auto expr = factor();
    while (match({TokenType::STAR, TokenType::SLASH})) {
        Token op = previous();
        auto right = factor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::expression() {
    auto expr = term();
    while (match({TokenType::PLUS, TokenType::MINUS})) {
        Token op = previous();
        auto right = term();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Stmt> Parser::statement() {
    if (match({TokenType::OUT})) {
        auto value = expression();
        return std::make_unique<OutStmt>(std::move(value));
    }
    return nullptr;
}

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!isAtEnd()) {
        if (auto stmt = statement()) {
            statements.push_back(std::move(stmt));
        } else {
            advance();
        }
    }
    return statements;
}