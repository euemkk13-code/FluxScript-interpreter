#include "parser.h"
#include <stdexcept>
#include <unordered_set>

Token Parser::consume(TokenType t, const std::string& m) {
    if (check(t)) {
        return tokens[current++];
    }

    throw std::runtime_error(
        "SyntaxError at line " + std::to_string(peek().line) + ": " + m
    );
}

void Parser::endStatement() {
    if (check(TokenType::NEWLINE)) {
        newlines();
        return;
    }

    if (check(TokenType::RBRACE) || atEnd()) {
        return;
    }

    throw std::runtime_error(
        "SyntaxError at line " + std::to_string(peek().line) +
        ": expected end of line"
    );
}

ExprPtr Parser::primary() {
    if (match(TokenType::NUMBER) ||
        match(TokenType::STRING) ||
        match(TokenType::TRUE) ||
        match(TokenType::FALSE) ||
        match(TokenType::NIL)) {
        return ExprPtr(new LiteralExpr(previous()));
    }

    if (match(TokenType::FSTRING)) {
        return ExprPtr(new FStringExpr(previous().lexeme));
    }

    if (match(TokenType::INPUT)) {
        consume(TokenType::LPAREN, "expected '('");

        ExprPtr prompt;
        if (!check(TokenType::RPAREN)) {
            prompt = expression();
        }

        consume(TokenType::RPAREN, "expected ')'");
        return ExprPtr(new InputExpr(std::move(prompt)));
    }

    if (match(TokenType::LBRACE)) {
        std::vector<std::pair<std::string, ExprPtr>> entries;
        newlines();

        if (!check(TokenType::RBRACE)) {
            do {
                newlines();

                Token key = consume(
                    TokenType::STRING,
                    "dictionary keys must be strings"
                );

                consume(
                    TokenType::COLON,
                    "expected ':' after dictionary key"
                );

                entries.emplace_back(key.lexeme, expression());
                newlines();
            } while (match(TokenType::COMMA));
        }

        consume(
            TokenType::RBRACE,
            "expected '}' after dictionary"
        );

        return ExprPtr(new DictExpr(std::move(entries)));
    }

    if (match(TokenType::LBRACKET)) {
        std::vector<ExprPtr> a;

        if (!check(TokenType::RBRACKET)) {
            do {
                a.push_back(expression());
            } while (match(TokenType::COMMA));
        }

        consume(TokenType::RBRACKET, "expected ']'");
        return ExprPtr(new ArrayExpr(std::move(a)));
    }

    if (match(TokenType::IDENTIFIER)) {
        return ExprPtr(new VariableExpr(previous()));
    }

    if (match(TokenType::LPAREN)) {
        ExprPtr e = expression();
        consume(TokenType::RPAREN, "expected ')'");
        return e;
    }

    throw std::runtime_error(
        "SyntaxError at line " + std::to_string(peek().line) +
        ": expected expression"
    );
}

