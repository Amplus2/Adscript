#include "utils.hh"
#include "compiler.hh"

#include <locale>
#include <codecvt>
#include <fstream>
#include <sstream>
#include <iostream>

std::ostream& operator << (std::ostream& os, const std::u32string& s) {
    return (os << std::to_string(s));
}

std::u32string operator + (const std::u32string& us, const std::string& s) {
    return us + std::stou32(s);
}

std::u32string operator + (const std::u32string& us, const char *s) {
    return us + std::stou32(s);
}

long std::stol(std::u32string str, size_t *idx, long base) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
    return stol(cv.to_bytes(str), idx, base);
}

double std::stod(std::u32string str) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
    return stod(cv.to_bytes(str));
}

std::u32string std::stou32(std::string str) {
    return std::u32string(str.begin(), str.end());
    // std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
    // return cv.from_bytes(str);
}

std::string std::to_string(std::u32string str) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
    return cv.to_bytes(str);
}

using namespace Adscript;

std::string Error::etToStr(ErrorType et) {
    switch (et) {
    case ERROR_LEXER: return "lexer error";
    case ERROR_PARSER: return "parser error";
    case ERROR_COMPILER: return "compile error";
    default: return "error";
    }
}

void Error::error(ErrorType et, const std::u32string& msg, const std::u32string& pos) {
    std::cout << "\x1B[91m\x1B[1m" << etToStr(et) << ":\x1B[0m " << msg;
    if (pos.size() > 0) std::cout << U" (before " + pos + U")";
    std::cout << std::endl;
    exit(1);
}

void Error::def(const std::u32string& msg, const std::u32string& pos) {
    Error::error(Error::ERROR_DEFAULT, msg, pos);
}

void Error::lexer(const std::u32string& msg, const std::u32string& pos) {
    Error::error(Error::ERROR_LEXER, msg, pos);
}

void Error::parser(const std::u32string& msg, const std::u32string& pos) {
    Error::error(Error::ERROR_PARSER, msg, pos);
}

void Error::compiler(const std::u32string& msg, const std::u32string& pos) {
    Error::error(Error::ERROR_COMPILER, msg, pos);
}

void Error::parserExpected(const std::u32string& expected, const std::u32string& got, const std::u32string& pos) {
    Error::parser(U"expected " + expected + U", got '" + got + U"'", pos);
}

void Error::lexerEOF() {
    Error::lexer(U"unexpected end of file");
}

void Error::warning(const std::u32string& msg, const std::u32string& pos) {
    std::cout << "\x1B[95m\x1B[1m" << "warning:\x1B[0m " << msg;
    if (pos.size() > 0) std::cout << U" (before " + pos + U")";
    std::cout << std::endl;
}

int Error::printUsage(char **argv, int r) {
    std::cout << "usage: " << argv[0] << " [-eh] [-o <file>] [-t <target-triple>] <files>" << std::endl;
    return r;
}


bool Utils::strEq(const std::u32string& str, const std::vector<std::u32string>& eqVals) {
    for (auto& val : eqVals) if (!str.compare(val)) return true;
    return false;
}

std::u32string Utils::readFile(const std::string& filename) {
    std::ifstream ifstream(filename);
    if (ifstream.bad())
        Error::error(Error::ERROR_DEFAULT,
            U"cannot read from file '" + std::stou32(filename) + U"'");
    ifstream.imbue(std::locale(std::locale::classic(), new std::codecvt_utf8<char32_t>));
    std::stringstream sstream;
    sstream << ifstream.rdbuf();
    return std::stou32(sstream.str());
}

std::u32string Utils::strReplaceAll(std::u32string str, const std::u32string& find, const std::u32string& replace) {
    std::string::size_type st = 0;
    while ((st = str.find(find, st)) != std::string::npos) {
        str.replace(st, find.size(), replace);
        st += replace.size();
    }
    return str;
}

