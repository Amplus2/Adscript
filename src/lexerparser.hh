#pragma once
#include <string>
#include <vector>

#include "ast.hh"

namespace Adscript {

class Lexer {
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
        std::u32string val;
        Token() : tt(TT_ERR), val(std::u32string()) {}
        //Token(TokenType tt, std::string val)
        //    : tt(tt), val(std::u32string(val.begin(), val.end())) {}
        Token(TokenType tt, std::u32string val) : tt(tt), val(val) {}
    };
private:
    std::u32string text;
    size_t idx = 0, lastIdx = 0;
    Token lastToken;
public:
    Lexer(const std::u32string& text) : text(text) {}

    char getc(size_t idx) {
        if (eofReached()) return -1;
        return text[idx];
    }

    void setIdx(size_t idx) {
        this->idx = idx;
    }

    size_t getIdx() {
        return idx;
    }

    bool eofReached() {
        return idx >= text.size();
    }

    Token back() {
        this->idx = lastIdx;
        return lastToken;
    }

    std::u32string pos() {
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

    Token nextT();
};


class Parser {
public:
    Lexer lexer;

    AST::Type* parseType(Lexer::Token& tmpT);

    AST::Expr* parseExpr(Lexer::Token& tmpT);
    AST::Expr* parseTopLevelExpr(Lexer::Token& tmpT);

    AST::Expr* parseHoArray(Lexer::Token& tmpT);
    AST::Expr* parseHeArray(Lexer::Token& tmpT);
    AST::Expr* parseBinExpr(Lexer::Token& tmpT, AST::BinExprType bet);
    AST::Cast* parseCast(Lexer::Token& tmpT);
    AST::If* parseIf(Lexer::Token& tmpT);

    AST::Function* parseFunction(Lexer::Token& tmpT);
    AST::Lambda* parseLambda(Lexer::Token& tmpT);
    AST::Call* parseCall(Lexer::Token& tmpT);

    Parser(const Lexer& lexer) : lexer(lexer) {}
    std::vector<AST::Expr*> parse();
};


}
