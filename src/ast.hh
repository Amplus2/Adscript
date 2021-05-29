#pragma once

#include <map>
#include <string>
#include <vector>

#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>

class CompileContext;

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
    virtual llvm::Value* llvmValue(CompileContext& ctx) = 0;
    virtual bool isIdExpr() { return false; }
};


#include "utils.hh"


class PrimType : public Type {
private:
    PT type;
public:
    PrimType(PT type) : type(type) {}

    llvm::Type* llvmType(llvm::LLVMContext &ctx) override;
    std::string str() override {
        switch (type) {
        case TYPE_I8: return "i8";
        case TYPE_I16: return "i16";
        case TYPE_I32: return "i32";
        case TYPE_I64: return "i64";
        case TYPE_FLOAT: return "float";
        case TYPE_DOUBLE: return "double";
        default: return "err";
        }
    }
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
    std::string str() override {
        return std::string() + "PointerType: " + type->str();
    }

    ~PointerType() {
        delete type;
    }
};

class IntExpr : public Expr {
private:
    const int64_t val;
public:
    IntExpr(const int64_t val) : val(val) {}

    llvm::Value* llvmValue(CompileContext& ctx) override;
    std::string str() override {
        return std::to_string(val);
    }
};

class FloatExpr : public Expr {
private:
    const double val;
public:
    FloatExpr(const double val) : val(val) {}

    llvm::Value* llvmValue(CompileContext& ctx) override;
    std::string str() override {
        return std::to_string(val);
    }
};

class CharExpr : public Expr {
private:
    const char val;
public:
    CharExpr(const char val) : val(val) {}

    llvm::Value* llvmValue(CompileContext& ctx) override;
    std::string str() override {
        return "\\" + std::string(1, (char) val);
    }
};

class IdExpr : public Expr {
private:
    const std::string val;
public:
    IdExpr(const std::string  val) : val(val) {}

    std::string getVal() { return val; };
    llvm::Value* llvmValue(CompileContext& ctx) override;
    std::string str() override {
        return val;
    }
    
    bool isIdExpr() override { return true; }
};

class StrExpr : public Expr {
private:
    const std::string val;
public:
    StrExpr(const std::string  val) : val(val) {}

    std::string getVal();
    llvm::Value* llvmValue(CompileContext& ctx) override;
    std::string str() override {
        return val;
    }
};

class UExpr : public Expr {
private:
    BinExprType type;
    Expr *expr;
public:
    UExpr(BinExprType type, Expr *expr)
        : type(type), expr(expr) {}

    llvm::Value* llvmValue(CompileContext& ctx) override;
    std::string str() override {
        return std::string() + "UExpr: { "
            + "op: " + betToStr(type)
            + ", expr: " + expr->str()
            + " }";
    }

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

    llvm::Value* llvmValue(CompileContext& ctx) override;
    std::string str() override {
        return std::string() + "BinExpr: { "
            + "op: " + betToStr(type)
            + ", left: " + left->str()
            + ", right: " + right->str()
            + " }";
    }

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

    llvm::Value* llvmValue(CompileContext& ctx) override;
    std::string str() override {
        return std::string() + "IfExpr: {"
            + "cond: " + cond->str()
            + ", exprTrue: " + exprTrue->str()
            + ", exprFalse: " + exprFalse->str()
            + " }";
    }

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

    llvm::Value* llvmValue(CompileContext& ctx) override;
    std::string str() override {
        return std::string() + "ArrayExpr: {"
            + "size: " + std::to_string(exprs.size())
            + ", exprs: " + exprVectorToStr(exprs)
            + " }";
    }

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

    llvm::Value* llvmValue(CompileContext& ctx) override;
    std::string str() override {
        return std::string() + "PtrArrayExpr: {"
            + "size: " + std::to_string(exprs.size())
            + ", exprs: " + exprVectorToStr(exprs)
            + " }";
    }

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

    llvm::Value* llvmValue(CompileContext& ctx) override;
    std::string str() override {
        return std::string() + "VarExpr: {"
            + "id: '" + id + "'";
            + ", val: " + val->str()
            + " }";
    }

    ~VarExpr() {
        delete val;
    }
};

class SetExpr : public Expr {
private:
    Expr *ptr, *val;
public:
    SetExpr(Expr *ptr, Expr *val) : ptr(ptr), val(val) {}

    llvm::Value* llvmValue(CompileContext& ctx) override;
    std::string str() override {
        return std::string() + "SetExpr: {"
            + "ptr: " + ptr->str();
            + ", val: " + val->str()
            + " }";
    }

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

    llvm::Value* llvmValue(CompileContext& ctx) override;
    std::string str() override {
        return std::string() + "RefExpr: {"
            + "val: " + val->str()
            + " }";
    }

    ~RefExpr() {
        delete val;
    }
};

class DerefExpr : public Expr {
private:
    Expr *ptr;
public:
    DerefExpr(Expr *ptr) : ptr(ptr) {}

    llvm::Value* llvmValue(CompileContext& ctx) override;
    std::string str() override {
        return std::string() + "DerefExpr: {"
            + "ptr: " + ptr->str()
            + " }";
    }

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

    llvm::Value* llvmValue(CompileContext& ctx) override;
    std::string str() override {
        return std::string() + "HeGetExpr: {"
            + "type: " + type->str()
            + ", ptr: " + ptr->str()
            + ", idx: " + idx->str()
            + " }";
    }

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

    llvm::Value* llvmValue(CompileContext& ctx) override;
    std::string str() override {
        return std::string() + "Cast { "
            + "type: " + type->str()
            + ", expr: " + expr->str()
            + " }";
    }

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
                const std::vector<std::pair<Type*, std::string>>& args,
                Type *retType, std::vector<Expr*>& body)
        : id(id), args(args), retType(retType), body(body) {}

    llvm::Value* llvmValue(CompileContext& ctx) override;
    std::string str() override {
        return std::string() + "Function: { "
            + "id: '" + id + "'"
            + ", args: " + argVectorToStr(args)
            + ", type: " + retType->str()
            + ", body: " + exprVectorToStr(body)
            + " }";
    }

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

    llvm::Value* llvmValue(CompileContext& ctx) override;
    std::string str() override {
        return std::string() + "Lambda: { "
            + "args: " + argVectorToStr(args)
            + ", type: " + retType->str()
            + ", body: " + exprVectorToStr(body)
            + " }";
    }

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

class CallExpr : public Expr {
public:
    Expr *callee;
    std::vector<Expr*> args;

    CallExpr(Expr *callee, const std::vector<Expr*>& args)
        : callee(callee), args(args) {}

    llvm::Value* llvmValue(CompileContext& ctx) override;
    std::string str() override {
        return std::string() + "FunctionCall: { "
            + "calle: " + callee->str()
            + ", args: " + exprVectorToStr(args)
            + " }";
    }

    ~CallExpr() {
        delete callee;
        for (auto& expr : args)
            delete expr;
    }
};