std::u32string Utils::unescapeStr(const std::u32string& s) {
    std::u32string str = s;
    str = Utils::strReplaceAll(str, U"\\\"", U"\"");
    str = Utils::strReplaceAll(str, U"\\\\", U"\\");
    str = Utils::strReplaceAll(str, U"\\a", U"\a");
    str = Utils::strReplaceAll(str, U"\\b", U"\b");
    str = Utils::strReplaceAll(str, U"\\f", U"\f");
    str = Utils::strReplaceAll(str, U"\\n", U"\n");
    str = Utils::strReplaceAll(str, U"\\r", U"\r");
    str = Utils::strReplaceAll(str, U"\\t", U"\t");
    str = Utils::strReplaceAll(str, U"\\v", U"\v");
    str = Utils::strReplaceAll(str, U"\\'", U"\'");
    str = Utils::strReplaceAll(str, U"\\?", U"\?");
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

bool Utils::isAscii(const std::u32string& str) {
    for (auto& c : str)
        if (c < 0 || c > 255)
            return false;
    return true;
}

std::string Utils::makeOutputPath(const std::string &input, bool exe) {
    auto lastdot = input.find_last_of('.');
    if (lastdot == std::string::npos)
        Error::def(U"Input files have to have an extension.");
    auto mod = input.substr(0, lastdot);
    return exe ? mod : mod + ".o";
}

std::u32string AST::betToStr(AST::BinExprType bet) {
    switch (bet) {
    case AST::BINEXPR_ADD:   return U"+";
    case AST::BINEXPR_SUB:   return U"-";
    case AST::BINEXPR_MUL:   return U"*";
    case AST::BINEXPR_DIV:   return U"/";
    case AST::BINEXPR_MOD:   return U"%";

    case AST::BINEXPR_OR:    return U"|";
    case AST::BINEXPR_AND:   return U"&";
    case AST::BINEXPR_XOR:   return U"^";
    case AST::BINEXPR_NOT:   return U"~";

    case AST::BINEXPR_EQ:    return U"=";
    case AST::BINEXPR_LT:    return U"<";
    case AST::BINEXPR_GT:    return U">";
    case AST::BINEXPR_LTEQ:  return U"<=";
    case AST::BINEXPR_GTEQ:  return U">=";
    case AST::BINEXPR_LOR:   return U"or";
    case AST::BINEXPR_LAND:  return U"and";
    case AST::BINEXPR_LXOR:  return U"xor";
    case AST::BINEXPR_LNOT:  return U"not";
    }
}

std::u32string AST::strVectorToStr(const std::vector<std::u32string>& vector) {
    if (vector.size() <= 0) return U"{ }";
    std::u32string result = U"{ ";
    for (size_t i = 0; i < vector.size() - 1; i++)
        result += vector.at(i) + U", ";
    return result + vector.at(vector.size() - 1) + U" }";
}

std::u32string AST::exprVectorToStr(const std::vector<AST::Expr*>& vector) {
    std::vector<std::u32string> tmp;
    for (size_t i = 0; i < vector.size(); i++)
        tmp.push_back(vector.at(i)->str());
    return AST::strVectorToStr(tmp);
}

std::u32string AST::argVectorToStr(const std::vector<std::pair<std::string, AST::Type*>>& vector) {
    std::vector<std::u32string> tmp;
    for (size_t i = 0; i < vector.size(); i++) {
        std::u32string str =
            std::stou32(vector.at(i).first) + U": " + vector.at(i).second->str();
        tmp.push_back(str);
    }
    return AST::strVectorToStr(tmp);
}

std::u32string AST::attrMapToStr(const std::map<std::string, AST::Type*>& map) {
    std::u32string result = U"{ ";
    size_t i = 0;
    for (auto& attr : map) {
        result += std::stou32(attr.first) + U": " + attr.second->str();
        if (i < map.size() - 1) result += U", ";
    }
    return result + U" }";
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

std::u32string Compiler::llvmTypeStr(llvm::Type *t) {
    std::string _s;
    llvm::raw_string_ostream s(_s);
    t->print(s);
    return std::stou32(s.str());
}

llvm::Value* Compiler::tryCast(Compiler::Context& ctx, llvm::Value *v, llvm::Type *t) {
    if (!t) return nullptr;

    llvm::Type *vT = v->getType();
    
    if (vT->getPointerTo() == t->getPointerTo()) return v;

    if (vT->isIntegerTy()) {
        if (t->isIntegerTy()) {
            return ctx.builder->CreateIntCast(v, t, true);
        } else if (t->isFloatingPointTy()) {
            return ctx.builder->CreateSIToFP(v, t);
        } else if (t->isPointerTy()) {
            return ctx.builder->CreateIntToPtr(v, t);
        }
    } else if (vT->isFloatingPointTy()) {
        if (t->isIntegerTy()) {
            return ctx.builder->CreateFPToSI(v, t);
        } else if (t->isFloatingPointTy()) {
            return ctx.builder->CreateFPCast(v, t);
        }
    } else if (vT->isArrayTy()) {
        if (t->isPointerTy()) {
            auto zero = Compiler::constInt(ctx, 0);
            return ctx.builder->CreateGEP(v, { zero, zero });
        }
    } else if (vT->isPointerTy()) {
        if (t->isIntegerTy()) {
            return ctx.builder->CreatePtrToInt(v, t);
        } else if (t->isPointerTy()) {
            return ctx.builder->CreatePointerCast(v, t);
        }
    }

    return nullptr;
}

llvm::Value* Compiler::cast(Compiler::Context& ctx, llvm::Value *v, llvm::Type *t) {
    auto v1 = Compiler::tryCast(ctx, v, t);
    if (!v1) Error::error(Error::ERROR_COMPILER, U"unable to create cast from '"
                + llvmTypeStr(v->getType()) + U"' to '" + llvmTypeStr(t) + U"'");
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

    Error::error(Error::ERROR_COMPILER, U"unable to create logical value");

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

bool Compiler::isFunctionTy(llvm::Type *t) {
    return t->isFunctionTy();
}
