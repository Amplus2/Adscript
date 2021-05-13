#pragma once

#include "ast.hh"

#include <string>
#include <vector>

typedef std::pair<llvm::Type*, llvm::Value*> ctx_var_t;

class CompileContext {
public:
    llvm::Module *mod;
    llvm::IRBuilder<> *builder;
    std::map<std::string, ctx_var_t> localVars;

    CompileContext(llvm::Module *mod, llvm::IRBuilder<> *builder)
        : mod(mod), builder(builder) {}
    
    bool isVar(const std::string& id);
    ctx_var_t getVar(const std::string& id);
};

void compile (const std::string& filename, std::vector<Expr*>& exprs);
