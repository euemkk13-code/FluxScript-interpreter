
#pragma once
#include "ast.h"
#include "value.h"
#include <unordered_map>
#include <memory>
struct ReturnSignal {
    Value value;
    explicit ReturnSignal(Value v)
        : value(std::move(v)) {}
};
class Interpreter {
    struct Function {
        const FuncStmt* definition;
    };
    std::unordered_map<std::string, Value> globals;
    std::vector<std::unordered_map<std::string, Value>> frames;
    std::unordered_map<std::string, Function> functions;
    struct Module {
        std::vector<StmtPtr> statements;
        std::unique_ptr<Interpreter> runtime;
    };
    std::unordered_map<std::string, std::unique_ptr<Module>> modules;
    std::string moduleDir = "module";
    Value evaluate(const Expr* e);
    void execute(const Stmt* s);
    Value lookup(const std::string& name) const;
    void assign(const std::string& name, Value v);
    Value call(const std::string& name, const std::vector<Value>& args);
    static Value binary(TokenType op, const Value& a, const Value& b);
public:
    void setModuleDir(std::string dir) {
        moduleDir = std::move(dir);
    }
    void interpret(const std::vector<StmtPtr>& statements);
};
