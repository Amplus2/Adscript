#include "utils.hh"
#include "lexerparser.hh"
#include "compiler.hh"

#include <iostream>
#include <unistd.h>
#include <getopt.h>

#include <llvm/Support/Host.h>

using namespace Adscript;

int main(int argc, char **argv) {
    if (argc < 2) return Error::printUsage(argv, 1);

    std::string output, target;
    bool exe = false, emitLLVM = false;
    int opt, idx;
    opterr = 1;
    
    static struct option long_getopt_options[] = {
        {"executable",  no_argument,        nullptr, 'e'},
        {"llvm-ir",     no_argument,        nullptr, 'l'},
        {"output",      required_argument,  nullptr, 'o'},
        {"help",        no_argument,        nullptr, 'h'},
        {"target",      required_argument,  nullptr, 't'},
        {"version",     no_argument,        nullptr, 'v'},
        {nullptr, 0, nullptr, 0},
    };

    const char *shortopts = "el:o:t:h:v";

    while ((opt = getopt_long(argc, argv, shortopts, long_getopt_options, &idx)) != -1) {
        switch (opt) {
            case 'e': exe = true; break;
            case 'h': Error::printUsage(argv, 1);
            case 'l': emitLLVM = true; break;
            case 'o': output = optarg; break;
            case 't': target = optarg; break;
            case 'v': std::cout << "0.4" << std::endl; exit(0);
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
            
            Compiler::compile(exprs, exe, output, target, emitLLVM);
            
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

        Compiler::compile(exprs, exe, output, target, emitLLVM);

        for (auto& expr : exprs) expr->~Expr();
    }
    return 0;
}
