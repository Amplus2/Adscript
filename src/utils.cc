#include "utils.hh"
#include "compiler.hh"

#include <regex>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace Adscript;

std::string Error::etToStr(ErrorType et) {
    switch (et) {
    case ERROR_LEXER: return "lexer error";
    case ERROR_PARSER: return "parser error";
    case ERROR_COMPILER: return "compile error";
    default: return "error";
    }
}

void Error::error(ErrorType et, const std::string& msg, const std::string& pos) {
    std::cout << etToStr(et) << ": " << msg;
    if (pos.size() > 0) std::cout << " (before " + pos + ")";
    std::cout << std::endl;
    exit(1);
}

void Error::def(const std::string& msg, const std::string& pos) {
    Error::error(Error::ERROR_DEFAULT, msg, pos);
}

void Error::lexer(const std::string& msg, const std::string& pos) {
    Error::error(Error::ERROR_LEXER, msg, pos);
}

void Error::parser(const std::string& msg, const std::string& pos) {
    Error::error(Error::ERROR_PARSER, msg, pos);
}

void Error::compiler(const std::string& msg, const std::string& pos) {
    Error::error(Error::ERROR_COMPILER, msg, pos);
}

void Error::parserExpected(const std::string& expected, const std::string& got,
                const std::string& pos) {
    Error::parser("expected " + expected + ", got '" + got + "'", pos);
}

void Error::lexerEOF() {
    error(ERROR_LEXER, "unexpected end of file");
}

void Error::warning(const std::string& msg, const std::string& pos) {
    std::cout << "warning: " << msg;
    if (pos.size() > 0) std::cout << " (before " + pos + ")";
    std::cout << std::endl;
}

int Error::printUsage(char **argv, int r) {
    std::cout << "usage: " << argv[0] << " [-eh] [-o <file>] [-t <target-triple>] <files>" << std::endl;
    return r;
}


bool Utils::strEq(const std::string& str, const std::vector<std::string>& eqVals) {
    for (auto& val : eqVals) if (!str.compare(val)) return true;
    return false;
}

std::string Utils::readFile(const std::string& filename) {
    std::ifstream ifstream(filename);
    if (ifstream.bad())
        Error::error(Error::ERROR_DEFAULT,
            "cannot read from file '" + filename + "'");
    std::stringstream sstream;
    sstream << ifstream.rdbuf();
    return sstream.str();
}

std::string Utils::strReplaceAll(std::string str,const std::string& find, const std::string& replace) {
    std::string::size_type st = 0;
    while ((st = str.find(find, st)) != std::string::npos) {
        str.replace(st, find.size(), replace);
        st += replace.size();
    }
    return str;
}

std::string Utils::unescapeStr(const std::string& s) {
    std::string str = s;
    str = Utils::strReplaceAll(str, "\\\"", "\"");
    str = Utils::strReplaceAll(str, "\\\\", "\\");
    str = Utils::strReplaceAll(str, "\\a", "\a");
    str = Utils::strReplaceAll(str, "\\b", "\b");
    str = Utils::strReplaceAll(str, "\\f", "\f");
    str = Utils::strReplaceAll(str, "\\n", "\n");
    str = Utils::strReplaceAll(str, "\\r", "\r");
    str = Utils::strReplaceAll(str, "\\t", "\t");
    str = Utils::strReplaceAll(str, "\\v", "\v");
    str = Utils::strReplaceAll(str, "\\'", "\'");
    str = Utils::strReplaceAll(str, "\\?", "\?");
    return str;
}

bool Utils::isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool Utils::isWhitespace(char c) {
    return (c >= 0 && c <= 32) || c == ',';
}

bool Utils::isSpecialChar(char c) {
    return c == '('
        || c == ')'
        || c == '['
        || c == ']'
        || c == '#'
        || c == '*';
}

bool Utils::isHexChar(char c) {
    return (c >= '0' && c <= '9')
        || (c >= 'A' && c <= 'F')
        || (c >= 'a' && c <= 'f');
}

std::string Utils::makeOutputPath(const std::string &input, bool exe) {
    auto lastdot = input.find_last_of('.');
    if (lastdot == std::string::npos)
        Error::def("Input files have to have an extension.");
    auto mod = input.substr(0, lastdot);
    return exe ? mod : mod + ".o";
}


