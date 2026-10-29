#include "parser.h"
#include <stdexcept>
#include <unordered_set>

Token Parser::consume(FluxTokenType t, const std::string& m) {
    if (check(t)) {
        return tokens[current++];
    }

    throw std::runtime_error(
        "SyntaxError at line " + std::to_string(peek().line) + ": " + m
    );
}

void Parser::endStatement() {
    if (check(FluxTokenType::NEWLINE)) {
        newlines();
        return;
    }

    if (check(FluxTokenType::RBRACE) || atEnd()) {
        return;
    }

    throw std::runtime_error(
        "SyntaxError at line " + std::to_string(peek().line) +
        ": expected end of line"
    );
}

ExprPtr Parser::primary() {
    if (match(FluxTokenType::DIMENSION) ||
        match(FluxTokenType::NUMBER) ||
        match(FluxTokenType::IMAGINARY) ||
        match(FluxTokenType::STRING) ||
        match(FluxTokenType::TRUE) ||
        match(FluxTokenType::FALSE) ||
        match(FluxTokenType::NIL)) {
        return ExprPtr(new LiteralExpr(previous()));
    }

    if (match(FluxTokenType::FSTRING)) {
        return ExprPtr(new FStringExpr(previous().lexeme));
    }

    if(match(FluxTokenType::FUNC)) {
        consume(FluxTokenType::LPAREN,"expected '(' after anonymous func");
        std::vector<Token> params;
        if(!check(FluxTokenType::RPAREN))do {
            params.push_back(consume(FluxTokenType::IDENTIFIER,"expected lambda parameter"));
        }while(match(FluxTokenType::COMMA));
        consume(FluxTokenType::RPAREN,"expected ')' after lambda parameters");
        consume(FluxTokenType::COLON,"expected ':' after lambda parameters");
        return ExprPtr(new LambdaExpr(std::move(params),expression()));
    }
    if (match(FluxTokenType::INPUT)) {
        consume(FluxTokenType::LPAREN, "expected '('");

        ExprPtr prompt;
        if (!check(FluxTokenType::RPAREN)) {
            prompt = expression();
        }

        consume(FluxTokenType::RPAREN, "expected ')'");
        return ExprPtr(new InputExpr(std::move(prompt)));
    }

    if (match(FluxTokenType::LBRACE)) {
        std::vector<std::pair<std::string, ExprPtr>> entries;
        newlines();

        if (!check(FluxTokenType::RBRACE)) {
            do {
                newlines();

                Token key = consume(
                    FluxTokenType::STRING,
                    "dictionary keys must be strings"
                );

                consume(
                    FluxTokenType::COLON,
                    "expected ':' after dictionary key"
                );

                entries.emplace_back(key.lexeme, expression());
                newlines();
            } while (match(FluxTokenType::COMMA));
        }

        consume(
            FluxTokenType::RBRACE,
            "expected '}' after dictionary"
        );

        return ExprPtr(new DictExpr(std::move(entries)));
    }

    if (match(FluxTokenType::LBRACKET)) {
        std::vector<ExprPtr> a;

        if (!check(FluxTokenType::RBRACKET)) {
            do {
                a.push_back(expression());
            } while (match(FluxTokenType::COMMA));
        }

        consume(FluxTokenType::RBRACKET, "expected ']'");
        return ExprPtr(new ArrayExpr(std::move(a)));
    }

    if (match(FluxTokenType::IDENTIFIER)) {
        return ExprPtr(new VariableExpr(previous()));
    }

    if (match(FluxTokenType::LPAREN)) {
        ExprPtr e = expression();
        consume(FluxTokenType::RPAREN, "expected ')'");
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
        if (match(FluxTokenType::LBRACKET)) {
            ExprPtr first;

            if (!check(FluxTokenType::COLON)) {
                first = expression();
            }

            if (match(FluxTokenType::COLON)) {
                ExprPtr last;
                ExprPtr step;

                if (!check(FluxTokenType::COLON) &&
                    !check(FluxTokenType::RBRACKET)) {
                    last = expression();
                }

                if (match(FluxTokenType::COLON) &&
                    !check(FluxTokenType::RBRACKET)) {
                    step = expression();
                }

                consume(FluxTokenType::RBRACKET, "expected ']'");

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

                consume(FluxTokenType::RBRACKET, "expected ']'");

                e = ExprPtr(new IndexExpr(
                    std::move(e),
                    std::move(first)
                ));
            }

            continue;
        }

        if (match(FluxTokenType::DOT)) {
            auto v = dynamic_cast<VariableExpr*>(e.get());

            if (!v) {
                throw std::runtime_error(
                    "SyntaxError: expected variable or module before '.'"
                );
            }

            std::string owner = v->name.lexeme;

            Token n = consume(
                FluxTokenType::IDENTIFIER,
                "expected member name after '.'"
            );

            std::vector<ExprPtr> args;
            std::vector<std::string> argNames;
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

            if ((zero || one || two) && !check(FluxTokenType::LPAREN)) {
                if (one || two) {
                    args.push_back(expression());

                    if (two) {
                        consume(
                            FluxTokenType::COMMA,
                            "expected ',' between method arguments"
                        );

                        args.push_back(expression());
                    }
                }
            } else if (!check(FluxTokenType::LPAREN)) {
                e = ExprPtr(new PropertyExpr(
                    std::move(e),
                    n
                ));

                continue;
            } else {
                consume(
                    FluxTokenType::LPAREN,
                    "expected '(' after module function"
                );

                if (!check(FluxTokenType::RPAREN)) {
                    do {
                        if(check(FluxTokenType::IDENTIFIER) && current+1<tokens.size() && tokens[current+1].type==FluxTokenType::ASSIGN) {
                            argNames.push_back(tokens[current++].lexeme);++current;
                        } else argNames.push_back("");
                        args.push_back(expression());
                    } while (match(FluxTokenType::COMMA));
                }

                consume(FluxTokenType::RPAREN, "expected ')'");
            }

            auto call=new CallExpr(n,owner,std::move(args));
            call->argNames=std::move(argNames);
            e=ExprPtr(call);

            continue;
        }

        if (match(FluxTokenType::LPAREN)) {
            auto v = dynamic_cast<VariableExpr*>(e.get());

            if (!v) {
                throw std::runtime_error(
                    "SyntaxError: invalid function call"
                );
            }

            Token n = v->name;
            std::vector<ExprPtr> a;
            std::vector<std::string> argNames;
            if (!check(FluxTokenType::RPAREN)) {
                do {
                    if(check(FluxTokenType::IDENTIFIER) && current+1<tokens.size() && tokens[current+1].type==FluxTokenType::ASSIGN) {
                        argNames.push_back(tokens[current++].lexeme);++current;
                    } else argNames.push_back("");
                    a.push_back(expression());
                } while (match(FluxTokenType::COMMA));
            }

            consume(FluxTokenType::RPAREN, "expected ')'");

            auto call=new CallExpr(n,"",std::move(a));
            call->argNames=std::move(argNames);
            e=ExprPtr(call);

            continue;
        }

        break;
    }

    return e;
}

