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
    if (match({TokenType::NUMBER, TokenType::STRING, TokenType::IDENTIFIER, TokenType::TRUE, TokenType::FALSE})) {
        return std::make_unique<LiteralExpr>(previous());
    }
    if (match({TokenType::LPAREN})) {
        auto expr = expression();
        if (!check(TokenType::RPAREN)) throw std::runtime_error("esperado ')'");
        advance();
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

// NOVO: comparacao
std::unique_ptr<Expr> Parser::comparison() {
    auto expr = term();
    while (match({TokenType::LT, TokenType::GT, TokenType::LTE, TokenType::GTE, TokenType::EQEQ, TokenType::NOT_EQ})) {
        Token op = previous();
        auto right = term();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::expression() {
    auto expr = comparison();
    while (match({TokenType::PLUS, TokenType::MINUS})) {
        Token op = previous();
        auto right = comparison();
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
    if (check(TokenType::IF)) { advance(); return ifStatement(); }
    if (check(TokenType::WHILE)) { advance(); return whileStatement(); }
    if (check(TokenType::LBRACE)) { advance(); return block(); }
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
    if (!check(TokenType::LPAREN)) throw std::runtime_error("esperado '(' no if");
    advance();
    auto cond = expression();
    if (!check(TokenType::RPAREN)) throw std::runtime_error("esperado ')' no if");
    advance();
    auto thenB = statement();
    std::unique_ptr<Stmt> elseB = nullptr;
    if (check(TokenType::ELSE)) { advance(); elseB = statement(); }
    return std::make_unique<IfStmt>(std::move(cond), std::move(thenB), std::move(elseB));
}

std::unique_ptr<Stmt> Parser::whileStatement() {
    if (!check(TokenType::LPAREN)) throw std::runtime_error("esperado '(' no while");
    advance();
    auto cond = expression();
    if (!check(TokenType::RPAREN)) throw std::runtime_error("esperado ')' no while");
    advance();
    auto body = statement();
    return std::make_unique<WhileStmt>(std::move(cond), std::move(body));
}

std::unique_ptr<Stmt> Parser::block() {
    std::vector<std::unique_ptr<Stmt>> stmts;
    while(!check(TokenType::RBRACE) && !isAtEnd()) {
        if (auto s = statement()) stmts.push_back(std::move(s));
        else advance();
    }
    if (!check(TokenType::RBRACE)) throw std::runtime_error("esperado '}'");
    advance();
    return std::make_unique<BlockStmt>(std::move(stmts));
}