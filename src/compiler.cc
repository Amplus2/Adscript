#include "compiler.hh"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <iostream>

void addFilenameToModuleInfo(const std::string& filename, llvm::Module *mod) {
    std::string sourceFilename = std::count(filename.begin(), filename.end(), '/') > 0
        ? filename.substr(filename.find_last_of('/'), filename.size() - 1)
        : filename;

    size_t dotCount = std::count(sourceFilename.begin(), sourceFilename.end(), '.');

    std::string moduleId = dotCount
        ? sourceFilename[0] == '.' && dotCount == 1
            ? sourceFilename.substr(1, sourceFilename.size() - 1)
            : sourceFilename.substr(0, sourceFilename.find_last_of('.') - 1)
        : sourceFilename;
    
    mod->setSourceFileName(sourceFilename);
    mod->setModuleIdentifier(moduleId);
}

void compileModuleToFile(llvm::Module *mod) {
    // TODO: Add object file compilation
}

void compile(const std::string& filename, std::vector<Expr*>& exprs) {
    llvm::LLVMContext *ctx = new llvm::LLVMContext();
    llvm::Module *mod = new llvm::Module(filename, *ctx);
    llvm::IRBuilder<> *builder = new llvm::IRBuilder<>(*ctx);

    addFilenameToModuleInfo(filename, mod);

    llvm::Function *mainFunc = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx), {}, false),
        llvm::Function::ExternalLinkage,
        "main",
        mod
    );

    llvm::BasicBlock *mainBB = llvm::BasicBlock::Create(*ctx, "", mainFunc);

    builder->SetInsertPoint(mainBB);

    for (auto& expr : exprs)
        expr->llvmValue(CompileContext(mod, builder));
    
    mod->print(llvm::errs(), 0);
}
