#include "interpreter.h"
#include <iostream>
#include <stdexcept>

std::string Interpreter::evaluate(Expr* expr) {
    if (auto var = dynamic_cast<VariableExpr*>(expr)) {
        if (variables.count(var->name.lexeme)) return variables[var->name.lexeme];
        throw std::runtime_error("NameError: Name not defined " + var->name.lexeme);
    }
    if (auto lit = dynamic_cast<LiteralExpr*>(expr)) {
        if (lit->value.type == TokenType::IDENTIFIER) {
            if (variables.count(lit->value.lexeme)) return variables[lit->value.lexeme];
            throw std::runtime_error("NameError: Name not defined " + lit->value.lexeme);
        }
        if (lit->value.type == TokenType::TRUE) return "1";
        if (lit->value.type == TokenType::FALSE) return "0";
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
                if (right == 0) throw std::runtime_error("MathError: Division by zero");
                result = left / right; break;
            case TokenType::EQEQ: result = (left == right) ? 1 : 0; break;
            case TokenType::NOT_EQ: result = (left != right) ? 1 : 0; break;
            case TokenType::LT: result = (left < right) ? 1 : 0; break;
            case TokenType::GT: result = (left > right) ? 1 : 0; break;
            case TokenType::LTE: result = (left <= right) ? 1 : 0; break;
            case TokenType::GTE: result = (left >= right) ? 1 : 0; break;
            default: break;
        }
        std::string s = std::to_string(result);
        s.erase(s.find_last_not_of('0') + 1, std::string::npos);
        if (!s.empty() && s.back() == '.') s.pop_back();
        if (s.empty()) s = "0";
        return s;
    }
    if (auto group = dynamic_cast<GroupingExpr*>(expr)) {
        return evaluate(group->expression.get());
    }
    return "";
}

void Interpreter::execute(Stmt* stmt) {
    if (auto assign = dynamic_cast<AssignStmt*>(stmt)) {
        variables[assign->name.lexeme] = evaluate(assign->value.get());
        return;
    }
    if (auto out = dynamic_cast<OutStmt*>(stmt)) {
        std::cout << evaluate(out->value.get()) << std::endl;
        return;
    }
    if (auto block = dynamic_cast<BlockStmt*>(stmt)) {
        for (auto& s : block->statements) execute(s.get());
        return;
    }
    if (auto ifStmt = dynamic_cast<IfStmt*>(stmt)) {
        std::string cond = evaluate(ifStmt->condition.get());
        bool truth = !(cond == "false" || cond == "0" || cond == "");
        if (truth) execute(ifStmt->thenBranch.get());
        else if (ifStmt->elseBranch) execute(ifStmt->elseBranch.get());
        return;
    }
    if (auto whileStmt = dynamic_cast<WhileStmt*>(stmt)) {
        while (true) {
            std::string cond = evaluate(whileStmt->condition.get());
            bool truth = !(cond == "false" || cond == "0" || cond == "");
            if (!truth) break;
            execute(whileStmt->body.get());
        }
        return;
    }
}

void Interpreter::interpret(std::vector<std::unique_ptr<Stmt>>& statements) {
    for (auto& stmt : statements) execute(stmt.get());
}