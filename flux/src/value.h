#pragma once

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <utility>
#include <map>

struct Value {
    enum class Type {
        NUMBER,
        STRING,
        BOOL,
        NIL,
        ARRAY,
        DICTIONARY
    } type;

    double number = 0;
    std::string text;
    bool boolean = false;

    std::shared_ptr<std::vector<Value>> array;
    std::shared_ptr<std::map<std::string, Value>> dictionary;

    Value() : type(Type::NIL) {}

    static Value num(double x) {
        if (!std::isfinite(x)) {
            throw std::runtime_error(
                "ArithmeticError: invalid number"
            );
        }

        Value v;
        v.type = Type::NUMBER;
        v.number = x;
        return v;
    }

    static Value str(std::string x) {
        Value v;
        v.type = Type::STRING;
        v.text = std::move(x);
        return v;
    }

    static Value truth(bool x) {
        Value v;
        v.type = Type::BOOL;
        v.boolean = x;
        return v;
    }

    static Value list(std::vector<Value> x) {
        Value v;
        v.type = Type::ARRAY;
        v.array = std::make_shared<std::vector<Value>>(
            std::move(x)
        );
        return v;
    }

    static Value dict(std::map<std::string, Value> x) {
        Value v;
        v.type = Type::DICTIONARY;
        v.dictionary =
            std::make_shared<std::map<std::string, Value>>(
                std::move(x)
            );
        return v;
    }

    bool isTruthy() const {
        switch (type) {
            case Type::BOOL:
                return boolean;

            case Type::NUMBER:
                return number != 0;

            case Type::STRING:
                return !text.empty();

            case Type::ARRAY:
                return !array->empty();

            case Type::DICTIONARY:
                return !dictionary->empty();

            default:
                return false;
        }
    }

    std::string display() const {
        switch (type) {
            case Type::STRING:
                return text;

            case Type::BOOL:
                return boolean ? "true" : "false";

            case Type::NIL:
                return "nil";

            case Type::DICTIONARY: {
                std::string t = "{";

                for (const auto& item : *dictionary) {
                    if (t.size() > 1) {
                        t += ", ";
                    }

                    t += "\"" + item.first + "\": ";

                    t += item.second.type == Type::STRING
                        ? "\"" + item.second.text + "\""
                        : item.second.display();
                }

                return t + "}";
            }

            case Type::ARRAY: {
                std::string t = "[";

                for (size_t i = 0; i < array->size(); ++i) {
                    if (i) {
                        t += ", ";
                    }

                    const Value& v = (*array)[i];

                    t += v.type == Type::STRING
                        ? "\"" + v.text + "\""
                        : v.display();
                }

                return t + "]";
            }

            default: {
                if (number == 0) {
                    return "0";
                }

                std::ostringstream o;
                o << std::setprecision(15) << number;
                return o.str();
            }
        }
    }
};
