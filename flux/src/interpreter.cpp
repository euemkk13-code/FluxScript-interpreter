
#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <limits>
#include <chrono>
#include <thread>
#include <ctime>
#include <algorithm>
#include <cstdlib>
#include <cctype>
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
            case Value::Type::DICTIONARY:
                return a.dictionary == b.dictionary;
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
        auto local = frames.back().find(n);
        if (local != frames.back().end()) local->second = std::move(v);
        else if (globals.find(n) != globals.end()) globals[n] = std::move(v);
        else frames.back()[n] = std::move(v);
    }
}
Value Interpreter::binary(TokenType op, const Value& a, const Value& b) {
    if (op == TokenType::IN) {
        if (b.type == Value::Type::ARRAY) {
            for (const Value& v : *b.array) if (equal(a, v)) return Value::truth(true);
            return Value::truth(false);
        }
        if (b.type == Value::Type::DICTIONARY && a.type == Value::Type::STRING) return Value::truth(b.dictionary->count(a.text) != 0);
        if (b.type == Value::Type::STRING && a.type == Value::Type::STRING)
            return Value::truth(b.text.find(a.text) != std::string::npos);
        throw std::runtime_error("TypeError: in requires an array or string");
    }
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
    if (n == "timestamp" || n == "wait" || n == "len" || n == "str" ||
        n == "int" || n == "float" || n == "bool" || n == "type" ||
        n == "readfile" || n == "writefile" || n == "addfile" || n == "fileexists")
        return native(n, args);
    auto it = functions.find(n);
    if (it == functions.end()) {
        throw std::runtime_error(
            "NameError: undefined function '" + n + "'"
        );
    }
    const FuncStmt* f = it->second.definition;
    size_t required = 0;
    for (const auto& d : f->defaults) if (!d) ++required;
    if (args.size() < required || args.size() > f->params.size()) {
        throw std::runtime_error(
            "TypeError: wrong number of arguments for '" + n + "'"
        );
    }
    std::unordered_map<std::string, Value> frame;
    for (size_t i = 0; i < args.size(); ++i) {
        frame[f->params[i].lexeme] = args[i];
    }
    // Evaluate default expressions in the caller's environment.
    for (size_t i = args.size(); i < f->params.size(); ++i)
        frame[f->params[i].lexeme] = evaluate(f->defaults[i].get());
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
Value Interpreter::native(const std::string& n, const std::vector<Value>& a) {
    auto count = [&](size_t k) {
        if (a.size() != k) throw std::runtime_error("TypeError: " + n + " expects " + std::to_string(k) + " arguments");
    };
    if (n == "timestamp") { count(0); return Value::num(static_cast<double>(std::time(nullptr))); }
    if (n == "wait") {
        count(1); double t = numeric(a[0]);
        if (t < 0 || t > 86400) throw std::runtime_error("ValueError: invalid wait duration");
        std::this_thread::sleep_for(std::chrono::duration<double>(t)); return Value();
    }
    if (n == "len") {
        count(1);
        if (a[0].type == Value::Type::STRING) return Value::num(static_cast<double>(a[0].text.size()));
        if (a[0].type == Value::Type::ARRAY) return Value::num(static_cast<double>(a[0].array->size()));
        if (a[0].type == Value::Type::DICTIONARY) return Value::num(static_cast<double>(a[0].dictionary->size()));
        throw std::runtime_error("TypeError: len requires string or array");
    }
    if (n == "str") { count(1); return Value::str(a[0].display()); }
    if (n == "type") {
        count(1);
        switch (a[0].type) {
            case Value::Type::NUMBER: return Value::str("number");
            case Value::Type::STRING: return Value::str("string");
            case Value::Type::BOOL: return Value::str("bool");
            case Value::Type::ARRAY: return Value::str("array");
            case Value::Type::DICTIONARY: return Value::str("dict");
            default: return Value::str("nil");
        }
    }
    if (n == "bool") { count(1); return Value::truth(a[0].isTruthy()); }
    if (n == "int" || n == "float") {
        count(1); double v = 0;
        if (a[0].type == Value::Type::NUMBER) v = a[0].number;
        else if (a[0].type == Value::Type::BOOL) v = a[0].boolean ? 1 : 0;
        else if (a[0].type == Value::Type::STRING) {
            size_t used = 0;
            try { v = std::stod(a[0].text, &used); }
            catch (...) { throw std::runtime_error("ValueError: invalid numeric string"); }
            if (used != a[0].text.size()) throw std::runtime_error("ValueError: invalid numeric string");
        } else throw std::runtime_error("TypeError: cannot convert to number");
        return Value::num(n == "int" ? std::trunc(v) : v);
    }
    if (n == "fileexists") { count(1); std::ifstream f(a[0].display(), std::ios::binary); return Value::truth(f.good()); }
    if (n == "readfile") {
        count(1); std::ifstream f(a[0].display(), std::ios::binary);
        if (!f) throw std::runtime_error("FileError: cannot read '" + a[0].display() + "'");
        std::ostringstream o; o << f.rdbuf(); return Value::str(o.str());
    }
    if (n == "writefile" || n == "addfile") {
        count(2);
        std::ofstream f(a[0].display(), std::ios::binary | (n == "addfile" ? std::ios::app : std::ios::trunc));
        if (!f) throw std::runtime_error("FileError: cannot write '" + a[0].display() + "'");
        f << a[1].display();
        if (!f) throw std::runtime_error("FileError: write failed");
        return Value();
    }
    throw std::runtime_error("NameError: unknown native function '" + n + "'");
}
Value Interpreter::method(const std::string& obj, const std::string& n, const std::vector<Value>& a) {
    Value v = lookup(obj);
    auto count = [&](size_t k) {
        if (a.size() != k) throw std::runtime_error("TypeError: " + obj + "." + n + " expects " + std::to_string(k) + " arguments");
    };
    if (v.type == Value::Type::DICTIONARY) {
        if (n == "remove") {
            count(1);
            if (a[0].type != Value::Type::STRING) throw std::runtime_error("TypeError: dictionary key must be string");
            if (!v.dictionary->erase(a[0].text)) throw std::runtime_error("KeyError: key not found");
            return Value();
        }
        if (n == "keys" || n == "values") {
            count(0); std::vector<Value> result;
            for (const auto& kv : *v.dictionary) result.push_back(n == "keys" ? Value::str(kv.first) : kv.second);
            return Value::list(std::move(result));
        }
        if (n == "len") { count(0); return Value::num(static_cast<double>(v.dictionary->size())); }
    }
    if (v.type == Value::Type::ARRAY) {
        if (n == "sort") {
            count(0);
            auto& items = *v.array;
            for (size_t i=0;i<items.size();++i) {
                if (items[i].type != Value::Type::NUMBER && items[i].type != Value::Type::STRING)
                    throw std::runtime_error("TypeError: sort requires numbers or strings");
                if (i && items[i].type != items[0].type)
                    throw std::runtime_error("TypeError: cannot sort mixed types");
            }
            std::sort(items.begin(),items.end(),[](const Value& x,const Value& y){
                return x.type==Value::Type::NUMBER ? x.number<y.number : x.text<y.text;
            }); return Value();
        }
        if (n == "rev") { count(0); std::reverse(v.array->begin(),v.array->end()); return Value(); }
        if (n == "index" || n == "count") {
            count(1); size_t hits=0;
            for (size_t i=0;i<v.array->size();++i) if (equal((*v.array)[i],a[0])) {
                if (n == "index") return Value::num(static_cast<double>(i));
                ++hits;
            }
            return Value::num(n == "index" ? -1 : static_cast<double>(hits));
        }
        if (n == "push" || n == "add" || n == "append") {
            count(1); v.array->push_back(a[0]); return Value();
        }
        if (n == "pop") {
            count(0); if (v.array->empty()) throw std::runtime_error("IndexError: pop from empty array");
            Value result = v.array->back(); v.array->pop_back(); return result;
        }
        if (n == "remove") {
            count(1);
            auto it = std::find_if(v.array->begin(), v.array->end(), [&](const Value& x){return equal(x,a[0]);});
            if (it == v.array->end()) throw std::runtime_error("ValueError: element not found");
            v.array->erase(it); return Value();
        }
    }
    if (v.type == Value::Type::STRING) {
        if (n == "trim") {
            count(0); size_t start=v.text.find_first_not_of(" \t\r\n");
            if (start==std::string::npos) return Value::str("");
            size_t end=v.text.find_last_not_of(" \t\r\n");
            return Value::str(v.text.substr(start,end-start+1));
        }
        if (n == "startswith" || n == "endswith" || n == "find" || n == "has") {
            count(1); std::string term=a[0].display();
            size_t pos=v.text.find(term);
            if (n == "find") return Value::num(pos==std::string::npos ? -1 : static_cast<double>(pos));
            if (n == "has") return Value::truth(pos!=std::string::npos);
            if (n == "startswith") return Value::truth(pos==0);
            return Value::truth(term.size()<=v.text.size() && v.text.compare(v.text.size()-term.size(),term.size(),term)==0);
        }
        if (n == "append" || n == "add") {
            count(1); v.text += a[0].display(); assign(obj,v); return v;
        }
        if (n == "upper" || n == "lower") {
            count(0); std::string t = v.text;
            for (char& c : t) {
                unsigned char ch = static_cast<unsigned char>(c);
                c = static_cast<char>(n == "upper" ? std::toupper(ch) : std::tolower(ch));
            }
            return Value::str(t);
        }
        if (n == "replace") {
            count(2); std::string from = a[0].display(), to = a[1].display(), t = v.text;
            if (from.empty()) throw std::runtime_error("ValueError: empty search string");
            size_t pos = 0;
            while ((pos = t.find(from,pos)) != std::string::npos) { t.replace(pos,from.size(),to); pos += to.size(); }
            return Value::str(t);
        }
        if (n == "split") {
            count(1); std::string delim = a[0].display();
            if (delim.empty()) throw std::runtime_error("ValueError: empty delimiter");
            std::vector<Value> out; size_t start = 0, pos;
            while ((pos = v.text.find(delim,start)) != std::string::npos) {
                out.push_back(Value::str(v.text.substr(start,pos-start))); start = pos + delim.size();
            }
            out.push_back(Value::str(v.text.substr(start))); return Value::list(std::move(out));
        }
    }
    throw std::runtime_error("TypeError: unsupported method '" + n + "' for " + obj);
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
            case TokenType::NIL:
                return Value();
            default:
                return Value::num(std::stod(p->token.lexeme));
        }
    }
    if (auto p = dynamic_cast<const VariableExpr*>(e)) {
        return lookup(p->name.lexeme);
    }
    if (auto p = dynamic_cast<const UnaryExpr*>(e)) {
        Value right = evaluate(p->right.get());
        if (p->op.type == TokenType::NOT) return Value::truth(!right.isTruthy());
        double n = numeric(right);
        return Value::num(
            p->op.type == TokenType::MINUS ? -n : n
        );
    }
    if (auto p = dynamic_cast<const TernaryExpr*>(e)) {
        return evaluate(p->condition.get()).isTruthy() ? evaluate(p->yes.get()) : evaluate(p->no.get());
    }
    if (auto p = dynamic_cast<const DictExpr*>(e)) {
        std::map<std::string,Value> entries;
        for (const auto& kv : p->entries) entries[kv.first] = evaluate(kv.second.get());
        return Value::dict(std::move(entries));
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

    if (a.type != Value::Type::ARRAY &&
        a.type != Value::Type::STRING) {
        throw std::runtime_error(
            "TypeError: indexing requires array or string"
        );
    }

    double n = numeric(i);

    size_t length = a.type == Value::Type::ARRAY
        ? a.array->size()
        : a.text.size();

    if (n < 0 ||
        std::floor(n) != n ||
        n >= static_cast<double>(length)) {
        throw std::runtime_error(
            "IndexError: index out of range"
        );
    }

    size_t index = static_cast<size_t>(n);

    if (a.type == Value::Type::STRING) {
        return Value::str(a.text.substr(index, 1));
    }

    return (*a.array)[index];
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
        // Infer the type of user input, keeping non-numeric text unchanged.
        const std::string::size_type first = line.find_first_not_of(" \t\r\n");
        if (first != std::string::npos) {
            const std::string::size_type last = line.find_last_not_of(" \t\r\n");
            const std::string trimmed = line.substr(first, last - first + 1);
            if (trimmed == "true") return Value::truth(true);
            if (trimmed == "false") return Value::truth(false);
            if (trimmed == "nil") return Value();
            try {
                size_t consumed = 0;
                double number = std::stod(trimmed, &consumed);
                if (consumed == trimmed.size() && std::isfinite(number))
                    return Value::num(number);
            } catch (const std::invalid_argument&) {
            } catch (const std::out_of_range&) {
            }
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
        if (modules.find(p->module) == modules.end()) return method(p->module, p->name.lexeme, args);
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
    if (dynamic_cast<const PassStmt*>(s)) return;
    if (dynamic_cast<const BreakStmt*>(s)) throw BreakSignal();
    if (dynamic_cast<const ContinueStmt*>(s)) throw ContinueSignal();
    if (auto p = dynamic_cast<const TryStmt*>(s)) {
        try { execute(p->body.get()); }
        catch (const std::exception& e) {
            if (p->hasName) assign(p->errorName.lexeme, Value::str(e.what()));
            execute(p->handler.get());
        }
        return;
    }
    if (auto p = dynamic_cast<const IndexAssignStmt*>(s)) {
        auto idx = dynamic_cast<const IndexExpr*>(p->target.get());
        Value base = evaluate(idx->base.get());
        if (base.type == Value::Type::DICTIONARY) {
            Value key=evaluate(idx->index.get());
            if (key.type!=Value::Type::STRING) throw std::runtime_error("TypeError: dictionary key must be string");
            Value v=evaluate(p->value.get());
            if (p->op.type!=TokenType::ASSIGN) {
                auto it=base.dictionary->find(key.text);
                if (it==base.dictionary->end()) throw std::runtime_error("KeyError: key not found");
                TokenType op=TokenType::PLUS;
                switch(p->op.type){case TokenType::MINUS_EQ:op=TokenType::MINUS;break;case TokenType::STAR_EQ:op=TokenType::STAR;break;case TokenType::SLASH_EQ:op=TokenType::SLASH;break;case TokenType::PERCENT_EQ:op=TokenType::PERCENT;break;case TokenType::CARET_EQ:op=TokenType::CARET;break;default:break;}
                v=binary(op,it->second,v);
            }
            (*base.dictionary)[key.text]=v; return;
        }
        if (base.type != Value::Type::ARRAY) throw std::runtime_error("TypeError: indexed assignment requires array");
        double i = numeric(evaluate(idx->index.get()));
        if (i < 0 || std::floor(i) != i || i >= base.array->size())
            throw std::runtime_error("IndexError: array index out of range");
        Value v = evaluate(p->value.get());
        if (p->op.type != TokenType::ASSIGN) {
            TokenType op = TokenType::PLUS;
            switch (p->op.type) {
                case TokenType::PLUS_EQ: op=TokenType::PLUS; break;
                case TokenType::MINUS_EQ: op=TokenType::MINUS; break;
                case TokenType::STAR_EQ: op=TokenType::STAR; break;
                case TokenType::SLASH_EQ: op=TokenType::SLASH; break;
                case TokenType::PERCENT_EQ: op=TokenType::PERCENT; break;
                case TokenType::CARET_EQ: op=TokenType::CARET; break;
                default: break;
            }
            v = binary(op, (*base.array)[static_cast<size_t>(i)], v);
        }
        (*base.array)[static_cast<size_t>(i)] = v;
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
            try { execute(p->body.get()); }
            catch (const ContinueSignal&) { continue; }
            catch (const BreakSignal&) { break; }
        }
        return;
    }
    if (auto p = dynamic_cast<const ForStmt*>(s)) {
        auto run = [&](Value v) -> bool {
            assign(p->name.lexeme, std::move(v));
            try { execute(p->body.get()); }
            catch (const ContinueSignal&) { return true; }
            catch (const BreakSignal&) { return false; }
            return true;
        };
        if (p->iterable) {
            Value values = evaluate(p->iterable.get());
            if (values.type == Value::Type::DICTIONARY) {
                for (const auto& kv : *values.dictionary) if (!run(Value::str(kv.first))) break;
            } else if (values.type == Value::Type::ARRAY) {
                auto snapshot = *values.array;
                for (const auto& v : snapshot) if (!run(v)) break;
            } else if (values.type == Value::Type::STRING) {
                for (char c : values.text) if (!run(Value::str(std::string(1,c)))) break;
            } else throw std::runtime_error("TypeError: for in requires array or string");
        } else {
            std::vector<double> nums;
            for (const auto& arg : p->args) nums.push_back(numeric(evaluate(arg.get())));
            double start = nums.size() == 1 ? 0 : nums[0];
            double stop = nums.size() == 1 ? nums[0] : nums[1];
            double step = nums.size() == 3 ? nums[2] : 1;
            if (step == 0) throw std::runtime_error("ValueError: for step cannot be zero");
            if (std::floor(start)!=start || std::floor(stop)!=stop || std::floor(step)!=step)
                throw std::runtime_error("ValueError: for arguments must be integers");
            size_t iterations = 0;
            for (double i=start; step>0 ? i<stop : i>stop; i+=step) {
                if (++iterations > 100000000) throw std::runtime_error("ValueError: for exceeded iteration limit");
                if (!run(Value::num(i))) break;
            }
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