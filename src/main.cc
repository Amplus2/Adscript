#include "utils.hh"
#include "lexerparser.hh"
#include "compiler.hh"

#include <iostream>
#include <unistd.h>

void printAST(const std::vector<Expr*>& ast) {
    for (auto& expr : ast) std::cout << expr->str() << std::endl;
}

inline int printUsage(char **argv, int r) {
    std::cout << "usage: " << argv[0] << " [-eh] [-o <file>] <files>" << std::endl;
    return r;
}

std::string makeOutputPath(const std::string &input, bool exe) {
    auto lastdot = input.find_last_of('.');
    if (lastdot == std::string::npos)
        error(ERROR_DEFAULT, "Input files have to have an extension.");
    auto mod = input.substr(0, lastdot);
    return exe ? mod : mod + ".o";
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
            std::string input = std::string(argv[i]);
            std::string output = makeOutputPath(input, exe);
            std::string text = readFile(input);
            Lexer lexer(text);
            Parser parser(lexer);
            auto exprs = parser.parse();
            compile(exprs, exe, output);
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
