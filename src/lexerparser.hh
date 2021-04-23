#include <string>
#include <vector>

#include "ast.hh"

enum TokenType {
    TT_PO,
    TT_PC,
    TT_ID,
    TT_INT,
    TT_FLOAT,
    TT_EOF,
    
    TT_ERR,
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
};

class Parser {
private:
    Lexer lexer;
public:
    Parser(const Lexer& lexer) : lexer(lexer) {}
    std::vector<Expr*> parse();
};
