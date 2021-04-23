#include "utils.hh"

#include <fstream>
#include <sstream>
#include <iostream>


std::string etToStr(ErrorType et) {
    switch (et) {
    case ERROR_LEXER: return "lexer error";
    case ERROR_PARSER: return "parser error";
    case ERROR_COMPILER: return "compile error";
    default: return "error";
    }
}

void error(ErrorType et, const std::string& msg) {
    std::cout << etToStr(et) << ": " << msg << std::endl;
}

void printUsage() {
    std::cout << "usage: adscript <file>" << std::endl;
}

std::string readFile(const std::string& filename) {
    std::ifstream ifstream(filename);
    if (ifstream.bad())
        error(ERROR_DEFAULT, "cannot read from file '" + filename + "'");
    std::stringstream sstream;
    sstream << ifstream.rdbuf();
    return sstream.str();
}
