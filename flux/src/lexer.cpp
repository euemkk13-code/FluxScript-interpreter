#include "lexer.h"
#include <stdexcept>
#include <unordered_map>
namespace {
    bool alpha(char c) {
        return (c >= 'a' && c <= 'z') ||
               (c >= 'A' && c <= 'Z') ||
               c == '_';
    }
    bool digit(char c) {
        return c >= '0' && c <= '9';
    }
}
void Lexer::scan() {
    char c = advance();
    switch (c) {
        case ' ':
        case '\t':
        case '\r':
            return;
        case '\n':
            add(FluxTokenType::NEWLINE, "\\n");
            ++line;
            return;
        case '(':
            add(FluxTokenType::LPAREN, "(");
            return;
        case ')':
            add(FluxTokenType::RPAREN, ")");
            return;
        case '{':
            add(FluxTokenType::LBRACE, "{");
            return;
        case '}':
            add(FluxTokenType::RBRACE, "}");
            return;
        case '[':
            add(FluxTokenType::LBRACKET, "[");
            return;
        case ']':
            add(FluxTokenType::RBRACKET, "]");
            return;
        case '.':
            add(FluxTokenType::DOT, ".");
            return;
        case ':':
            add(FluxTokenType::COLON, ":");
            return;
        case ',':
            add(FluxTokenType::COMMA, ",");
            return;
        case '+':
            if (peek() == '=') {
                advance();
                add(FluxTokenType::PLUS_EQ, "+=");
            } else {
                add(FluxTokenType::PLUS, "+");
            }
            return;
        case '*':
            if (peek() == '=') {
                advance();
                add(FluxTokenType::STAR_EQ, "*=");
            } else {
                add(FluxTokenType::STAR, "*");
            }
            return;
        case '%':
            if (peek() == '=') {
                advance();
                add(FluxTokenType::PERCENT_EQ, "%=");
            } else {
                add(FluxTokenType::PERCENT, "%");
            }
            return;
        case '^':
            if (peek() == '=') {
                advance();
                add(FluxTokenType::CARET_EQ, "^=");
            } else {
                add(FluxTokenType::CARET, "^");
            }
            return;
        case '-':
            if (peek() == '-' &&
                peek(1) == '[' &&
                peek(2) == '[') {
                current += 3;
                bool closed = false;
                while (!end()) {
                    if (peek() == ']' && peek(1) == ']') {
                        current += 2;
                        closed = true;
                        break;
                    }
                    if (advance() == '\n') {
                        ++line;
                    }
                }
                if (!closed) {
                    throw std::runtime_error(
                        "SyntaxError: unterminated block comment at line " +
                        std::to_string(line)
                    );
                }
            } else if (peek() == '=') {
                advance();
                add(FluxTokenType::MINUS_EQ, "-=");
            } else {
                add(FluxTokenType::MINUS, "-");
            }
            return;
        case '/':
            if (peek() == '=') {
                advance();
                add(FluxTokenType::SLASH_EQ, "/=");
                return;
            }
            if (peek() == '/') {
                // Ambiguity: '//' at start of a line is a comment;
                // between values it is floor division.
                // To avoid ambiguity, use // comments on their own line;
                // inline // is division when followed by an operand.
                bool lhs = !tokens.empty() &&
                    (tokens.back().type == FluxTokenType::NUMBER ||
                     tokens.back().type == FluxTokenType::IDENTIFIER ||
                     tokens.back().type == FluxTokenType::RPAREN ||
                     tokens.back().type == FluxTokenType::RBRACKET);
                size_t k = current + 1;
                while (k < source.size() &&
                       (source[k] == ' ' || source[k] == '\t')) {
                    ++k;
                }
                bool rhs = k < source.size() &&
                    (digit(source[k]) ||
                     alpha(source[k]) ||
                     source[k] == '(' ||
                     source[k] == '-' ||
                     source[k] == '+');
                if (lhs && rhs) {
                    advance();
                    add(FluxTokenType::FLOOR, "//");
                } else {
                    while (!end() && peek() != '\n') {
                        advance();
                    }
                }
                return;
            }
            add(FluxTokenType::SLASH, "/");
            return;
        case '=':
            if (peek() == '=') {
                advance();
                add(FluxTokenType::EQEQ, "==");
            } else {
                add(FluxTokenType::ASSIGN, "=");
            }
            return;
        case '!':
            if (peek() == '=') {
                advance();
                add(FluxTokenType::NOT_EQ, "!=");
                return;
            }
            break;
        case '<':
            if (peek() == '=') {
                advance();
                add(FluxTokenType::LTE, "<=");
            } else {
                add(FluxTokenType::LT, "<");
            }
            return;
        case '>':
            if (peek() == '=') {
                advance();
                add(FluxTokenType::GTE, ">=");
            } else {
                add(FluxTokenType::GT, ">");
            }
            return;
        case '\'':
        case '"': {
            std::string t;
            int start = line;
            bool closed = false;
            while (!end()) {
                char x = advance();
                if (x == c) {
                    closed = true;
                    break;
                }
                if (x == '\\') {
                    if (end()) {
                        break;
                    }
                    char e = advance();
                    switch (e) {
                        case 'n':
                            t += '\n';
                            break;
                        case 't':
                            t += '\t';
                            break;
                        case 'r':
                            t += '\r';
                            break;
                        case '\\':
                            t += '\\';
                            break;
                        case '\'':
                            t += '\'';
                            break;
                        case '"':
                            t += '"';
                            break;
                        default:
                            throw std::runtime_error(
                                "SyntaxError: invalid escape at line " +
                                std::to_string(line)
                            );
                    }
                } else {
                    if (x == '\n') {
                        ++line;
                    }
                    t += x;
                }
            }
            if (!closed) {
                throw std::runtime_error(
                    "SyntaxError: unterminated string at line " +
                    std::to_string(start)
                );
            }
            add(FluxTokenType::STRING, t);
            return;
        }
        default:
            break;
    }
    if (digit(c)) {
        size_t st = current - 1;
        while (digit(peek())) {
            advance();
        }
        if (peek() == 'x' && digit(peek(1))) {
            advance();
            while (digit(peek())) advance();
            add(FluxTokenType::DIMENSION, source.substr(st, current - st));
            return;
        }
        if (peek() == '.' && digit(peek(1))) {
            advance();
            while (digit(peek())) {
                advance();
            }
        }
        if(peek()=='i') { advance();add(FluxTokenType::IMAGINARY,source.substr(st,current-st-1));return; }
        add(
            FluxTokenType::NUMBER,
            source.substr(st, current - st)
        );
        return;
    }
    if (alpha(c)) {
        size_t st = current - 1;
        while (alpha(peek()) || digit(peek())) {
            advance();
        }
        std::string t = source.substr(st, current - st);
        FluxTokenType k = FluxTokenType::IDENTIFIER;
        if (t == "assert") { k = FluxTokenType::ASSERT; } else if (t == "class") { k = FluxTokenType::CLASS; } else if (t == "out") {
            k = FluxTokenType::OUT;
        } else if (t == "if") {
            k = FluxTokenType::IF;
        } else if (t == "else") {
            k = FluxTokenType::ELSE;
        } else if (t == "elif") {
            k = FluxTokenType::ELIF;
        } else if (t == "break") {
            k = FluxTokenType::BREAK;
        } else if (t == "continue") {
            k = FluxTokenType::CONTINUE;
        } else if (t == "not") {
            k = FluxTokenType::NOT;
        } else if (t == "in") {
            k = FluxTokenType::IN;
        } else if (t == "try") {
            k = FluxTokenType::TRY;
        } else if (t == "catch") {
            k = FluxTokenType::CATCH;
        } else if (t == "pass") {
            k = FluxTokenType::PASS;
        } else if (t == "while") {
            k = FluxTokenType::WHILE;
        } else if (t == "for") {
            k = FluxTokenType::FOR;
        } else if (t == "get") {
            k = FluxTokenType::GET;
        } else if (t == "func") {
            k = FluxTokenType::FUNC;
        } else if (t == "return") {
            k = FluxTokenType::RETURN;
        } else if (t == "input") {
            k = FluxTokenType::INPUT;
        } else if (t == "and") {
            k = FluxTokenType::AND;
        } else if (t == "or") {
            k = FluxTokenType::OR;
        } else if (t == "true" || t == "True") {
            k = FluxTokenType::TRUE;
        } else if (t == "nil") {
            k = FluxTokenType::NIL;
        } else if (t == "false" || t == "False") {
            k = FluxTokenType::FALSE;
        }
        if (t == "r" && (peek() == '"' || peek() == '\'')) {
            char quote=advance(); std::string body; bool closed=false;
            while(!end()) { char ch=advance(); if(ch==quote){closed=true;break;} if(ch=='\n')++line; body+=ch; }
            if(!closed)throw std::runtime_error("SyntaxError: unterminated raw string");
            add(FluxTokenType::STRING,body);return;
        }
        if (t == "f" && (peek() == '"' || peek() == '\'')) {
            char quote = advance(); std::string body; bool closed = false;
            while (!end()) { char ch = advance(); if (ch == quote) { closed = true; break; }
                if (ch == '\\' && !end()) { char esc = advance(); if (esc == 'n') body += '\n'; else if (esc == 't') body += '\t'; else { body += '\\'; body += esc; } }
                else { if (ch == '\n') ++line; body += ch; }
            }
            if (!closed) throw std::runtime_error("SyntaxError: unterminated f-string");
            add(FluxTokenType::FSTRING, body); return;
        }
        add(k, t);
        return;
    }
    throw std::runtime_error(
        "SyntaxError: unexpected character '" +
        std::string(1, c) +
        "' at line " +
        std::to_string(line)
    );
}
std::vector<Token> Lexer::scanTokens() {
    tokens.clear();
    current = 0;
    line = 1;
    while (!end()) {
        scan();
    }
    add(FluxTokenType::END_OF_FILE, "");
    // Aliases are file-local and take effect only after declaration.
    // The declaration is removed before parsing, so it also supports
    // alias alias = nickname, followed by nickname out = potato.
    std::unordered_map<std::string, Token> aliases;
    std::vector<Token> result;
    for (size_t i = 0; i < tokens.size();) {
        Token t = tokens[i];
        auto found = aliases.find(t.lexeme);
        bool aliasKeyword = (t.type == FluxTokenType::IDENTIFIER && t.lexeme == "alias") ||
            (found != aliases.end() && found->second.lexeme == "alias");
        if (aliasKeyword && i + 3 < tokens.size() &&
            (tokens[i+1].type == FluxTokenType::IDENTIFIER ||
             (tokens[i+1].type != FluxTokenType::NEWLINE && tokens[i+1].type != FluxTokenType::END_OF_FILE)) &&
            tokens[i+2].type == FluxTokenType::ASSIGN &&
            tokens[i+3].type == FluxTokenType::IDENTIFIER) {
            Token target = tokens[i+1];
            auto prior = aliases.find(target.lexeme);
            if (prior != aliases.end()) target = prior->second;
            const std::string nickname = tokens[i+3].lexeme;
            if (nickname == "alias" || aliases.count(nickname))
                throw std::runtime_error("SyntaxError: duplicate alias '" + nickname + "'");
            aliases.emplace(nickname, target);
            i += 4;
            continue;
        }
        if (found != aliases.end()) {
            t.type = found->second.type;
            t.lexeme = found->second.lexeme;
        }
        result.push_back(std::move(t));
        ++i;
    }
    return result;
}