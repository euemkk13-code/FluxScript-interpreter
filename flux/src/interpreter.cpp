
#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <limits>
namespace {
    double numeric(const Value& v) {
        if (v.type != Value::Type::NUMBER) {
            throw std::runtime_error(
                "TypeError: expected number, got '" + v.display() + "'"
            );
        }
        return v.number;
    }
    bool equal(const Value& a, const Value& b) {
        if (a.type != b.type) {
            return false;
        }
        switch (a.type) {
            case Value::Type::NUMBER:
                return a.number == b.number;
            case Value::Type::STRING:
                return a.text == b.text;
            case Value::Type::BOOL:
                return a.boolean == b.boolean;
            case Value::Type::ARRAY:
                return a.array == b.array;
            default:
                return true;
        }
    }
}
Value Interpreter::lookup(const std::string& n) const {
    if (!frames.empty()) {
        auto it = frames.back().find(n);
        if (it != frames.back().end()) {
            return it->second;
        }
    }
    auto it = globals.find(n);
    if (it != globals.end()) {
        return it->second;
    }
    throw std::runtime_error(
        "NameError: undefined variable '" + n + "'"
    );
}
void Interpreter::assign(const std::string& n, Value v) {
    if (frames.empty()) {
        globals[n] = std::move(v);
    } else {
        frames.back()[n] = std::move(v);
    }
}
Value Interpreter::binary(TokenType op, const Value& a, const Value& b) {
    if (op == TokenType::EQEQ) {
        return Value::truth(equal(a, b));
    }
    if (op == TokenType::NOT_EQ) {
        return Value::truth(!equal(a, b));
    }
    if (op == TokenType::PLUS &&
        a.type == Value::Type::STRING &&
        b.type == Value::Type::STRING) {
        return Value::str(a.text + b.text);
    }
    if (op == TokenType::LT ||
        op == TokenType::GT ||
        op == TokenType::LTE ||
        op == TokenType::GTE) {
        bool result;
        if (a.type == Value::Type::STRING &&
            b.type == Value::Type::STRING) {
            if (op == TokenType::LT) {
                result = a.text < b.text;
            } else if (op == TokenType::GT) {
                result = a.text > b.text;
            } else if (op == TokenType::LTE) {
                result = a.text <= b.text;
            } else {
                result = a.text >= b.text;
            }
        } else {
            double x = numeric(a);
            double y = numeric(b);
            if (op == TokenType::LT) {
                result = x < y;
            } else if (op == TokenType::GT) {
                result = x > y;
            } else if (op == TokenType::LTE) {
                result = x <= y;
            } else {
                result = x >= y;
            }
        }
        return Value::truth(result);
    }
    double x = numeric(a);
    double y = numeric(b);
    switch (op) {
        case TokenType::PLUS:
            return Value::num(x + y);
        case TokenType::MINUS:
            return Value::num(x - y);
        case TokenType::STAR:
            return Value::num(x * y);
        case TokenType::SLASH:
            if (y == 0) {
                throw std::runtime_error(
                    "ZeroDivisionError: division by zero"
                );
            }
            return Value::num(x / y);
        case TokenType::FLOOR:
            if (y == 0) {
                throw std::runtime_error(
                    "ZeroDivisionError: integer division by zero"
                );
            }
            return Value::num(std::floor(x / y));
        case TokenType::PERCENT:
            if (y == 0) {
                throw std::runtime_error(
                    "ZeroDivisionError: modulo by zero"
                );
            }
            return Value::num(x - std::floor(x / y) * y);
        case TokenType::CARET:
            return Value::num(std::pow(x, y));
        default:
            throw std::runtime_error(
                "SyntaxError: unknown operator"
            );
    }
}
Value Interpreter::call(
    const std::string& n,
    const std::vector<Value>& args
) {
    auto it = functions.find(n);
    if (it == functions.end()) {
        throw std::runtime_error(
            "NameError: undefined function '" + n + "'"
        );
    }
    const FuncStmt* f = it->second.definition;
    if (args.size() != f->params.size()) {
        throw std::runtime_error(
            "TypeError: wrong number of arguments for '" + n + "'"
        );
    }
    std::unordered_map<std::string, Value> frame;
    for (size_t i = 0; i < args.size(); ++i) {
        frame[f->params[i].lexeme] = args[i];
    }
    frames.push_back(std::move(frame));
    Value result;
    try {
        execute(f->body.get());
    } catch (const ReturnSignal& r) {
        result = r.value;
    } catch (...) {
        frames.pop_back();
        throw;
    }
    frames.pop_back();
    return result;
}
Value Interpreter::evaluate(const Expr* e) {
    if (auto p = dynamic_cast<const LiteralExpr*>(e)) {
        switch (p->token.type) {
            case TokenType::STRING:
                return Value::str(p->token.lexeme);
            case TokenType::TRUE:
                return Value::truth(true);
            case TokenType::FALSE:
                return Value::truth(false);
            default:
                return Value::num(std::stod(p->token.lexeme));
        }
    }
    if (auto p = dynamic_cast<const VariableExpr*>(e)) {
        return lookup(p->name.lexeme);
    }
    if (auto p = dynamic_cast<const UnaryExpr*>(e)) {
        double n = numeric(evaluate(p->right.get()));
        return Value::num(
            p->op.type == TokenType::MINUS ? -n : n
        );
    }
    if (auto p = dynamic_cast<const ArrayExpr*>(e)) {
        std::vector<Value> v;
        for (const auto& a : p->elements) {
            v.push_back(evaluate(a.get()));
        }
        return Value::list(std::move(v));
    }
    if (auto p = dynamic_cast<const IndexExpr*>(e)) {
        Value a = evaluate(p->base.get());
        Value i = evaluate(p->index.get());
        if (a.type != Value::Type::ARRAY) {
            throw std::runtime_error(
                "TypeError: indexing requires array"
            );
        }
        double n = numeric(i);
        if (n < 0 ||
            std::floor(n) != n ||
            n >= a.array->size()) {
            throw std::runtime_error(
                "IndexError: array index out of range"
            );
        }
        return (*a.array)[static_cast<size_t>(n)];
    }
    if (auto p = dynamic_cast<const InputExpr*>(e)) {
        if (p->prompt) {
            std::cout << evaluate(p->prompt.get()).display()
                      << std::flush;
        }
        std::string line;
        if (!std::getline(std::cin, line)) {
            throw std::runtime_error(
                "InputError: end of input"
            );
        }
        return Value::str(line);
    }
    if (auto p = dynamic_cast<const CallExpr*>(e)) {
        std::vector<Value> args;
        for (const auto& a : p->args) {
            args.push_back(evaluate(a.get()));
        }
        if (p->module.empty()) {
            return call(p->name.lexeme, args);
        }
        auto it = modules.find(p->module);
        if (it == modules.end()) {
            throw std::runtime_error(
                "ModuleError: module '" + p->module + "' not loaded"
            );
        }
        return it->second->runtime->call(p->name.lexeme, args);
    }
    if (auto p = dynamic_cast<const BinaryExpr*>(e)) {
        Value a = evaluate(p->left.get());
        if (p->op.type == TokenType::AND) {
            return Value::truth(
                a.isTruthy() &&
                evaluate(p->right.get()).isTruthy()
            );
        }
        if (p->op.type == TokenType::OR) {
            return Value::truth(
                a.isTruthy() ||
                evaluate(p->right.get()).isTruthy()
            );
        }
        return binary(
            p->op.type,
            a,
            evaluate(p->right.get())
        );
    }
    throw std::runtime_error(
        "InternalError: unknown expression"
    );
}
void Interpreter::execute(const Stmt* s) {
    if (auto p = dynamic_cast<const OutStmt*>(s)) {
        std::cout << evaluate(p->value.get()).display() << '\n';
        return;
    }
    if (auto p = dynamic_cast<const AssignStmt*>(s)) {
        Value v = evaluate(p->value.get());
        if (p->op.type != TokenType::ASSIGN) {
            TokenType op = TokenType::PLUS;
            switch (p->op.type) {
                case TokenType::PLUS_EQ:
                    op = TokenType::PLUS;
                    break;
                case TokenType::MINUS_EQ:
                    op = TokenType::MINUS;
                    break;
                case TokenType::STAR_EQ:
                    op = TokenType::STAR;
                    break;
                case TokenType::SLASH_EQ:
                    op = TokenType::SLASH;
                    break;
                case TokenType::PERCENT_EQ:
                    op = TokenType::PERCENT;
                    break;
                case TokenType::CARET_EQ:
                    op = TokenType::CARET;
                    break;
                default:
                    break;
            }
            v = binary(
                op,
                lookup(p->name.lexeme),
                v
            );
        }
        assign(p->name.lexeme, std::move(v));
        return;
    }
    if (auto p = dynamic_cast<const ExprStmt*>(s)) {
        evaluate(p->value.get());
        return;
    }
    if (auto p = dynamic_cast<const BlockStmt*>(s)) {
        for (const auto& child : p->body) {
            execute(child.get());
        }
        return;
    }
    if (auto p = dynamic_cast<const IfStmt*>(s)) {
        if (evaluate(p->condition.get()).isTruthy()) {
            execute(p->yes.get());
        } else if (p->no) {
            execute(p->no.get());
        }
        return;
    }
    if (auto p = dynamic_cast<const WhileStmt*>(s)) {
        while (evaluate(p->condition.get()).isTruthy()) {
            execute(p->body.get());
        }
        return;
    }
    if (auto p = dynamic_cast<const ForStmt*>(s)) {
        double n = numeric(evaluate(p->count.get()));
        if (n < 0 ||
            std::floor(n) != n ||
            n > 100000000) {
            throw std::runtime_error(
                "ValueError: for count must be a nonnegative integer <= 100000000"
            );
        }
        for (long long i = 0;
             i < static_cast<long long>(n);
             ++i) {
            assign(
                p->name.lexeme,
                Value::num(static_cast<double>(i))
            );
            execute(p->body.get());
        }
        return;
    }
    if (auto p = dynamic_cast<const FuncStmt*>(s)) {
        functions[p->name.lexeme] = Function{p};
        return;
    }
    if (auto p = dynamic_cast<const ReturnStmt*>(s)) {
        throw ReturnSignal(
            p->value
                ? evaluate(p->value.get())
                : Value()
        );
    }
    if (auto p = dynamic_cast<const GetStmt*>(s)) {
        std::string name = p->module.lexeme;
        if (modules.count(name)) {
            return;
        }
        std::string path = moduleDir + "/" + name + ".flux";
        std::ifstream file(path.c_str());
        if (!file) {
            throw std::runtime_error(
                "ModuleError: cannot open '" + path + "'"
            );
        }
        std::ostringstream buffer;
        buffer << file.rdbuf();
        Lexer lexer(buffer.str());
        Parser parser(lexer.scanTokens());
        std::unique_ptr<Module> mod(new Module());
        mod->statements = parser.parse();
        mod->runtime.reset(new Interpreter());
        mod->runtime->setModuleDir(moduleDir);
        mod->runtime->interpret(mod->statements);
        modules[name] = std::move(mod);
        return;
    }
    throw std::runtime_error(
        "InternalError: unknown statement"
    );
}
void Interpreter::interpret(
    const std::vector<StmtPtr>& statements
) {
    for (const auto& s : statements) {
        execute(s.get());
    }
}