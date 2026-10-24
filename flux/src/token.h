#pragma once
#include <string>
enum class TokenType {
    END_OF_FILE,
    IDENTIFIER,
    NUMBER,
    STRING,
    OUT,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    ASSIGN,
    EQEQ,
    LPAREN,
    RPAREN,
    COMMENT,
    IF,
    ELSE,
    WHILE,
    AND,
    OR,
    TRUE,
    FALSE,
    LT,
    GT,
    LTE,
    GTE,
    NOT_EQ,
    LBRACE,
    RBRACE
};
struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    Token(TokenType t, std::string l, int ln)
        : type(t), lexeme(l), line(ln) {}
};