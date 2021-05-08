#include "ast.hh"
#include "utils.hh"

#include <iostream>

#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/Utils/Cloning.h>

// TODO: move some functions or methods to 'utils.cc'
// AST str methods

std::string IntExpr::str() {
    return std::to_string(val);
}

std::string FloatExpr::str() {
    return std::to_string(val);
}

std::string IdExpr::str() {
    return val;
}

std::string betStr(BinExprType bet) {
    switch (bet) {
    case BINEXPR_ADD:   return "+";
    case BINEXPR_SUB:   return "-";
    case BINEXPR_MUL:   return "*";
    case BINEXPR_DIV:   return "/";
    case BINEXPR_MOD:   return "%";

    case BINEXPR_OR:    return "|";
    case BINEXPR_AND:   return "&";
    case BINEXPR_XOR:   return "^";

    case BINEXPR_EQ:    return "=";
    case BINEXPR_LT:    return "<";
    case BINEXPR_GT:    return ">";
    case BINEXPR_LTEQ:  return "<=";
    case BINEXPR_GTEQ:  return ">=";
    case BINEXPR_LOR:   return "or";
    case BINEXPR_LAND:  return "and";
    case BINEXPR_LXOR:  return "xor";
    case BINEXPR_NOT:   return "not";
    }
}

std::string UExpr::str() {
    return std::string() + "UExpr: { "
        + "op: " + betStr(type)
        + ", expr: " + expr->str()
        + " }";
}

std::string BinExpr::str() {
    return std::string() + "BinExpr: { "
        + "op: " + betStr(type)
        + ", left: " + left->str()
        + ", right: " + right->str()
        + " }";
}

std::string IfExpr::str() {
    return std::string() + "IfExpr: {"
        + "cond: " + cond->str()
        + ", exprTrue: " + exprTrue->str()
        + ", exprFalse: " + exprFalse->str()
        + " }";
}


std::string ArrayExpr::str() {
    return std::string() + "ArrayExpr: {"
        + "size: " + std::to_string(exprs.size())
        + ", exprs: " + exprVectorToStr(exprs)
        + " }";
}


std::string PtrArrayExpr::str() {
    return std::string() + "PtrArrayExpr: {"
        + "size: " + std::to_string(exprs.size())
        + ", exprs: " + exprVectorToStr(exprs)
        + " }";
}


std::string PrimType::str() {
    switch (type) {
    case TYPE_I8: return "i8";
    case TYPE_I16: return "i16";
    case TYPE_I32: return "i32";
    case TYPE_I64: return "i64";
    case TYPE_FLOAT: return "float";
    case TYPE_DOUBLE: return "double";
    default: return "err";
    }
}

std::string PointerType::str() {
    return std::string() + "Pointer: " + type->str();
}

std::string CastExpr::str() {
    return std::string() + "Cast { "
        + "type: " + type->str()
        + ", expr: " + expr->str()
        + " }";
}

std::string Function::str() {
    return std::string() + "Function: { "
        + "id: '" + id + "'"
        + ", args: " + argVectorToStr(args)
        + ", type: " + retType->str()
        + ", body: " + exprVectorToStr(body)
        + " }";
}

std::string FunctionCall::str() {
    return std::string() + "FunctionCall: { "
        + "id: '" + calleeId + "'"
        + ", args: " + exprVectorToStr(args)
        + " }";
}

std::string IdExpr::getVal() {
    return val;
}

// AST llvm Methods

llvm::Value* constInt(CompileContext& ctx, int64_t val) {
    return llvm::ConstantInt::get(llvm::IntegerType::getInt64Ty(ctx.mod->getContext()), val);
}

llvm::Value* constFP(CompileContext& ctx, double val) {
    return llvm::ConstantFP::get(llvm::IntegerType::getDoubleTy(ctx.mod->getContext()), val);
}

llvm::Type* PrimType::llvmType(llvm::LLVMContext &ctx) {
    switch (type) {
    case TYPE_I8:       return llvm::Type::getInt8Ty(ctx);
    case TYPE_I16:      return llvm::Type::getInt16Ty(ctx);
    case TYPE_I32:      return llvm::Type::getInt32Ty(ctx);
    case TYPE_I64:      return llvm::Type::getInt64Ty(ctx);
    case TYPE_FLOAT:    return llvm::Type::getFloatTy(ctx);
    case TYPE_DOUBLE:   return llvm::Type::getDoubleTy(ctx);
    default: error(ERROR_COMPILER, "unknown type name");
    }
    return nullptr;
}

