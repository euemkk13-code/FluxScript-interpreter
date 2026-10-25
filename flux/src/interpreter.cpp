#include "interpreter.h"
#include <iostream>
#include <stdexcept>

std::string Interpreter::evaluate(Expr* expr) {
    if (auto var = dynamic_cast<VariableExpr*>(expr)) {
        if (variables.count(var->name.lexeme)) return variables[var->name.lexeme];
        throw std::runtime_error("NameError: Name not defined " + var->name.lexeme);
    }
    if (auto lit = dynamic_cast<LiteralExpr*>(expr)) {
        if (lit->value.type == TokenType::TRUE) return "1";
        if (lit->value.type == TokenType::FALSE) return "0";
        return lit->value.lexeme;
    }
    if (auto inp = dynamic_cast<InputExpr*>(expr)) {
        if (inp->prompt) std::cout << evaluate(inp->prompt.get());
        std::string line; std::getline(std::cin, line);
        return line;
    }
    if (auto call = dynamic_cast<CallExpr*>(expr)) {
        if (!functions.count(call->callee.lexeme)) throw std::runtime_error("NameError: Function not defined " + call->callee.lexeme);
        auto& func = functions.at(call->callee.lexeme);
        if (call->args.size() != func.params.size()) throw std::runtime_error("TypeError: Expected " + std::to_string(func.params.size()) + " args but got " + std::to_string(call->args.size()));
        std::unordered_map<std::string, std::string> saved;
        std::vector<std::string> toRestore;
        for (size_t i=0; i<func.params.size(); ++i) {
            std::string pname = func.params[i].lexeme;
            if (variables.count(pname)) saved[pname] = variables[pname];
            toRestore.push_back(pname);
            variables[pname] = evaluate(call->args[i].get());
        }
        std::string ret = "0";
        try { execute(func.body.get()); }
        catch (ReturnException& r) { ret = r.value; }
        for (auto& name : toRestore) {
            if (saved.count(name)) variables[name] = saved[name];
            else variables.erase(name);
        }
        return ret;
    }
    if (auto bin = dynamic_cast<BinaryExpr*>(expr)) {
        std::string ls = evaluate(bin->left.get());
        std::string rs = evaluate(bin->right.get());
        if (bin->op.type == TokenType::PLUS) {
            try {
                double l = std::stod(ls); double r = std::stod(rs);
                double res = l + r;
                std::string s = std::to_string(res);
                s.erase(s.find_last_not_of('0') + 1, std::string::npos);
                if (!s.empty() && s.back() == '.') s.pop_back();
                if (s.empty()) s = "0";
                return s;
            } catch (...) { return ls + rs; }
        }
        if (bin->op.type == TokenType::PLUS) {
    try {
        double l = std::stod(ls); double r = std::stod(rs);
        double res = l + r;
        std::string s = std::to_string(res);
        s.erase(s.find_last_not_of('0') + 1, std::string::npos);
        if (!s.empty() && s.back() == '.') s.pop_back();
        if (s.empty()) s = "0";
        return s;
    } catch (...) { return ls + rs; }
}
if (bin->op.type == TokenType::EQEQ) {
    try { return (std::stod(ls) == std::stod(rs)) ? "1" : "0"; }
    catch (...) { return (ls == rs) ? "1" : "0"; }
}
if (bin->op.type == TokenType::NOT_EQ) {
    try { return (std::stod(ls) != std::stod(rs)) ? "1" : "0"; }
    catch (...) { return (ls != rs) ? "1" : "0"; }
}

double left = std::stod(ls);
double right = std::stod(rs);
double result = 0;
switch (bin->op.type) {
    case TokenType::MINUS: result = left - right; break;
    case TokenType::STAR: result = left * right; break;
    case TokenType::SLASH: if (right == 0) throw std::runtime_error("Division by zero"); result = left / right; break;
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
    if (auto group = dynamic_cast<GroupingExpr*>(expr)) return evaluate(group->expression.get());
    return "";
}
void Interpreter::execute(Stmt* stmt) {
    if (auto assign = dynamic_cast<AssignStmt*>(stmt)) { variables[assign->name.lexeme] = evaluate(assign->value.get()); return; }
    if (auto out = dynamic_cast<OutStmt*>(stmt)) { std::cout << evaluate(out->value.get()) << std::endl; return; }
    if (auto block = dynamic_cast<BlockStmt*>(stmt)) { for (auto& s : block->statements) execute(s.get()); return; }
    if (auto ifStmt = dynamic_cast<IfStmt*>(stmt)) {
        std::string cond = evaluate(ifStmt->condition.get());
        bool truth = !(cond == "false" || cond == "0" || cond == "");
        if (truth) execute(ifStmt->thenBranch.get()); else if (ifStmt->elseBranch) execute(ifStmt->elseBranch.get());
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
    if (auto func = dynamic_cast<FuncStmt*>(stmt)) {
        functions.emplace(func->name.lexeme, Function(func->params, std::move(func->body)));
        return;
    }
    if (auto ret = dynamic_cast<ReturnStmt*>(stmt)) {
        std::string v = ret->value ? evaluate(ret->value.get()) : "0";
        throw ReturnException(v);
    }
    if (auto exprStmt = dynamic_cast<ExprStmt*>(stmt)) { evaluate(exprStmt->expression.get()); return; }
}
void Interpreter::interpret(std::vector<std::unique_ptr<Stmt>>& statements) { for (auto& stmt : statements) execute(stmt.get()); }