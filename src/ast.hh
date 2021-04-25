#ifndef AST_HH_
#define AST_HH_

#include <map>
#include <string>
#include <vector>

#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>

class CompileContext {
public:
    llvm::Module *mod;
    llvm::IRBuilder<> *builder;
    std::map<std::string, std::pair<llvm::Type*, llvm::Value*>> vars;

    CompileContext(llvm::Module *mod, llvm::IRBuilder<> *builder)
        : mod(mod), builder(builder) {}
};

class Expr {
public:
    virtual ~Expr() = default;
    virtual std::string toStr() = 0;
    virtual llvm::Value* llvmValue(CompileContext ctx) = 0;
};

class IntExpr : public Expr {
private:
    const int val;
public:

    IntExpr(const int val) : val(val) {}

    std::string toStr() override;
    llvm::Value* llvmValue(CompileContext ctx) override;
};

class FloatExpr : public Expr {
private:
    const float val;
public:

    FloatExpr(const float val) : val(val) {}

    std::string toStr() override;
    llvm::Value* llvmValue(CompileContext ctx) override;
};

class IdExpr : public Expr {
public:
    const std::string val;

    IdExpr(const std::string  val) : val(val) {}

    std::string toStr() override;
    std::string getVal();
    llvm::Value* llvmValue(CompileContext ctx) override;
};

enum BinExprType {
    BINEXPR_ADD,
    BINEXPR_SUB,
    BINEXPR_MUL,
    BINEXPR_DIV,
    BINEXPR_MOD,
};

class BinExpr : public Expr {
private:
    BinExprType type;
    Expr *left, *right;
public:
    BinExpr(BinExprType type, Expr *left, Expr *right)
        : type(type), left(left), right(right) {}

    std::string toStr() override;
    llvm::Value* llvmValue(CompileContext ctx) override;
};

class TypeAST {
public:
    virtual ~TypeAST() = default;
    virtual std::string toStr() = 0;
    virtual llvm::Type* llvmType(llvm::LLVMContext &ctx) = 0;
};


enum PrimType {
    TYPE_ERR,

    TYPE_INT,
    TYPE_FLOAT,
};

class PrimTypeAST : public TypeAST {
private:
    PrimType type;
public:
    PrimTypeAST(PrimType type) : type(type) {}

    std::string toStr() override;
    llvm::Type* llvmType(llvm::LLVMContext &ctx) override;
};

class Function : public Expr {
public:
    const std::string id;
    const std::vector<std::pair<TypeAST*, std::string>> args;
    TypeAST *retType;
    const std::vector<Expr*> body;

    Function(const std::string& id, 
                std::vector<std::pair<TypeAST*, std::string>>& args,
                TypeAST *retType, std::vector<Expr*>& body)
        : id(id), args(args), retType(retType), body(body) {}

    std::string toStr() override;
    llvm::Value* llvmValue(CompileContext ctx) override;
};

class FunctionCall : public Expr {
public:
    const std::string calleeId;
    const std::vector<Expr*> args;

    FunctionCall(const std::string& calleeId, const std::vector<Expr*>& args)
        : calleeId(calleeId), args(args) {}

    std::string toStr() override;
    llvm::Value* llvmValue(CompileContext ctx) override;
};



#endif
