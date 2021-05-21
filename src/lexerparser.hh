#pragma once
#include <string>
#include <vector>

#include "ast.hh"

enum TokenType {
    TT_ERR,

    TT_PO,      // '('
    TT_PC,      // ')'
    TT_BRO,     // '['
    TT_BRC,     // ']'

    TT_STAR,    // '*'
    TT_HASH,    // '#'
    TT_QUOTE,   // '\''

    TT_ID,      // [.^[0-9]]+
    TT_INT,     // [0-9]+
    TT_FLOAT,   // [0-9]*'.'[0-9]+
    TT_CHAR,    // '\\'.

    TT_STR,

    TT_EOF,     // end of file
};

class Token {
public:
    TokenType tt;
    std::string val;
    Token() : tt(TT_ERR), val(std::string()) {}
    Token(TokenType tt, std::string val) : tt(tt), val(val) {}
};

class Lexer {
private:
    size_t idx;
    std::string text;
public:
    Lexer(const std::string& text) : idx(0), text(text) {}

    inline char getc(size_t idx);

    void setIdx(size_t idx);
    size_t getIdx();

    bool eofReached();
    Token nextT();

    std::string pos();
};

class Parser {
private:
    Lexer lexer;
    Token tmpT;

    Type* parseType();

    Expr* parseExpr();
    Expr* parseTopLevelExpr();

    Expr* parseArrayExpr();
    Expr* parsePtrArrayExpr();
    Expr* parseBinExpr(BinExprType bet);
    Expr* parseCastExpr(Type *t);
    Expr* parseIfExpr();

    Expr* parseFunction();
    Expr* parseCall();
public:
    Parser(const Lexer& lexer) : lexer(lexer) {}
    std::vector<Expr*> parse();
};
