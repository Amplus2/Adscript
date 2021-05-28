#pragma once

#include <map>
#include <string>
#include <vector>

#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>

class CompileContext;

class Type {
public:
    virtual ~Type() = default;
    virtual std::string str() = 0;
    virtual llvm::Type* llvmType(llvm::LLVMContext &ctx) = 0;
};


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

class PrimType : public Type {
private:
    PT type;
public:
    PrimType(PT type) : type(type) {}

    std::string str() override;
    llvm::Type* llvmType(llvm::LLVMContext &ctx) override;
};

class PointerType : public Type {
private:
    Type *type;
    uint8_t quantity;
public:
    PointerType(Type *type) : type(type), quantity(1) {}
    PointerType(Type *type, uint8_t quantity)
        : type(type), quantity(quantity) {}

    std::string str() override;
    llvm::Type* llvmType(llvm::LLVMContext &ctx) override;

    ~PointerType() {
        delete type;
    }
};

class Expr {
public:
    virtual ~Expr() = default;
    virtual std::string str() = 0;
    virtual llvm::Value* llvmValue(CompileContext& ctx) = 0;
    virtual bool isIdExpr() { return false; }
};

class IntExpr : public Expr {
private:
    const int64_t val;
public:
    IntExpr(const int64_t val) : val(val) {}

    std::string str() override;
    llvm::Value* llvmValue(CompileContext& ctx) override;
};

class FloatExpr : public Expr {
private:
    const double val;
public:
    FloatExpr(const double val) : val(val) {}

    std::string str() override;
    llvm::Value* llvmValue(CompileContext& ctx) override;
};

class CharExpr : public Expr {
private:
    const char val;
public:
    CharExpr(const char val) : val(val) {}

    std::string str() override;
    llvm::Value* llvmValue(CompileContext& ctx) override;
};

class IdExpr : public Expr {
private:
    const std::string val;
public:
    IdExpr(const std::string  val) : val(val) {}

    std::string str() override;
    std::string getVal() { return val; };
    llvm::Value* llvmValue(CompileContext& ctx) override;
    
    bool isIdExpr() override { return true; }
};

class StrExpr : public Expr {
private:
    const std::string val;
public:
    StrExpr(const std::string  val) : val(val) {}

    std::string str() override;
    std::string getVal();
    llvm::Value* llvmValue(CompileContext& ctx) override;
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

class UExpr : public Expr {
private:
    BinExprType type;
    Expr *expr;
public:
    UExpr(BinExprType type, Expr *expr)
        : type(type), expr(expr) {}

    std::string str() override;
    llvm::Value* llvmValue(CompileContext& ctx) override;

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

    std::string str() override;
    llvm::Value* llvmValue(CompileContext& ctx) override;

    ~BinExpr() {
        delete left;
        delete right;
    }
};

class IfExpr : public Expr {
private:
    Expr *cond, *exprTrue, *exprFalse;
public:
    IfExpr(Expr *cond, Expr *exprTrue, Expr *exprFalse)
        : cond(cond), exprTrue(exprTrue), exprFalse(exprFalse) {}

    std::string str() override;
    llvm::Value* llvmValue(CompileContext& ctx) override;

    ~IfExpr() {
        delete cond;
        delete exprTrue;
        delete exprFalse;
    }
};

class ArrayExpr : public Expr {
private:
    std::vector<Expr*> exprs;
public:
    ArrayExpr(const std::vector<Expr*>& exprs) : exprs(exprs) {}

    std::string str() override;
    llvm::Value* llvmValue(CompileContext& ctx) override;

    ~ArrayExpr() {
        for (auto& expr : exprs)
            delete expr;
    }
};

class PtrArrayExpr : public Expr {
private:
    std::vector<Expr*> exprs;
public:
    PtrArrayExpr(const std::vector<Expr*>& exprs) : exprs(exprs) {}

    std::string str() override;
    llvm::Value* llvmValue(CompileContext& ctx) override;

    ~PtrArrayExpr() {
        for (auto& expr : exprs)
            delete expr;
    }
};

class VarExpr : public Expr {
private:
    Expr *val;
    const std::string id;
public:
    VarExpr(Expr *val, const std::string& id) : val(val), id(id) {}

    std::string str() override;
    llvm::Value* llvmValue(CompileContext& ctx) override;

    ~VarExpr() {
        delete val;
    }
};

class SetExpr : public Expr {
private:
    Expr *ptr, *val;
public:
    SetExpr(Expr *ptr, Expr *val) : ptr(ptr), val(val) {}

    std::string str() override;
    llvm::Value* llvmValue(CompileContext& ctx) override;

    ~SetExpr() {
        delete ptr;
        delete val;
    }
};

class RefExpr : public Expr {
private:
    Expr *val;
public:
    RefExpr(Expr *val) : val(val) {}

    std::string str() override;
    llvm::Value* llvmValue(CompileContext& ctx) override;

    ~RefExpr() {
        delete val;
    }
};

class DerefExpr : public Expr {
private:
    Expr *ptr;
public:
    DerefExpr(Expr *ptr) : ptr(ptr) {}

    std::string str() override;
    llvm::Value* llvmValue(CompileContext& ctx) override;

    ~DerefExpr() {
        delete ptr;
    }
};

class HeGetExpr : public Expr {
private:
    Type *type;
    Expr *ptr, *idx;
public:
    HeGetExpr(Type *type, Expr *ptr, Expr *idx)
        : type(type), ptr(ptr), idx(idx) {}

    std::string str() override;
    llvm::Value* llvmValue(CompileContext& ctx) override;

    ~HeGetExpr() {
        delete type;
        delete ptr;
        delete idx;
    }
};

class CastExpr : public Expr {
private:
    Type *type;
    Expr *expr;
public:
    CastExpr(Type *type, Expr *expr) : type(type), expr(expr) {}

    std::string str() override;
    llvm::Value* llvmValue(CompileContext& ctx) override;

    ~CastExpr() {
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
                std::vector<std::pair<Type*, std::string>>& args,
                Type *retType, std::vector<Expr*>& body)
        : id(id), args(args), retType(retType), body(body) {}

    std::string str() override;
    llvm::Value* llvmValue(CompileContext& ctx) override;

    ~Function() {
        for (auto& arg : args)
            delete arg.first;
        for (auto& expr : body)
            delete expr;
    }
};

class CallExpr : public Expr {
public:
    Expr *callee;
    std::vector<Expr*> args;

    CallExpr(Expr *callee, const std::vector<Expr*>& args)
        : callee(callee), args(args) {}

    std::string str() override;
    llvm::Value* llvmValue(CompileContext& ctx) override;

    ~CallExpr() {
        delete callee;
        for (auto& expr : args)
            delete expr;
    }
};
