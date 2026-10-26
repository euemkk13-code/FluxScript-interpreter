#pragma once

#include <string>
#include <utility>
#ifdef TokenType
#error TokenType is already defined as a macro!
#endif
enum class TokenType {
    END_OF_FILE,
    NEWLINE,
    IDENTIFIER,
    NUMBER,
    STRING,
    OUT,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    FLOOR,
    PERCENT,
    CARET,
    ASSIGN,
    PLUS_EQ,
    MINUS_EQ,
    STAR_EQ,
    SLASH_EQ,
    PERCENT_EQ,
    CARET_EQ,
    EQEQ,
    NOT_EQ,
    LPAREN,
    RPAREN,
    LBRACKET,
    RBRACKET,
    LBRACE,
    RBRACE,
    DOT,
    COMMA,
    COLON,
    GET,
    FOR,
    IF,
    ELSE,
    ELIF,
    WHILE,
    AND,
    OR,
    NOT,
    IN,
    TRUE,
    FALSE,
    NIL,
    LT,
    GT,
    LTE,
    GTE,
    FUNC,
    RETURN,
    INPUT,
    BREAK,
    CONTINUE,
    TRY,
    CATCH,
    PASS,
    CLASS,
    FSTRING
};
struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    Token(TokenType t, std::string s, int n)
        : type(t),
          lexeme(std::move(s)),
          line(n) {}
};