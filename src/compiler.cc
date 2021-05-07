#include "utils.hh"
#include "compiler.hh"

#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/PassManager.h>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>

#include <llvm/Passes/PassBuilder.h>

#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

#include <llvm/Support/Host.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/TargetRegistry.h>

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

void runMPM(llvm::Module *mod) {
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

    mam.clear();
    gam.clear();
    fam.clear();
    lam.clear();
}

void compileModuleToFile(llvm::Module *mod) {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    // TODO: make configurable
    std::string targetTriple = llvm::sys::getDefaultTargetTriple();
    mod->setTargetTriple(targetTriple);

    std::string err;
    const llvm::Target *target = llvm::TargetRegistry::lookupTarget(targetTriple, err);

    if (!target) error(ERROR_COMPILER, err);

    llvm::TargetOptions options;
    llvm::Optional<llvm::Reloc::Model> relocModel;
    llvm::TargetMachine *targetMachine = target->createTargetMachine(
        targetTriple,
        // TODO: make configurable
        llvm::sys::getHostCPUName(), "",
        options,
        relocModel
    );

    mod->setDataLayout(targetMachine->createDataLayout());

    std::string outputFilename = mod->getModuleIdentifier() + ".o";

    std::error_code ec;
    llvm::raw_fd_ostream dest(outputFilename, ec, llvm::sys::fs::OF_None);

    if (ec) error(ERROR_COMPILER, ec.message());

    llvm::legacy::PassManager pm;
    
    bool objResult = targetMachine->addPassesToEmitFile(pm, dest, nullptr, llvm::CGFT_ObjectFile);

    if (objResult) error(ERROR_COMPILER, "cannot write to file '" + outputFilename + "'");

    pm.run(*mod);
    dest.flush();

    int linkResult = system(("gcc " + outputFilename + " -o " + mod->getModuleIdentifier()).c_str());

    if (linkResult) error(ERROR_COMPILER, "error while linking '" + outputFilename + "'");

    if (std::remove(outputFilename.c_str()))
        error(ERROR_DEFAULT, "cannot remove '" + outputFilename + "'");
}

void compile(const std::string& filename, std::vector<Expr*>& exprs) {
    llvm::LLVMContext ctx;
    llvm::Module mod(filename, ctx);
    llvm::IRBuilder<> builder(ctx);

    addFilenameToModuleInfo(filename, &mod);

    CompileContext cctx(&mod, &builder);
    for (auto& expr : exprs)
        expr->llvmValue(cctx);

    runMPM(&mod);

    // mod->print(llvm::errs(), 0);

    compileModuleToFile(&mod);
}
