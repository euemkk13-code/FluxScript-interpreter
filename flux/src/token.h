#pragma once
#include <string>

enum class TokenType {
    // fim
    END_OF_FILE,

    // literais
    IDENTIFIER,
    NUMBER,
    STRING,

    // palavra chave 0.1.0
    OUT,

    // operadores
    EQUALS,
    PLUS,
    MINUS,
    STAR,
    SLASH,

    // simbolos
    LPAREN,      // (
    RPAREN,      // )

    // ignorados (mas mapeados)
    COMMENT
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    Token(TokenType t, std::string l, int ln) 
        : type(t), lexeme(l), line(ln) {}
};