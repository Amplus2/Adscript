#include "utils.hh"
#include "compiler.hh"

#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/PassManager.h>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>

#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

#include <llvm/Support/Host.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/TargetRegistry.h>

#include <llvm/CodeGen/Passes.h>
#include <llvm/CodeGen/MachineModuleInfo.h>

#include <memory>
#include <fstream>
#include <iostream>

#include <unistd.h>

using namespace Adscript;

bool Compiler::Context::isVar(const std::string& id) {
    return varScope.find(id) != varScope.end();
}

bool Compiler::Context::isType(const std::string& id) {
    return types.find(id) != types.end();
}

bool Compiler::Context::isFinal(const std::string& id) {
    return finalScope.find(id) != finalScope.end();
}

bool Compiler::Context::isFunction(const std::string& id) {
    for (auto& f : mod->getFunctionList()) {
        if (!id.compare(f.getName().str())) return true;
    }
    return false;
}

Compiler::ctx_var_t Compiler::Context::getVar(const std::string& id) {
    if (isVar(id)) return varScope[id];
    return { nullptr, nullptr };
}

Compiler::ctx_var_t Compiler::Context::getFinal(const std::string& id) {
    if (isFinal(id)) return finalScope[id];
    return { nullptr, nullptr };
}

llvm::Function* Compiler::Context::getFunction(const std::string& id) {
    if (isFunction(id)) return mod->getFunction(id);
    return nullptr;
}

void Compiler::Context::runFPM(llvm::Function *f) {
    if (!f) return;
    fpm.run(*f, fam);
}

std::string getFileName(const std::string& path) {
    auto s = path.find_last_of("/\\");
    return s == std::string::npos ? path : path.substr(s + 1);
}

std::string getModuleId(const std::string& filename) {
    size_t dotCount = std::count(filename.begin(), filename.end(), '.');

    return dotCount
        ? filename[0] == '.'
            ? dotCount == 1
                ? filename.substr(1, filename.size() - 1)
                : filename.substr(1, filename.find_last_of('.'))
            : filename.substr(0, filename.find_last_of('.'))
        : filename;
}

void compileModuleToFile(llvm::Module *mod, const std::string &output, const std::string &target) {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    mod->setTargetTriple(target);

    std::string err;
    const llvm::Target *t =
        llvm::TargetRegistry::lookupTarget(target, err);

    if (!t) Error::compiler(std::stou32(err));

    llvm::TargetMachine *targetMachine = t->createTargetMachine(
        target,
        "", "",
        llvm::TargetOptions(),
        // TODO: make configurable
        llvm::Reloc::PIC_
    );

    mod->setDataLayout(targetMachine->createDataLayout());

    std::error_code ec;
    llvm::raw_fd_ostream dest(output, ec, llvm::sys::fs::OF_None);

    if (ec) Error::compiler(std::stou32(ec.message()));

    llvm::legacy::PassManager pm;

    auto& tm = (llvm::LLVMTargetMachine&) *targetMachine;

    pm.add(new llvm::TargetLibraryInfoWrapperPass());
    pm.add(new llvm::MachineModuleInfoWrapperPass(&tm));
    
    bool objResult = targetMachine->addPassesToEmitFile(
        pm, dest, nullptr, llvm::CGFT_ObjectFile);

    if (objResult)
        Error::compiler(U"cannot write to file '" + std::stou32(output) + U"'");

    pm.run(*mod);
    dest.flush();
}

void link(const std::string &obj, const std::string &exe) {
    int linkResult = system(("cc " + obj + " -o " + exe).c_str());

    if (linkResult)
        Error::compiler(std::stou32("error while linking '" + exe + "'"));

    if (std::remove(obj.c_str()))
        Error::def(std::stou32("cannot remove '" + obj + "'"));
}

std::string tempfile() {
    char file[] = "/tmp/adscript-XXXXXXXX.o";
    mkstemp(file);
    return file;
}

void Compiler::compile(std::vector<AST::Expr*>& exprs, bool exe, const std::string &output, const std::string &target, bool emitLLVM) {
    std::string filename = getFileName(output);
    std::string moduleId = getModuleId(filename);

    llvm::LLVMContext ctx;
    llvm::Module mod(moduleId, ctx);
    llvm::IRBuilder<> builder(ctx);

    Compiler::Context cctx(&mod, &builder);
    for (auto& expr : exprs) expr->llvmValue(cctx);

    cctx.clear();

    if (emitLLVM) {
        auto idx = output.find_last_of("/\\");
        std::string fname;

        if (idx != std::string::npos) fname = output.substr(0, idx + 1);

        fname = fname + getFileName(getModuleId(output)) + ".ll";

        std::error_code ec;
        llvm::raw_fd_ostream dest(fname, ec, llvm::sys::fs::OF_None);

        if (ec) Error::compiler(std::stou32(ec.message()));
        mod.print(dest, 0);
    }

    std::string obj = exe ? tempfile() : output;
    compileModuleToFile(&mod, obj, target);
    if (exe) link(obj, output);
}
