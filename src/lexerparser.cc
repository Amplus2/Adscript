#include "ast.hh"
#include "utils.hh"
#include "lexerparser.hh"

#include <iostream>

using namespace Adscript;

Lexer::Token Lexer::nextT() {
    // section declaration for goto statement we need later on
    nextT_start:

    // helper variable
    char c = getc(idx);

    // eat up whitespaces
    while (Utils::isWhitespace((c = getc(idx))) && idx < text.size())
        idx += 1;
    
    // handle comments
    if (c == ';') {
        if (getc(++idx) != ';') {
            Error::warning(U"comment beginning with only one ';'", pos());
        }

        // eat up until end of line
        while ((c = getc(idx)) != '\n' && c != '\r' && idx <= text.size())
            idx += 1;

        // eat up end of line
        idx += 1;

        // go to the beginning of nextT()
        goto nextT_start;
    }

    // handle end of file
    if (eofReached()) return Token(TT_EOF, U"end of file");

    // handle parentheses and brackets
    switch (c) {
    case '(':
        idx += 1;
        return Token(TT_PO, U"(");
    case ')':
        idx += 1;
        return Token(TT_PC, U")");
    case '[':
        idx += 1;
        return Token(TT_BRO, U"[");
    case ']':
        idx += 1;
        return Token(TT_BRC, U"]");
    case '*':
        idx += 1;
        return Token(TT_STAR, U"*");
    case '#':
        idx += 1;
        return Token(TT_HASH, U"#");
    case '\'':
        idx += 1;
        return Token(TT_QUOTE, U"'");
    }
    
    // helper variable for temporary string storage
    std::u32string tmpStr;

    // handle hex literals
    if (c == '0' && getc(idx +  1) == 'x') {
        tmpStr += U"0x";
        idx += 2;

        while (Utils::isHexChar((c = getc(idx)))) {
            tmpStr += c;
            idx += 1;
        }

        c = getc(idx);
        if (!Utils::isWhitespace(c) && !Utils::isSpecialChar(c))
            Error::warning(U"no whitespace after hex literal", pos());

        return Token(TT_HEX, tmpStr);
    }

    // handle integers and front part of FPs
    if (Utils::isDigit(c)) {
        idx+= 1;
        tmpStr += c;

        while (Utils::isDigit((c = getc(idx)))) {
            idx += 1;
            tmpStr += c;
        }

        if (c != '.') return Token(TT_INT, tmpStr);
    }
    
    // handle floats
    if (c == '.' && Utils::isDigit(getc(idx + 1))) {
        // append '.' to tmpStr
        tmpStr += getc(idx++);

        if (eofReached()) Error::lexerEOF();

        // append all digits after the '.' to tmpStr
        while (Utils::isDigit((c = getc(idx)))) {
            idx += 1;
            tmpStr += c;
        }

        return Token(TT_FLOAT, tmpStr);
    } else if (tmpStr.size() > 0 && c == '.') {
        return Token(TT_FLOAT, tmpStr + U".0");
    } else if (c == '.') {
        Error::lexer(U"expected digit after '.', got '"
            + std::u32string(1, getc(idx + 1)) + U"'", pos());
    }

    if (c == '"') {
        // eat up '"'
        idx += 1;

        if (eofReached()) Error::lexerEOF();

        bool lastBS = false;
        while (((c = getc(idx)) != '"' || lastBS) && idx <= text.size()) {
            tmpStr += c;
            lastBS = !lastBS && c == '\\';
            idx += 1;
        }

        if (eofReached()) Error::lexerEOF();

        // eat up '"'
        idx += 1;

        return Token(TT_STR, tmpStr);
    } else if ((c = getc(idx)) == '\\') {
        // eat up '\\'
        idx += 1;

        if (eofReached()) Error::lexerEOF();

        c = getc(idx);

        // eat up char
        idx += 1;

        return Token(TT_CHAR, std::u32string(1, c));
    }

    // handle identifiers
    while (!Utils::isWhitespace((c = getc(idx))) && !Utils::isSpecialChar(c) && idx < text.size()) {
        idx += 1;
        tmpStr += c;
    }

    return Token(TT_ID, tmpStr);
}

