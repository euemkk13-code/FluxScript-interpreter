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
    if (check(TokenType::IDENTIFIER)) {
        if (current + 1 < tokens.size() && tokens[current + 1].type == TokenType::ASSIGN) {
            Token name = advance();
            advance();
            auto value = expression();
            return std::make_unique<AssignStmt>(name, std::move(value));
        }
    }
    if (check(TokenType::IF)) { advance(); auto s = ifStatement(); return s; }
    if (check(TokenType::WHILE)) { advance(); auto s = whileStatement(); return s; }
    if (check(TokenType::LBRACE)) { advance(); auto s = block(); return s; }
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
std::unique_ptr<Stmt> Parser::ifStatement() {
    if (!check(TokenType::LPAREN)) throw std::runtime_error("esperado '('");
    advance();
    auto cond = expression();
    if (!check(TokenType::RPAREN)) throw std::runtime_error("esperado ')'");
    advance();
    auto thenB = statement();
    std::unique_ptr<Stmt> elseB = nullptr;
    if (check(TokenType::ELSE)) { advance(); elseB = statement(); }
    return std::make_unique<IfStmt>(std::move(cond), std::move(thenB), std::move(elseB));
}
std::unique_ptr<Stmt> Parser::whileStatement() {
    if (!check(TokenType::LPAREN)) throw std::runtime_error("esperado '('");
    advance();
    auto cond = expression();
    if (!check(TokenType::RPAREN)) throw std::runtime_error("esperado ')'");
    advance();
    auto body = statement();
    return std::make_unique<WhileStmt>(std::move(cond), std::move(body));
}
std::unique_ptr<Stmt> Parser::block() {
    std::vector<std::unique_ptr<Stmt>> stmts;
    while(!check(TokenType::RBRACE) && !isAtEnd()) stmts.push_back(statement());
    if (!check(TokenType::RBRACE)) throw std::runtime_error("esperado '}'");
    advance();
    return std::make_unique<BlockStmt>(std::move(stmts));
}