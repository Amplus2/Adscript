#include "ast.hh"
#include "utils.hh"
#include "lexerparser.hh"

#include <iostream>

using namespace Adscript;

bool Lexer::eofReached() {
    return idx >= text.size();
}

char Lexer::getc(size_t idx) {
    if (idx >= text.size()) return -1;
    return text[idx];
}

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

size_t Lexer::getIdx() {
    return idx;
}

void Lexer::setIdx(size_t idx) {
    this->idx = idx;
}

std::u32string Lexer::pos() {
    if (idx >= text.size()) return U"end of file";

    size_t tmpIdx = 0, line = 1, col = 1;

    while (tmpIdx < idx) {
        if (text[tmpIdx + 1] == '\n' || text[tmpIdx + 1] == '\r') {
            col = 1;
            line += 1;
        } else col += 1;
        tmpIdx += 1;
    }

    return std::stou32(std::to_string(line) + ":" + std::to_string(col));
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
    if (Utils::strEq(tmpT.val, {U"char", U"i8"}))
        t = new AST::PrimType(AST::TYPE_I8);
    else if (!tmpT.val.compare(U"i16"))
        t = new AST::PrimType(AST::TYPE_I16);
    else if (Utils::strEq(tmpT.val, {U"int", U"i32", U"bool"}))
        t = new AST::PrimType(AST::TYPE_I32);
    else if (Utils::strEq(tmpT.val, {U"long", U"i64"}))
        t = new AST::PrimType(AST::TYPE_I64);
    else if (!tmpT.val.compare(U"float"))
        t = new AST::PrimType(AST::TYPE_FLOAT);
    else if (!tmpT.val.compare(U"double"))
        t = new AST::PrimType(AST::TYPE_DOUBLE);
    else return nullptr;

    Lexer::Token tmpTmpT = tmpT;
    size_t tmpIdx = lexer.getIdx();
    tmpT = lexer.nextT();

    if (tmpT.tt == Lexer::TT_STAR) {
        tmpTmpT = tmpT;
        tmpIdx = lexer.getIdx();

        // eat up '*'
        tmpT = lexer.nextT();
        
        uint8_t quanity = 1;
        while (tmpT.tt == Lexer::TT_STAR) {
            tmpTmpT = tmpT;
            tmpIdx = lexer.getIdx();

            // eat up '*' 
            tmpT = lexer.nextT();

            quanity += 1;
        }

        lexer.setIdx(tmpIdx);
        tmpT = tmpTmpT;

        return new AST::PointerType(t, quanity);
    }

    lexer.setIdx(tmpIdx);
    tmpT = tmpTmpT;
    return t;
}


