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
        return Token(TT_EOF, std::string());

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
    if (!s.compare("int")) return TYPE_INT;
    else if (!s.compare("float")) return TYPE_FLOAT;
    return TYPE_ERR;
}

Expr* tokenToExpr(Token t) {
    switch (t.tt) {
    case TT_ID:     return new IdExpr(t.val);
    case TT_INT:    return new IntExpr(std::stoi(t.val));
    case TT_FLOAT:  return new FloatExpr(std::stof(t.val));
    default:        return nullptr;
    }
}


Expr* Parser::parseExpr(Token& tmpT) {
    if (tmpT.tt == TT_PO) {
        tmpT = lexer.nextT();
        if (tmpT.tt == TT_EOF)
            error(ERROR_PARSER, "unexpected end of file");
        else if (tmpT.tt == TT_BRO)
            return parseFunction(tmpT);
        else if (tmpT.tt == TT_ID)
            return parseFunctionCall(tmpT);
        
        // error if '(' isn't followed by fun or funcall
        parseError("identifier or '['", tmpT.val);
        return 0;
    }
    else {
        Expr* tmp = tokenToExpr(tmpT);
        if (!tmp) parseError("primitive expression", tmpT.val);
        return tmp;
    }
}

Expr* Parser::parseFunction(Token& tmpT) {
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

    // parse body
    std::vector<Expr*> body;
    while (tmpT.tt != TT_EOF && tmpT.tt != TT_PC) {
        body.push_back(parseExpr(tmpT));

        // eat up remaining token
        tmpT = lexer.nextT();
    }

    if (tmpT.tt == TT_EOF)
        error(ERROR_PARSER, "unexpected end of file");

    return new Function(args, body);
}

Expr* Parser::parseFunctionCall(Token& tmpT) {
    // error if function call dos not start with id
    if (tmpT.tt != TT_ID) parseError("identifier", tmpT.val);

    // store the calllee's id into temporary variable
    std::string id = tmpT.val;

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

    return new FunctionCall(id, args);
}

std::vector<Expr*> Parser::parse() {
    std::vector<Expr*> result;

    // helper variables
    Token tmpT = lexer.nextT();
    std::string tmpStr;
    std::vector<Expr*> tmpExprs;

    while (tmpT.tt != TT_EOF) {
        result.push_back(parseExpr(tmpT));

        // eat up remaining token
        tmpT = lexer.nextT();
    }

    // reset lexer
    lexer.reset();
    
    return result;
}
