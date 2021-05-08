#include "utils.hh"
#include "lexerparser.hh"
#include "compiler.hh"

#include <iostream>

void printAST(const std::vector<Expr*>& ast) {
    for (auto& expr : ast) std::cout << expr->str() << std::endl;
}

void printUsage(char *argv0) {
    std::cout << "usage: " << argv0 << " <file>" << std::endl;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string text = readFile(argv[1]);
    Lexer lexer(text);
    Parser parser(lexer);

    auto exprs = parser.parse();

    compile(argv[1], exprs);

    for (auto& expr : exprs) expr->~Expr();

    return 0;
}
