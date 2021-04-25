#include "utils.hh"
#include "lexerparser.hh"
#include "compiler.hh"

#include <iostream>

void printAST(const std::vector<Expr*>& ast) {
    for (auto& expr : ast) std::cout << expr->toStr() << std::endl;
}

int main (int argc, char **argv) {

    if (argc != 2) {
        printUsage();
        return 1;
    }

    std::string text = readFile(argv[1]);
    Lexer lexer(text);
    Parser parser(lexer);

    auto exprs = parser.parse();

    // printAST(exprs);

    std::cout << std::string("abc").find_last_of('/') << std::endl;

    compile(argv[1], exprs);

    return 0;
}
