
#include "parser.h"
#include <stdexcept>
#include <unordered_set>
Token Parser::consume(TokenType t, const std::string& m) {
    if (check(t)) {
        return tokens[current++];
    }
    throw std::runtime_error(
        "SyntaxError at line " + std::to_string(peek().line) + ": " + m
    );
}
void Parser::endStatement() {
    if (check(TokenType::NEWLINE)) {
        newlines();
        return;
    }
    if (check(TokenType::RBRACE) || atEnd()) {
        return;
    }
    throw std::runtime_error(
        "SyntaxError at line " + std::to_string(peek().line) +
        ": expected end of line"
    );
}
ExprPtr Parser::primary() {
    if (match(TokenType::NUMBER) ||
        match(TokenType::STRING) ||
        match(TokenType::TRUE) ||
        match(TokenType::FALSE)) {
        return ExprPtr(new LiteralExpr(previous()));
    }
    if (match(TokenType::INPUT)) {
        consume(TokenType::LPAREN, "expected '('");
        ExprPtr prompt;
        if (!check(TokenType::RPAREN)) {
            prompt = expression();
        }
        consume(TokenType::RPAREN, "expected ')'");
        return ExprPtr(new InputExpr(std::move(prompt)));
    }
    if (match(TokenType::LBRACKET)) {
        std::vector<ExprPtr> a;
        if (!check(TokenType::RBRACKET)) {
            do {
                a.push_back(expression());
            } while (match(TokenType::COMMA));
        }
        consume(TokenType::RBRACKET, "expected ']'");
        return ExprPtr(new ArrayExpr(std::move(a)));
    }
    if (match(TokenType::IDENTIFIER)) {
        Token n = previous();
        return ExprPtr(new VariableExpr(n));
    }
    if (match(TokenType::LPAREN)) {
        ExprPtr e = expression();
        consume(TokenType::RPAREN, "expected ')'");
        return e;
    }
    throw std::runtime_error(
        "SyntaxError at line " + std::to_string(peek().line) +
        ": expected expression"
    );
}
ExprPtr Parser::postfix() {
    ExprPtr e = primary();
    while (true) {
        if (match(TokenType::LBRACKET)) {
            ExprPtr i = expression();
            consume(TokenType::RBRACKET, "expected ']'");
            e = ExprPtr(new IndexExpr(std::move(e), std::move(i)));
            continue;
        }
        std::string mod;
        if (match(TokenType::DOT)) {
            auto v = dynamic_cast<VariableExpr*>(e.get());
            if (!v) {
                throw std::runtime_error(
                    "SyntaxError: expected module name before '.'"
                );
            }
            mod = v->name.lexeme;
            Token n = consume(
                TokenType::IDENTIFIER,
                "expected function name after '.'"
            );
            consume(
                TokenType::LPAREN,
                "expected '(' after module function"
            );
            std::vector<ExprPtr> a;
            if (!check(TokenType::RPAREN)) {
                do {
                    a.push_back(expression());
                } while (match(TokenType::COMMA));
            }
            consume(TokenType::RPAREN, "expected ')'");
            e = ExprPtr(new CallExpr(n, mod, std::move(a)));
            continue;
        }
        if (match(TokenType::LPAREN)) {
            auto v = dynamic_cast<VariableExpr*>(e.get());
            if (!v) {
                throw std::runtime_error(
                    "SyntaxError: invalid function call"
                );
            }
            Token n = v->name;
            std::vector<ExprPtr> a;
            if (!check(TokenType::RPAREN)) {
                do {
                    a.push_back(expression());
                } while (match(TokenType::COMMA));
            }
            consume(TokenType::RPAREN, "expected ')'");
            e = ExprPtr(new CallExpr(n, "", std::move(a)));
            continue;
        }
        break;
    }
    return e;
}
ExprPtr Parser::power() {
    ExprPtr e = postfix();
    if (match(TokenType::CARET)) {
        Token op = previous();
        e = ExprPtr(new BinaryExpr(std::move(e), op, unary()));
    }
    return e;
}
ExprPtr Parser::unary() {
    if (match(TokenType::MINUS) || match(TokenType::PLUS)) {
        Token op = previous();
        return ExprPtr(new UnaryExpr(op, unary()));
    }
    return power();
}
ExprPtr Parser::multiplication() {
    ExprPtr e = unary();
    while (check(TokenType::STAR) ||
           check(TokenType::SLASH) ||
           check(TokenType::FLOOR) ||
           check(TokenType::PERCENT)) {
        Token op = tokens[current++];
        e = ExprPtr(new BinaryExpr(std::move(e), op, unary()));
    }
    return e;
}
ExprPtr Parser::addition() {
    ExprPtr e = multiplication();
    while (check(TokenType::PLUS) ||
           check(TokenType::MINUS)) {
        Token op = tokens[current++];
        e = ExprPtr(new BinaryExpr(std::move(e), op, multiplication()));
    }
    return e;
}
ExprPtr Parser::comparison() {
    ExprPtr e = addition();
    while (check(TokenType::LT) ||
           check(TokenType::GT) ||
           check(TokenType::LTE) ||
           check(TokenType::GTE)) {
        Token op = tokens[current++];
        e = ExprPtr(new BinaryExpr(std::move(e), op, addition()));
    }
    return e;
}
ExprPtr Parser::equality() {
    ExprPtr e = comparison();
    while (check(TokenType::EQEQ) ||
           check(TokenType::NOT_EQ)) {
        Token op = tokens[current++];
        e = ExprPtr(new BinaryExpr(std::move(e), op, comparison()));
    }
    return e;
}
ExprPtr Parser::logicalAnd() {
    ExprPtr e = equality();
    while (match(TokenType::AND)) {
        Token op = previous();
        e = ExprPtr(new BinaryExpr(std::move(e), op, equality()));
    }
    return e;
}
ExprPtr Parser::logicalOr() {
    ExprPtr e = logicalAnd();
    while (match(TokenType::OR)) {
        Token op = previous();
        e = ExprPtr(new BinaryExpr(std::move(e), op, logicalAnd()));
    }
    return e;
}
ExprPtr Parser::expression() {
    return logicalOr();
}
StmtPtr Parser::block() {
    consume(TokenType::LBRACE, "expected '{'");
    newlines();
    std::vector<StmtPtr> b;
    while (!check(TokenType::RBRACE) && !atEnd()) {
        b.push_back(statement());
        newlines();
    }
    consume(TokenType::RBRACE, "expected '}'");
    return StmtPtr(new BlockStmt(std::move(b)));
}
StmtPtr Parser::func() {
    Token name = consume(
        TokenType::IDENTIFIER,
        "expected function name"
    );
    consume(TokenType::LPAREN, "expected '('");
    std::vector<Token> params;
    std::unordered_set<std::string> seen;
    if (!check(TokenType::RPAREN)) {
        do {
            Token t = consume(
                TokenType::IDENTIFIER,
                "expected parameter"
            );
            if (!seen.insert(t.lexeme).second) {
                throw std::runtime_error(
                    "SyntaxError: duplicate parameter '" + t.lexeme + "'"
                );
            }
            params.push_back(t);
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RPAREN, "expected ')'");
    newlines();
    ++functionDepth;
    StmtPtr b;
    try {
        b = block();
    } catch (...) {
        --functionDepth;
        throw;
    }
    --functionDepth;
    return StmtPtr(
        new FuncStmt(name, std::move(params), std::move(b))
    );
}
StmtPtr Parser::ifStmt() {
    ExprPtr c = expression();
    newlines();
    StmtPtr yes = block();
    newlines();
    StmtPtr no;
    if (match(TokenType::ELSE)) {
        newlines();
        no = block();
    }
    return StmtPtr(
        new IfStmt(std::move(c), std::move(yes), std::move(no))
    );
}
StmtPtr Parser::whileStmt() {
    ExprPtr c = expression();
    newlines();
    return StmtPtr(
        new WhileStmt(std::move(c), block())
    );
}
StmtPtr Parser::forStmt() {
    Token name = consume(
        TokenType::IDENTIFIER,
        "expected loop variable"
    );
    consume(TokenType::LPAREN, "expected '('");
    ExprPtr count = expression();
    consume(TokenType::RPAREN, "expected ')'");
    newlines();
    return StmtPtr(
        new ForStmt(name, std::move(count), block())
    );
}
StmtPtr Parser::statement() {
    newlines();
    if (match(TokenType::FUNC)) {
        return func();
    }
    if (match(TokenType::IF)) {
        return ifStmt();
    }
    if (match(TokenType::WHILE)) {
        return whileStmt();
    }
    if (match(TokenType::FOR)) {
        return forStmt();
    }
    if (match(TokenType::GET)) {
        Token name = consume(
            TokenType::IDENTIFIER,
            "expected module name"
        );
        endStatement();
        return StmtPtr(new GetStmt(name));
    }
    if (check(TokenType::LBRACE)) {
        return block();
    }
    if (match(TokenType::RETURN)) {
        if (functionDepth == 0) {
            throw std::runtime_error(
                "SyntaxError: return outside function"
            );
        }
        ExprPtr e;
        if (!check(TokenType::NEWLINE) &&
            !check(TokenType::RBRACE) &&
            !atEnd()) {
            e = expression();
        }
        endStatement();
        return StmtPtr(new ReturnStmt(std::move(e)));
    }
    if (match(TokenType::OUT)) {
        ExprPtr e = expression();
        endStatement();
        return StmtPtr(new OutStmt(std::move(e)));
    }
    if (check(TokenType::IDENTIFIER) &&
        current + 1 < tokens.size()) {
        TokenType t = tokens[current + 1].type;
        if (t == TokenType::ASSIGN ||
            t == TokenType::PLUS_EQ ||
            t == TokenType::MINUS_EQ ||
            t == TokenType::STAR_EQ ||
            t == TokenType::SLASH_EQ ||
            t == TokenType::PERCENT_EQ ||
            t == TokenType::CARET_EQ) {
            Token name = tokens[current++];
            Token op = tokens[current++];
            ExprPtr e = expression();
            endStatement();
            return StmtPtr(
                new AssignStmt(name, op, std::move(e))
            );
        }
    }
    ExprPtr e = expression();
    endStatement();
    return StmtPtr(new ExprStmt(std::move(e)));
}
std::vector<StmtPtr> Parser::parse() {
    std::vector<StmtPtr> s;
    newlines();
    while (!atEnd()) {
        s.push_back(statement());
        newlines();
    }
    return s;
}