AST::Expr* tokenToExpr(Lexer::Token t) {
    switch (t.tt) {
    case Lexer::TT_ID:     return new AST::Identifier(std::to_string(t.val));
    case Lexer::TT_INT:    return new AST::Int(std::stol(t.val));
    case Lexer::TT_HEX:    return new AST::Int(std::stol(t.val, NULL, 16));
    case Lexer::TT_FLOAT:  return new AST::Float(std::stod(t.val));
    case Lexer::TT_CHAR:   return new AST::Char(t.val[0]);
    case Lexer::TT_STR:    return new AST::String(Utils::unescapeStr(t.val));
    default:        return nullptr;
    }
}

AST::Type* Parser::parseType(Lexer::Token& tmpT) {
    AST::Type *t = nullptr;

    // general types
    if (Utils::strEq(tmpT.val, {U"char", U"i8"})) {
        t = new AST::PrimType(AST::TYPE_I8);
    } else if (tmpT == "i16") {
        t = new AST::PrimType(AST::TYPE_I16);
    } else if (Utils::strEq(tmpT.val, {U"int", U"i32", U"bool"})) {
        t = new AST::PrimType(AST::TYPE_I32);
    } else if (Utils::strEq(tmpT.val, {U"long", U"i64"})) {
        t = new AST::PrimType(AST::TYPE_I64);
    } else if (tmpT == "float") {
        t = new AST::PrimType(AST::TYPE_FLOAT);
    } else if (tmpT == "double") {
        t = new AST::PrimType(AST::TYPE_DOUBLE);
    } else if (tmpT == Lexer::TT_ID) {
        t = new AST::IdentifierType(std::to_string(tmpT.val));
    } else if (tmpT == Lexer::TT_QUOTE) {
        // eat up quote
        tmpT = lexer.nextT();

        if (tmpT != Lexer::TT_PO)
            Error::parserExpected(U"'('", tmpT.val, lexer.pos());
        
        tmpT = lexer.nextT();

        std::map<std::string, AST::Type*> attrs;

        while (tmpT != Lexer::TT_PC && tmpT != Lexer::TT_EOF) {
            auto t1 = parseType(tmpT);

            if (!t1) Error::parserExpected(U"data type", tmpT.val, lexer.pos());

            if (tmpT != Lexer::TT_ID)
                Error::parserExpected(U"identifier", tmpT.val, lexer.pos());
            
            auto id = std::to_string(tmpT.val);

            if (attrs.find(id) != attrs.end())
                Error::parserExpected(
                    U"unique attribute identifier", tmpT.val, lexer.pos());
            
            attrs[id] = t1;

            tmpT = lexer.nextT();
        }

        if (tmpT == Lexer::TT_EOF)
            Error::parserExpected(U"')'", tmpT.val);

        //tmpT = lexer.nextT();

        t = new AST::StructType(attrs);
    } else return nullptr;

    tmpT = lexer.nextT();
        
    uint8_t quantity = 0;
    while (tmpT == Lexer::TT_STAR) {
        // eat up '*' 
        tmpT = lexer.nextT();

        quantity += 1;
    }

    if (quantity > 0) return new AST::PointerType(t, quantity);

    return t;
}

template<class T>
AST::Expr* parseTExpr1(Parser *p, Lexer::Token& tmpT) {
    // eat up remaining token
    tmpT = p->lexer.nextT();

    auto expr1 = p->parseExpr(tmpT);

    // eat up remaining token
    tmpT = p->lexer.nextT();

    return new T(expr1);
}

template<class T>
AST::Expr* parseTExpr2(Parser *p, Lexer::Token& tmpT) {
    // eat up remaining token
    tmpT = p->lexer.nextT();

    auto expr1 = p->parseExpr(tmpT);

    // eat up remaining token
    tmpT = p->lexer.nextT();

    auto expr2 = p->parseExpr(tmpT);

    // eat up remaining token
    tmpT = p->lexer.nextT();

    return new T(expr1, expr2);
}

template<class T>
AST::Expr* parseTExpr3(Parser *p, Lexer::Token& tmpT) {
    // eat up remaining token
    tmpT = p->lexer.nextT();

    auto expr1 = p->parseExpr(tmpT);

    // eat up remaining token
    tmpT = p->lexer.nextT();

    auto expr2 = p->parseExpr(tmpT);

    // eat up remaining token
    tmpT = p->lexer.nextT();

    auto expr3 = p->parseExpr(tmpT);

    // eat up remaining token
    tmpT = p->lexer.nextT();

    return new T(expr1, expr2, expr3);
}


