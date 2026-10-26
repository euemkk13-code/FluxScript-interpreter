#include "lexer.h"
#include <stdexcept>
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
            add(TokenType::NEWLINE, "\\n");
            ++line;
            return;
        case '(':
            add(TokenType::LPAREN, "(");
            return;
        case ')':
            add(TokenType::RPAREN, ")");
            return;
        case '{':
            add(TokenType::LBRACE, "{");
            return;
        case '}':
            add(TokenType::RBRACE, "}");
            return;
        case '[':
            add(TokenType::LBRACKET, "[");
            return;
        case ']':
            add(TokenType::RBRACKET, "]");
            return;
        case '.':
            add(TokenType::DOT, ".");
            return;
        case ',':
            add(TokenType::COMMA, ",");
            return;
        case '+':
            if (peek() == '=') {
                advance();
                add(TokenType::PLUS_EQ, "+=");
            } else {
                add(TokenType::PLUS, "+");
            }
            return;
        case '*':
            if (peek() == '=') {
                advance();
                add(TokenType::STAR_EQ, "*=");
            } else {
                add(TokenType::STAR, "*");
            }
            return;
        case '%':
            if (peek() == '=') {
                advance();
                add(TokenType::PERCENT_EQ, "%=");
            } else {
                add(TokenType::PERCENT, "%");
            }
            return;
        case '^':
            if (peek() == '=') {
                advance();
                add(TokenType::CARET_EQ, "^=");
            } else {
                add(TokenType::CARET, "^");
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
                add(TokenType::MINUS_EQ, "-=");
            } else {
                add(TokenType::MINUS, "-");
            }
            return;
        case '/':
            if (peek() == '=') {
                advance();
                add(TokenType::SLASH_EQ, "/=");
                return;
            }
            if (peek() == '/') {
                // Ambiguity: '//' at start of a line is a comment;
                // between values it is floor division.
                // To avoid ambiguity, use // comments on their own line;
                // inline // is division when followed by an operand.
                bool lhs = !tokens.empty() &&
                    (tokens.back().type == TokenType::NUMBER ||
                     tokens.back().type == TokenType::IDENTIFIER ||
                     tokens.back().type == TokenType::RPAREN ||
                     tokens.back().type == TokenType::RBRACKET);
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
                    add(TokenType::FLOOR, "//");
                } else {
                    while (!end() && peek() != '\n') {
                        advance();
                    }
                }
                return;
            }
            add(TokenType::SLASH, "/");
            return;
        case '=':
            if (peek() == '=') {
                advance();
                add(TokenType::EQEQ, "==");
            } else {
                add(TokenType::ASSIGN, "=");
            }
            return;
        case '!':
            if (peek() == '=') {
                advance();
                add(TokenType::NOT_EQ, "!=");
                return;
            }
            break;
        case '<':
            if (peek() == '=') {
                advance();
                add(TokenType::LTE, "<=");
            } else {
                add(TokenType::LT, "<");
            }
            return;
        case '>':
            if (peek() == '=') {
                advance();
                add(TokenType::GTE, ">=");
            } else {
                add(TokenType::GT, ">");
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
            add(TokenType::STRING, t);
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
        if (peek() == '.' && digit(peek(1))) {
            advance();
            while (digit(peek())) {
                advance();
            }
        }
        add(
            TokenType::NUMBER,
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
        TokenType k = TokenType::IDENTIFIER;
        if (t == "out") {
            k = TokenType::OUT;
        } else if (t == "if") {
            k = TokenType::IF;
        } else if (t == "else") {
            k = TokenType::ELSE;
        } else if (t == "elif") {
            k = TokenType::ELIF;
        } else if (t == "break") {
            k = TokenType::BREAK;
        } else if (t == "continue") {
            k = TokenType::CONTINUE;
        } else if (t == "not") {
            k = TokenType::NOT;
        } else if (t == "in") {
            k = TokenType::IN;
        } else if (t == "try") {
            k = TokenType::TRY;
        } else if (t == "catch") {
            k = TokenType::CATCH;
        } else if (t == "pass") {
            k = TokenType::PASS;
        } else if (t == "while") {
            k = TokenType::WHILE;
        } else if (t == "for") {
            k = TokenType::FOR;
        } else if (t == "get") {
            k = TokenType::GET;
        } else if (t == "func") {
            k = TokenType::FUNC;
        } else if (t == "return") {
            k = TokenType::RETURN;
        } else if (t == "input") {
            k = TokenType::INPUT;
        } else if (t == "and") {
            k = TokenType::AND;
        } else if (t == "or") {
            k = TokenType::OR;
        } else if (t == "true") {
            k = TokenType::TRUE;
        } else if (t == "false") {
            k = TokenType::FALSE;
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
    add(TokenType::END_OF_FILE, "");
    return tokens;
}