#pragma once
#include "ast.h"
#include <string>
#include <unordered_map>
class Interpreter {
private:
    std::unordered_map<std::string, std::string> variables;
    std::string evaluate(Expr* expr);
public:
    void execute(Stmt* stmt);
    void interpret(std::vector<std::unique_ptr<Stmt>>& statements);
};