llvm::Type* PointerType::llvmType(llvm::LLVMContext &ctx) {
    return llvm::PointerType::getUnqual(type->llvmType(ctx));
}

bool llvmTypeEq(llvm::Value *v, llvm::Type *t) {
    return v->getType()->getPointerTo() == t->getPointerTo();
}

std::string llvmTypeStr(llvm::Type *t) {
    std::string _s;
    llvm::raw_string_ostream s(_s);
    t->getPointerTo()->print(s);
    return s.str();
}

llvm::Value* tryCast(CompileContext& ctx, llvm::Value *v, llvm::Type *t) {
    if (v->getType()->getPointerTo() == t->getPointerTo()) return v;

    if (v->getType()->isIntegerTy()) {
        if (t->isIntegerTy()) return ctx.builder->CreateIntCast(v, t, true);
        else if (t->isFloatingPointTy()) return ctx.builder->CreateSIToFP(v, t);
        else if (t->isPointerTy()) return ctx.builder->CreateIntToPtr(v, t);
    } else if (v->getType()->isFloatingPointTy()) {
        if (t->isIntegerTy()) return ctx.builder->CreateFPToSI(v, t);
        else if (t->isFloatingPointTy()) return ctx.builder->CreateFPCast(v, t);
    } else if (v->getType()->isPointerTy()) {
        if (t->isIntegerTy()) return ctx.builder->CreatePtrToInt(v, t);
        else if (t->isPointerTy()) return ctx.builder->CreatePointerCast(v, t);
    }
    return nullptr;
}

llvm::Value* cast(CompileContext& ctx, llvm::Value *v, llvm::Type *t) {
    llvm::Value *v1 = tryCast(ctx, v, t);
    if (!v1) error(ERROR_COMPILER, "unable to create cast from '" + llvmTypeStr(v->getType()) + "' to '" + llvmTypeStr(t) + "'");
    return v1;
}

llvm::Value* createLogicalVal(CompileContext& ctx, llvm::Value *v) {
    if (llvmTypeEq(v, llvm::Type::getInt1Ty(ctx.mod->getContext())))
        return v;
    else if (v->getType()->isIntegerTy())
        return ctx.builder->CreateICmpNE(v, cast(ctx, constInt(ctx, 0), v->getType()));
    else if (v->getType()->isFloatingPointTy())
        return ctx.builder->CreateFCmpUNE(v, cast(ctx, constFP(ctx, 0), v->getType()));
    else if (v->getType()->isPointerTy())
        return ctx.builder->CreateICmpNE(
            cast(ctx, v, llvm::Type::getInt32Ty(ctx.mod->getContext())),
            constInt(ctx, 0)
        );

    error(ERROR_COMPILER, "unable to create logical value");

    return nullptr;
}

llvm::Value* IntExpr::llvmValue(CompileContext& ctx) {
    return constInt(ctx, val);
}

llvm::Value* FloatExpr::llvmValue(CompileContext& ctx) {
    return constFP(ctx, val);
}

llvm::Value* IdExpr::llvmValue(CompileContext& ctx) {
    if (ctx.vars.count(val) <= 0)
        error(ERROR_COMPILER, "undefined reference to '" + val + "'");
    return ctx.builder->CreateLoad(ctx.vars[val].first, ctx.vars[val].second);
}

llvm::Value* UExpr::llvmValue(CompileContext& ctx) {
    llvm::Value *v = expr->llvmValue(ctx);

    switch (type) {
    case BINEXPR_ADD: return v;
    case BINEXPR_SUB: {
        if (v->getType()->isIntegerTy())
            return ctx.builder->CreateSub(cast(ctx, constInt(ctx, 0), v->getType()), v);
        else if (v->getType()->isFloatingPointTy())
            return ctx.builder->CreateFSub(cast(ctx, constFP(ctx, 0), v->getType()), v);
    }
    case BINEXPR_NOT: {
        return ctx.builder->CreateNot(createLogicalVal(ctx, expr->llvmValue(ctx)));
    }
    default: ;
    }

    error(ERROR_COMPILER, "unknown type name in unary expression");

    return nullptr;
}

