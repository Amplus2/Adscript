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
    while (isWhitespace(text[idx + 1]) && idx + 3 < text.size())
        idx += 1;

    if (idx + 2 >= text.size()) {
        idx = 0;
        return Token(TT_EOF, std::string());
    }

    switch (text[idx + 1]) {
    case '(': return Token(TT_PO, "(");
    case ')': return Token(TT_PO, ")");
    }
    
    std::string tmpStr;

    if (isDigit(text[idx + 1])) {
        idx += 1;
        if (text[idx + 1] != '.') return Token(TT_INT, tmpStr);
    }
    
    if (text[idx + 1] == '.' && isDigit(text[idx + 2])) {
        idx += 1;
        while (isDigit(text[++idx])) tmpStr += text[idx];
        return Token(TT_FLOAT, tmpStr);
    }
    
    error("unexpected character: '" + std::string(1, text[idx + 1]) + "'");

    return Token(TT_ERR, std::string());
}

std::vector<Expr*> Parser::parse() {
    return {};
}
