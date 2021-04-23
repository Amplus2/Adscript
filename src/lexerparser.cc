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
    nextT_start:

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
    
    // var for temporary string storage
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
    } else if (text[idx + 1] == '.') error("expected digit after '.'");

    // handle identifiers
    while (!isWhitespace(text[idx + 1]) && idx + 1 < text.size())
        tmpStr =+ text[++idx];
    return Token(TT_ID, tmpStr);
}

std::vector<Expr*> Parser::parse() {
    return {};
}
