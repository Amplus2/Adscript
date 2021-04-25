#include <string>
#include <vector>

#include "ast.hh"

enum TokenType {
    TT_ERR,

    TT_PO,      // '('
    TT_PC,      // ')'
    TT_BRO,     // '['
    TT_BRC,     // ']'

    TT_ID,      // [.^[0-9]]+
    TT_INT,     // [0-9]+
    TT_FLOAT,   // [0-9]*'.'[0-9]+
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
    Token nextT();
    void reset();
};

class Parser {
private:
    Lexer lexer;

    Expr* parseExpr(Token& tmpT);
    Expr* parseTopLevelExpr(Token& tmpT);
    Expr* parseBinExpr(Token& tmpT, BinExprType bet);
    Expr* parseFunction(Token& tmpT);
    Expr* parseFunctionCall(Token& tmpT);
public:
    Parser(const Lexer& lexer) : lexer(lexer) {}
    std::vector<Expr*> parse();
};