llvm::Value* BinExpr::llvmValue(CompileContext& ctx) {
    llvm::Value *leftV = left->llvmValue(ctx);
    llvm::Value *rightV = right->llvmValue(ctx);

    if (type >= BINEXPR_LOR && type <= BINEXPR_LXOR) {
        leftV = createLogicalVal(ctx, leftV);
        rightV = createLogicalVal(ctx, rightV);

        switch (type) {
        case BINEXPR_LOR:   return ctx.builder->CreateOr(leftV, rightV);
        case BINEXPR_LAND:  return ctx.builder->CreateAnd(leftV, rightV);
        case BINEXPR_LXOR:  return ctx.builder->CreateXor(leftV, rightV);
        default: ;
        }
    } else if (leftV->getType()->isIntegerTy() && rightV->getType()->isIntegerTy()) {
        llvm::Type *calcIntType = llvm::Type::getInt64Ty(ctx.mod->getContext());

        leftV = cast(ctx, leftV, calcIntType);
        rightV = cast(ctx, rightV, calcIntType);
    } else if (leftV->getType()->isFloatingPointTy() && rightV->getType()->isFloatingPointTy()) {
        llvm::Type *calcFPType = llvm::Type::getDoubleTy(ctx.mod->getContext());

        leftV = cast(ctx, leftV, calcFPType);
        rightV = cast(ctx, rightV, calcFPType);
    } else error(ERROR_COMPILER, "binary expression operand types do not match");

    switch (type) {
    case BINEXPR_OR:    return ctx.builder->CreateOr(leftV, rightV);
    case BINEXPR_AND:   return ctx.builder->CreateAnd(leftV, rightV);
    case BINEXPR_XOR:   return ctx.builder->CreateXor(leftV, rightV);
    default: ;
    }

    if (leftV->getType()->isIntegerTy()) {
        switch (type) {
        case BINEXPR_ADD:   return ctx.builder->CreateAdd(leftV, rightV);
        case BINEXPR_SUB:   return ctx.builder->CreateSub(leftV, rightV);
        case BINEXPR_MUL:   return ctx.builder->CreateMul(leftV, rightV);
        case BINEXPR_DIV:   return ctx.builder->CreateSDiv(leftV, rightV);
        case BINEXPR_MOD:   return ctx.builder->CreateSRem(leftV, rightV);

        case BINEXPR_EQ:    return ctx.builder->CreateICmpEQ(leftV, rightV);
        case BINEXPR_LT:    return ctx.builder->CreateICmpULT(leftV, rightV);
        case BINEXPR_GT:    return ctx.builder->CreateICmpUGT(leftV, rightV);
        case BINEXPR_LTEQ:  return ctx.builder->CreateICmpULE(leftV, rightV);
        case BINEXPR_GTEQ:  return ctx.builder->CreateICmpUGE(leftV, rightV);
        default: ;
        }
    } else if (leftV->getType()->isFloatingPointTy()) {
        switch (type) {
        case BINEXPR_ADD:   return ctx.builder->CreateFAdd(leftV, rightV);
        case BINEXPR_SUB:   return ctx.builder->CreateFSub(leftV, rightV);
        case BINEXPR_MUL:   return ctx.builder->CreateFMul(leftV, rightV);
        case BINEXPR_DIV:   return ctx.builder->CreateFDiv(leftV, rightV);
        case BINEXPR_MOD:   return ctx.builder->CreateFRem(leftV, rightV);

        case BINEXPR_EQ:    return ctx.builder->CreateFCmpUEQ(leftV, rightV);
        case BINEXPR_LT:    return ctx.builder->CreateFCmpULT(leftV, rightV);
        case BINEXPR_GT:    return ctx.builder->CreateFCmpUGT(leftV, rightV);
        case BINEXPR_LTEQ:  return ctx.builder->CreateFCmpULE(leftV, rightV);
        case BINEXPR_GTEQ:  return ctx.builder->CreateFCmpUGE(leftV, rightV);
        default: ;
        }
    }

    error(ERROR_COMPILER, "unknown type name in binary expression");

    return nullptr;
}

llvm::Value* IfExpr::llvmValue(CompileContext& ctx) {
    llvm::Value *condV = createLogicalVal(ctx, cond->llvmValue(ctx));

    llvm::Value *trueV = exprTrue->llvmValue(ctx);
    llvm::Value *falseV = exprFalse->llvmValue(ctx);

    if (trueV->getType()->getPointerTo() != falseV->getType()->getPointerTo())
        error(ERROR_COMPILER, "conditional expression operand types do not match");

    return ctx.builder->CreateSelect(condV, trueV, falseV);
}

