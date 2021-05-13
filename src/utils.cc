#include "utils.hh"

#include <regex>
#include <fstream>
#include <sstream>
#include <iostream>

bool strEq(const std::string& str, const std::vector<std::string>& eqVals) {
    for (auto& val : eqVals) if (!str.compare(val)) return true;
    return false;
}

std::string etToStr(ErrorType et) {
    switch (et) {
    case ERROR_LEXER: return "lexer error";
    case ERROR_PARSER: return "parser error";
    case ERROR_COMPILER: return "compile error";
    default: return "error";
    }
}

void error(ErrorType et, const std::string& msg, const std::string& pos) {
    std::cout << etToStr(et) << ": " << msg;
    if (pos.size() > 0) std::cout << " (before " + pos + ")";
    std::cout << std::endl;
    exit(1);
}

void parseError(const std::string& expected, const std::string& got,
                const std::string& pos) {
    error(ERROR_PARSER, "expected " + expected + ", got '" + got + "'", pos);
}

void lexerEOFError() {
    error(ERROR_LEXER, "unexpected end of file");
}

std::string readFile(const std::string& filename) {
    std::ifstream ifstream(filename);
    if (ifstream.bad())
        error(ERROR_DEFAULT, "cannot read from file '" + filename + "'");
    std::stringstream sstream;
    sstream << ifstream.rdbuf();
    return sstream.str();
}

std::string strReplaceAll(std::string str, const std::string& find,
                            const std::string& replace) {
    std::string::size_type st = 0;
    while ((st = str.find(find, st)) != std::string::npos) {
        str.replace(st, find.size(), replace);
        st += replace.size();
    }
    return str;
}

std::string unescapeStr(std::string str) {
    str = strReplaceAll(str, "\\\"", "\"");
    str = strReplaceAll(str, "\\\\", "\\");
    str = strReplaceAll(str, "\\a", "\a");
    str = strReplaceAll(str, "\\b", "\b");
    str = strReplaceAll(str, "\\f", "\f");
    str = strReplaceAll(str, "\\n", "\n");
    str = strReplaceAll(str, "\\r", "\r");
    str = strReplaceAll(str, "\\t", "\t");
    str = strReplaceAll(str, "\\v", "\v");
    str = strReplaceAll(str, "\\'", "\'");
    str = strReplaceAll(str, "\\?", "\?");
    return str;
}


std::string strVectorToStr(const std::vector<std::string>& vector) {
    if (vector.size() <= 0) return "{ }";
    std::string result = "{ ";
    for (size_t i = 0; i < vector.size() - 1; i++)
        result += vector.at(i) + ", ";
    return result + vector.at(vector.size() - 1) + " }";
}

std::string exprVectorToStr(const std::vector<Expr*>& vector) {
    std::vector<std::string> tmp;
    for (size_t i = 0; i < vector.size(); i++)
        tmp.push_back(vector.at(i)->str());
    return strVectorToStr(tmp);
}

std::string
argVectorToStr(const std::vector<std::pair<Type*, std::string>>& vector) {
    std::vector<std::string> tmp;
    for (size_t i = 0; i < vector.size(); i++)
        tmp.push_back(vector.at(i).second + ": " + vector.at(i).first->str());
    return strVectorToStr(tmp);
}
