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
#include <functional>
#include <csignal>
#ifdef _WIN32
#include <conio.h>
#endif
#include <cstdio>
#include <cerrno>
#include <cstring>
namespace {
    volatile std::sig_atomic_t fluxRunning=1;
    void fluxStop(int) {
        fluxRunning=0;
    }
}
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
    }
    else {
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
            }
            else if (op == TokenType::GT) {
                result = a.text > b.text;
            }
            else if (op == TokenType::LTE) {
                result = a.text <= b.text;
            }
            else {
                result = a.text >= b.text;
            }
        }
        else {
            double x = numeric(a);
            double y = numeric(b);
            if (op == TokenType::LT) {
                result = x < y;
            }
            else if (op == TokenType::GT) {
                result = x > y;
            }
            else if (op == TokenType::LTE) {
                result = x <= y;
            }
            else {
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
Value Interpreter::invoke(const FuncStmt* f,const std::vector<Value>& args,const Value* self) {
    size_t required=0;
    for(const auto& d:f->defaults) if(!d) ++required;
    if(args.size()<required || args.size()>f->params.size()) throw std::runtime_error("TypeError: wrong number of arguments");
    std::unordered_map<std::string,Value> frame;
    if(self) frame["self"]=*self;
    for(size_t i=0;i<args.size();++i) frame[f->params[i].lexeme]=args[i];
    for(size_t i=args.size();i<f->params.size();++i) frame[f->params[i].lexeme]=evaluate(f->defaults[i].get());
    frames.push_back(std::move(frame));
    Value result;
    try {
        execute(f->body.get());
    }
    catch(const ReturnSignal& r) {
        result=r.value;
    }
    catch(...) {
        frames.pop_back();
        throw;
    }
    frames.pop_back();
    return result;
}
Value Interpreter::call(
const std::string& n,
const std::vector<Value>& args
) {
    if (n == "timestamp" || n == "wait" || n == "len" || n == "str" ||
    n == "int" || n == "float" || n == "bool" || n == "type" ||
    n == "readfile" || n == "writefile" || n == "addfile" ||
    n == "fileexists" || n == "fileExists" || n == "deleteFile" ||
    n == "readFile" || n == "writeFile" || n == "saveFile" || n == "addFile")
    return native(n, args);
    auto it = functions.find(n);
    if (it == functions.end()) {
        throw std::runtime_error(
        "NameError: undefined function '" + n + "'"
        );
    }
    return invoke(it->second.definition,args);
}
Value Interpreter::native(const std::string& n, const std::vector<Value>& a) {
    auto count = [&](size_t k) {
        if (a.size() != k) throw std::runtime_error("TypeError: " + n + " expects " + std::to_string(k) + " arguments");
    }
    ;
    if (n == "timestamp") {
        count(0);
        return Value::num(static_cast<double>(std::time(nullptr)));
    }
    if (n == "wait") {
        count(1);
        double t = numeric(a[0]);
        if (t < 0 || t > 86400) throw std::runtime_error("ValueError: invalid wait duration");
        std::this_thread::sleep_for(std::chrono::duration<double>(t));
        return Value();
    }
    if (n == "len") {
        count(1);
        if (a[0].type == Value::Type::STRING) return Value::num(static_cast<double>(a[0].text.size()));
        if (a[0].type == Value::Type::ARRAY) return Value::num(static_cast<double>(a[0].array->size()));
        if (a[0].type == Value::Type::DICTIONARY) return Value::num(static_cast<double>(a[0].dictionary->size()));
        throw std::runtime_error("TypeError: len requires string or array");
    }
    if (n == "str") {
        count(1);
        return Value::str(a[0].display());
    }
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
    if (n == "bool") {
        count(1);
        return Value::truth(a[0].isTruthy());
    }
    if (n == "int" || n == "float") {
        count(1);
        double v = 0;
        if (a[0].type == Value::Type::NUMBER) v = a[0].number;
        else if (a[0].type == Value::Type::BOOL) v = a[0].boolean ? 1 : 0;
        else if (a[0].type == Value::Type::STRING) {
            size_t used = 0;
            try {
                v = std::stod(a[0].text, &used);
            }
            catch (...) {
                throw std::runtime_error("ValueError: invalid numeric string");
            }
            if (used != a[0].text.size()) throw std::runtime_error("ValueError: invalid numeric string");
        }
        else throw std::runtime_error("TypeError: cannot convert to number");
        return Value::num(n == "int" ? std::trunc(v) : v);
    }
    if (n == "fileexists" || n == "fileExists") {
        count(1);
        std::ifstream f(a[0].display(), std::ios::binary);
        return Value::truth(f.good());
    }
    if (n == "deleteFile") {
        count(1);
        if (a[0].type != Value::Type::STRING)
        throw std::runtime_error("TypeError: deleteFile expects a string path");
        if (std::remove(a[0].text.c_str()) != 0)
        throw std::runtime_error("FileError: cannot delete '" + a[0].text +
        "': " + std::strerror(errno));
        return Value();
    }
    if (n == "readfile" || n == "readFile") {
        count(1);
        std::ifstream f(a[0].display(), std::ios::binary);
        if (!f) throw std::runtime_error("FileError: cannot read '" + a[0].display() + "'");
        std::ostringstream o;
        o << f.rdbuf();
        return Value::str(o.str());
    }
    if (n == "writefile" || n == "writeFile" || n == "saveFile" ||
    n == "addfile" || n == "addFile") {
        count(2);
        std::ofstream f(a[0].display(), std::ios::binary | ((n == "addfile" || n == "addFile") ? std::ios::app : std::ios::trunc));
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
    }
    ;
    if (v.type == Value::Type::DICTIONARY) {
        if (n == "remove") {
            count(1);
            if (a[0].type != Value::Type::STRING) throw std::runtime_error("TypeError: dictionary key must be string");
            if (!v.dictionary->erase(a[0].text)) throw std::runtime_error("KeyError: key not found");
            return Value();
        }
        if (n == "keys" || n == "values") {
            count(0);
            std::vector<Value> result;
            for (const auto& kv : *v.dictionary) result.push_back(n == "keys" ? Value::str(kv.first) : kv.second);
            return Value::list(std::move(result));
        }
        if (n == "len") {
            count(0);
            return Value::num(static_cast<double>(v.dictionary->size()));
        }
    }
    if (v.type == Value::Type::ARRAY) {
        if (n=="sum" || n=="min" || n=="max" || n=="avg") {
            count(0);
            if(v.array->empty() && n!="sum") throw std::runtime_error("ValueError: empty array");
            double total=0, lo=0,hi=0;
            bool first=true;
            for(const Value& item:*v.array) {
                double x=numeric(item);
                total+=x;
                if(first || x<lo) lo=x;
                if(first || x>hi) hi=x;
                first=false;
            }
            return Value::num(n=="sum"?total:n=="avg"?total/v.array->size():n=="min"?lo:hi);
        }
        if (n == "sort") {
            count(0);
            auto& items = *v.array;
            for (size_t i=0;i<items.size();++i) {
                if (items[i].type != Value::Type::NUMBER && items[i].type != Value::Type::STRING)
                throw std::runtime_error("TypeError: sort requires numbers or strings");
                if (i && items[i].type != items[0].type)
                throw std::runtime_error("TypeError: cannot sort mixed types");
            }
            std::sort(items.begin(),items.end(),[](const Value& x,const Value& y) {
                return x.type==Value::Type::NUMBER ? x.number<y.number : x.text<y.text;
            }
            );
            return Value();
        }
        if (n == "rev") {
            count(0);
            std::reverse(v.array->begin(),v.array->end());
            return Value();
        }
        if (n == "index" || n == "count") {
            count(1);
            size_t hits=0;
            for (size_t i=0;i<v.array->size();++i) if (equal((*v.array)[i],a[0])) {
                if (n == "index") return Value::num(static_cast<double>(i));
                ++hits;
            }
            return Value::num(n == "index" ? -1 : static_cast<double>(hits));
        }
        if (n == "push" || n == "add" || n == "append") {
            count(1);
            v.array->push_back(a[0]);
            return Value();
        }
        if (n == "pop") {
            count(0);
            if (v.array->empty()) throw std::runtime_error("IndexError: pop from empty array");
            Value result = v.array->back();
            v.array->pop_back();
            return result;
        }
        if (n == "remove") {
            count(1);
            auto it = std::find_if(v.array->begin(), v.array->end(), [&](const Value& x) {
                return equal(x,a[0]);
            }
            );
            if (it == v.array->end()) throw std::runtime_error("ValueError: element not found");
            v.array->erase(it);
            return Value();
        }
    }
    if (v.type == Value::Type::STRING) {
        if (n == "trim") {
            count(0);
            size_t start=v.text.find_first_not_of(" \t\r\n");
            if (start==std::string::npos) return Value::str("");
            size_t end=v.text.find_last_not_of(" \t\r\n");
            return Value::str(v.text.substr(start,end-start+1));
        }
        if (n == "startswith" || n == "endswith" || n == "find" || n == "has") {
            count(1);
            std::string term=a[0].display();
            size_t pos=v.text.find(term);
            if (n == "find") return Value::num(pos==std::string::npos ? -1 : static_cast<double>(pos));
            if (n == "has") return Value::truth(pos!=std::string::npos);
            if (n == "startswith") return Value::truth(pos==0);
            return Value::truth(term.size()<=v.text.size() && v.text.compare(v.text.size()-term.size(),term.size(),term)==0);
        }
        if (n == "append" || n == "add") {
            count(1);
            v.text += a[0].display();
            assign(obj,v);
            return v;
        }
        if (n == "upper" || n == "lower") {
            count(0);
            std::string t = v.text;
            for (char& c : t) {
                unsigned char ch = static_cast<unsigned char>(c);
                c = static_cast<char>(n == "upper" ? std::toupper(ch) : std::tolower(ch));
            }
            return Value::str(t);
        }
        if (n == "replace") {
            count(2);
            std::string from = a[0].display(), to = a[1].display(), t = v.text;
            if (from.empty()) throw std::runtime_error("ValueError: empty search string");
            size_t pos = 0;
            while ((pos = t.find(from,pos)) != std::string::npos) {
                t.replace(pos,from.size(),to);
                pos += to.size();
            }
            return Value::str(t);
        }
        if (n == "split") {
            count(1);
            std::string delim = a[0].display();
            if (delim.empty()) throw std::runtime_error("ValueError: empty delimiter");
            std::vector<Value> out;
            size_t start = 0, pos;
            while ((pos = v.text.find(delim,start)) != std::string::npos) {
                out.push_back(Value::str(v.text.substr(start,pos-start)));
                start = pos + delim.size();
            }
            out.push_back(Value::str(v.text.substr(start)));
            return Value::list(std::move(out));
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
    if (auto p=dynamic_cast<const FStringExpr*>(e)) {
        std::string out;
        const std::string& s=p->text;
        for(size_t i=0;i<s.size();) {
            if(s[i]=='{' && i+1<s.size() && s[i+1]=='{') {
                out+='{';
                i+=2;
                continue;
            }
            if(s[i]=='}' && i+1<s.size() && s[i+1]=='}') {
                out+='}';
                i+=2;
                continue;
            }
            if(s[i]=='{') {
                size_t j=i+1;
                int depth=1;
                for(;j<s.size();++j) {
                    if(s[j]=='{')++depth;
                    else if(s[j]=='}' && --depth==0)break;
                }
                if(j==s.size()) throw std::runtime_error("SyntaxError: unmatched f-string brace");
                Lexer l(s.substr(i+1,j-i-1));
                Parser parser(l.scanTokens());
                // Evaluate a single expression through an internal expression statement.
                auto ast=parser.parse();
                if(ast.size()!=1) throw std::runtime_error("SyntaxError: invalid f-string expression");
                auto expr=dynamic_cast<ExprStmt*>(ast[0].get());
                if(!expr) throw std::runtime_error("SyntaxError: invalid f-string expression");
                out+=evaluate(expr->value.get()).display();
                i=j+1;
                continue;
            }
            if(s[i]=='}') throw std::runtime_error("SyntaxError: unmatched f-string brace");
            out+=s[i++];
        }
        return Value::str(out);
    }
    if(auto p=dynamic_cast<const SliceExpr*>(e)) {
        Value v=evaluate(p->base.get());
        if(v.type!=Value::Type::ARRAY && v.type!=Value::Type::STRING) throw std::runtime_error("TypeError: slicing requires array or string");
        long long n=v.type==Value::Type::ARRAY?static_cast<long long>(v.array->size()):static_cast<long long>(v.text.size());
        auto index=[&](const ExprPtr& x,long long fallback)->long long {
            if(!x)return fallback;
            double d=numeric(evaluate(x.get()));
            if(!std::isfinite(d)||std::floor(d)!=d)throw std::runtime_error("TypeError: slice indices must be integers");
            return static_cast<long long>(d);
        }
        ;
        long long step=index(p->step,1);
        if(step==0)throw std::runtime_error("ValueError: slice step cannot be zero");
        auto norm=[&](long long i)->long long {
            if(i<0)i+=n;
            return step>0?std::max(0LL,std::min(i,n)):std::max(-1LL,std::min(i,n-1));
        }
        ;
        long long a=p->start?norm(index(p->start,0)):(step>0?0:n-1);
        long long b=p->stop?norm(index(p->stop,0)):(step>0?n:-1);
        std::vector<Value> items;
        std::string result;
        for(long long i=a;step>0?i<b:i>b;i+=step) {
            if(v.type==Value::Type::ARRAY)items.push_back((*v.array)[static_cast<size_t>(i)]);
            else result+=v.text[static_cast<size_t>(i)];
        }
        return v.type==Value::Type::ARRAY?Value::list(std::move(items)):Value::str(result);
    }
    if(auto p=dynamic_cast<const PropertyExpr*>(e)) {
        Value v=evaluate(p->base.get());
        if(v.type!=Value::Type::OBJECT) throw std::runtime_error("TypeError: property access requires object");
        auto it=v.object->find(p->name.lexeme);
        if(it==v.object->end()) throw std::runtime_error("AttributeError: unknown property '" + p->name.lexeme + "'");
        return it->second;
    }
    if (auto p = dynamic_cast<const IndexExpr*>(e)) {
        Value a = evaluate(p->base.get());
        Value i = evaluate(p->index.get());
        if(a.type==Value::Type::DICTIONARY) {
            if(i.type!=Value::Type::STRING)throw std::runtime_error("TypeError: dictionary key must be string");
            auto it=a.dictionary->find(i.text);
            if(it==a.dictionary->end())throw std::runtime_error("KeyError: key not found");
            return it->second;
        }
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
            }
            catch (const std::invalid_argument&) {
            }
            catch (const std::out_of_range&) {
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
            auto ci=classes.find(p->name.lexeme);
            if(ci!=classes.end()) {
                Value obj=Value::instance(p->name.lexeme);
                bool init=false;
                for(const auto& m:ci->second->methods) if(m->name.lexeme=="-init") {
                    invoke(m.get(),args,&obj);
                    init=true;
                    break;
                }
                if(!init && !args.empty()) throw std::runtime_error("TypeError: constructor expects no arguments");
                return obj;
            }
            return call(p->name.lexeme, args);
        }
        try {
            Value obj=lookup(p->module);
            if(obj.type==Value::Type::OBJECT) {
                auto ci=classes.find(obj.className);
                if(ci==classes.end()) throw std::runtime_error("NameError: class not found");
                for(const auto& m:ci->second->methods) if(m->name.lexeme==p->name.lexeme) return invoke(m.get(),args,&obj);
                throw std::runtime_error("AttributeError: unknown method");
            }
        }
        catch(const std::runtime_error& er) {
            if(std::string(er.what()).find("NameError: undefined variable")!=0) throw;
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
    if(auto p=dynamic_cast<const ClassStmt*>(s)) {
        classes[p->name.lexeme]=p;
        return;
    }
    if(auto p=dynamic_cast<const SpecialStmt*>(s)) {
        events[p->name]=p;
        return;
    }
    if(auto p=dynamic_cast<const PropertyAssignStmt*>(s)) {
        Value obj=evaluate(p->base.get());
        if(obj.type!=Value::Type::OBJECT)throw std::runtime_error("TypeError: property assignment requires object");
        Value v=evaluate(p->value.get());
        if(p->op.type!=TokenType::ASSIGN) {
            auto it=obj.object->find(p->name.lexeme);
            if(it==obj.object->end())throw std::runtime_error("AttributeError: unknown property");
            TokenType op=p->op.type==TokenType::PLUS_EQ?TokenType::PLUS:p->op.type==TokenType::MINUS_EQ?TokenType::MINUS:p->op.type==TokenType::STAR_EQ?TokenType::STAR:TokenType::SLASH;
            v=binary(op,it->second,v);
        }
        (*obj.object)[p->name.lexeme]=v;
        return;
    }
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
        try {
            execute(p->body.get());
        }
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
                switch(p->op.type) {
                    case TokenType::MINUS_EQ:op=TokenType::MINUS;
                    break;
                    case TokenType::STAR_EQ:op=TokenType::STAR;
                    break;
                    case TokenType::SLASH_EQ:op=TokenType::SLASH;
                    break;
                    case TokenType::PERCENT_EQ:op=TokenType::PERCENT;
                    break;
                    case TokenType::CARET_EQ:op=TokenType::CARET;
                    break;
                    default:break;
                }
                v=binary(op,it->second,v);
            }
            (*base.dictionary)[key.text]=v;
            return;
        }
        if (base.type != Value::Type::ARRAY) throw std::runtime_error("TypeError: indexed assignment requires array");
        double i = numeric(evaluate(idx->index.get()));
        if (i < 0 || std::floor(i) != i || i >= base.array->size())
        throw std::runtime_error("IndexError: array index out of range");
        Value v = evaluate(p->value.get());
        if (p->op.type != TokenType::ASSIGN) {
            TokenType op = TokenType::PLUS;
            switch (p->op.type) {
                case TokenType::PLUS_EQ: op=TokenType::PLUS;
                break;
                case TokenType::MINUS_EQ: op=TokenType::MINUS;
                break;
                case TokenType::STAR_EQ: op=TokenType::STAR;
                break;
                case TokenType::SLASH_EQ: op=TokenType::SLASH;
                break;
                case TokenType::PERCENT_EQ: op=TokenType::PERCENT;
                break;
                case TokenType::CARET_EQ: op=TokenType::CARET;
                break;
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
        }
        else if (p->no) {
            execute(p->no.get());
        }
        return;
    }
    if (auto p = dynamic_cast<const WhileStmt*>(s)) {
        while (evaluate(p->condition.get()).isTruthy()) {
            try {
                execute(p->body.get());
            }
            catch (const ContinueSignal&) {
                continue;
            }
            catch (const BreakSignal&) {
                break;
            }
        }
        return;
    }
    if (auto p = dynamic_cast<const ForStmt*>(s)) {
        auto run = [&](Value v) -> bool {
            assign(p->name.lexeme, std::move(v));
            try {
                execute(p->body.get());
            }
            catch (const ContinueSignal&) {
                return true;
            }
            catch (const BreakSignal&) {
                return false;
            }
            return true;
        }
        ;
        if (p->iterable) {
            Value values = evaluate(p->iterable.get());
            if (values.type == Value::Type::DICTIONARY) {
                for (const auto& kv : *values.dictionary) if (!run(Value::str(kv.first))) break;
            }
            else if (values.type == Value::Type::ARRAY) {
                auto snapshot = *values.array;
                for (const auto& v : snapshot) if (!run(v)) break;
            }
            else if (values.type == Value::Type::STRING) {
                for (char c : values.text) if (!run(Value::str(std::string(1,c)))) break;
            }
            else throw std::runtime_error("TypeError: for in requires array or string");
        }
        else {
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
        functions[p->name.lexeme] = Function {
            p
        }
        ;
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
void Interpreter::runEvent(const SpecialStmt* s) {
    if(s) execute(s->body.get());
}
void Interpreter::interpret(
const std::vector<StmtPtr>& statements
) {
    for (const auto& s : statements) {
        execute(s.get());
    }
    if(events.count("start")) runEvent(events.at("start"));
    const SpecialStmt* tick=events.count("always")?events.at("always"):nullptr;
    const SpecialStmt* key=events.count("keypress")?events.at("keypress"):nullptr;
    double fps=60;
    if(tick && !tick->args.empty()) fps=std::max(1.0,std::min(60.0,numeric(evaluate(tick->args[0].get()))));
    if(tick || key) {
        fluxRunning=1;
        auto previousHandler=std::signal(SIGINT,fluxStop);
        using clock=std::chrono::steady_clock;
        auto next=clock::now();
        while(fluxRunning) {
#ifdef _WIN32
            if(key && _kbhit()) {
                int ch=_getch();
                std::string pressed(1,static_cast<char>(ch));
                if(ch==3) {
                    fluxRunning=0;
                    break;
                }
                if(ch==32)pressed="space";
                if(!key->args.empty() && evaluate(key->args[0].get()).display()==pressed) runEvent(key);
            }
#endif
            if(tick && clock::now()>=next) {
                runEvent(tick);
                next+=std::chrono::duration_cast<clock::duration>(std::chrono::duration<double>(1.0/fps));
                if(next<clock::now())next=clock::now();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        std::signal(SIGINT,previousHandler);
    }
    if(events.count("exit")) runEvent(events.at("exit"));
}
