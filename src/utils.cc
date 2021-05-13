#include "utils.hh"
#include "compiler.hh"

#include <regex>
#include <fstream>
#include <sstream>
#include <iostream>

bool strEq(const std::string& str, const std::vector<std::string>& eqVals) {
    for (auto& val : eqVals) if (!str.compare(val)) return true;
    return false;
}

std::string betToStr(BinExprType bet) {
    switch (bet) {
    case BINEXPR_ADD:   return "+";
    case BINEXPR_SUB:   return "-";
    case BINEXPR_MUL:   return "*";
    case BINEXPR_DIV:   return "/";
    case BINEXPR_MOD:   return "%";

    case BINEXPR_OR:    return "|";
    case BINEXPR_AND:   return "&";
    case BINEXPR_XOR:   return "^";

    case BINEXPR_EQ:    return "=";
    case BINEXPR_LT:    return "<";
    case BINEXPR_GT:    return ">";
    case BINEXPR_LTEQ:  return "<=";
    case BINEXPR_GTEQ:  return ">=";
    case BINEXPR_LOR:   return "or";
    case BINEXPR_LAND:  return "and";
    case BINEXPR_LXOR:  return "xor";
    case BINEXPR_NOT:   return "not";
    }
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

llvm::Value* constInt(CompileContext& ctx, int64_t val) {
    return llvm::ConstantInt::get(
        llvm::IntegerType::getInt64Ty(ctx.mod->getContext()), val);
}

llvm::Value* constFP(CompileContext& ctx, double val) {
    return llvm::ConstantFP::get(
        llvm::IntegerType::getDoubleTy(ctx.mod->getContext()), val);
}

bool llvmTypeEq(llvm::Value *v, llvm::Type *t) {
    return v->getType()->getPointerTo() == t->getPointerTo();
}

std::string llvmTypeStr(llvm::Type *t) {
    std::string _s;
    llvm::raw_string_ostream s(_s);
    t->print(s);
    return s.str();
}

llvm::Value* tryCast(CompileContext& ctx, llvm::Value *v, llvm::Type *t) {
    if (v->getType()->getPointerTo() == t->getPointerTo()) return v;

    if (v->getType()->isIntegerTy()) {
        if (t->isIntegerTy())
            return ctx.builder->CreateIntCast(v, t, true);
        else if (t->isFloatingPointTy())
            return ctx.builder->CreateSIToFP(v, t);
        else if (t->isPointerTy())
            return ctx.builder->CreateIntToPtr(v, t);
    } else if (v->getType()->isFloatingPointTy()) {
        if (t->isIntegerTy())
            return ctx.builder->CreateFPToSI(v, t);
        else if (t->isFloatingPointTy())
            return ctx.builder->CreateFPCast(v, t);
    } else if (v->getType()->isPointerTy()) {
        if (t->isIntegerTy())
            return ctx.builder->CreatePtrToInt(v, t);
        else if (t->isPointerTy())
            return ctx.builder->CreatePointerCast(v, t);
    } else if (v->getType()->isArrayTy()) {
        if (t->isPointerTy())
            return ctx.builder->CreateGEP(v, 
                {constInt(ctx, 0), constInt(ctx, 0)});
    }

    return nullptr;
}

llvm::Value* cast(CompileContext& ctx, llvm::Value *v, llvm::Type *t) {
    auto v1 = tryCast(ctx, v, t);
    if (!v1) error(ERROR_COMPILER, "unable to create cast from '"
                + llvmTypeStr(v->getType()) + "' to '" + llvmTypeStr(t) + "'");
    return v1;
}

llvm::Value* createLogicalVal(CompileContext& ctx, llvm::Value *v) {
    if (llvmTypeEq(v, llvm::Type::getInt1Ty(ctx.mod->getContext())))
        return v;
    else if (v->getType()->isIntegerTy())
        return ctx.builder->CreateICmpNE(v, 
            cast(ctx, constInt(ctx, 0), v->getType()));
    else if (v->getType()->isFloatingPointTy())
        return ctx.builder->CreateFCmpUNE(v, 
            cast(ctx, constFP(ctx, 0), v->getType()));
    else if (v->getType()->isPointerTy())
        return ctx.builder->CreateICmpNE(
            cast(ctx, v, llvm::Type::getInt32Ty(ctx.mod->getContext())),
            constInt(ctx, 0)
        );

    error(ERROR_COMPILER, "unable to create logical value");

    return nullptr;
}

llvm::AllocaInst* createAlloca(llvm::Function *f, llvm::Type *type) {
    llvm::IRBuilder<> builder(
        &(f->getEntryBlock()), f->getEntryBlock().begin());
    return builder.CreateAlloca(type);
}
