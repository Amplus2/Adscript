#include "utils.hh"
#include "lexerparser.hh"

#include <iostream>

bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool isWhitespace(char c) {
    return (c >= 0 && c <= 32) || c == ',';
}

bool isSpecialChar(char c) {
    return c == '(' || c == ')' || c == '[' || c == ']';
}

Token Lexer::nextT() {
    // section declaration for goto statement we need later on
    nextT_start:

    // eat up whitespaces
    while (isWhitespace(text[idx]) && idx < text.size())
        idx += 1;
    
    // handle comments
    if (text[idx] == ';' && text[idx + 1] == ';') {
        // eat up ';;'
        idx += 2;

        // eat up until end of line
        while (text[idx] != '\n' && text[idx] != '\r')
            idx += 1;

        // eat up end of line
        idx += 1;

        // go to the beginning of nextT()
        goto nextT_start;
    }

    // handle end of file
    if (idx >= text.size())
        return Token(TT_EOF, "eof");

    // handle parentheses and brackets
    switch (text[idx]) {
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
    }
    
    // helper variable for temporary string storage
    std::string tmpStr;

    // handle integers and front part of floats
    if (isDigit(text[idx])) {
        while (isDigit(text[idx])) tmpStr += text[idx++];
        if (text[idx] != '.') return Token(TT_INT, tmpStr);
    }
    
    // handle floats
    if (text[idx] == '.' && isDigit(text[idx + 1])) {
        // append '.' to tmpStr
        tmpStr += text[idx++];

        // append all digits after the '.' to tmpStr
        while (isDigit(text[idx])) tmpStr += text[idx++];

        return Token(TT_FLOAT, tmpStr);
    } else if (text[idx] == '.')
        error(ERROR_LEXER, "expected digit after '.', got '" + std::string(1, text[idx]) + "'");

    // handle identifiers
    while (!isWhitespace(text[idx]) && !isSpecialChar(text[idx]) && idx < text.size())
        tmpStr += text[idx++];
    return Token(TT_ID, tmpStr);
}

void Lexer::reset() {
    idx = 0;
}

PrimType strToPT(const std::string& s) {
    // general types
    if (!s.compare("char")
        || !s.compare("i8"))        return TYPE_I8;
    else if (!s.compare("i16"))     return TYPE_I16;
    else if (!s.compare("int")
            || !s.compare("i32"))   return TYPE_I32;
    else if (!s.compare("long")
            || !s.compare("i64"))   return TYPE_I64;
    else if (!s.compare("float"))   return TYPE_FLOAT;
    else if (!s.compare("double"))  return TYPE_FLOAT;
    return TYPE_ERR;
}

Expr* tokenToExpr(Token t) {
    switch (t.tt) {
    case TT_ID:     return new IdExpr(t.val);
    case TT_INT:    return new IntExpr(std::stol(t.val));
    case TT_FLOAT:  return new FloatExpr(std::stod(t.val));
    default:        return nullptr;
    }
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
            else if (!tmpT.val.compare("!="))       return parseBinExpr(tmpT, BINEXPR_NEQ);
            else if (!tmpT.val.compare("<"))        return parseBinExpr(tmpT, BINEXPR_LT);
            else if (!tmpT.val.compare(">"))        return parseBinExpr(tmpT, BINEXPR_GT);
            else if (!tmpT.val.compare("<="))       return parseBinExpr(tmpT, BINEXPR_LTEQ);
            else if (!tmpT.val.compare(">="))       return parseBinExpr(tmpT, BINEXPR_GTEQ);
            else if (!tmpT.val.compare("or"))       return parseBinExpr(tmpT, BINEXPR_LOR);
            else if (!tmpT.val.compare("and"))      return parseBinExpr(tmpT, BINEXPR_LAND);
            else if (!tmpT.val.compare("xor"))      return parseBinExpr(tmpT, BINEXPR_LXOR);
            else if (!tmpT.val.compare("if"))       return parseIfExpr(tmpT);
            else if (!tmpT.val.compare("i8")
                    || !tmpT.val.compare("char"))   return parseCastExpr(tmpT, TYPE_I8);
            else if (!tmpT.val.compare("i16"))      return parseCastExpr(tmpT, TYPE_I16);
            else if (!tmpT.val.compare("i32")
                    || !tmpT.val.compare("int"))    return parseCastExpr(tmpT, TYPE_I32);
            else if (!tmpT.val.compare("i64")
                    || !tmpT.val.compare("long"))   return parseCastExpr(tmpT, TYPE_I64);
            else if (!tmpT.val.compare("float"))    return parseCastExpr(tmpT, TYPE_FLOAT);
            else if (!tmpT.val.compare("double"))   return parseCastExpr(tmpT, TYPE_DOUBLE);

            return parseFunctionCall(tmpT);
        }
        
        // error if '(' isn't followed by a funcall
        parseError("identifier", tmpT.val);
        return nullptr;
    }
    else {
        Expr* tmp = tokenToExpr(tmpT);
        if (!tmp) parseError("primitive expression", tmpT.val);
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
            
            parseError("built-in top-level function call identifier", tmpT.val);
        }
        
        // error if '(' isn't followed by fun or funcall
        parseError("identifier", tmpT.val);
    }
    parseError("top level expression", tmpT.val);
    return nullptr;
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

    if (exprs.size() == 1 && bet >= BINEXPR_ADD && bet <= BINEXPR_SUB)
        return new UExpr(bet, exprs[0]);
    else if (exprs.size() < 1)
        error(ERROR_PARSER, "expected at least 2 arguments");

    Expr *tmpExpr = new BinExpr(bet, exprs[0], exprs[1]);
    for (size_t i = 2; i < exprs.size(); i++)
        tmpExpr = new BinExpr(bet, tmpExpr, exprs[i]);

    return tmpExpr;
}

Expr* Parser::parseCastExpr(Token& tmpT, PrimType pt) {
    // eat up cast type
    tmpT = lexer.nextT();

    Expr *expr = parseExpr(tmpT);

    // eat up remaining token
    tmpT = lexer.nextT();

    return new CastExpr(new PrimTypeAST(pt), expr);
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

    if (tmpT.tt != TT_BRO) parseError("'['", tmpT.val);

    // eat up '['
    tmpT = lexer.nextT();

    // parse arguments/parameters
    std::vector<std::pair<TypeAST*, std::string>> args;
    while (tmpT.tt != TT_EOF && tmpT.tt != TT_BRC) {
        // get argument/parameter type
        PrimType pt = strToPT(tmpT.val);
        if (pt == TYPE_ERR) parseError("data type", tmpT.val);

        // eat up data type
        tmpT = lexer.nextT();

        // get argument/parameter id
        if (tmpT.tt != TT_ID) parseError("identifier", tmpT.val);

        args.push_back(std::pair<TypeAST*, std::string>(new PrimTypeAST(pt), tmpT.val));

        // eat up identifier
        tmpT = lexer.nextT();
    }

    if (tmpT.tt == TT_EOF)
        error(ERROR_PARSER, "unexpected end of file");

    // eat up ']'
    tmpT = lexer.nextT();

    PrimType pt = strToPT(tmpT.val);
    if (pt == TYPE_ERR) parseError("return type", tmpT.val);
    TypeAST *retType = new PrimTypeAST(pt);

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
        error(ERROR_PARSER, "functions can only be defined at the top level");

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
    lexer.reset();
    
    return result;
}
