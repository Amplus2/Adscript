#include "compiler.hh"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <iostream>

void compile(const std::string& filename, std::vector<Expr*>& exprs) {
    llvm::LLVMContext *ctx = new llvm::LLVMContext();
    llvm::Module *mod = new llvm::Module(filename, *ctx);
    llvm::IRBuilder<> *builder = new llvm::IRBuilder<>(*ctx);

    llvm::Function *mainFunc = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx), {}, false),
        llvm::Function::InternalLinkage,
        "main",
        mod
    );

    llvm::BasicBlock *mainBB = llvm::BasicBlock::Create(*ctx, "entry", mainFunc);

    builder->SetInsertPoint(mainBB);

    for (auto& expr : exprs)
        expr->llvmValue(CompileContext(mod, builder));
    
    mod->print(llvm::errs(), 0);
}