AST::Expr* Parser::parseExpr(Lexer::Token& tmpT) {
    if (tmpT == Lexer::TT_PO) {
        tmpT = lexer.nextT();
        if (tmpT == Lexer::TT_EOF)
            Error::parser(U"unexpected end of file");
        else if (tmpT == Lexer::TT_STAR)
            return parseBinExpr(tmpT, AST::BINEXPR_MUL);
        else if (tmpT == Lexer::TT_ID) {
            if (tmpT == "+")
                return parseBinExpr(tmpT, AST::BINEXPR_ADD);
            else if (tmpT == "-")
                return parseBinExpr(tmpT, AST::BINEXPR_SUB);
            else if (tmpT == "/")
                return parseBinExpr(tmpT, AST::BINEXPR_DIV);
            else if (tmpT == "%")
                return parseBinExpr(tmpT, AST::BINEXPR_MOD);
            else if (tmpT == "|")
                return parseBinExpr(tmpT, AST::BINEXPR_OR);
            else if (tmpT == "&")
                return parseBinExpr(tmpT, AST::BINEXPR_AND);
            else if (tmpT == "^")
                return parseBinExpr(tmpT, AST::BINEXPR_XOR);
            else if (tmpT == "~")
                return parseBinExpr(tmpT, AST::BINEXPR_NOT);
            else if (tmpT == "=")
                return parseBinExpr(tmpT, AST::BINEXPR_EQ);
            else if (tmpT == "<")
                return parseBinExpr(tmpT, AST::BINEXPR_LT);
            else if (tmpT == ">")
                return parseBinExpr(tmpT, AST::BINEXPR_GT);
            else if (tmpT == "<=")
                return parseBinExpr(tmpT, AST::BINEXPR_LTEQ);
            else if (tmpT == ">=")
                return parseBinExpr(tmpT, AST::BINEXPR_GTEQ);
            else if (tmpT == "or")
                return parseBinExpr(tmpT, AST::BINEXPR_LOR);
            else if (tmpT == "and")
                return parseBinExpr(tmpT, AST::BINEXPR_LAND);
            else if (tmpT == "xor")
                return parseBinExpr(tmpT, AST::BINEXPR_LXOR);
            else if (tmpT == "not")
                return parseBinExpr(tmpT, AST::BINEXPR_LNOT);
            else if (tmpT == "if")
                return parseTExpr3<AST::If>(this, tmpT);
            else if (tmpT == "fn")
                return parseLambda(tmpT);
            else if (tmpT == "cast")
                return parseCast(tmpT);
            else if (tmpT == "ref") {
                return parseTExpr1<AST::Ref>(this, tmpT);
            } else if (tmpT == "deref") {
                return parseTExpr1<AST::Deref>(this, tmpT);
            } else if (tmpT == "set") {
                return parseTExpr2<AST::Set>(this, tmpT);
            } else if (tmpT == "setptr") {
                return parseTExpr2<AST::SetPtr>(this, tmpT);
            } else if (tmpT == "var") {
                // eat up 'var'
                tmpT = lexer.nextT();

                if (tmpT != Lexer::TT_ID)
                    Error::parserExpected(U"identifier", tmpT.val, lexer.pos());
                auto id = tmpT.val;

                // eat up id
                tmpT = lexer.nextT();

                auto val = parseExpr(tmpT);

                // eat up remaining token
                tmpT = lexer.nextT();

                return new AST::Var(val, std::to_string(id));
            } else if (tmpT == "let") {
                // eat up 'def'
                tmpT = lexer.nextT();

                // error if token is not of type identifier
                if (tmpT != Lexer::TT_ID)
                    Error::parserExpected(U"identifer", tmpT.val);
                
                auto id = tmpT.val;

                // eat up identifier
                tmpT = lexer.nextT();

                auto expr = parseExpr(tmpT);

                // eat up remaining token
                tmpT = lexer.nextT();

                return new AST::Let(expr, std::to_string(id));
            } else if (tmpT == "heget") {
                // eat up 'heget'
                tmpT = lexer.nextT();

                auto t = parseType(tmpT);
                if (!t)
                    Error::parserExpected(U"data type", tmpT.val, lexer.pos());

                auto ptr = parseExpr(tmpT);

                // eat up remaining token
                tmpT = lexer.nextT();

                auto idx = parseExpr(tmpT);

                // eat up remaining token
                tmpT = lexer.nextT();

                return new AST::HeGet(t, ptr, idx);
            }
        }

        return parseCall(tmpT);

        // error if '(' isn't followed by a funcall
        Error::parserExpected(U"identifier", tmpT.val, lexer.pos());
        return nullptr;
    } else if (tmpT == Lexer::TT_HASH) {
        return parseHoArray(tmpT);
    } else if (tmpT == Lexer::TT_BRO) {
        return parseHeArray(tmpT);
    } else {
        AST::Expr *tmp = tokenToExpr(tmpT);
        if (!tmp) Error::parserExpected(U"expression", tmpT.val, lexer.pos());
        return tmp;
    }
}

