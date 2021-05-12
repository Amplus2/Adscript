#include "utils.hh"
#include "lexerparser.hh"

#include <iostream>

static inline bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static inline bool isWhitespace(char c) {
    return (c >= 0 && c <= 32) || c == ',';
}

static inline bool isSpecialChar(char c) {
    return c == '(' || c == ')' || c == '[' || c == ']';
}

bool Lexer::eofReached() {
    return idx >= text.size();
}

inline char Lexer::getc(size_t idx) {
    if (idx >= text.size()) return -1;
    return text[idx];
}

Token Lexer::nextT() {
    // helper variable
    char c;

    // section declaration for goto statement we need later on
    nextT_start:

    // eat up whitespaces
    while (isWhitespace(getc(idx)) && idx < text.size())
        idx += 1;
    
    // handle comments
    if (getc(idx) == ';' && getc(idx + 1) == ';') {
        // eat up ';;'
        idx += 2;

        // eat up until end of line
        while ((c = getc(idx)) != '\n' && c != '\r')
            idx += 1;

        // eat up end of line
        idx += 1;

        // go to the beginning of nextT()
        goto nextT_start;
    }

    // handle end of file
    if (eofReached()) return Token(TT_EOF, "end of file");

    // handle parentheses and brackets
    switch (getc(idx)) {
    case '(':
        idx += 1;
        return Token(TT_PO, "(");
    case ')':
        idx += 1;
        return Token(TT_PC, ")");
    case '[':
        idx += 1;
        return Token(TT_BRO, "[");
    case ']':
        idx += 1;
        return Token(TT_BRC, "]");
    case '~':
        idx += 1;
        return Token(TT_WAVE, "~");
    case '#':
        idx += 1;
        return Token(TT_HASH, "#");
    case '\'':
        idx += 1;
        return Token(TT_QUOTE, "'");
    }
    
    // helper variable for temporary string storage
    std::string tmpStr;

    // handle integers and front part of floats
    if (isDigit(getc(idx))) {
        while (isDigit((c = getc(idx)))) {
            tmpStr += c;
            idx += 1;
        }

        if (c != '.') return Token(TT_INT, tmpStr);
    }
    
    // handle floats
    if (getc(idx) == '.' && isDigit(getc(idx + 1))) {
        // append '.' to tmpStr
        tmpStr += getc(idx++);

        if (eofReached()) lexerEOFError();

        // append all digits after the '.' to tmpStr
        while (isDigit(getc(idx))) tmpStr += getc(idx++);

        return Token(TT_FLOAT, tmpStr);
    } else if ((c = getc(idx)) == '.')
        error(ERROR_LEXER, "expected digit after '.', got '" + std::string(1, c) + "'", pos());

    if (getc(idx) == '"') {
        // eat up '"'
        idx += 1;

        if (eofReached()) lexerEOFError();

        bool lastBS = false;
        while ((c = getc(idx)) != '"' || lastBS) {
            tmpStr += c;
            lastBS = c == '\\';
            idx += 1;

            if (eofReached()) lexerEOFError();
        }

        // eat up '"'
        idx += 1;

        return Token(TT_STR, tmpStr);
    } else if ((c = getc(idx)) == '\\') {
        // eat up '\\'
        idx += 1;

        if (eofReached()) lexerEOFError();

        c = getc(idx);

        // eat up char
        idx += 1;

        return Token(TT_CHAR, std::string(1, c));
    }

    // handle identifiers
    while (!isWhitespace((c = getc(idx))) && !isSpecialChar(c) && idx < text.size())
        tmpStr += getc(idx++);

    return Token(TT_ID, tmpStr);
}

size_t Lexer::getIdx() {
    return idx;
}

void Lexer::setIdx(size_t idx) {
    this->idx = idx;
}

std::string Lexer::pos() {
    if (idx >= text.size()) return "end of file";

    size_t tmpIdx = 0, line = 1, col = 1;

    while (tmpIdx < idx) {
        if (text[tmpIdx + 1] == '\n' || text[tmpIdx + 1] == '\r') {
            col = 1;
            line += 1;
        } else col += 1;
        tmpIdx += 1;
    }

    return std::to_string(line) + ":" + std::to_string(col);
}

