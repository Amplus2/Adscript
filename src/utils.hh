#pragma once

#include "ast.hh"

#include <string>
#include <vector>

// errors and warnings
enum ErrorType {
    ERROR_DEFAULT,
    ERROR_LEXER,
    ERROR_PARSER,
    ERROR_COMPILER,
};

std::string etToStr(ErrorType et);

void error(ErrorType et, const std::string& msg, const std::string& pos = "");
void parseError(const std::string& expected, const std::string& got,
                const std::string& pos);
void lexerEOFError();

void warning(const std::string& msg, const std::string& pos = "");

// other stuff
bool strEq(const std::string& str, const std::vector<std::string>& eqVals);

std::string readFile(const std::string& filename);

std::string strReplaceAll(std::string str, const std::string& find,
                            const std::string& replace);
std::string unescapeStr(std::string str);

// AST string helper functions

std::string betToStr(BinExprType bet);

std::string strVectorToStr(const std::vector<std::string>& vector);
std::string exprVectorToStr(const std::vector<Expr*>& vector);
std::string
argVectorToStr(const std::vector<std::pair<Type*, std::string>>& vector);

// compiler helper functions

llvm::Value* constInt(CompileContext& ctx, int64_t val);
llvm::Value* constFP(CompileContext& ctx, double val);

bool llvmTypeEq(llvm::Value *v, llvm::Type *t);
std::string llvmTypeStr(llvm::Type *t);

llvm::Value* tryCast(CompileContext& ctx, llvm::Value *v, llvm::Type *t);
llvm::Value* cast(CompileContext& ctx, llvm::Value *v, llvm::Type *t);
llvm::Value* createLogicalVal(CompileContext& ctx, llvm::Value *v);

llvm::AllocaInst* createAlloca(llvm::Function *f, llvm::Type *type);

bool isNumTy(llvm::Type *t);