AST::Expr* Parser::parseTopLevelExpr(Lexer::Token& tmpT) {
    if (tmpT == Lexer::TT_PO) {
        tmpT = lexer.nextT();
        if (tmpT == Lexer::TT_EOF)
            Error::parser(U"unexpected end of file");
        else if (tmpT == Lexer::TT_ID) {
            if (tmpT == "defn")
                return parseFunction(tmpT);
            else if (tmpT == "deft") {
                // eat up 'deft'
                tmpT = lexer.nextT();

                // error if token is not of type identifier
                if (tmpT != Lexer::TT_ID)
                    Error::parserExpected(U"identifer", tmpT.val);
                
                auto id = tmpT.val;

                // eat up identifier
                tmpT = lexer.nextT();

                auto type = parseType(tmpT);

                // error if no type was parsed
                if (!type) Error::parserExpected(U"data type", tmpT.val);

                return new AST::Deft(type, std::to_string(id));
            }
            
            Error::parserExpected(U"built-in top-level function call identifier",
                tmpT.val, lexer.pos());
        }
        
        // error if '(' isn't followed by fun or funcall
        Error::parserExpected(U"identifier", tmpT.val, lexer.pos());
    }

    Error::parserExpected(U"(", tmpT.val, lexer.pos());

    return nullptr;
}

AST::Expr* Parser::parseHoArray(Lexer::Token& tmpT) {
    // eat up '#'
    tmpT = lexer.nextT();

    if (tmpT != Lexer::TT_BRO)
        Error::parserExpected(U"'['", tmpT.val, lexer.pos());
    
    // eat up '['
    tmpT = lexer.nextT();

    std::vector<AST::Expr*> exprs;

    while (tmpT != Lexer::TT_EOF && tmpT != Lexer::TT_BRC) {
        exprs.push_back(parseExpr(tmpT));

        // eat up remaining token
        tmpT = lexer.nextT();
    }

    if (tmpT == Lexer::TT_EOF) Error::parser(U"unexpected end of file");

    return new AST::HoArray(exprs);
}

AST::Expr* Parser::parseHeArray(Lexer::Token& tmpT) {
    // eat up '['
    tmpT = lexer.nextT();

    std::vector<AST::Expr*> exprs;

    while (tmpT != Lexer::TT_EOF && tmpT != Lexer::TT_BRC) {
        exprs.push_back(parseExpr(tmpT));

        // eat up remaining token
        tmpT = lexer.nextT();
    }

    if (tmpT == Lexer::TT_EOF) Error::parser(U"unexpected end of file");

    return new AST::HeArray(exprs);
}

AST::Expr* Parser::parseBinExpr(Lexer::Token& tmpT, AST::BinExprType bet) {
    // eat up operator
    tmpT = lexer.nextT();

    std::vector<AST::Expr*> exprs;
    while (tmpT != Lexer::TT_EOF && tmpT != Lexer::TT_PC) {
        exprs.push_back(parseExpr(tmpT));

        // eat up remaining token
        tmpT = lexer.nextT();
    }

    if (tmpT == Lexer::TT_EOF)
        Error::parser(U"unexpected end of file");

    auto unaryOP = bet == AST::BINEXPR_LNOT || bet == AST::BINEXPR_NOT;
    auto isUnaryExpr = exprs.size() == 1
        && ((bet >= AST::BINEXPR_ADD && bet <= AST::BINEXPR_SUB) || unaryOP);

    if (isUnaryExpr)
        return new AST::UExpr(bet, exprs[0]);
    else if (unaryOP && exprs.size() != 1)
        Error::parser(U"too many arguments for unary expression", lexer.pos());
    else if (exprs.size() < 1)
        Error::parser(U"expected at least 2 arguments", lexer.pos());

    auto tmpExpr = new AST::BinExpr(bet, exprs[0], exprs[1]);
    for (size_t i = 2; i < exprs.size(); i++)
        tmpExpr = new AST::BinExpr(bet, tmpExpr, exprs[i]);

    return tmpExpr;
}