Expr* tokenToExpr(Token t) {
    switch (t.tt) {
    case TT_ID:     return new IdExpr(t.val);
    case TT_INT:    return new IntExpr(std::stol(t.val));
    case TT_FLOAT:  return new FloatExpr(std::stod(t.val));
    case TT_CHAR:   return new CharExpr(t.val[0]);
    case TT_STR:    return new StrExpr(unescapeStr(t.val));
    default:        return nullptr;
    }
}

Type* Parser::parseType(Token& tmpT) {
    Type *t = nullptr;

    // general types
    if (strEq(tmpT.val, {"void"}))              t = new PrimType(TYPE_VOID);
    else if (strEq(tmpT.val, {"char", "i8"}))   t = new PrimType(TYPE_I8);
    else if (strEq(tmpT.val, {"i16"}))          t = new PrimType(TYPE_I16);
    else if (strEq(tmpT.val, {"int", "i32"}))   t = new PrimType(TYPE_I32);
    else if (strEq(tmpT.val, {"long", "i64"}))  t = new PrimType(TYPE_I64);
    else if (strEq(tmpT.val, {"float"}))        t = new PrimType(TYPE_FLOAT);
    else if (strEq(tmpT.val, {"double"}))       t = new PrimType(TYPE_DOUBLE);
    else return nullptr;

    Token tmpTmpT = tmpT;
    size_t tmpIdx = lexer.getIdx();
    tmpT = lexer.nextT();

    if (tmpT.tt == TT_WAVE) {
        tmpTmpT = tmpT;
        tmpIdx = lexer.getIdx();

        // eat up '~'
        tmpT = lexer.nextT();
        
        uint8_t quanity = 1;
        while (tmpT.tt == TT_WAVE) {
            tmpTmpT = tmpT;
            tmpIdx = lexer.getIdx();

            // eat up '~' 
            tmpT = lexer.nextT();

            quanity += 1;
        }

        lexer.setIdx(tmpIdx);
        tmpT = tmpTmpT;

        return new PointerType(t, quanity);
    }

    lexer.setIdx(tmpIdx);
    tmpT = tmpTmpT;
    return t;
}


Expr* Parser::parseExpr(Token& tmpT) {
    if (tmpT.tt == TT_PO) {
        tmpT = lexer.nextT();
        if (tmpT.tt == TT_EOF)
            error(ERROR_PARSER, "unexpected end of file");
        else if (tmpT.tt == TT_ID) {
            if (!tmpT.val.compare("+"))             return parseBinExpr(tmpT, BINEXPR_ADD);
            else if (!tmpT.val.compare("-"))        return parseBinExpr(tmpT, BINEXPR_SUB);
            else if (!tmpT.val.compare("*"))        return parseBinExpr(tmpT, BINEXPR_MUL);
            else if (!tmpT.val.compare("/"))        return parseBinExpr(tmpT, BINEXPR_DIV);
            else if (!tmpT.val.compare("%"))        return parseBinExpr(tmpT, BINEXPR_MOD);
            else if (!tmpT.val.compare("|"))        return parseBinExpr(tmpT, BINEXPR_OR);
            else if (!tmpT.val.compare("&"))        return parseBinExpr(tmpT, BINEXPR_AND);
            else if (!tmpT.val.compare("^"))        return parseBinExpr(tmpT, BINEXPR_XOR);
            else if (!tmpT.val.compare("="))        return parseBinExpr(tmpT, BINEXPR_EQ);
            else if (!tmpT.val.compare("<"))        return parseBinExpr(tmpT, BINEXPR_LT);
            else if (!tmpT.val.compare(">"))        return parseBinExpr(tmpT, BINEXPR_GT);
            else if (!tmpT.val.compare("<="))       return parseBinExpr(tmpT, BINEXPR_LTEQ);
            else if (!tmpT.val.compare(">="))       return parseBinExpr(tmpT, BINEXPR_GTEQ);
            else if (!tmpT.val.compare("or"))       return parseBinExpr(tmpT, BINEXPR_LOR);
            else if (!tmpT.val.compare("and"))      return parseBinExpr(tmpT, BINEXPR_LAND);
            else if (!tmpT.val.compare("xor"))      return parseBinExpr(tmpT, BINEXPR_LXOR);
            else if (!tmpT.val.compare("not"))      return parseBinExpr(tmpT, BINEXPR_NOT);
            else if (!tmpT.val.compare("if"))       return parseIfExpr(tmpT);
            else if (!tmpT.val.compare("set"))      return parseSetExpr(tmpT);
            else if (!tmpT.val.compare("elptr"))    return parseElPtrExpr(tmpT);

            Type *t = parseType(tmpT);
            if (t) return parseCastExpr(tmpT, t);

            return parseFunctionCall(tmpT);
        }

        // error if '(' isn't followed by a funcall
        parseError("identifier", tmpT.val, lexer.pos());
        return nullptr;
    } else if (tmpT.tt == TT_HASH) {
        return parseArrayExpr(tmpT);
    } else if (tmpT.tt == TT_BRO) {
        return parsePtrArrayExpr(tmpT);
    } else {
        Expr* tmp = tokenToExpr(tmpT);
        if (!tmp) parseError("primitive expression", tmpT.val, lexer.pos());
        return tmp;
    }
}

