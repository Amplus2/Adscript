#include <string>
#include <vector>

#include "ast.hh"

enum TokenType {
    // TODO: define token types
};

class Lexer {
private:
    size_t idx;
    std::string text;
public:
    Lexer(const std::string& text) : idx(0), text(text) {}
    TokenType nextTT();
};

class Parser {
private:
    TokenType cacheTT;
public:
    std::vector<Expr*> parse();
};
