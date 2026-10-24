#pragma once
#include "token.h"
#include <memory>
#include <string>
#include <vector>
struct Expr { virtual ~Expr() = default; };

struct LiteralExpr : Expr {
    Token value;
    LiteralExpr(Token t) : value(t) {}
};

struct VariableExpr : Expr {
    Token name;
    VariableExpr(Token n) : name(n) {}
};

struct GroupingExpr : Expr {
    std::unique_ptr<Expr> expression;
    GroupingExpr(std::unique_ptr<Expr> e) : expression(std::move(e)) {}
};

struct BinaryExpr : Expr {
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
    BinaryExpr(std::unique_ptr<Expr> l, Token o, std::unique_ptr<Expr> r) : left(std::move(l)), op(o), right(std::move(r)) {}
};

struct Stmt { virtual ~Stmt() = default; };

struct OutStmt : Stmt {
    std::unique_ptr<Expr> value;
    OutStmt(std::unique_ptr<Expr> v) : value(std::move(v)) {}
};

struct AssignStmt : Stmt {
    Token name;
    std::unique_ptr<Expr> value;
    AssignStmt(Token n, std::unique_ptr<Expr> v) : name(n), value(std::move(v)) {}
};
struct BlockStmt : Stmt {
    std::vector<std::unique_ptr<Stmt>> statements;
    BlockStmt(std::vector<std::unique_ptr<Stmt>> s) : statements(std::move(s)) {}
};
struct IfStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch;
    IfStmt(std::unique_ptr<Expr> c, std::unique_ptr<Stmt> t, std::unique_ptr<Stmt> e) : condition(std::move(c)), thenBranch(std::move(t)), elseBranch(std::move(e)) {}
};
struct WhileStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
    WhileStmt(std::unique_ptr<Expr> c, std::unique_ptr<Stmt> b) : condition(std::move(c)), body(std::move(b)) {}
};
struct ExprStmt : Stmt {
    std::unique_ptr<Expr> expression;
    ExprStmt(std::unique_ptr<Expr> e) : expression(std::move(e)) {}
};