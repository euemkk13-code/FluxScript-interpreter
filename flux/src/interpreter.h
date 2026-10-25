#pragma once
#include "ast.h"
#include <string>
#include <unordered_map>
struct Function { std::vector<Token> params; std::unique_ptr<Stmt> body; Function(std::vector<Token> p, std::unique_ptr<Stmt> b) : params(std::move(p)), body(std::move(b)) {} };
struct ReturnException { std::string value; ReturnException(std::string v): value(v) {} };
class Interpreter {
private:
    std::unordered_map<std::string, std::string> variables;
    std::unordered_map<std::string, Function> functions;
    std::string evaluate(Expr* expr);
public:
    void execute(Stmt* stmt);
    void interpret(std::vector<std::unique_ptr<Stmt>>& statements);
};