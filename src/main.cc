#include "utils.hh"
#include "lexerparser.hh"

#include <iostream>

int main (int argc, char **argv) {

    if (argc != 2) {
        printUsage();
        return 1;
    }

    Parser parser(Lexer(readFile(argv[1])));
    auto exprs = parser.parse();
    return 0;
}
