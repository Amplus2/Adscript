#pragma once

#include <map>
#include <string>
#include <vector>

#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>


namespace Adscript {
namespace Compiler { class Context; }
namespace AST {

enum PT {
    TYPE_ERR,

    TYPE_VOID,
    TYPE_I8,
    TYPE_I16,
    TYPE_I32,
    TYPE_I64,
    TYPE_FLOAT,
    TYPE_DOUBLE,
};

enum BinExprType {
    BINEXPR_ADD,
    BINEXPR_SUB,
    BINEXPR_MUL,
    BINEXPR_DIV,
    BINEXPR_MOD,

    BINEXPR_OR,
    BINEXPR_AND,
    BINEXPR_XOR,
    BINEXPR_NOT,

    BINEXPR_EQ,
    BINEXPR_LT,
    BINEXPR_GT,
    BINEXPR_LTEQ,
    BINEXPR_GTEQ,
    BINEXPR_LOR,
    BINEXPR_LAND,
    BINEXPR_LXOR,
    BINEXPR_LNOT,
};

class Type {
public:
    virtual ~Type() = default;
    virtual std::string str() = 0;
    virtual llvm::Type* llvmType(llvm::LLVMContext &ctx) = 0;
};

class Expr {
public:
    virtual ~Expr() = default;
    virtual std::string str() = 0;
    virtual llvm::Value* llvmValue(::Adscript::Compiler::Context& ctx) = 0;
    virtual bool isLambda() { return false; }
    virtual bool isIdentifier() { return false; }
};

}
}

#include "utils.hh"

namespace Adscript {
namespace AST {

class PrimType : public Type {
private:
    PT type;
public:
    PrimType(PT type) : type(type) {}

    llvm::Type* llvmType(llvm::LLVMContext &ctx) override;
    std::string str() override;
};

class PointerType : public Type {
private:
    Type *type;
    uint8_t quantity;
public:
    PointerType(Type *type) : type(type), quantity(1) {}
    PointerType(Type *type, uint8_t quantity)
        : type(type), quantity(quantity) {}

    llvm::Type* llvmType(llvm::LLVMContext &ctx) override;
    std::string str() override;

    ~PointerType() {
        delete type;
    }
};

class StructType : public Type {
private:
    std::vector<std::pair<Type*, std::string>> attrs;
public:
    StructType(const std::vector<std::pair<Type*, std::string>>& attrs)
        : attrs(attrs) {}

    llvm::Type* llvmType(llvm::LLVMContext &ctx) override;
    std::string str() override;
};

class Int : public Expr {
private:
    const int64_t val;
public:
    Int(const int64_t val) : val(val) {}

    llvm::Value* llvmValue(::Adscript::Compiler::Context& ctx) override;
    std::string str() override;
};

class Float : public Expr {
private:
    const double val;
public:
    Float(const double val) : val(val) {}

    llvm::Value* llvmValue(::Adscript::Compiler::Context& ctx) override;
    std::string str() override;
};

class Char : public Expr {
private:
    const char val;
public:
    Char(const char val) : val(val) {}

    llvm::Value* llvmValue(::Adscript::Compiler::Context& ctx) override;
    std::string str() override;
};

class Identifier : public Expr {
private:
    const std::string val;
public:
    Identifier(const std::string  val) : val(val) {}

    std::string getVal() { return val; };
    llvm::Value* llvmValue(::Adscript::Compiler::Context& ctx) override;
    std::string str() override;
    
    bool isIdentifier() override { return true; }
};

class String : public Expr {
private:
    const std::string val;
public:
    String(const std::string  val) : val(val) {}

    std::string getVal();
    llvm::Value* llvmValue(::Adscript::Compiler::Context& ctx) override;
    std::string str() override;
};

class UExpr : public Expr {
private:
    BinExprType type;
    Expr *expr;
public:
    UExpr(BinExprType type, Expr *expr)
        : type(type), expr(expr) {}

    llvm::Value* llvmValue(::Adscript::Compiler::Context& ctx) override;
    std::string str() override;

    ~UExpr() {
        delete expr;
    }
};

class BinExpr : public Expr {
private:
    BinExprType type;
    Expr *left, *right;
public:
    BinExpr(BinExprType type, Expr *left, Expr *right)
        : type(type), left(left), right(right) {}

    llvm::Value* llvmValue(::Adscript::Compiler::Context& ctx) override;
    std::string str() override;

    ~BinExpr() {
        delete left;
        delete right;
    }
};

class If : public Expr {
private:
    Expr *cond, *exprTrue, *exprFalse;
public:
    If(Expr *cond, Expr *exprTrue, Expr *exprFalse)
        : cond(cond), exprTrue(exprTrue), exprFalse(exprFalse) {}

    llvm::Value* llvmValue(::Adscript::Compiler::Context& ctx) override;
    std::string str() override;

