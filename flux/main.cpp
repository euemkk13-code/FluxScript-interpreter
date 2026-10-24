#include "src/lexer.h"
#include "src/parser.h"
#include "src/interpreter.h"
#include <fstream>
#include <sstream>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Uso: flux.exe arquivo.flux\n";
        return 0;
    }
    std::ifstream file(argv[1]);
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    Lexer lexer(source);
    auto tokens = lexer.scanTokens();

    Parser parser(tokens);
    auto stmts = parser.parse();

    Interpreter interpreter;
    interpreter.interpret(stmts);

    return 0;
}