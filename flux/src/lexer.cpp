#include "lexer.h"
bool Lexer::isAtEnd() { return current >= source.length(); }
char Lexer::advance() { return source[current++]; }
char Lexer::peek() { if (isAtEnd()) return '\0'; return source[current]; }
char Lexer::peekNext() { if (current + 1 >= source.length()) return '\0'; return source[current + 1]; }
bool Lexer::match(char expected) { if (isAtEnd()) return false; if (source[current] != expected) return false; current++; return true; }
void Lexer::addToken(TokenType type, std::string lexeme) { tokens.emplace_back(type, lexeme, line); }
bool Lexer::isDigit(char c) { return c >= '0' && c <= '9'; }
bool Lexer::isAlpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
bool Lexer::isAlphaNumeric(char c) { return isAlpha(c) || isDigit(c); }
void Lexer::string(char quote) {
    while (peek() != quote && !isAtEnd()) { if (peek() == '\n') line++; advance(); }
    if (isAtEnd()) return;
    advance();
    std::string value = source.substr(start + 1, current - start - 2);
    addToken(TokenType::STRING, value);
}
void Lexer::number() {
    while (isDigit(peek())) advance();
    if (peek() == '.' && isDigit(peekNext())) { advance(); while (isDigit(peek())) advance(); }
    std::string value = source.substr(start, current - start);
    addToken(TokenType::NUMBER, value);
}
void Lexer::identifier() {
    while (isAlphaNumeric(peek())) advance();
    std::string text = source.substr(start, current - start);
    if (text == "out") addToken(TokenType::OUT, text);
    else if (text == "while") addToken(TokenType::WHILE, text);
    else if (text == "if") addToken(TokenType::IF, text);
    else if (text == "else") addToken(TokenType::ELSE, text);
    else if (text == "true") addToken(TokenType::TRUE, text);
    else if (text == "false") addToken(TokenType::FALSE, text);
    else if (text == "func") addToken(TokenType::FUNC, text);
    else if (text == "return") addToken(TokenType::RETURN, text);
    else if (text == "input") addToken(TokenType::INPUT, text);
    else addToken(TokenType::IDENTIFIER, text);
}
void Lexer::lineComment() { while (peek() != '\n' && !isAtEnd()) advance(); addToken(TokenType::COMMENT, "//"); }
void Lexer::blockComment() {
    while (!isAtEnd()) {
        if (peek() == ']' && peekNext() == ']') { advance(); advance(); break; }
        if (peek() == '\n') line++;
        advance();
    }
    addToken(TokenType::COMMENT, "--[[ ]]");
}
void Lexer::scanToken() {
    char c = advance();
    switch (c) {
        case ' ': case '\r': case '\t': break;
        case '\n': line++; break;
        case '(': addToken(TokenType::LPAREN, "("); break;
        case ')': addToken(TokenType::RPAREN, ")"); break;
        case '{': addToken(TokenType::LBRACE, "{"); break;
        case '}': addToken(TokenType::RBRACE, "}"); break;
        case ',': addToken(TokenType::COMMA, ","); break;
        case '+': addToken(TokenType::PLUS, "+"); break;
        case '-':
            if (peek() == '-' && peekNext() == '[') { advance(); advance(); advance(); blockComment(); }
            else addToken(TokenType::MINUS, "-");
            break;
        case '*': addToken(TokenType::STAR, "*"); break;
        case '/': if (match('/')) lineComment(); else addToken(TokenType::SLASH, "/"); break;
        case '=': if (match('=')) addToken(TokenType::EQEQ, "=="); else addToken(TokenType::ASSIGN, "="); break;
        case '<': if (match('=')) addToken(TokenType::LTE, "<="); else addToken(TokenType::LT, "<"); break;
        case '>': if (match('=')) addToken(TokenType::GTE, ">="); else addToken(TokenType::GT, ">"); break;
        case '!': if (match('=')) addToken(TokenType::NOT_EQ, "!="); break;
        case '"': case '\'': string(c); break;
        default: if (isDigit(c)) number(); else if (isAlpha(c)) identifier(); break;
    }
}
std::vector<Token> Lexer::scanTokens() {
    while (!isAtEnd()) { start = current; scanToken(); }
    tokens.emplace_back(TokenType::END_OF_FILE, "", line);
    return tokens;
}