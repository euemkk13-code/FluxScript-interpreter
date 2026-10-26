
#pragma once

#include "token.h"

#include <memory>
#include <vector>

struct Expr {
    virtual ~Expr() = default;
};

using ExprPtr = std::unique_ptr<Expr>;

struct LiteralExpr : Expr {
    Token token;

    explicit LiteralExpr(Token t)
        : token(std::move(t)) {}
};

struct VariableExpr : Expr {
    Token name;

    explicit VariableExpr(Token t)
        : name(std::move(t)) {}
};

struct UnaryExpr : Expr {
    Token op;
    ExprPtr right;

    UnaryExpr(Token t, ExprPtr e)
        : op(std::move(t)),
          right(std::move(e)) {}
};

struct BinaryExpr : Expr {
    ExprPtr left, right;
    Token op;

    BinaryExpr(ExprPtr l, Token t, ExprPtr r)
        : left(std::move(l)),
          right(std::move(r)),
          op(std::move(t)) {}
};

struct InputExpr : Expr {
    ExprPtr prompt;

    explicit InputExpr(ExprPtr p)
        : prompt(std::move(p)) {}
};

struct DictExpr : Expr {
    std::vector<std::pair<std::string, ExprPtr>> entries;
    explicit DictExpr(std::vector<std::pair<std::string, ExprPtr>> e) : entries(std::move(e)) {}
};
struct TernaryExpr : Expr {
    ExprPtr yes, condition, no;
    TernaryExpr(ExprPtr y, ExprPtr c, ExprPtr n) : yes(std::move(y)), condition(std::move(c)), no(std::move(n)) {}
};
struct ArrayExpr : Expr {
    std::vector<ExprPtr> elements;

    explicit ArrayExpr(std::vector<ExprPtr> e)
        : elements(std::move(e)) {}
};

struct IndexExpr : Expr {
    ExprPtr base, index;

    IndexExpr(ExprPtr b, ExprPtr i)
        : base(std::move(b)),
          index(std::move(i)) {}
};

struct CallExpr : Expr {
    Token name;
    std::string module;
    std::vector<ExprPtr> args;

    CallExpr(Token t, std::string m, std::vector<ExprPtr> a)
        : name(std::move(t)),
          module(std::move(m)),
          args(std::move(a)) {}
};

struct Stmt {
    virtual ~Stmt() = default;
};

using StmtPtr = std::unique_ptr<Stmt>;

struct OutStmt : Stmt {
    ExprPtr value;

    explicit OutStmt(ExprPtr v)
        : value(std::move(v)) {}
};

struct AssignStmt : Stmt {
    Token name, op;
    ExprPtr value;

    AssignStmt(Token t, Token o, ExprPtr v)
        : name(std::move(t)),
          op(std::move(o)),
          value(std::move(v)) {}
};

struct ExprStmt : Stmt {
    ExprPtr value;

    explicit ExprStmt(ExprPtr v)
        : value(std::move(v)) {}
};

struct BlockStmt : Stmt {
    std::vector<StmtPtr> body;

    explicit BlockStmt(std::vector<StmtPtr> b)
        : body(std::move(b)) {}
};

struct IfStmt : Stmt {
    ExprPtr condition;
    StmtPtr yes, no;

    IfStmt(ExprPtr c, StmtPtr y, StmtPtr n)
        : condition(std::move(c)),
          yes(std::move(y)),
          no(std::move(n)) {}
};

struct WhileStmt : Stmt {
    ExprPtr condition;
    StmtPtr body;

    WhileStmt(ExprPtr c, StmtPtr b)
        : condition(std::move(c)),
          body(std::move(b)) {}
};

struct ForStmt : Stmt {
    Token name;
    std::vector<ExprPtr> args;
    ExprPtr iterable;
    StmtPtr body;
    ForStmt(Token n, std::vector<ExprPtr> a, ExprPtr it, StmtPtr b)
        : name(std::move(n)), args(std::move(a)), iterable(std::move(it)), body(std::move(b)) {}
};

struct GetStmt : Stmt {
    Token module;

    explicit GetStmt(Token m)
        : module(std::move(m)) {}
};

struct FuncStmt : Stmt {
    Token name;
    std::vector<Token> params;
    std::vector<ExprPtr> defaults;
    StmtPtr body;

    FuncStmt(Token n, std::vector<Token> p, std::vector<ExprPtr> d, StmtPtr b)
        : name(std::move(n)),
          params(std::move(p)), defaults(std::move(d)),
          body(std::move(b)) {}
};

struct ReturnStmt : Stmt {
    ExprPtr value;
    explicit ReturnStmt(ExprPtr v)
        : value(std::move(v)) {}
};
struct IndexAssignStmt : Stmt {
    ExprPtr target, value;
    Token op;
    IndexAssignStmt(ExprPtr t, Token o, ExprPtr v) : target(std::move(t)), value(std::move(v)), op(std::move(o)) {}
};
struct BreakStmt : Stmt {};
struct ContinueStmt : Stmt {};
struct PassStmt : Stmt {};
struct TryStmt : Stmt {
    StmtPtr body, handler;
    Token errorName;
    bool hasName;
    TryStmt(StmtPtr b, StmtPtr h, Token n, bool named)
        : body(std::move(b)), handler(std::move(h)), errorName(std::move(n)), hasName(named) {}
};