Expr* Parser::parseTopLevelExpr(Token& tmpT) {
    if (tmpT.tt == TT_PO) {
        tmpT = lexer.nextT();
        if (tmpT.tt == TT_EOF)
            error(ERROR_PARSER, "unexpected end of file");
        else if (tmpT.tt == TT_ID) {
            std::string tmpStr = tmpT.val;

            if (!tmpT.val.compare("defn"))
                return parseFunction(tmpT);
            
            parseError("built-in top-level function call identifier", tmpT.val, lexer.pos());
        }
        
        // error if '(' isn't followed by fun or funcall
        parseError("identifier", tmpT.val, lexer.pos());
    }
    parseError("top level expression", tmpT.val, lexer.pos());
    return nullptr;
}

Expr* Parser::parseArrayExpr(Token& tmpT) {
    // eat up '#'
    tmpT = lexer.nextT();

    if (tmpT.tt != TT_BRO)
        parseError("'['", tmpT.val, lexer.pos());
    
    // eat up '['
    tmpT = lexer.nextT();

    std::vector<Expr*> exprs;

    while (tmpT.tt != TT_EOF && tmpT.tt != TT_BRC) {
        exprs.push_back(parseExpr(tmpT));

        // eat up remaining token
        tmpT = lexer.nextT();
    }

    if (tmpT.tt == TT_EOF)
        error(ERROR_PARSER, "unexpected end of file");

    return new ArrayExpr(exprs);
}

Expr* Parser::parsePtrArrayExpr(Token& tmpT) {
    // eat up '['
    tmpT = lexer.nextT();

    std::vector<Expr*> exprs;

    while (tmpT.tt != TT_EOF && tmpT.tt != TT_BRC) {
        exprs.push_back(parseExpr(tmpT));

        // eat up remaining token
        tmpT = lexer.nextT();
    }

    if (tmpT.tt == TT_EOF)
        error(ERROR_PARSER, "unexpected end of file");

    return new PtrArrayExpr(exprs);
}

Expr* Parser::parseElPtrExpr(Token& tmpT) {
    // eat up '('
    tmpT = lexer.nextT();

    Expr *ptr = parseExpr(tmpT);

    // eat up remaining token
    tmpT = lexer.nextT();

    Expr *idxExpr = parseExpr(tmpT);

    // eat up remaining token
    tmpT = lexer.nextT();

    return new ElPtrExpr(ptr, idxExpr);
}

Expr* Parser::parseSetExpr(Token& tmpT) {
    // eat up '('
    tmpT = lexer.nextT();

    Expr *ptr = parseExpr(tmpT);

    // eat up remaining token
    tmpT = lexer.nextT();

    Expr *val = parseExpr(tmpT);

    // eat up remaining token
    tmpT = lexer.nextT();

    return new SetExpr(ptr, val);
}

Expr* Parser::parseBinExpr(Token& tmpT, BinExprType bet) {
    // eat up operator
    tmpT = lexer.nextT();

    std::vector<Expr*> exprs;
    while (tmpT.tt != TT_EOF && tmpT.tt != TT_PC) {
        exprs.push_back(parseExpr(tmpT));

        // eat up remaining token
        tmpT = lexer.nextT();
    }

    if (tmpT.tt == TT_EOF)
        error(ERROR_PARSER, "unexpected end of file");

    if (exprs.size() == 1 && ((bet >= BINEXPR_ADD && bet <= BINEXPR_SUB) || bet == BINEXPR_NOT))
        return new UExpr(bet, exprs[0]);
    else if (bet == BINEXPR_NOT && exprs.size() != 1)
        error(ERROR_PARSER, "too many arguments for unary expression", lexer.pos());
    else if (exprs.size() < 1)
        error(ERROR_PARSER, "expected at least 2 arguments", lexer.pos());

    Expr *tmpExpr = new BinExpr(bet, exprs[0], exprs[1]);
    for (size_t i = 2; i < exprs.size(); i++)
        tmpExpr = new BinExpr(bet, tmpExpr, exprs[i]);

    return tmpExpr;
}

