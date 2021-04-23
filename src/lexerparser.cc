#include "utils.hh"
#include "lexerparser.hh"

#include <iostream>

bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool isWhitespace(char c) {
    return (c >= 0 && c <= 32) || c == ',';
}

Token Lexer::nextT() {
    // section declaration for goto statement we need later on
    nextT_start:

    // skip whitespaces
    while (isWhitespace(text[idx + 1]) && idx + 3 < text.size())
        idx += 1;
    
    // handle comments
    if (text[idx + 1] == ';') {
        idx += 1;
        while (text[++idx] != '\n' && text[++idx] != '\r')
            idx += 1;
        idx += 1;
        goto nextT_start;
    }

    // handle end of file
    if (idx + 2 >= text.size()) {
        idx = 0;
        return Token(TT_EOF, std::string());
    }

    // handle parentheses (some people may call them brackets)
    switch (text[idx + 1]) {
    case '(': return Token(TT_PO, "(");
    case ')': return Token(TT_PO, ")");
    }
    
    // helper variable for temporary string storage
    std::string tmpStr;

    // handle integers and front part of floats
    if (isDigit(text[idx + 1])) {
        idx += 1;
        if (text[idx + 1] != '.') return Token(TT_INT, tmpStr);
    }
    
    // handle floats
    if (text[idx + 1] == '.' && isDigit(text[idx + 2])) {
        idx += 1;
        while (isDigit(text[++idx])) tmpStr += text[idx];
        return Token(TT_FLOAT, tmpStr);
    } else if (text[idx + 1] == '.') error(ERROR_LEXER, "expected digit after '.'");

    // handle identifiers
    while (!isWhitespace(text[idx + 1]) && idx + 1 < text.size())
        tmpStr =+ text[++idx];
    return Token(TT_ID, tmpStr);
}

Expr* tokenToExpr(Token t) {
    switch (t.tt) {
    case TT_ID:     return new IdExpr(t.val);
    case TT_INT:    return new IntExpr(std::stoi(t.val));
    case TT_FLOAT:  return new FloatExpr(std::stof(t.val));
    default:        return nullptr;
    }
}

std::vector<Expr*> Parser::parse() {
    std::vector<Expr*> result;

    // helper variables
    Token tmpT = lexer.nextT();
    std::string tmpStr;
    std::vector<Expr*> tmpExprs;

    while (tmpT.tt != TT_EOF) {
        // handle function calls
        if (tmpT.tt == TT_PO) {
            // eat up '(' and get identifier
            tmpT = lexer.nextT();
            if (tmpT.tt != TT_ID)
                error(ERROR_PARSER, "expected identifier");
            tmpStr = tmpT.val;

            // eat up identifier
            tmpT = lexer.nextT();

            // TODO: Allow functions/lambdas and function calls as parameters 
            // parse the function call's arguments/parameters
            while (tmpT.tt != TT_EOF && tmpT.tt != TT_PC) {
                tmpExprs.push_back(tokenToExpr(tmpT));
                tmpT = lexer.nextT();
            }

            // error if eof is reached before end of function call
            if (tmpT.tt == TT_EOF)
                error(ERROR_PARSER, "unexpected end of file");

            // add function call to parse result
            result.push_back(new FunctionCall(tmpStr, tmpExprs));
            
            // reset helper variables
            tmpStr = "";
            tmpExprs = {};
        }

        // eat up remaining token
        tmpT = lexer.nextT();
    }

    return result;
}
