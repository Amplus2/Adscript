#pragma once

#include "ast.hh"

#include <string>
#include <vector>


std::ostream& operator << (std::ostream& os, const std::u32string& s);
std::u32string operator + (std::u32string& us, const std::string& s);
std::u32string operator + (const std::u32string& us, const char *s);

namespace std {

long stol(u32string str, size_t *idx = nullptr, long base = 10);
double stod(u32string str);
u32string stou32(string str);
string to_string(u32string str);

}

namespace Adscript {
namespace Error {

enum ErrorType {
    ERROR_DEFAULT,
    ERROR_LEXER,
    ERROR_PARSER,
    ERROR_COMPILER,
};

std::string etToStr(ErrorType et);

void error(ErrorType et, const std::u32string& msg, const std::u32string& pos = U"");
void def(const std::u32string& msg, const std::u32string& pos = U"");
void lexer(const std::u32string& msg, const std::u32string& pos = U"");
void parser(const std::u32string& msg, const std::u32string& pos = U"");
void compiler(const std::u32string& msg, const std::u32string& pos = U"");

void lexerEOF();
void parserExpected(const std::u32string& expected, const std::u32string& got, const std::u32string& pos = U"");

void warning(const std::u32string& msg, const std::u32string& pos = U"");

int printUsage(char **argv, int r);

}

namespace Utils {

bool strEq(const std::u32string& str, const std::vector<std::u32string>& eqVals);
std::u32string readFile(const std::string& filename);
std::u32string strReplaceAll(std::u32string str, const std::u32string& find,
                            const std::u32string& replace);
std::u32string unescapeStr(const std::u32string& str);

bool isDigit(char c);
bool isWhitespace(char c);
bool isSpecialChar(char c);
bool isHexChar(char c);

bool isAscii(const std::u32string& str);

std::string makeOutputPath(const std::string &input, bool exe);

}

namespace AST {

std::u32string betToStr(BinExprType bet);
std::u32string strVectorToStr(const std::vector<std::u32string>& vector);
std::u32string exprVectorToStr(const std::vector<Expr*>& vector);
std::u32string argVectorToStr(const std::vector<std::pair<Type*, std::string>>& vector);
std::u32string attrMapToStr(const std::map<std::string, Type*>& map);

void print(const std::vector<Expr*>& ast);

}

namespace Compiler {

llvm::Value* constInt(::Adscript::Compiler::Context& ctx, int64_t val);
llvm::Value* constFP(::Adscript::Compiler::Context& ctx, double val);

bool llvmTypeEq(llvm::Value *v, llvm::Type *t);
std::u32string llvmTypeStr(llvm::Type *t);

llvm::Value* tryCast(::Adscript::Compiler::Context& ctx, llvm::Value *v, llvm::Type *t);
llvm::Value* cast(::Adscript::Compiler::Context& ctx, llvm::Value *v, llvm::Type *t);
llvm::Value* createLogicalVal(::Adscript::Compiler::Context& ctx, llvm::Value *v);

llvm::AllocaInst* createAlloca(llvm::Function *f, llvm::Type *type);

bool isNumTy(llvm::Type *t);
bool isFunctionTy(llvm::Type *t);

}
}
