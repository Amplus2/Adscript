#include "ast.hh"
#include "utils.hh"

#include <iostream>

#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/Utils/Cloning.h>

std::string IntExpr::toStr() {
    return std::to_string(val);
}

std::string FloatExpr::toStr() {
    return std::to_string(val);
}

std::string IdExpr::toStr() {
    return val;
}

std::string PrimTypeAST::toStr() {
    switch (type) {
    case TYPE_INT: return "int";
    case TYPE_FLOAT: return "float";
    default: return "err";
    }
}

std::string Function::toStr() {
    return std::string() + "fn: { "
        + "id: '" + id + "'"
        + ", args: " + argVectorToStr(args)
        + ", type: " + retType->toStr()
        + ", body: " + exprVectorToStr(body)
        + " }";
}

std::string FunctionCall::toStr() {
    return std::string() + "fncall: { "
        + "id: '" + calleeId + "'"
        + ", args: " + exprVectorToStr(args)
        + " }";
}


std::string IdExpr::getVal() {
    return val;
}

llvm::Type* PrimTypeAST::llvmType(llvm::LLVMContext &ctx) {
    switch (type) {
        case TYPE_INT: return llvm::Type::getInt32Ty(ctx);
        case TYPE_FLOAT: return llvm::Type::getFloatTy(ctx);
        default: error(ERROR_COMPILER, "unknown type name");
    }
    return nullptr;
}

llvm::Value* IntExpr::llvmValue(CompileContext ctx) {
    return llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(ctx.mod->getContext()), val);
}

llvm::Value* FloatExpr::llvmValue(CompileContext ctx) {
    return llvm::ConstantFP::get(llvm::IntegerType::getFloatTy(ctx.mod->getContext()), val);
}

llvm::Value* IdExpr::llvmValue(CompileContext ctx) {
    if (ctx.vars.count(val) <= 0)
        error(ERROR_COMPILER, "undefined reference to '" + val + "'");
    return ctx.builder->CreateLoad(ctx.vars[val].first, ctx.vars[val].second);
}

llvm::AllocaInst* createAlloca(llvm::Function *f, llvm::StringRef id, llvm::Type *type) {
    llvm::IRBuilder<> builder(&(f->getEntryBlock()), f->getEntryBlock().begin());
    return builder.CreateAlloca(type, nullptr, id);
} 

llvm::Value* Function::llvmValue(CompileContext ctx) {
    std::vector<llvm::Type*> ftArgs;
    for (auto& arg : args)
        ftArgs.push_back(arg.first->llvmType(ctx.mod->getContext()));
    
    llvm::FunctionType *ft =
        llvm::FunctionType::get(retType->llvmType(ctx.mod->getContext()), ftArgs, false);

    llvm::Function *f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, id, ctx.mod);

    if (body.size() <= 0) {
        size_t i = 0;
        for (auto& arg : f->args()) arg.setName(args[i].second);
        return f;
    }

    llvm::BasicBlock *tmpBB = ctx.builder->GetInsertBlock();
    ctx.builder->SetInsertPoint(llvm::BasicBlock::Create(ctx.mod->getContext(), "", f));

    size_t i = 0;
    for (auto& arg : f->args()) {
        arg.setName(args[i].second);

        llvm::AllocaInst *alloca = createAlloca(f, arg.getName(), ftArgs[i]);

        ctx.builder->CreateStore(&arg, alloca);

        ctx.vars[args[i++].second] = std::pair<llvm::Type*, llvm::Value*>(arg.getType(), alloca);
    }

    for (size_t i = 0; i < body.size() - 1; i++)
        body[i]->llvmValue(ctx);
    
    ctx.builder->CreateRet(body[body.size() - 1]->llvmValue(ctx));
    
    ctx.vars.clear();

    ctx.builder->SetInsertPoint(tmpBB);

    if (llvm::verifyFunction(*f))
        error(ERROR_COMPILER, "error in function '" + id + "'");

    return f;
}

llvm::Value* FunctionCall::llvmValue(CompileContext ctx) {
    llvm::Function *f = ctx.mod->getFunction(calleeId);

    if (!f) error(ERROR_COMPILER, "undefined reference to '" + calleeId + "'");

    if (args.size() > f->arg_size())
        error(ERROR_COMPILER, "too many arguments for function '" + calleeId + "'");
    else if (args.size() < f->arg_size())
        error(ERROR_COMPILER, "too few arguments for function '" + calleeId + "'");

    std::vector<llvm::Value*> callArgs;

    for (size_t i = 0; i < args.size(); i++) {
        callArgs.push_back(args[i]->llvmValue(ctx));
        if (!callArgs.back()) error(ERROR_COMPILER, "unexpected compilation error");
    }

    size_t i = 0;
    for (auto& arg : f->args())
        if (callArgs[i++]->getType()->getPointerTo() != arg.getType()->getPointerTo())
            error(ERROR_COMPILER, "invalid argument type for function '" + calleeId + "'");

    llvm::CallInst *call = ctx.builder->CreateCall(f, callArgs, "calltmp");

    return call;
}