ExprPtr Parser::postfix() {
    ExprPtr e = primary();

    while (true) {
        if (match(TokenType::LBRACKET)) {
            ExprPtr first;

            if (!check(TokenType::COLON)) {
                first = expression();
            }

            if (match(TokenType::COLON)) {
                ExprPtr last;
                ExprPtr step;

                if (!check(TokenType::COLON) &&
                    !check(TokenType::RBRACKET)) {
                    last = expression();
                }

                if (match(TokenType::COLON) &&
                    !check(TokenType::RBRACKET)) {
                    step = expression();
                }

                consume(TokenType::RBRACKET, "expected ']'");

                e = ExprPtr(new SliceExpr(
                    std::move(e),
                    std::move(first),
                    std::move(last),
                    std::move(step)
                ));
            } else {
                if (!first) {
                    throw std::runtime_error(
                        "SyntaxError: expected index"
                    );
                }

                consume(TokenType::RBRACKET, "expected ']'");

                e = ExprPtr(new IndexExpr(
                    std::move(e),
                    std::move(first)
                ));
            }

            continue;
        }

        if (match(TokenType::DOT)) {
            auto v = dynamic_cast<VariableExpr*>(e.get());

            if (!v) {
                throw std::runtime_error(
                    "SyntaxError: expected variable or module before '.'"
                );
            }

            std::string owner = v->name.lexeme;

            Token n = consume(
                TokenType::IDENTIFIER,
                "expected member name after '.'"
            );

            std::vector<ExprPtr> args;
            const std::string method = n.lexeme;

            const bool zero =
                method == "sum" ||
                method == "min" ||
                method == "max" ||
                method == "avg" ||
                method == "pop" ||
                method == "upper" ||
                method == "lower" ||
                method == "sort" ||
                method == "rev" ||
                method == "trim" ||
                method == "keys" ||
                method == "values" ||
                method == "len";

            const bool one =
                method == "append" ||
                method == "add" ||
                method == "push" ||
                method == "remove" ||
                method == "split" ||
                method == "index" ||
                method == "count" ||
                method == "startswith" ||
                method == "endswith" ||
                method == "find" ||
                method == "has";

            const bool two = method == "replace";

            if (zero || one || two) {
                if (one || two) {
                    args.push_back(expression());

                    if (two) {
                        consume(
                            TokenType::COMMA,
                            "expected ',' between method arguments"
                        );

                        args.push_back(expression());
                    }
                }
            } else if (!check(TokenType::LPAREN)) {
                e = ExprPtr(new PropertyExpr(
                    std::move(e),
                    n
                ));

                continue;
            } else {
                consume(
                    TokenType::LPAREN,
                    "expected '(' after module function"
                );

                if (!check(TokenType::RPAREN)) {
                    do {
                        args.push_back(expression());
                    } while (match(TokenType::COMMA));
                }

                consume(TokenType::RPAREN, "expected ')'");
            }

            e = ExprPtr(new CallExpr(
                n,
                owner,
                std::move(args)
            ));

            continue;
        }

        if (match(TokenType::LPAREN)) {
            auto v = dynamic_cast<VariableExpr*>(e.get());

            if (!v) {
                throw std::runtime_error(
                    "SyntaxError: invalid function call"
                );
            }

            Token n = v->name;
            std::vector<ExprPtr> a;

            if (!check(TokenType::RPAREN)) {
                do {
                    a.push_back(expression());
                } while (match(TokenType::COMMA));
            }

            consume(TokenType::RPAREN, "expected ')'");

            e = ExprPtr(new CallExpr(
                n,
                "",
                std::move(a)
            ));

            continue;
        }

        break;
    }

    return e;
}

ExprPtr Parser::power() {
    ExprPtr e = postfix();

    if (match(TokenType::CARET)) {
        Token op = previous();

        e = ExprPtr(new BinaryExpr(
            std::move(e),
            op,
            unary()
        ));
    }

    return e;
}

ExprPtr Parser::unary() {
    if (match(TokenType::MINUS) ||
        match(TokenType::PLUS)) {
        Token op = previous();

        return ExprPtr(new UnaryExpr(
            op,
            unary()
        ));
    }

    return power();
}

ExprPtr Parser::multiplication() {
    ExprPtr e = unary();

    while (check(TokenType::STAR) ||
           check(TokenType::SLASH) ||
           check(TokenType::FLOOR) ||
           check(TokenType::PERCENT)) {
        Token op = tokens[current++];

        e = ExprPtr(new BinaryExpr(
            std::move(e),
            op,
            unary()
        ));
    }

    return e;
}

ExprPtr Parser::addition() {
    ExprPtr e = multiplication();

    while (check(TokenType::PLUS) ||
           check(TokenType::MINUS)) {
        Token op = tokens[current++];

        e = ExprPtr(new BinaryExpr(
            std::move(e),
            op,
            multiplication()
        ));
    }

    return e;
}

ExprPtr Parser::comparison() {
    ExprPtr e = addition();

    while (check(TokenType::IN) ||
           check(TokenType::LT) ||
           check(TokenType::GT) ||
           check(TokenType::LTE) ||
           check(TokenType::GTE)) {
        Token op = tokens[current++];

        e = ExprPtr(new BinaryExpr(
            std::move(e),
            op,
            addition()
        ));
    }

    return e;
}

ExprPtr Parser::equality() {
    ExprPtr e = comparison();

    while (check(TokenType::EQEQ) ||
           check(TokenType::NOT_EQ)) {
        Token op = tokens[current++];

        e = ExprPtr(new BinaryExpr(
            std::move(e),
            op,
            comparison()
        ));
    }

    return e;
}

ExprPtr Parser::logicalNot() {
    if (match(TokenType::NOT)) {
        Token op = previous();

        return ExprPtr(new UnaryExpr(
            op,
            logicalNot()
        ));
    }

    return equality();
}

