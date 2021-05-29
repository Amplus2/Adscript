#pragma once
#include <string>
#include <vector>

#include "ast.hh"

namespace Adscript {

class Lexer {
private:
    size_t idx;
    std::string text;
public:
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
        TT_HEX,     // "0x"[0-9A-Fa-f]+
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

    Lexer(const std::string& text) : idx(0), text(text) {}

    char getc(size_t idx);

    void setIdx(size_t idx);
    size_t getIdx();

    bool eofReached();
    Token nextT();

    std::string pos();
};


class Parser {
private:
    Lexer lexer;

    AST::Type* parseType(Lexer::Token& tmpT);

    AST::Expr* parseExpr(Lexer::Token& tmpT);
    AST::Expr* parseTopLevelExpr(Lexer::Token& tmpT);

    AST::Expr* parseHoArray(Lexer::Token& tmpT);
    AST::Expr* parseHeArray(Lexer::Token& tmpT);
    AST::Expr* parseBinExpr(Lexer::Token& tmpT, AST::BinExprType bet);
    AST::Cast* parseCast(Lexer::Token& tmpT, AST::Type *t);
    AST::If* parseIf(Lexer::Token& tmpT);

    AST::Function* parseFunction(Lexer::Token& tmpT);
    AST::Lambda* parseLambda(Lexer::Token& tmpT);
    AST::Call* parseCall(Lexer::Token& tmpT);
public:
    Parser(const Lexer& lexer) : lexer(lexer) {}
    std::vector<AST::Expr*> parse();
};


}