    ~If() {
        delete cond;
        delete exprTrue;
        delete exprFalse;
    }
};

class HoArray : public Expr {
private:
    std::vector<Expr*> exprs;
public:
    HoArray(const std::vector<Expr*>& exprs) : exprs(exprs) {}

    llvm::Value* llvmValue(::Adscript::Compiler::Context& ctx) override;
    std::string str() override;

    ~HoArray() {
        for (auto& expr : exprs)
            delete expr;
    }
};

class HeArray : public Expr {
private:
    std::vector<Expr*> exprs;
public:
    HeArray(const std::vector<Expr*>& exprs) : exprs(exprs) {}

    llvm::Value* llvmValue(::Adscript::Compiler::Context& ctx) override;
    std::string str() override;

    ~HeArray() {
        for (auto& expr : exprs)
            delete expr;
    }
};

class Var : public Expr {
private:
    Expr *val;
    const std::string id;
public:
    Var(Expr *val, const std::string& id) : val(val), id(id) {}

    llvm::Value* llvmValue(::Adscript::Compiler::Context& ctx) override;
    std::string str() override;

    ~Var() {
        delete val;
    }
};

class Set : public Expr {
private:
    Expr *ptr, *val;
public:
    Set(Expr *ptr, Expr *val) : ptr(ptr), val(val) {}

    llvm::Value* llvmValue(::Adscript::Compiler::Context& ctx) override;
    std::string str() override;

    ~Set() {
        delete ptr;
        delete val;
    }
};

class SetPtr : public Expr {
private:
    Expr *ptr, *val;
public:
    SetPtr(Expr *ptr, Expr *val) : ptr(ptr), val(val) {}

    llvm::Value* llvmValue(::Adscript::Compiler::Context& ctx) override;
    std::string str() override;

    ~SetPtr() {
        delete ptr;
        delete val;
    }
};

class Ref : public Expr {
private:
    Expr *val;
public:
    Ref(Expr *val) : val(val) {}

    llvm::Value* llvmValue(::Adscript::Compiler::Context& ctx) override;
    std::string str() override;

    ~Ref() {
        delete val;
    }
};

class Deref : public Expr {
private:
    Expr *ptr;
public:
    Deref(Expr *ptr) : ptr(ptr) {}

    llvm::Value* llvmValue(::Adscript::Compiler::Context& ctx) override;
    std::string str() override;

    ~Deref() {
        delete ptr;
    }
};

class HeGet : public Expr {
private:
    Type *type;
    Expr *ptr, *idx;
public:
    HeGet(Type *type, Expr *ptr, Expr *idx)
        : type(type), ptr(ptr), idx(idx) {}

    llvm::Value* llvmValue(::Adscript::Compiler::Context& ctx) override;
    std::string str() override;

    ~HeGet() {
        delete type;
        delete ptr;
        delete idx;
    }
};

class Cast : public Expr {
private:
    Type *type;
    Expr *expr;
public:
    Cast(Type *type, Expr *expr) : type(type), expr(expr) {}

    llvm::Value* llvmValue(::Adscript::Compiler::Context& ctx) override;
    std::string str() override;

    ~Cast() {
        delete type;
        delete expr;
    }
};

class Function : public Expr {
private:
    const std::string id;
    const std::vector<std::pair<Type*, std::string>> args;
    Type *retType;
    std::vector<Expr*> body;
public:
    Function(const std::string& id, 
                const std::vector<std::pair<Type*, std::string>>& args,
                Type *retType, std::vector<Expr*>& body)
        : id(id), args(args), retType(retType), body(body) {}

    llvm::Value* llvmValue(::Adscript::Compiler::Context& ctx) override;
    std::string str() override;

    ~Function() {
        for (auto& arg : args)
            delete arg.first;
        for (auto& expr : body)
            delete expr;
    }
};

class Lambda : public Expr {
private:
    const std::vector<std::pair<Type*, std::string>> args;
    Type *retType;
    std::vector<Expr*> body;
public:
    Lambda(std::vector<std::pair<Type*, std::string>>& args,
            Type *retType, std::vector<Expr*>& body)
        : args(args), retType(retType), body(body) {}

    llvm::Value* llvmValue(::Adscript::Compiler::Context& ctx) override;
    std::string str() override;

    bool isLambda() override { return true; }

    Function* toFunc(const std::string& id) {
        return new Function(id, args, retType, body);
    }

    ~Lambda() {
        for (auto& arg : args)
            delete arg.first;
        for (auto& expr : body)
            delete expr;
    }
};

class Call : public Expr {
public:
    Expr *callee;
    std::vector<Expr*> args;

    Call(Expr *callee, const std::vector<Expr*>& args)
        : callee(callee), args(args) {}

    llvm::Value* llvmValue(::Adscript::Compiler::Context& ctx) override;
    std::string str() override;

    ~Call() {
        delete callee;
        for (auto& expr : args)
            delete expr;
    }
};

}
}
