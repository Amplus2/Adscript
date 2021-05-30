#pragma once

#include "ast.hh"

#include <string>
#include <vector>

namespace Adscript {

namespace Error {

enum ErrorType {
    ERROR_DEFAULT,
    ERROR_LEXER,
    ERROR_PARSER,
    ERROR_COMPILER,
};

std::string etToStr(ErrorType et);

void error(ErrorType et, const std::string& msg, const std::string& pos = "");
void def(const std::string& msg, const std::string& pos = "");
void lexer(const std::string& msg, const std::string& pos = "");
void parser(const std::string& msg, const std::string& pos = "");
void compiler(const std::string& msg, const std::string& pos = "");

void lexerEOF();
void parserExpected(const std::string& expected, const std::string& got,
                            const std::string& pos = "");

void warning(const std::string& msg, const std::string& pos = "");

int printUsage(char **argv, int r);

}

namespace Utils {

bool strEq(const std::string& str, const std::vector<std::string>& eqVals);
std::string readFile(const std::string& filename);
std::string strReplaceAll(std::string str, const std::string& find,
                            const std::string& replace);
std::string unescapeStr(const std::string& str);
bool isDigit(char c);
bool isWhitespace(char c);
bool isSpecialChar(char c);
bool isHexChar(char c);

std::string makeOutputPath(const std::string &input, bool exe);

}

namespace AST {

std::string betToStr(BinExprType bet);
std::string strVectorToStr(const std::vector<std::string>& vector);
std::string exprVectorToStr(const std::vector<Expr*>& vector);
std::string argVectorToStr(const std::vector<std::pair<Type*, std::string>>& vector);

void print(const std::vector<Expr*>& ast);

}

namespace Compiler {

llvm::Value* constInt(::Adscript::Compiler::Context& ctx, int64_t val);
llvm::Value* constFP(::Adscript::Compiler::Context& ctx, double val);

bool llvmTypeEq(llvm::Value *v, llvm::Type *t);
std::string llvmTypeStr(llvm::Type *t);

llvm::Value* tryCast(::Adscript::Compiler::Context& ctx, llvm::Value *v, llvm::Type *t);
llvm::Value* cast(::Adscript::Compiler::Context& ctx, llvm::Value *v, llvm::Type *t);
llvm::Value* createLogicalVal(::Adscript::Compiler::Context& ctx, llvm::Value *v);

llvm::AllocaInst* createAlloca(llvm::Function *f, llvm::Type *type);

bool isNumTy(llvm::Type *t);
bool isFunctionTy(llvm::Type *t);

}
}
