#pragma once

#include "ast.hh"

#include <string>
#include <vector>


namespace Adscript {
namespace Compiler {

typedef std::pair<llvm::Type*, llvm::Value*> ctx_var_t;

class Context {
public:
    llvm::Module *mod;
    llvm::IRBuilder<> *builder;
    std::map<std::string, ctx_var_t> vars;
    std::map<std::string, ctx_var_t> finals;
    std::map<std::string, llvm::Type*> types;

    bool needsRef = false;

    Context(llvm::Module *mod, llvm::IRBuilder<> *builder)
        : mod(mod), builder(builder) {}
    
    bool isVar(const std::string& id);
    bool isType(const std::string& id);
    bool isFinal(const std::string& id);
    bool isFunction(const std::string& id);

    ctx_var_t getVar(const std::string& id);
    ctx_var_t getFinal(const std::string& id);
    llvm::Function* getFunction(const std::string& id);
};

void compile(std::vector<AST::Expr*>& exprs, bool exe, const std::string &output, const std::string &target);

}
}
