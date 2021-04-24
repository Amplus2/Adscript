#include "ast.hh"
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
    exit(1);
}

void parseError(const std::string& expected, const std::string& got) {
    error(ERROR_PARSER, "expected " + expected + ", got '" + got + "'");
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

std::string strVectorToStr(const std::vector<std::string>& vector) {
    std::string result = "{ ";
    for (size_t i = 0; i < vector.size() - 1; i++)
        result += vector.at(i) + ", ";
    return result + vector.at(vector.size() - 1) + " }";
}

std::string exprVectorToStr(const std::vector<Expr*>& vector) {
    std::vector<std::string> tmp;
    for (size_t i = 0; i < vector.size(); i++)
        tmp.push_back(vector.at(i)->toStr());
    return strVectorToStr(tmp);
}

std::string argVectorToStr(const std::vector<std::pair<TypeAST*, std::string>>& vector) {
    std::vector<std::string> tmp;
    for (size_t i = 0; i < vector.size(); i++)
        tmp.push_back(vector.at(i).second + ": " + vector.at(i).first->toStr());
    return strVectorToStr(tmp);
}
