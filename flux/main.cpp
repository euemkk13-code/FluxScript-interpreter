#include "src/lexer.h"
#include "src/parser.h"
#include "src/interpreter.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <exception>
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: flux.exe file.flx\n";
        return 1;
    }
    std::ifstream file(argv[1], std::ios::binary);
    if (!file) {
        std::cerr << "FileError: cannot open " << argv[1] << "\n";
        return 1;
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    if (!file.good() && !file.eof()) {
        std::cerr << "FileError: failed to read file\n";
        return 1;
    }
    try {
        Lexer lexer(buffer.str());
        Parser parser(lexer.scanTokens());
        auto program = parser.parse();

        Interpreter interpreter;
        interpreter.interpret(program);
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}