AST::Cast* Parser::parseCast(Lexer::Token& tmpT) {
    // eat up 'cast'
    tmpT = lexer.nextT();

    auto type = parseType(tmpT);

    if (!type) Error::parserExpected(U"data type", tmpT.val);

    auto expr = parseExpr(tmpT);

    // eat up remaining token
    tmpT = lexer.nextT();

    return new AST::Cast(type, expr);
}

AST::If* Parser::parseIf(Lexer::Token& tmpT) {
    // eat up 'if'
    tmpT = lexer.nextT();

    auto cond = parseExpr(tmpT);

    // eat up remaining token
    tmpT = lexer.nextT();

    auto exprTrue = parseExpr(tmpT);

    // eat up remaining token
    tmpT = lexer.nextT();

    auto exprFalse = parseExpr(tmpT);

    // eat up remaining token
    tmpT = lexer.nextT();

    return new AST::If(cond, exprTrue, exprFalse);
}

AST::Function* Parser::parseFunction(Lexer::Token& tmpT) {
    // eat up 'defn'
    tmpT = lexer.nextT();

    auto id = tmpT.val;

    auto lambda = parseLambda(tmpT);

    return lambda->toFunc(std::to_string(id));
}

AST::Lambda* Parser::parseLambda(Lexer::Token& tmpT) {
    // eat up 'fn' (or any other previous token)
    tmpT = lexer.nextT();

    bool varArg = false;
    std::vector<std::pair<std::string, AST::Type*>> args;

    auto retType = parseType(tmpT);

    if (!retType) {

        if (tmpT != Lexer::TT_BRO) Error::parserExpected(U"'['", tmpT.val, lexer.pos());

        // eat up '['
        tmpT = lexer.nextT();

        // parse arguments/parameters
        while (tmpT != Lexer::TT_EOF && tmpT != Lexer::TT_BRC) {
            // get argument/parameter type
            auto t = parseType(tmpT);
            if (!t) Error::parserExpected(U"data type", tmpT.val, lexer.pos());

            // get argument/parameter id
            if (tmpT != Lexer::TT_ID)
                Error::parserExpected(U"identifier", tmpT.val, lexer.pos());

            if (Utils::pairVectorKeyExists(args, std::to_string(tmpT.val)))
                Error::parserExpected(U"unique argument identifier", tmpT.val, lexer.pos());

            args.push_back({ std::to_string(tmpT.val), t });

            // eat up identifier
            tmpT = lexer.nextT();
        }

        // eat up ']'
        tmpT = lexer.nextT();

        if (tmpT == Lexer::TT_QUOTE) {
            varArg = true;
            tmpT = lexer.nextT();
        }

        retType = parseType(tmpT);
        if (!retType) Error::parserExpected(U"return type", tmpT.val, lexer.pos());
    }

    // parse body
    std::vector<AST::Expr*> body;
    while (tmpT != Lexer::TT_EOF && tmpT != Lexer::TT_PC) {
        body.push_back(parseExpr(tmpT));

        // eat up remaining token
        tmpT = lexer.nextT();
    }

    if (tmpT == Lexer::TT_EOF) Error::parser(U"unexpected end of file");

    return new AST::Lambda(args, retType, body, varArg);
}

AST::Call* Parser::parseCall(Lexer::Token& tmpT) {
    if (tmpT == "defn")
        Error::parser(U"functions can only be defined at top level", lexer.pos());
    else if (tmpT == "deft")
        Error::parser(U"data types can only be defined at top level", lexer.pos());

    auto callee = parseExpr(tmpT);

    // eat up remaining token
    tmpT = lexer.nextT();

    // parse arguments/parameters
    std::vector<AST::Expr*> args;
    while (tmpT != Lexer::TT_EOF && tmpT != Lexer::TT_PC) {
        args.push_back(parseExpr(tmpT));

        // eat up remaining token
        tmpT = lexer.nextT();
    }

    if (tmpT == Lexer::TT_EOF)
        Error::parser(U"unexpected end of file");

    return new AST::Call(callee, args);
}

std::vector<AST::Expr*> Parser::parse() {
    std::vector<AST::Expr*> result;

    // helper variables
    Lexer::Token tmpT = lexer.nextT();
    std::string tmpStr;
    std::vector<AST::Expr*> tmpExprs;

    while (tmpT != Lexer::TT_EOF) {
        result.push_back(parseTopLevelExpr(tmpT));

        // eat up remaining token
        tmpT = lexer.nextT();
    }

    // reset lexer
    lexer.setIdx(0);
    
    return result;
}
