
#include "src/lexer.h"
#include "src/parser.h"
#include "src/interpreter.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <exception>
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Uso: flux.exe arquivo.flx\n";
        return 0;
    }
    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "FileError: cannot open " << argv[1] << "\n";
        return 1;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    try {
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(tokens);
        auto stmts = parser.parse();
        Interpreter interpreter;
        interpreter.interpret(stmts);
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    } catch (const ReturnSignal&) {
        std::cerr << "SyntaxError: return outside function\n";
        return 1;
    }
    return 0;
}
