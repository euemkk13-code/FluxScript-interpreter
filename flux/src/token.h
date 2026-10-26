#pragma once
#include <string>
#include <utility>
enum class TokenType {
    END_OF_FILE, NEWLINE, IDENTIFIER, NUMBER, STRING, OUT, PLUS, MINUS, STAR, SLASH, FLOOR, PERCENT, CARET, ASSIGN, PLUS_EQ, MINUS_EQ, STAR_EQ, SLASH_EQ, PERCENT_EQ, CARET_EQ, EQEQ, LPAREN, RPAREN, LBRACKET, RBRACKET, DOT, GET, FOR, IF, ELSE, WHILE, AND, OR, TRUE, FALSE, LT, GT, LTE, GTE, NOT_EQ, LBRACE, RBRACE, COMMA, FUNC, RETURN, INPUT, ELIF, BREAK, CONTINUE, NOT, IN, TRY, CATCH, PASS };
struct Token {
    TokenType type;
std::string lexeme;
int line;
Token(TokenType t,std::string s,int n):type(t),lexeme(std::move(s)),line(n){} };