ExprPtr Parser::logicalAnd() {
    ExprPtr e = logicalNot();

    while (match(TokenType::AND)) {
        Token op = previous();

        e = ExprPtr(new BinaryExpr(
            std::move(e),
            op,
            logicalNot()
        ));
    }

    return e;
}

ExprPtr Parser::logicalOr() {
    ExprPtr e = logicalAnd();

    while (match(TokenType::OR)) {
        Token op = previous();

        e = ExprPtr(new BinaryExpr(
            std::move(e),
            op,
            logicalAnd()
        ));
    }

    return e;
}

ExprPtr Parser::ternary() {
    ExprPtr yes = logicalOr();

    if (match(TokenType::IF)) {
        ExprPtr condition = logicalOr();

        consume(
            TokenType::ELSE,
            "expected else in ternary expression"
        );

        ExprPtr no = ternary();

        return ExprPtr(new TernaryExpr(
            std::move(yes),
            std::move(condition),
            std::move(no)
        ));
    }

    return yes;
}

ExprPtr Parser::expression() {
    return ternary();
}

StmtPtr Parser::block() {
    consume(TokenType::LBRACE, "expected '{'");
    newlines();

    std::vector<StmtPtr> b;

    while (!check(TokenType::RBRACE) && !atEnd()) {
        b.push_back(statement());
        newlines();
    }

    consume(TokenType::RBRACE, "expected '}'");

    return StmtPtr(new BlockStmt(std::move(b)));
}

