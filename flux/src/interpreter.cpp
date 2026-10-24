#include "interpreter.h"
#include <iostream>
#include <stdexcept>

std::string Interpreter::evaluate(Expr* expr) {
    if (auto lit = dynamic_cast<LiteralExpr*>(expr)) {
        return lit->value.lexeme;
    }
    if (auto bin = dynamic_cast<BinaryExpr*>(expr)) {
        double left = std::stod(evaluate(bin->left.get()));
        double right = std::stod(evaluate(bin->right.get()));
        double result = 0;

        switch (bin->op.type) {
            case TokenType::PLUS: result = left + right; break;
            case TokenType::MINUS: result = left - right; break;
            case TokenType::STAR: result = left * right; break;
            case TokenType::SLASH: 
                if (right == 0) throw std::runtime_error("Divisao por zero");
                result = left / right; 
                break;
            default: break;
        }
        // se for inteiro, mostra sem .0
        std::string s = std::to_string(result);
        // remove zeros desnecessários
        s.erase(s.find_last_not_of('0') + 1, std::string::npos);
        if (s.back() == '.') s.pop_back();
        return s;
    }
    if (auto group = dynamic_cast<GroupingExpr*>(expr)) {
        return evaluate(group->expression.get());
    }
    return "";
}

void Interpreter::execute(Stmt* stmt) {
    if (auto out = dynamic_cast<OutStmt*>(stmt)) {
        std::cout << evaluate(out->value.get()) << std::endl;
    }
}

void Interpreter::interpret(std::vector<std::unique_ptr<Stmt>>& statements) {
    for (auto& stmt : statements) {
        execute(stmt.get());
    }
}