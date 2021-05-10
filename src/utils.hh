#pragma once
#include <string>
#include <vector>

#include "ast.hh"

bool strEq(const std::string& str, const std::vector<std::string>& eqVals);

enum ErrorType {
    ERROR_DEFAULT,
    ERROR_LEXER,
    ERROR_PARSER,
    ERROR_COMPILER,
};

void error(ErrorType et, const std::string& msg, const std::string& pos = "");
void parseError(const std::string& expected, const std::string& got, const std::string& pos);

std::string readFile(const std::string& filename);

std::string strReplaceAll(std::string str, const std::string& find, const std::string& replace);
std::string unescapeStr(std::string str);

std::string strVectorToStr(const std::vector<std::string>& vector);
std::string exprVectorToStr(const std::vector<Expr*>& vector);
std::string argVectorToStr(const std::vector<std::pair<Type*, std::string>>& vector);