StmtPtr Parser::func() {
    Token name = consume(
        TokenType::IDENTIFIER,
        "expected function name"
    );

    consume(TokenType::LPAREN, "expected '('");

    std::vector<Token> params;
    std::vector<ExprPtr> defaults;
    bool seenDefault = false;
    std::unordered_set<std::string> seen;

    if (!check(TokenType::RPAREN)) {
        do {
            Token t = consume(
                TokenType::IDENTIFIER,
                "expected parameter"
            );

            if (!seen.insert(t.lexeme).second) {
                throw std::runtime_error(
                    "SyntaxError: duplicate parameter '" +
                    t.lexeme + "'"
                );
            }

            params.push_back(t);

            if (match(TokenType::ASSIGN)) {
                seenDefault = true;
                defaults.push_back(expression());
            } else {
                if (seenDefault) {
                    throw std::runtime_error(
                        "SyntaxError: required parameter after default parameter"
                    );
                }

                defaults.push_back(ExprPtr());
            }
        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RPAREN, "expected ')'");
    newlines();

    ++functionDepth;

    int oldLoop = loopDepth;
    loopDepth = 0;

    StmtPtr b;

    try {
        b = block();
    } catch (...) {
        --functionDepth;
        loopDepth = oldLoop;
        throw;
    }

    --functionDepth;
    loopDepth = oldLoop;

    return StmtPtr(new FuncStmt(
        name,
        std::move(params),
        std::move(defaults),
        std::move(b)
    ));
}

StmtPtr Parser::classStmt() {
    Token name = consume(
        TokenType::IDENTIFIER,
        "expected class name"
    );

    newlines();

    consume(
        TokenType::LBRACE,
        "expected '{' after class name"
    );

    newlines();

    std::vector<std::unique_ptr<FuncStmt>> methods;

    ++classDepth;

    while (!check(TokenType::RBRACE) && !atEnd()) {
        consume(
            TokenType::FUNC,
            "expected func inside class"
        );

        bool special = match(TokenType::MINUS);

        if (special &&
            !(check(TokenType::IDENTIFIER) &&
              peek().lexeme == "init")) {
            throw std::runtime_error(
                "SyntaxError: only -init allowed in class"
            );
        }

        StmtPtr m = func();

        auto* f = dynamic_cast<FuncStmt*>(m.release());

        if (special) {
            f->name.lexeme = "-init";
        }

        methods.emplace_back(f);
        newlines();
    }

    consume(
        TokenType::RBRACE,
        "expected '}' after class"
    );

    --classDepth;

    return StmtPtr(new ClassStmt(
        name,
        std::move(methods)
    ));
}

StmtPtr Parser::ifTail() {
    ExprPtr c = expression();
    newlines();

    StmtPtr yes = block();
    newlines();

    StmtPtr no;

    if (match(TokenType::ELIF)) {
        no = ifTail();
    } else if (match(TokenType::ELSE)) {
        newlines();
        no = block();
    }

    return StmtPtr(new IfStmt(
        std::move(c),
        std::move(yes),
        std::move(no)
    ));
}

StmtPtr Parser::ifStmt() {
    return ifTail();
}

StmtPtr Parser::whileStmt() {
    ExprPtr c = expression();
    newlines();

    ++loopDepth;

    StmtPtr b;

    try {
        b = block();
    } catch (...) {
        --loopDepth;
        throw;
    }

    --loopDepth;

    return StmtPtr(new WhileStmt(
        std::move(c),
        std::move(b)
    ));
}

StmtPtr Parser::forStmt() {
    Token name = consume(
        TokenType::IDENTIFIER,
        "expected loop variable"
    );

    std::vector<ExprPtr> args;
    ExprPtr iterable;

    if (match(TokenType::IN)) {
        iterable = expression();
    } else {
        consume(
            TokenType::LPAREN,
            "expected '(' or 'in'"
        );

        args.push_back(expression());

        while (match(TokenType::COMMA)) {
            args.push_back(expression());
        }

        consume(TokenType::RPAREN, "expected ')'");

        if (args.size() > 3) {
            throw std::runtime_error(
                "SyntaxError: for accepts at most 3 arguments"
            );
        }
    }

    newlines();
    ++loopDepth;

    StmtPtr b;

    try {
        b = block();
    } catch (...) {
        --loopDepth;
        throw;
    }

    --loopDepth;

    return StmtPtr(new ForStmt(
        name,
        std::move(args),
        std::move(iterable),
        std::move(b)
    ));
}

StmtPtr Parser::statement() {
    newlines();

    if (match(TokenType::PASS)) {
        endStatement();
        return StmtPtr(new PassStmt());
    }

    if (match(TokenType::BREAK)) {
        if (!loopDepth) {
            throw std::runtime_error(
                "SyntaxError: break outside loop"
            );
        }

        endStatement();
        return StmtPtr(new BreakStmt());
    }

    if (match(TokenType::CONTINUE)) {
        if (!loopDepth) {
            throw std::runtime_error(
                "SyntaxError: continue outside loop"
            );
        }

        endStatement();
        return StmtPtr(new ContinueStmt());
    }

    if (match(TokenType::TRY)) {
        newlines();

        StmtPtr body = block();
        newlines();

        consume(
            TokenType::CATCH,
            "expected catch after try"
        );

        Token error(
            TokenType::IDENTIFIER,
            "",
            peek().line
        );

        bool named = false;

        if (match(TokenType::LPAREN)) {
            error = consume(
                TokenType::IDENTIFIER,
                "expected error variable"
            );

            consume(TokenType::RPAREN, "expected ')'");
            named = true;
        }

        newlines();

        return StmtPtr(new TryStmt(
            std::move(body),
            block(),
            error,
            named
        ));
    }

    if (match(TokenType::CLASS)) {
        return classStmt();
    }

    if (match(TokenType::FUNC)) {
        if (match(TokenType::MINUS)) {
            Token name = consume(
                TokenType::IDENTIFIER,
                "expected special function name"
            );

            if (name.lexeme != "init" &&
                name.lexeme != "start" &&
                name.lexeme != "always" &&
                name.lexeme != "keypress" &&
                name.lexeme != "exit") {
                throw std::runtime_error(
                    "SyntaxError: unknown special function"
                );
            }

            if ((name.lexeme == "init") !=
                (classDepth > 0)) {
                throw std::runtime_error(
                    "SyntaxError: -init must be inside a class, other events outside classes"
                );
            }

            std::vector<ExprPtr> args;

            if (match(TokenType::LPAREN)) {
                if (!check(TokenType::RPAREN)) {
                    do {
                        args.push_back(expression());
                    } while (match(TokenType::COMMA));
                }

                consume(TokenType::RPAREN, "expected )");
            }

            newlines();
            ++functionDepth;

            StmtPtr body = block();

            --functionDepth;

            return StmtPtr(new SpecialStmt(
                name.lexeme,
                std::move(args),
                std::move(body)
            ));
        }

        return func();
    }

    if (match(TokenType::IF)) {
        return ifStmt();
    }

    if (match(TokenType::WHILE)) {
        return whileStmt();
    }

    if (match(TokenType::FOR)) {
        return forStmt();
    }

    if (match(TokenType::GET)) {
        Token name = consume(
            TokenType::IDENTIFIER,
            "expected module name"
        );

        endStatement();
        return StmtPtr(new GetStmt(name));
    }

    if (check(TokenType::LBRACE)) {
        return block();
    }

    if (match(TokenType::RETURN)) {
        if (functionDepth == 0) {
            throw std::runtime_error(
                "SyntaxError: return outside function"
            );
        }

        ExprPtr e;

        if (!check(TokenType::NEWLINE) &&
            !check(TokenType::RBRACE) &&
            !atEnd()) {
            e = expression();
        }

        endStatement();

        return StmtPtr(new ReturnStmt(std::move(e)));
    }

    if (match(TokenType::OUT)) {
        ExprPtr e = expression();
        endStatement();

        return StmtPtr(new OutStmt(std::move(e)));
    }

    if (check(TokenType::IDENTIFIER) &&
        current + 1 < tokens.size()) {
        TokenType t = tokens[current + 1].type;

        if (t == TokenType::ASSIGN ||
            t == TokenType::PLUS_EQ ||
            t == TokenType::MINUS_EQ ||
            t == TokenType::STAR_EQ ||
            t == TokenType::SLASH_EQ ||
            t == TokenType::PERCENT_EQ ||
            t == TokenType::CARET_EQ) {
            Token name = tokens[current++];
            Token op = tokens[current++];

            ExprPtr e = expression();
            endStatement();

            return StmtPtr(new AssignStmt(
                name,
                op,
                std::move(e)
            ));
        }
    }

    // File commands use statement syntax without parentheses.
    // Keep them as CallExpr so the existing interpreter can execute them.
    if (check(TokenType::IDENTIFIER)) {
        const std::string& command = peek().lexeme;
        const bool oneArg = command == "deleteFile" || command == "readFile" ||
                            command == "readfile" || command == "fileExists" ||
                            command == "fileexists";
        const bool twoArgs = command == "writeFile" || command == "saveFile" ||
                             command == "addFile" || command == "writefile" ||
                             command == "addfile";
        if ((oneArg || twoArgs) &&
            current + 1 < tokens.size() &&
            tokens[current + 1].type != TokenType::LPAREN &&
            tokens[current + 1].type != TokenType::ASSIGN &&
            tokens[current + 1].type != TokenType::DOT) {
            Token name = tokens[current++];
            std::vector<ExprPtr> args;
            args.push_back(expression());
            if (twoArgs) {
                consume(TokenType::COMMA, "expected ',' between file command arguments");
                args.push_back(expression());
            }
            endStatement();
            return StmtPtr(new ExprStmt(ExprPtr(new CallExpr(
                name, "", std::move(args)
            ))));
        }
    }

    ExprPtr e = expression();

    if (dynamic_cast<PropertyExpr*>(e.get())) {
        if (check(TokenType::ASSIGN) ||
            check(TokenType::PLUS_EQ) ||
            check(TokenType::MINUS_EQ) ||
            check(TokenType::STAR_EQ) ||
            check(TokenType::SLASH_EQ)) {
            Token op = tokens[current++];

            auto* prop =
                static_cast<PropertyExpr*>(e.release());

            ExprPtr value = expression();
            endStatement();

            StmtPtr result(new PropertyAssignStmt(
                std::move(prop->base),
                prop->name,
                op,
                std::move(value)
            ));

            delete prop;
            return result;
        }
    }

    if (dynamic_cast<SliceExpr*>(e.get()) &&
        check(TokenType::ASSIGN)) {
        throw std::runtime_error(
            "SyntaxError: slice assignment not supported"
        );
    }

    if (dynamic_cast<IndexExpr*>(e.get())) {
        if (check(TokenType::ASSIGN) ||
            check(TokenType::PLUS_EQ) ||
            check(TokenType::MINUS_EQ) ||
            check(TokenType::STAR_EQ) ||
            check(TokenType::SLASH_EQ) ||
            check(TokenType::PERCENT_EQ) ||
            check(TokenType::CARET_EQ)) {
            Token op = tokens[current++];

            ExprPtr value = expression();
            endStatement();

            return StmtPtr(new IndexAssignStmt(
                std::move(e),
                op,
                std::move(value)
            ));
        }
    }

    endStatement();

    return StmtPtr(new ExprStmt(std::move(e)));
}
std::vector<StmtPtr> Parser::parse() {
    std::vector<StmtPtr> s;
    newlines();
    while (!atEnd()) {
        s.push_back(statement());
        newlines();
    }
    return s;
}