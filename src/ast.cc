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

std::string betToStr(BinExprType bet) {
    switch (bet) {
    case BINEXPR_ADD: return "+";
    case BINEXPR_SUB: return "-";
    case BINEXPR_MUL: return "*";
    case BINEXPR_DIV: return "/";
    case BINEXPR_MOD: return "%";
    }
}

std::string BinExpr::toStr() {
    return std::string() + "BinExpr: { "
        + "op: " + betToStr(type)
        + ", left: " + left->toStr()
        + ", right: " + right->toStr()
        + " }";
}

std::string IfExpr::toStr() {
    return std::string() + "IfExpr: {"
        + "cond: " + cond->toStr()
        + ", exprTrue: " + exprTrue->toStr()
        + ", exprFalse: " + exprFalse->toStr()
        + " }";
}

std::string PrimTypeAST::toStr() {
    switch (type) {
    case TYPE_INT: return "int";
    case TYPE_FLOAT: return "float";
    default: return "err";
    }
}

std::string Function::toStr() {
    return std::string() + "Function: { "
        + "id: '" + id + "'"
        + ", args: " + argVectorToStr(args)
        + ", type: " + retType->toStr()
        + ", body: " + exprVectorToStr(body)
        + " }";
}

std::string FunctionCall::toStr() {
    return std::string() + "FunctionCall: { "
        + "id: '" + calleeId + "'"
        + ", args: " + exprVectorToStr(args)
        + " }";
}


std::string IdExpr::getVal() {
    return val;
}

llvm::Value* constInt(CompileContext ctx, int val) {
    return llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(ctx.mod->getContext()), val);
}

llvm::Value* constFP(CompileContext ctx, float val) {
    return llvm::ConstantFP::get(llvm::IntegerType::getFloatTy(ctx.mod->getContext()), val);
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
    return constInt(ctx, val);
}

llvm::Value* FloatExpr::llvmValue(CompileContext ctx) {
    return constFP(ctx, val);
}

llvm::Value* IdExpr::llvmValue(CompileContext ctx) {
    if (ctx.vars.count(val) <= 0)
        error(ERROR_COMPILER, "undefined reference to '" + val + "'");
    return ctx.builder->CreateLoad(ctx.vars[val].first, ctx.vars[val].second);
}

bool llvmTypeEqual(llvm::Value *v, llvm::Type *t) {
    return v->getType()->getPointerTo() == t->getPointerTo();
}

llvm::Value* BinExpr::llvmValue(CompileContext ctx) {
    llvm::Value *leftV = left->llvmValue(ctx);
    llvm::Value *rightV = right->llvmValue(ctx);

    if (leftV->getType()->getPointerTo() != rightV->getType()->getPointerTo())
        error(ERROR_COMPILER, "binary expression operand types do not match");
    
    if (llvmTypeEqual(leftV, llvm::Type::getInt32Ty(ctx.mod->getContext()))) {
        switch (type) {
        case BINEXPR_ADD: return ctx.builder->CreateAdd(leftV, rightV);
        case BINEXPR_SUB: return ctx.builder->CreateSub(leftV, rightV);
        case BINEXPR_MUL: return ctx.builder->CreateMul(leftV, rightV);
        case BINEXPR_DIV: return ctx.builder->CreateSDiv(leftV, rightV);
        case BINEXPR_MOD: return ctx.builder->CreateSRem(leftV, rightV);
        }
    } else if (llvmTypeEqual(leftV, llvm::Type::getFloatTy(ctx.mod->getContext()))) {
        switch (type) {
        case BINEXPR_ADD: return ctx.builder->CreateFAdd(leftV, rightV);
        case BINEXPR_SUB: return ctx.builder->CreateFSub(leftV, rightV);
        case BINEXPR_MUL: return ctx.builder->CreateFMul(leftV, rightV);
        case BINEXPR_DIV: return ctx.builder->CreateFDiv(leftV, rightV);
        case BINEXPR_MOD: return ctx.builder->CreateFRem(leftV, rightV);
        }
    }

    error(ERROR_COMPILER, "unknown type name in expression");

    return nullptr;
}

llvm::Value* IfExpr::llvmValue(CompileContext ctx) {
    llvm::Value *condV = cond->llvmValue(ctx);

    if (llvmTypeEqual(condV, llvm::Type::getInt32Ty(ctx.mod->getContext())))
        condV = ctx.builder->CreateICmpNE(condV, constInt(ctx, 0));
    else if (llvmTypeEqual(condV, llvm::Type::getFloatTy(ctx.mod->getContext())))
        condV = ctx.builder->CreateFCmpUNE(condV, constFP(ctx, 0));


    llvm::Value *trueV = exprTrue->llvmValue(ctx);
    llvm::Value *falseV = exprFalse->llvmValue(ctx);

    if (trueV->getType()->getPointerTo() != falseV->getType()->getPointerTo())
        error(ERROR_COMPILER, "conditional expression operand types do not match");

    return ctx.builder->CreateSelect(condV, trueV, falseV);
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

    llvm::CallInst *call = ctx.builder->CreateCall(f, callArgs);

    return call;
}
