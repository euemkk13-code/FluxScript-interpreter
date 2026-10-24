#pragma once
#include "ast.h"
#include <string>

class Interpreter {
private:
    std::string evaluate(Expr* expr);
public:
    void execute(Stmt* stmt);
    void interpret(std::vector<std::unique_ptr<Stmt>>& statements);
};