llvm::Value* ArrayExpr::llvmValue(CompileContext& ctx) {
    std::vector<llvm::Value*> elements;
    for (auto& expr : exprs) elements.push_back(expr->llvmValue(ctx));

    llvm::Type *t = elements.size() == 0
            ? llvm::Type::getVoidTy(ctx.mod->getContext())
            : elements[0]->getType();
    llvm::Type *arrT = llvm::ArrayType::get(t, elements.size());
    llvm::AllocaInst *arrayAlloca = ctx.builder->CreateAlloca(arrT);

    for (size_t i = 0; i < elements.size(); i++) {
        llvm::Value *v = tryCast(ctx, elements[i], t);
        if (!v) error(ERROR_COMPILER, "element types do not match inside of homogenous array");

        llvm::Value *ptr = ctx.builder->CreateGEP(arrayAlloca, constInt(ctx, i));

        // ! this is creating an error because the 'ptr' is [n x T] and not T*
        ctx.builder->CreateStore(elements[i], ptr);
    }

    return arrayAlloca;
}

llvm::Value* PtrArrayExpr::llvmValue(CompileContext& ctx) {
    return nullptr;
}

llvm::Value* CastExpr::llvmValue(CompileContext& ctx) {
    return cast(ctx, expr->llvmValue(ctx), type->llvmType(ctx.mod->getContext()));
}

llvm::AllocaInst* createAlloca(llvm::Function *f, llvm::StringRef id, llvm::Type *type) {
    llvm::IRBuilder<> builder(&(f->getEntryBlock()), f->getEntryBlock().begin());
    return builder.CreateAlloca(type, nullptr, id);
} 

llvm::Value* Function::llvmValue(CompileContext& ctx) {
    std::vector<llvm::Type*> ftArgs;
    for (auto& arg : args)
        ftArgs.push_back(arg.first->llvmType(ctx.mod->getContext()));

    llvm::Function *f = ctx.mod->getFunction(id);

    if (f) {
        if (ftArgs.size() != f->arg_size())
            error(ERROR_COMPILER, "invalid redefenition of function '" + id + "'");

        for (size_t i = 0; i < ftArgs.size(); i++)
            if (ftArgs.at(i)->getPointerTo() != f->getArg(i)->getType()->getPointerTo())
                error(ERROR_COMPILER, "invalid redefenition of function '" + id + "'");
        
        f->setLinkage(llvm::Function::ExternalLinkage);
    } else {
        llvm::FunctionType *ft =
            llvm::FunctionType::get(retType->llvmType(ctx.mod->getContext()), ftArgs, false);

        f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, id, ctx.mod);
    }

    if (body.size() <= 0) {
        size_t i = 0;
        for (auto& arg : f->args()) arg.setName(args[i].second);
        f->setLinkage(llvm::Function::InternalLinkage);
        return f;
    }

    ctx.builder->SetInsertPoint(llvm::BasicBlock::Create(ctx.mod->getContext(), "", f));

    size_t i = 0;
    for (auto& arg : f->args()) {
        if (args[i].second.size() <= 0)
            error(ERROR_COMPILER, "function definiton with body must have named arguments");

        arg.setName(args[i].second);

        llvm::AllocaInst *alloca = createAlloca(f, arg.getName(), ftArgs[i]);

        ctx.builder->CreateStore(&arg, alloca);

        ctx.vars[args[i++].second] = std::pair<llvm::Type*, llvm::Value*>(arg.getType(), alloca);
    }

    for (size_t i = 0; i < body.size() - 1; i++)
        body[i]->llvmValue(ctx);
    
    ctx.builder->CreateRet(cast(ctx, body[body.size() - 1]->llvmValue(ctx), f->getReturnType()));
    
    ctx.vars.clear();

    if (llvm::verifyFunction(*f))
        error(ERROR_COMPILER, "error in function '" + id + "'");

    return f;
}

llvm::Value* FunctionCall::llvmValue(CompileContext& ctx) {
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
    for (auto& arg : f->args()) {
        if (callArgs[i]->getType()->getPointerTo() != arg.getType()->getPointerTo()) {
            error(ERROR_COMPILER, "invalid argument type for function `" + calleeId
                  + "' (expected: '" + llvmTypeStr(arg.getType()) + "', got: '" + llvmTypeStr(callArgs[i]->getType()) + "')");
        }
        i++;
    }

    llvm::CallInst *call = ctx.builder->CreateCall(f, callArgs);

    return call;
}