std::string AST::betToStr(AST::BinExprType bet) {
    switch (bet) {
    case AST::BINEXPR_ADD:   return "+";
    case AST::BINEXPR_SUB:   return "-";
    case AST::BINEXPR_MUL:   return "*";
    case AST::BINEXPR_DIV:   return "/";
    case AST::BINEXPR_MOD:   return "%";

    case AST::BINEXPR_OR:    return "|";
    case AST::BINEXPR_AND:   return "&";
    case AST::BINEXPR_XOR:   return "^";
    case AST::BINEXPR_NOT:   return "~";

    case AST::BINEXPR_EQ:    return "=";
    case AST::BINEXPR_LT:    return "<";
    case AST::BINEXPR_GT:    return ">";
    case AST::BINEXPR_LTEQ:  return "<=";
    case AST::BINEXPR_GTEQ:  return ">=";
    case AST::BINEXPR_LOR:   return "or";
    case AST::BINEXPR_LAND:  return "and";
    case AST::BINEXPR_LXOR:  return "xor";
    case AST::BINEXPR_LNOT:  return "not";
    }
}

std::string AST::strVectorToStr(const std::vector<std::string>& vector) {
    if (vector.size() <= 0) return "{ }";
    std::string result = "{ ";
    for (size_t i = 0; i < vector.size() - 1; i++)
        result += vector.at(i) + ", ";
    return result + vector.at(vector.size() - 1) + " }";
}

std::string AST::exprVectorToStr(const std::vector<AST::Expr*>& vector) {
    std::vector<std::string> tmp;
    for (size_t i = 0; i < vector.size(); i++)
        tmp.push_back(vector.at(i)->str());
    return AST::strVectorToStr(tmp);
}

std::string AST::argVectorToStr(const std::vector<std::pair<AST::Type*, std::string>>& vector) {
    std::vector<std::string> tmp;
    for (size_t i = 0; i < vector.size(); i++)
        tmp.push_back(vector.at(i).second + ": " + vector.at(i).first->str());
    return AST::strVectorToStr(tmp);
}

void AST::print(const std::vector<AST::Expr*>& ast) {
    for (auto& expr : ast) std::cout << expr->str() << std::endl;
}

llvm::Value* Compiler::constInt(Compiler::Context& ctx, int64_t val) {
    return llvm::ConstantInt::get(
        llvm::IntegerType::getInt64Ty(ctx.mod->getContext()), val);
}

llvm::Value* Compiler::constFP(Compiler::Context& ctx, double val) {
    return llvm::ConstantFP::get(
        llvm::IntegerType::getDoubleTy(ctx.mod->getContext()), val);
}

bool Compiler::llvmTypeEq(llvm::Value *v, llvm::Type *t) {
    return v->getType()->getPointerTo() == t->getPointerTo();
}

std::string Compiler::llvmTypeStr(llvm::Type *t) {
    std::string _s;
    llvm::raw_string_ostream s(_s);
    t->print(s);
    return s.str();
}

llvm::Value* Compiler::tryCast(Compiler::Context& ctx, llvm::Value *v, llvm::Type *t) {
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
                {Compiler::constInt(ctx, 0), Compiler::constInt(ctx, 0)});
    }

    return nullptr;
}

llvm::Value* Compiler::cast(Compiler::Context& ctx, llvm::Value *v, llvm::Type *t) {
    auto v1 = Compiler::tryCast(ctx, v, t);
    if (!v1) Error::error(Error::ERROR_COMPILER, "unable to create cast from '"
                + llvmTypeStr(v->getType()) + "' to '" + llvmTypeStr(t) + "'");
    return v1;
}

llvm::Value* Compiler::createLogicalVal(Compiler::Context& ctx, llvm::Value *v) {
    if (llvmTypeEq(v, llvm::Type::getInt1Ty(ctx.mod->getContext())))
        return v;
    else if (v->getType()->isIntegerTy())
        return ctx.builder->CreateICmpNE(v, 
            cast(ctx, Compiler::constInt(ctx, 0), v->getType()));
    else if (v->getType()->isFloatingPointTy())
        return ctx.builder->CreateFCmpUNE(v, 
            cast(ctx, Compiler::constFP(ctx, 0), v->getType()));
    else if (v->getType()->isPointerTy())
        return ctx.builder->CreateICmpNE(
            cast(ctx, v, llvm::Type::getInt32Ty(ctx.mod->getContext())),
            Compiler::constInt(ctx, 0)
        );

    Error::error(Error::ERROR_COMPILER, "unable to create logical value");

    return nullptr;
}

llvm::AllocaInst* Compiler::createAlloca(llvm::Function *f, llvm::Type *type) {
    llvm::IRBuilder<> builder(
        &(f->getEntryBlock()), f->getEntryBlock().begin());
    return builder.CreateAlloca(type);
}

bool Compiler::isNumTy(llvm::Type *t) {
    return t->isFloatTy()
        || t->isIntegerTy();
}
