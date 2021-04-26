#include "compiler.hh"

#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/LLVMContext.h>

#include <llvm/Passes/PassBuilder.h>

#include <memory>
#include <iostream>

void addFilenameToModuleInfo(const std::string& filename, llvm::Module *mod) {
    std::string sourceFilename = std::count(filename.begin(), filename.end(), '/')
        ? filename.substr(filename.find_last_of('/') + 1, filename.size() - 1)
        : filename;

    size_t dotCount = std::count(sourceFilename.begin(), sourceFilename.end(), '.');

    std::string moduleId = dotCount
        ? sourceFilename[0] == '.'
            ? dotCount == 1 
                ? sourceFilename.substr(1, sourceFilename.size() - 1)
                : sourceFilename.substr(1, sourceFilename.find_last_of('.'))
            : sourceFilename.substr(0, sourceFilename.find_last_of('.'))
        : sourceFilename;
    
    mod->setSourceFileName(sourceFilename);
    mod->setModuleIdentifier(moduleId);
}

inline void runMPM(llvm::Module *mod) {
    llvm::PassBuilder passBuilder;

    llvm::ModuleAnalysisManager     mam;
    llvm::CGSCCAnalysisManager      gam;
    llvm::FunctionAnalysisManager   fam;
    llvm::LoopAnalysisManager       lam;

    passBuilder.registerModuleAnalyses  (mam);
    passBuilder.registerCGSCCAnalyses   (gam);
    passBuilder.registerFunctionAnalyses(fam);
    passBuilder.registerLoopAnalyses    (lam);

    passBuilder.crossRegisterProxies(lam, fam, gam, mam);

    llvm::ModulePassManager mpm = passBuilder.buildPerModuleDefaultPipeline(
        llvm::PassBuilder::OptimizationLevel::O3);
    
    mpm.run(*mod, mam);

    mod->print(llvm::errs(), 0);

    mam.clear();
    gam.clear();
    fam.clear();
    lam.clear();
}

void compileModuleToFile(llvm::Module *mod) {
    // TODO: Add object file compilation
}

void compile(const std::string& filename, std::vector<Expr*>& exprs) {
    llvm::LLVMContext *ctx = new llvm::LLVMContext();
    llvm::Module *mod = new llvm::Module(filename, *ctx);
    llvm::IRBuilder<> *builder = new llvm::IRBuilder<>(*ctx);

    addFilenameToModuleInfo(filename, mod);

    for (auto& expr : exprs)
        expr->llvmValue(CompileContext(mod, builder));

    runMPM(mod);

    std::free(ctx);
    std::free(mod);
    std::free(builder);
}
