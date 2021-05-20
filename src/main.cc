#include "utils.hh"
#include "lexerparser.hh"
#include "compiler.hh"

#include <iostream>
#include <unistd.h>

void printAST(const std::vector<Expr*>& ast) {
    for (auto& expr : ast) std::cout << expr->str() << std::endl;
}

inline int printUsage(char **argv, int r) {
    std::cout << "usage: " << argv[0] << " [-eh] [-o <file>] <file>" << std::endl;
    return r;
}

int main(int argc, char **argv) {
    if (argc < 2) return printUsage(argv, 1);
    std::string output;
    bool exe = false;
    int opt;
    opterr = 0;
    while ((opt = getopt(argc, argv, "eo:h")) != -1) {
        switch (opt) {
            case 'e': exe = true; break;
            case 'o': output = optarg; break;
            case 'h': return printUsage(argv, 0);
            case '?': return printUsage(argv, 1);
        }
    }

    argc -= optind;
    if (!argc) return printUsage(argv, 1);
    argv += optind;

    if (output == "") {
        for (int i = 0; i < argc; i++) {
            std::string text = readFile(argv[i]);
            Lexer lexer(text);
            Parser parser(lexer);
            auto exprs = parser.parse();
            // TODO: remove ".adscript" at the end, add ".o" if exe = false
            compile(exprs, exe, std::string(argv[i]) + ".out");
            for (auto& expr : exprs) expr->~Expr();
        }
    } else {
        std::vector<Expr*> exprs;

        for (int i = 0; i < argc; i++) {
            std::string text = readFile(argv[i]);
            Lexer lexer(text);
            Parser parser(lexer);
            auto newexprs = parser.parse();
            exprs.insert(exprs.end(), newexprs.begin(), newexprs.end());
        }

        compile(exprs, exe, output);

        for (auto& expr : exprs) expr->~Expr();
    }
    return 0;
}