AST::Expr* Parser::parseExpr(Lexer::Token& tmpT) {
    if (tmpT.tt == Lexer::TT_PO) {
        tmpT = lexer.nextT();
        if (tmpT.tt == Lexer::TT_EOF)
            Error::parser(U"unexpected end of file");
        else if (tmpT.tt == Lexer::TT_STAR)
            return parseBinExpr(tmpT, AST::BINEXPR_MUL);
        else if (tmpT.tt == Lexer::TT_ID) {
            if (!tmpT.val.compare(U"+"))
                return parseBinExpr(tmpT, AST::BINEXPR_ADD);
            else if (!tmpT.val.compare(U"-"))
                return parseBinExpr(tmpT, AST::BINEXPR_SUB);
            else if (!tmpT.val.compare(U"/"))
                return parseBinExpr(tmpT, AST::BINEXPR_DIV);
            else if (!tmpT.val.compare(U"%"))
                return parseBinExpr(tmpT, AST::BINEXPR_MOD);
            else if (!tmpT.val.compare(U"|"))
                return parseBinExpr(tmpT, AST::BINEXPR_OR);
            else if (!tmpT.val.compare(U"&"))
                return parseBinExpr(tmpT, AST::BINEXPR_AND);
            else if (!tmpT.val.compare(U"^"))
                return parseBinExpr(tmpT, AST::BINEXPR_XOR);
            else if (!tmpT.val.compare(U"~"))
                return parseBinExpr(tmpT, AST::BINEXPR_NOT);
            else if (!tmpT.val.compare(U"="))
                return parseBinExpr(tmpT, AST::BINEXPR_EQ);
            else if (!tmpT.val.compare(U"<"))
                return parseBinExpr(tmpT, AST::BINEXPR_LT);
            else if (!tmpT.val.compare(U">"))
                return parseBinExpr(tmpT, AST::BINEXPR_GT);
            else if (!tmpT.val.compare(U"<="))
                return parseBinExpr(tmpT, AST::BINEXPR_LTEQ);
            else if (!tmpT.val.compare(U">="))
                return parseBinExpr(tmpT, AST::BINEXPR_GTEQ);
            else if (!tmpT.val.compare(U"or"))
                return parseBinExpr(tmpT, AST::BINEXPR_LOR);
            else if (!tmpT.val.compare(U"and"))
                return parseBinExpr(tmpT, AST::BINEXPR_LAND);
            else if (!tmpT.val.compare(U"xor"))
                return parseBinExpr(tmpT, AST::BINEXPR_LXOR);
            else if (!tmpT.val.compare(U"not"))
                return parseBinExpr(tmpT, AST::BINEXPR_LNOT);
            else if (!tmpT.val.compare(U"if"))
                return parseIf(tmpT);
            else if (!tmpT.val.compare(U"fn"))
                return parseLambda(tmpT);
            else if (!tmpT.val.compare(U"var")) {
                // eat up 'var'
                tmpT = lexer.nextT();

                if (tmpT.tt != Lexer::TT_ID)
                    Error::parserExpected(U"identifier", tmpT.val, lexer.pos());
                auto id = tmpT.val;

                // eat up id
                tmpT = lexer.nextT();

                auto val = parseExpr(tmpT);

                // eat up remaining token
                tmpT = lexer.nextT();

                return new AST::Var(val, std::to_string(id));
            } else if (!tmpT.val.compare(U"set")) {
                // eat up 'set'
                tmpT = lexer.nextT();

                auto ptr = parseExpr(tmpT);

                // eat up remaining token
                tmpT = lexer.nextT();

                auto val = parseExpr(tmpT);

                // eat up remaining token
                tmpT = lexer.nextT();

                return new AST::Set(ptr, val);
            } else if (!tmpT.val.compare(U"setptr")) {
                // eat up 'setptr'
                tmpT = lexer.nextT();

                auto ptr = parseExpr(tmpT);

                // eat up remaining token
                tmpT = lexer.nextT();

                auto val = parseExpr(tmpT);

                // eat up remaining token
                tmpT = lexer.nextT();

                return new AST::SetPtr(ptr, val);
            } else if (!tmpT.val.compare(U"ref")) {
                // eat up 'set'
                tmpT = lexer.nextT();

                auto val = parseExpr(tmpT);

                // eat up remaining token
                tmpT = lexer.nextT();

                return new AST::Ref(val);
            } else if (!tmpT.val.compare(U"deref")) {
                // eat up 'deref'
                tmpT = lexer.nextT();

                auto ptr = parseExpr(tmpT);

                // eat up remaining token
                tmpT = lexer.nextT();

                return new AST::Deref(ptr);
            } else if (!tmpT.val.compare(U"heget")) {
                // eat up 'heget'
                tmpT = lexer.nextT();

                auto t = parseType(tmpT);
                if (!t)
                    Error::parserExpected(U"data type", tmpT.val, lexer.pos());

                // eat up remaining token
                tmpT = lexer.nextT();

                auto ptr = parseExpr(tmpT);

                // eat up remaining token
                tmpT = lexer.nextT();

                auto idx = parseExpr(tmpT);

                // eat up remaining token
                tmpT = lexer.nextT();

                return new AST::HeGet(t, ptr, idx);
            }

            auto t = parseType(tmpT);
            if (t) return parseCast(tmpT, t);
        }

        return parseCall(tmpT);

        // error if '(' isn't followed by a funcall
        Error::parserExpected(U"identifier", tmpT.val, lexer.pos());
        return nullptr;
    } else if (tmpT.tt == Lexer::TT_HASH) {
        return parseHoArray(tmpT);
    } else if (tmpT.tt == Lexer::TT_BRO) {
        return parseHeArray(tmpT);
    } else {
        AST::Expr *tmp = tokenToExpr(tmpT);
        if (!tmp) Error::parserExpected(U"expression", tmpT.val, lexer.pos());
        return tmp;
    }
}

AST::Expr* Parser::parseTopLevelExpr(Lexer::Token& tmpT) {
    if (tmpT.tt == Lexer::TT_PO) {
        tmpT = lexer.nextT();
        if (tmpT.tt == Lexer::TT_EOF)
            Error::parser(U"unexpected end of file");
        else if (tmpT.tt == Lexer::TT_ID) {
            if (!tmpT.val.compare(U"defn"))
                return parseFunction(tmpT);
            
            Error::parserExpected(U"built-in top-level function call identifier",
                tmpT.val, lexer.pos());
        }
        
        // error if '(' isn't followed by fun or funcall
        Error::parserExpected(U"identifier", tmpT.val, lexer.pos());
    }

    Error::parserExpected(U"top level expression", tmpT.val, lexer.pos());

    return nullptr;
}