Expr* Parser::parseCastExpr(Token& tmpT, Type *t) {
    // eat remaining token
    tmpT = lexer.nextT();

    Expr *expr = parseExpr(tmpT);

    // eat up remaining token
    tmpT = lexer.nextT();

    return new CastExpr(t, expr);
}

Expr* Parser::parseIfExpr(Token& tmpT) {
    // eat up 'if'
    tmpT = lexer.nextT();

    Expr *cond = parseExpr(tmpT);

    // eat up remaining token
    tmpT = lexer.nextT();

    Expr *exprTrue = parseExpr(tmpT);

    // eat up remaining token
    tmpT = lexer.nextT();

    Expr *exprFalse = parseExpr(tmpT);

    // eat up remaining token
    tmpT = lexer.nextT();

    return new IfExpr(cond, exprTrue, exprFalse);
}

Expr* Parser::parseFunction(Token& tmpT) {
    // eat up 'defn'
    tmpT = lexer.nextT();

    std::string id = tmpT.val;

    // eat up identifier
    tmpT = lexer.nextT();

    if (tmpT.tt != TT_BRO) parseError("'['", tmpT.val, lexer.pos());

    // eat up '['
    tmpT = lexer.nextT();

    // parse arguments/parameters
    std::vector<std::pair<Type*, std::string>> args;
    while (tmpT.tt != TT_EOF && tmpT.tt != TT_BRC) {
        // get argument/parameter type
        Type *t = parseType(tmpT);
        if (!t) parseError("data type", tmpT.val, lexer.pos());

        // eat up data type
        tmpT = lexer.nextT();

        // get argument/parameter id
        if (tmpT.tt != TT_ID) parseError("identifier", tmpT.val, lexer.pos());

        args.push_back(std::pair<Type*, std::string>(t, tmpT.val));

        // eat up identifier
        tmpT = lexer.nextT();
    }

    if (tmpT.tt == TT_EOF)
        error(ERROR_PARSER, "unexpected end of file");

    // eat up ']'
    tmpT = lexer.nextT();

    Type *retType = parseType(tmpT);
    if (!retType) parseError("return type", tmpT.val, lexer.pos());

    // eat up return type
    tmpT = lexer.nextT();

    // parse body
    std::vector<Expr*> body;
    while (tmpT.tt != TT_EOF && tmpT.tt != TT_PC) {
        body.push_back(parseExpr(tmpT));

        // eat up remaining token
        tmpT = lexer.nextT();
    }

    if (tmpT.tt == TT_EOF)
        error(ERROR_PARSER, "unexpected end of file");

    return new Function(id, args, retType, body);
}

Expr* Parser::parseFunctionCall(Token& tmpT) {
    if (!tmpT.val.compare("defn"))
        error(ERROR_PARSER, "functions can only be defined at the top level", lexer.pos());

    std::string calleeId = tmpT.val;

    // eat up identifier
    tmpT = lexer.nextT();

    // parse arguments/parameters
    std::vector<Expr*> args;
    while (tmpT.tt != TT_EOF && tmpT.tt != TT_PC) {
        args.push_back(parseExpr(tmpT));

        // eat up remaining token
        tmpT = lexer.nextT();
    }

    if (tmpT.tt == TT_EOF)
        error(ERROR_PARSER, "unexpected end of file");

    return new FunctionCall(calleeId, args);
}

std::vector<Expr*> Parser::parse() {
    std::vector<Expr*> result;

    // helper variables
    Token tmpT = lexer.nextT();
    std::string tmpStr;
    std::vector<Expr*> tmpExprs;

    while (tmpT.tt != TT_EOF) {
        result.push_back(parseTopLevelExpr(tmpT));

        // eat up remaining token
        tmpT = lexer.nextT();
    }

    // reset lexer
    lexer.setIdx(0);
    
    return result;
}
