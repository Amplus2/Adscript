#include <string>

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