AST::Expr* Parser::parseHoArray(Lexer::Token& tmpT) {
    // eat up '#'
    tmpT = lexer.nextT();

    if (tmpT.tt != Lexer::TT_BRO)
        Error::parserExpected(U"'['", tmpT.val, lexer.pos());
    
    // eat up '['
    tmpT = lexer.nextT();

    std::vector<AST::Expr*> exprs;

    while (tmpT.tt != Lexer::TT_EOF && tmpT.tt != Lexer::TT_BRC) {
        exprs.push_back(parseExpr(tmpT));

        // eat up remaining token
        tmpT = lexer.nextT();
    }

    if (tmpT.tt == Lexer::TT_EOF) Error::parser(U"unexpected end of file");

    return new AST::HoArray(exprs);
}

AST::Expr* Parser::parseHeArray(Lexer::Token& tmpT) {
    // eat up '['
    tmpT = lexer.nextT();

    std::vector<AST::Expr*> exprs;

    while (tmpT.tt != Lexer::TT_EOF && tmpT.tt != Lexer::TT_BRC) {
        exprs.push_back(parseExpr(tmpT));

        // eat up remaining token
        tmpT = lexer.nextT();
    }

    if (tmpT.tt == Lexer::TT_EOF) Error::parser(U"unexpected end of file");

    return new AST::HeArray(exprs);
}

AST::Expr* Parser::parseBinExpr(Lexer::Token& tmpT, AST::BinExprType bet) {
    // eat up operator
    tmpT = lexer.nextT();

    std::vector<AST::Expr*> exprs;
    while (tmpT.tt != Lexer::TT_EOF && tmpT.tt != Lexer::TT_PC) {
        exprs.push_back(parseExpr(tmpT));

        // eat up remaining token
        tmpT = lexer.nextT();
    }

    if (tmpT.tt == Lexer::TT_EOF)
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

AST::Cast* Parser::parseCast(Lexer::Token& tmpT, AST::Type *t) {
    // eat remaining token
    tmpT = lexer.nextT();

    auto expr = parseExpr(tmpT);

    // eat up remaining token
    tmpT = lexer.nextT();

    return new AST::Cast(t, expr);
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

    std::vector<std::pair<AST::Type*, std::string>> args;

    auto retType = parseType(tmpT);

    if (!retType) {
        if (tmpT.tt != Lexer::TT_BRO) Error::parserExpected(U"'['", tmpT.val, lexer.pos());

        // eat up '['
        tmpT = lexer.nextT();

        // parse arguments/parameters
        while (tmpT.tt != Lexer::TT_EOF && tmpT.tt != Lexer::TT_BRC) {
            // get argument/parameter type
            auto t = parseType(tmpT);
            if (!t) Error::parserExpected(U"data type", tmpT.val, lexer.pos());

            // eat up data type
            tmpT = lexer.nextT();

            // get argument/parameter id
            if (tmpT.tt != Lexer::TT_ID) Error::parserExpected(U"identifier", tmpT.val, lexer.pos());

            args.push_back({ t, std::to_string(tmpT.val) });

            // eat up identifier
            tmpT = lexer.nextT();
        }

        // eat up ']'
        tmpT = lexer.nextT();
    }

    retType = parseType(tmpT);
    if (!retType) Error::parserExpected(U"return type", tmpT.val, lexer.pos());

    // eat up return type
    tmpT = lexer.nextT();

    // parse body
    std::vector<AST::Expr*> body;
    while (tmpT.tt != Lexer::TT_EOF && tmpT.tt != Lexer::TT_PC) {
        body.push_back(parseExpr(tmpT));

        // eat up remaining token
        tmpT = lexer.nextT();
    }

    if (tmpT.tt == Lexer::TT_EOF) Error::parser(U"unexpected end of file");

    return new AST::Lambda(args, retType, body);
}

AST::Call* Parser::parseCall(Lexer::Token& tmpT) {
    if (!tmpT.val.compare(U"defn"))
        Error::parser(U"functions can only be defined at top level", lexer.pos());

    auto callee = parseExpr(tmpT);

    // eat up remaining token
    tmpT = lexer.nextT();

    // parse arguments/parameters
    std::vector<AST::Expr*> args;
    while (tmpT.tt != Lexer::TT_EOF && tmpT.tt != Lexer::TT_PC) {
        args.push_back(parseExpr(tmpT));

        // eat up remaining token
        tmpT = lexer.nextT();
    }

    if (tmpT.tt == Lexer::TT_EOF)
        Error::parser(U"unexpected end of file");

    return new AST::Call(callee, args);
}

std::vector<AST::Expr*> Parser::parse() {
    std::vector<AST::Expr*> result;

    // helper variables
    Lexer::Token tmpT = lexer.nextT();
    std::string tmpStr;
    std::vector<AST::Expr*> tmpExprs;

    while (tmpT.tt != Lexer::TT_EOF) {
        result.push_back(parseTopLevelExpr(tmpT));

        // eat up remaining token
        tmpT = lexer.nextT();
    }

    // reset lexer
    lexer.setIdx(0);
    
    return result;
}
