#pragma once

#include "ast.hh"

#include <string>
#include <vector>

#include <llvm/ADT/Hashing.h>
#include <llvm/Passes/PassBuilder.h>

namespace Adscript {
namespace Compiler {

typedef std::pair<llvm::Type*, llvm::Value*> ctx_var_t;
typedef std::map<std::string, ctx_var_t> scope;

class Context {
private:
    llvm::ModuleAnalysisManager     mam;
    llvm::CGSCCAnalysisManager      gam;
    llvm::FunctionAnalysisManager   fam;
    llvm::LoopAnalysisManager       lam;
    llvm::FunctionPassManager       fpm;     
public:
    llvm::Module *mod;
    llvm::IRBuilder<> *builder;
    scope varScope;
    scope finalScope;
    std::map<std::string, llvm::Type*> types;

    bool needsRef = false;

    Context(llvm::Module *mod, llvm::IRBuilder<> *builder) : mod(mod), builder(builder) {
        llvm::PassBuilder passBuilder;

        passBuilder.registerModuleAnalyses   (mam);
        passBuilder.registerCGSCCAnalyses    (gam);
        passBuilder.registerFunctionAnalyses (fam);
        passBuilder.registerLoopAnalyses     (lam);

        passBuilder.crossRegisterProxies(lam, fam, gam, mam);

        fpm = passBuilder.buildFunctionSimplificationPipeline(
            llvm::PassBuilder::OptimizationLevel::O3,
            llvm::ThinOrFullLTOPhase::None);
    }
    
    bool isVar(const std::string& id);
    bool isType(const std::string& id);
    bool isFinal(const std::string& id);
    bool isFunction(const std::string& id);

    ctx_var_t getVar(const std::string& id);
    ctx_var_t getFinal(const std::string& id);
    llvm::Function* getFunction(const std::string& id);

    void runFPM(llvm::Function *f);

    void clear() {
        mam.clear();
        gam.clear();
        fam.clear();
        lam.clear();
    }
};

void compile(std::vector<AST::Expr*>& exprs, bool exe, const std::string &output, const std::string &target, bool emitLLVM);

}
}