ExprPtr Parser::power() {
    ExprPtr e = postfix();

    if (match(FluxTokenType::CARET)) {
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
    if (match(FluxTokenType::MINUS) ||
        match(FluxTokenType::PLUS)) {
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

    while (check(FluxTokenType::STAR) ||
           check(FluxTokenType::SLASH) ||
           check(FluxTokenType::FLOOR) ||
           check(FluxTokenType::PERCENT)) {
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

    while (check(FluxTokenType::PLUS) ||
           check(FluxTokenType::MINUS)) {
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

    while (check(FluxTokenType::IN) ||
           check(FluxTokenType::LT) ||
           check(FluxTokenType::GT) ||
           check(FluxTokenType::LTE) ||
           check(FluxTokenType::GTE)) {
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

    while (check(FluxTokenType::EQEQ) ||
           check(FluxTokenType::NOT_EQ)) {
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
    if (match(FluxTokenType::NOT)) {
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

    while (match(FluxTokenType::AND)) {
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

    while (match(FluxTokenType::OR)) {
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

    if (match(FluxTokenType::IF)) {
        ExprPtr condition = logicalOr();

        consume(
            FluxTokenType::ELSE,
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
    consume(FluxTokenType::LBRACE, "expected '{'");
    newlines();

    std::vector<StmtPtr> b;

    while (!check(FluxTokenType::RBRACE) && !atEnd()) {
        b.push_back(statement());
        newlines();
    }

    consume(FluxTokenType::RBRACE, "expected '}'");

    return StmtPtr(new BlockStmt(std::move(b)));
}

StmtPtr Parser::func() {
    Token name = consume(
        FluxTokenType::IDENTIFIER,
        "expected function name"
    );

    consume(FluxTokenType::LPAREN, "expected '('");

    std::vector<Token> params;
    std::vector<ExprPtr> defaults;
    bool seenDefault = false;
    std::unordered_set<std::string> seen;

    if (!check(FluxTokenType::RPAREN)) {
        do {
            Token t = consume(
                FluxTokenType::IDENTIFIER,
                "expected parameter"
            );

            if (!seen.insert(t.lexeme).second) {
                throw std::runtime_error(
                    "SyntaxError: duplicate parameter '" +
                    t.lexeme + "'"
                );
            }

            params.push_back(t);

            if (match(FluxTokenType::ASSIGN)) {
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
        } while (match(FluxTokenType::COMMA));
    }

    consume(FluxTokenType::RPAREN, "expected ')'");
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
        FluxTokenType::IDENTIFIER,
        "expected class name"
    );

    std::string parent;
    if(match(FluxTokenType::LPAREN)) {
        parent=consume(FluxTokenType::IDENTIFIER,"expected parent class").lexeme;
        consume(FluxTokenType::RPAREN,"expected ')' after parent class");
    }
    newlines();
    consume(
        FluxTokenType::LBRACE,
        "expected '{' after class name"
    );

    newlines();

    std::vector<std::unique_ptr<FuncStmt>> methods;

    ++classDepth;

    while (!check(FluxTokenType::RBRACE) && !atEnd()) {
        consume(
            FluxTokenType::FUNC,
            "expected func inside class"
        );

        bool special = match(FluxTokenType::MINUS);

        if (special &&
            !(check(FluxTokenType::IDENTIFIER) &&
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
        FluxTokenType::RBRACE,
        "expected '}' after class"
    );

    --classDepth;

    return StmtPtr(new ClassStmt(
        name,
        parent,
        std::move(methods)
    ));
}

StmtPtr Parser::ifTail() {
    ExprPtr c = expression();
    newlines();

    StmtPtr yes = block();
    newlines();

    StmtPtr no;

    if (match(FluxTokenType::ELIF)) {
        no = ifTail();
    } else if (match(FluxTokenType::ELSE)) {
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
        FluxTokenType::IDENTIFIER,
        "expected loop variable"
    );

    std::vector<ExprPtr> args;
    ExprPtr iterable;

    if (match(FluxTokenType::IN)) {
        iterable = expression();
    } else {
        consume(
            FluxTokenType::LPAREN,
            "expected '(' or 'in'"
        );

        args.push_back(expression());

        while (match(FluxTokenType::COMMA)) {
            args.push_back(expression());
        }

        consume(FluxTokenType::RPAREN, "expected ')'");

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

    if (match(FluxTokenType::PASS)) {
        endStatement();
        return StmtPtr(new PassStmt());
    }

    if (match(FluxTokenType::BREAK)) {
        if (!loopDepth) {
            throw std::runtime_error(
                "SyntaxError: break outside loop"
            );
        }

        endStatement();
        return StmtPtr(new BreakStmt());
    }

    if (match(FluxTokenType::CONTINUE)) {
        if (!loopDepth) {
            throw std::runtime_error(
                "SyntaxError: continue outside loop"
            );
        }

        endStatement();
        return StmtPtr(new ContinueStmt());
    }

    if (match(FluxTokenType::TRY)) {
        newlines();

        StmtPtr body = block();
        newlines();

        consume(
            FluxTokenType::CATCH,
            "expected catch after try"
        );

        Token error(
            FluxTokenType::IDENTIFIER,
            "",
            peek().line
        );

        bool named = false;

        if (match(FluxTokenType::LPAREN)) {
            error = consume(
                FluxTokenType::IDENTIFIER,
                "expected error variable"
            );

            consume(FluxTokenType::RPAREN, "expected ')'");
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

    if(match(FluxTokenType::ASSERT)) {
        ExprPtr condition=expression(),message;
        if(match(FluxTokenType::COMMA))message=expression();
        endStatement();return StmtPtr(new AssertStmt(std::move(condition),std::move(message)));
    }
    if (match(FluxTokenType::CLASS)) {
        return classStmt();
    }

    if (match(FluxTokenType::FUNC)) {
        if (match(FluxTokenType::MINUS)) {
            Token name = consume(
                FluxTokenType::IDENTIFIER,
                "expected special function name"
            );

            if (name.lexeme != "init" &&
                name.lexeme != "start" &&
                name.lexeme != "ready" &&
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
            if (match(FluxTokenType::LPAREN)) {
                if (!check(FluxTokenType::RPAREN)) {
                    do { args.push_back(expression()); } while (match(FluxTokenType::COMMA));
                }
                consume(FluxTokenType::RPAREN, "expected )");
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

    if (match(FluxTokenType::IF)) {
        return ifStmt();
    }

    if (match(FluxTokenType::WHILE)) {
        return whileStmt();
    }

    if (match(FluxTokenType::FOR)) {
        return forStmt();
    }

    if (match(FluxTokenType::GET)) {
        Token name = consume(
            FluxTokenType::IDENTIFIER,
            "expected module name"
        );

        while(match(FluxTokenType::DOT)) {
            name.lexeme += "." + consume(FluxTokenType::IDENTIFIER,"expected module path segment").lexeme;
        }
        std::string alias;
        if (check(FluxTokenType::IDENTIFIER) && peek().lexeme == "as") {
            ++current;
            alias = consume(FluxTokenType::IDENTIFIER, "expected module alias").lexeme;
        }
        endStatement();
        return StmtPtr(new GetStmt(name, alias));
    }

    if (check(FluxTokenType::LBRACE)) {
        return block();
    }

    if (match(FluxTokenType::RETURN)) {
        if (functionDepth == 0) {
            throw std::runtime_error(
                "SyntaxError: return outside function"
            );
        }

        ExprPtr e;

        if (!check(FluxTokenType::NEWLINE) &&
            !check(FluxTokenType::RBRACE) &&
            !atEnd()) {
            e = expression();
        }

        endStatement();

        return StmtPtr(new ReturnStmt(std::move(e)));
    }

    if (match(FluxTokenType::OUT)) {
        ExprPtr e = expression();
        endStatement();

        return StmtPtr(new OutStmt(std::move(e)));
    }

    if (check(FluxTokenType::IDENTIFIER) &&
        current + 1 < tokens.size()) {
        FluxTokenType t = tokens[current + 1].type;

        if (t == FluxTokenType::ASSIGN ||
            t == FluxTokenType::PLUS_EQ ||
            t == FluxTokenType::MINUS_EQ ||
            t == FluxTokenType::STAR_EQ ||
            t == FluxTokenType::SLASH_EQ ||
            t == FluxTokenType::PERCENT_EQ ||
            t == FluxTokenType::CARET_EQ) {
            Token name = tokens[current++];
            Token op = tokens[current++];

            ExprPtr e = expression();
            if (check(FluxTokenType::LBRACE)) {
                auto* c = dynamic_cast<CallExpr*>(e.get());
                if (!c || (c->name.lexeme != "windowAdd" && !(c->module == "gui" && c->name.lexeme == "add")))
                    throw std::runtime_error("SyntaxError: block only supported for windowAdd/gui.add");
                StmtPtr assignment(new AssignStmt(name, op, std::move(e)));
                StmtPtr body = block();
                std::vector<StmtPtr> parts;
                parts.push_back(std::move(assignment));
                parts.push_back(StmtPtr(new GuiBlockStmt(ExprPtr(new VariableExpr(name)), std::move(body), "context")));
                return StmtPtr(new BlockStmt(std::move(parts)));
            }
            endStatement();

            return StmtPtr(new AssignStmt(
                name,
                op,
                std::move(e)
            ));
        }
    }

    ExprPtr e = expression();
    if (check(FluxTokenType::LBRACE)) {
        if (auto* prop=dynamic_cast<PropertyExpr*>(e.get())) {
            if (prop->name.lexeme == "context" || prop->name.lexeme == "onclick" || prop->name.lexeme == "onrclick" || prop->name.lexeme == "onchange" || prop->name.lexeme == "detectMouseMoving" || prop->name.lexeme == "mouseEnter" || prop->name.lexeme == "mouseLeave") {
                std::string kind=prop->name.lexeme;
                ExprPtr target=std::move(prop->base);
                StmtPtr body=block();
                return StmtPtr(new GuiBlockStmt(std::move(target),std::move(body),kind));
            }
        }
        if (auto* c=dynamic_cast<CallExpr*>(e.get())) {
            if (c->module.empty() && c->name.lexeme == "timer")
                return StmtPtr(new GuiBlockStmt(std::move(e),block(),"timer"));
            if (c->name.lexeme == "windowAdd" || (c->module == "gui" && c->name.lexeme == "add"))
                return StmtPtr(new GuiBlockStmt(std::move(e),block(),"context"));
        }
    }

    if (dynamic_cast<PropertyExpr*>(e.get())) {
        if (check(FluxTokenType::ASSIGN) ||
            check(FluxTokenType::PLUS_EQ) ||
            check(FluxTokenType::MINUS_EQ) ||
            check(FluxTokenType::STAR_EQ) ||
            check(FluxTokenType::SLASH_EQ)) {
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
        check(FluxTokenType::ASSIGN)) {
        throw std::runtime_error(
            "SyntaxError: slice assignment not supported"
        );
    }

    if (dynamic_cast<IndexExpr*>(e.get())) {
        if (check(FluxTokenType::ASSIGN) ||
            check(FluxTokenType::PLUS_EQ) ||
            check(FluxTokenType::MINUS_EQ) ||
            check(FluxTokenType::STAR_EQ) ||
            check(FluxTokenType::SLASH_EQ) ||
            check(FluxTokenType::PERCENT_EQ) ||
            check(FluxTokenType::CARET_EQ)) {
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