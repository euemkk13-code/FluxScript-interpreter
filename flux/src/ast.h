#pragma once
#include "token.h"
#include <memory>
#include <vector>
#include <string>

// === EXPRESSÕES ===
struct Expr {
    virtual ~Expr() = default;
};

struct LiteralExpr : Expr {
    Token value;
    LiteralExpr(Token t) : value(t) {}
};

struct BinaryExpr : Expr {
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
    
    BinaryExpr(std::unique_ptr<Expr> l, Token o, std::unique_ptr<Expr> r)
        : left(std::move(l)), op(o), right(std::move(r)) {}
};

struct GroupingExpr : Expr {
    std::unique_ptr<Expr> expression;
    GroupingExpr(std::unique_ptr<Expr> e) : expression(std::move(e)) {}
};

// === COMANDOS ===
struct Stmt {
    virtual ~Stmt() = default;
};

struct OutStmt : Stmt {
    std::unique_ptr<Expr> value;
    OutStmt(std::unique_ptr<Expr> v) : value(std::move(v)) {}
};

struct ExprStmt : Stmt {
    std::unique_ptr<Expr> expression;
    ExprStmt(std::unique_ptr<Expr> e) : expression(std::move(e)) {}
};