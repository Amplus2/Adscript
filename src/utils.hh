#include <string>
#include <vector>

#include "ast.hh"

typedef std::string str;

enum ErrorType {
    ERROR_DEFAULT,
    ERROR_LEXER,
    ERROR_PARSER,
    ERROR_COMPILER,
};

void error(ErrorType et, const std::string& msg);

void printUsage();

std::string readFile(const std::string& filename);

std::string strVectorToStr(const std::vector<std::string>& vector);
std::string exprVectorToStr(const std::vector<Expr*>& vector);
std::string argVectorToStr(const std::vector<std::pair<TypeAST*, std::string>>& vector);
