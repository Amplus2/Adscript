#pragma once

#include "ast.hh"

#include <string>
#include <vector>

class CompileContext {
public:
    llvm::Module *mod;
    llvm::IRBuilder<> *builder;
    std::map<std::string, std::pair<llvm::Type*, llvm::Value*>> localVars;

    CompileContext(llvm::Module *mod, llvm::IRBuilder<> *builder)
        : mod(mod), builder(builder) {}
    
    bool isVar(const std::string& id);
    std::pair<llvm::Type *, llvm::Value *> getVar(const std::string& id);
};

void compile (const std::string& filename, std::vector<Expr*>& exprs);
