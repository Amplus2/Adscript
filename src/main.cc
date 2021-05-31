#include "utils.hh"
#include "lexerparser.hh"
#include "compiler.hh"

#include <iostream>
#include <unistd.h>

#include <llvm/Support/Host.h>

using namespace Adscript;

int main(int argc, char **argv) {
    if (argc < 2) return Error::printUsage(argv, 1);
    std::string output, target;
    bool exe = false;
    int opt;
    opterr = 0;
    while ((opt = getopt(argc, argv, "eo:t:h")) != -1) {
        switch (opt) {
            case 'e': exe = true; break;
            case 'o': output = optarg; break;
            case 't': target = optarg; break;
            case 'h': return Error::printUsage(argv, 0);
            case '?': return Error::printUsage(argv, 1);
        }
    }

    argc -= optind;
    if (!argc) return Error::printUsage(argv, 1);
    argv += optind;

    if (target == "") target = llvm::sys::getDefaultTargetTriple();

    if (output == "") {
        for (int i = 0; i < argc; i++) {
            std::string input = std::string(argv[i]);
            std::string output = Utils::makeOutputPath(input, exe);
            std::u32string text = Utils::readFile(input);

            Lexer lexer(text);
            Parser parser(lexer);
            auto exprs = parser.parse();

            //printAST(exprs);
            
            Compiler::compile(exprs, exe, output, target);
            
            for (auto& expr : exprs) expr->~Expr();
        }
    } else {
        std::vector<AST::Expr*> exprs;

        for (int i = 0; i < argc; i++) {
            std::u32string text = Utils::readFile(argv[i]);
            Lexer lexer(text);
            Parser parser(lexer);
            auto newexprs = parser.parse();
            exprs.insert(exprs.end(), newexprs.begin(), newexprs.end());
        }

        //printAST(exprs);

        Compiler::compile(exprs, exe, output, target);

        for (auto& expr : exprs) expr->~Expr();
    }
    return 0;
}
