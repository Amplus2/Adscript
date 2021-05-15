#include "ast.hh"
#include "utils.hh"
#include "compiler.hh"

#include <iostream>

#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/Utils/Cloning.h>

// AST str methods

std::string IntExpr::str() {
    return std::to_string(val);
}

std::string FloatExpr::str() {
    return std::to_string(val);
}

std::string CharExpr::str() {
    return "\\" + std::string(1, (char) val);
}

std::string IdExpr::str() {
    return val;
}

std::string StrExpr::str() {
    return val;
}

std::string UExpr::str() {
    return std::string() + "UExpr: { "
        + "op: " + betToStr(type)
        + ", expr: " + expr->str()
        + " }";
}

std::string BinExpr::str() {
    return std::string() + "BinExpr: { "
        + "op: " + betToStr(type)
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

std::string VarExpr::str() {
    return std::string() + "VarExpr: {"
        + "id: '" + id + "'";
        + ", val: " + val->str()
        + " }";
}

std::string SetExpr::str() {
    return std::string() + "SetExpr: {"
        + "ptr: " + ptr->str();
        + ", val: " + val->str()
        + " }";
}

std::string RefExpr::str() {
    return std::string() + "RefExpr: {"
        + "val: " + val->str()
        + " }";
}

std::string DerefExpr::str() {
    return std::string() + "DerefExpr: {"
        + "ptr: " + ptr->str()
        + " }";
}

std::string HeGetExpr::str() {
    return std::string() + "HeGetExpr: {"
        + "type: " + type->str()
        + ", ptr: " + ptr->str()
        + ", idx: " + idx->str()
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

std::string CallExpr::str() {
    return std::string() + "FunctionCall: { "
        + "calle: " + callee->str()
        + ", args: " + exprVectorToStr(args)
        + " }";
}

// AST llvm methods

llvm::Type* PrimType::llvmType(llvm::LLVMContext &ctx) {
    switch (type) {
    case TYPE_VOID:     return llvm::Type::getVoidTy(ctx);
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
    if (quantity == 0)
        error(ERROR_COMPILER, "quantity of pointer type cannot be zero");
    auto t = type->llvmType(ctx)->getPointerTo();
    for (int i = 1; i < quantity; i++) t = t->getPointerTo();
    return t;
}

llvm::Value* IntExpr::llvmValue(CompileContext& ctx) {
    return constInt(ctx, val);
}

llvm::Value* FloatExpr::llvmValue(CompileContext& ctx) {
    return constFP(ctx, val);
}

llvm::Value* CharExpr::llvmValue(CompileContext& ctx) {
    return llvm::ConstantInt::get(llvm::Type::getInt8Ty(ctx.mod->getContext()), val);
}

llvm::Value* IdExpr::llvmValue(CompileContext& ctx) {
    auto var = ctx.getVar(val);
    if (!var.second)
        error(ERROR_COMPILER, "undefined reference to '" + val + "'");
    if (ctx.needsRef) return var.second;
    return ctx.builder->CreateLoad(var.first, var.second);
}

llvm::Value* StrExpr::llvmValue(CompileContext& ctx) {
    return ctx.builder->CreateGlobalStringPtr(val);
}

llvm::Value* UExpr::llvmValue(CompileContext& ctx) {
    auto v = expr->llvmValue(ctx);

    switch (type) {
    case BINEXPR_ADD: return v;
    case BINEXPR_SUB: {
        if (v->getType()->isIntegerTy())
            return ctx.builder->CreateSub(
                cast(ctx, constInt(ctx, 0), v->getType()), v);
        else if (v->getType()->isFloatingPointTy())
            return ctx.builder->CreateFSub(
                cast(ctx, constFP(ctx, 0), v->getType()), v);
    }
    case BINEXPR_NOT: {
        return ctx.builder->CreateNot(
            createLogicalVal(ctx, expr->llvmValue(ctx)));
    }
    default: ;
    }

    error(ERROR_COMPILER, "unknown type name in unary expression");

    return nullptr;
}

llvm::Value* BinExpr::llvmValue(CompileContext& ctx) {
    auto lv = left->llvmValue(ctx);
    auto rv = right->llvmValue(ctx);

    if (type >= BINEXPR_LOR && type <= BINEXPR_LXOR) {
        lv = createLogicalVal(ctx, lv);
        rv = createLogicalVal(ctx, rv);

        switch (type) {
        case BINEXPR_LOR:   return ctx.builder->CreateOr(lv, rv);
        case BINEXPR_LAND:  return ctx.builder->CreateAnd(lv, rv);
        case BINEXPR_LXOR:  return ctx.builder->CreateXor(lv, rv);
        default: ;
        }
    } else {
        auto lvT = lv->getType();
        auto rvT = rv->getType();

        if (lvT->isIntegerTy() && rvT->isIntegerTy()) {
            auto calcIntType = llvm::Type::getInt64Ty(ctx.mod->getContext());

            lv = cast(ctx, lv, calcIntType);
            rv = cast(ctx, rv, calcIntType);
        } else if (lvT->isFloatingPointTy() && rvT->isFloatingPointTy()) {
            auto calcFPType = llvm::Type::getDoubleTy(ctx.mod->getContext());

            lv = cast(ctx, lv, calcFPType);
            rv = cast(ctx, rv, calcFPType);
        } else {
            error(ERROR_COMPILER,
                "binary expression operand types do not match");
        }
    }

    switch (type) {
    case BINEXPR_OR:        return ctx.builder->CreateOr(lv, rv);
    case BINEXPR_AND:       return ctx.builder->CreateAnd(lv, rv);
    case BINEXPR_XOR:       return ctx.builder->CreateXor(lv, rv);
    default: ;
    }

    if (lv->getType()->isIntegerTy()) {
        switch (type) {
        case BINEXPR_ADD:   return ctx.builder->CreateAdd(lv, rv);
        case BINEXPR_SUB:   return ctx.builder->CreateSub(lv, rv);
        case BINEXPR_MUL:   return ctx.builder->CreateMul(lv, rv);
        case BINEXPR_DIV:   return ctx.builder->CreateSDiv(lv, rv);
        case BINEXPR_MOD:   return ctx.builder->CreateSRem(lv, rv);

        case BINEXPR_EQ:    return ctx.builder->CreateICmpEQ(lv, rv);
        case BINEXPR_LT:    return ctx.builder->CreateICmpULT(lv, rv);
        case BINEXPR_GT:    return ctx.builder->CreateICmpUGT(lv, rv);
        case BINEXPR_LTEQ:  return ctx.builder->CreateICmpULE(lv, rv);
        case BINEXPR_GTEQ:  return ctx.builder->CreateICmpUGE(lv, rv);
        default: ;
        }
    } else if (lv->getType()->isFloatingPointTy()) {
        switch (type) {
        case BINEXPR_ADD:   return ctx.builder->CreateFAdd(lv, rv);
        case BINEXPR_SUB:   return ctx.builder->CreateFSub(lv, rv);
        case BINEXPR_MUL:   return ctx.builder->CreateFMul(lv, rv);
        case BINEXPR_DIV:   return ctx.builder->CreateFDiv(lv, rv);
        case BINEXPR_MOD:   return ctx.builder->CreateFRem(lv, rv);

        case BINEXPR_EQ:    return ctx.builder->CreateFCmpUEQ(lv, rv);
        case BINEXPR_LT:    return ctx.builder->CreateFCmpULT(lv, rv);
        case BINEXPR_GT:    return ctx.builder->CreateFCmpUGT(lv, rv);
        case BINEXPR_LTEQ:  return ctx.builder->CreateFCmpULE(lv, rv);
        case BINEXPR_GTEQ:  return ctx.builder->CreateFCmpUGE(lv, rv);
        default: ;
        }
    }

    error(ERROR_COMPILER, "unknown type name in binary expression");

    return nullptr;
}

llvm::Value* IfExpr::llvmValue(CompileContext& ctx) {
    auto condV = createLogicalVal(ctx, cond->llvmValue(ctx));

    auto trueV = exprTrue->llvmValue(ctx);
    auto falseV = exprFalse->llvmValue(ctx);

    if (trueV->getType()->getPointerTo() != falseV->getType()->getPointerTo())
        error(ERROR_COMPILER, 
            "conditional expression operand types do not match");

    return ctx.builder->CreateSelect(condV, trueV, falseV);
}

llvm::Value* ArrayExpr::llvmValue(CompileContext& ctx) {
    std::vector<llvm::Value*> elements;
    for (auto& expr : exprs) elements.push_back(expr->llvmValue(ctx));

    auto t = elements.size() == 0
            ? llvm::Type::getVoidTy(ctx.mod->getContext())
            : elements[0]->getType();
    auto arrT = llvm::ArrayType::get(t, elements.size());
    auto arr = (llvm::Value*) ctx.builder->CreateAlloca(arrT);
    arr = ctx.builder->CreateGEP(arr, { constInt(ctx, 0), constInt(ctx, 0) });

    for (size_t i = 0; i < elements.size(); i++) {
        auto v = tryCast(ctx, elements[i], t);
        if (!v)
            error(ERROR_COMPILER,
                "element types do not match inside of homogenous array");

        auto ptr = ctx.builder->CreateGEP(arr, constInt(ctx, i));

        ctx.builder->CreateStore(v, ptr);
    }

    return arr;
}

llvm::Value* PtrArrayExpr::llvmValue(CompileContext& ctx) {
    std::vector<llvm::Value*> elements;
    for (auto& expr : exprs) {
        auto v = expr->llvmValue(ctx);
        auto ptr = ctx.builder->CreateAlloca(v->getType());
        ctx.builder->CreateStore(v, ptr);
        elements.push_back(ptr);
    }

    auto t = llvm::Type::getVoidTy(ctx.mod->getContext())->getPointerTo();
    auto arrT = llvm::ArrayType::get(t, elements.size());
    auto arr = (llvm::Value*) ctx.builder->CreateAlloca(arrT);
    arr = ctx.builder->CreateGEP(arr, { constInt(ctx, 0), constInt(ctx, 0) });

    for (size_t i = 0; i < elements.size(); i++) {
        auto ptr = ctx.builder->CreateGEP(arr, constInt(ctx, i));

        ctx.builder->CreateStore(cast(ctx, elements[i], t), ptr);
    }

    return arr;
}

llvm::Value* VarExpr::llvmValue(CompileContext& ctx) {
    if (ctx.isVar(id))
        error(ERROR_COMPILER, "var '" + id + "' already defined");

    auto v = val->llvmValue(ctx);
    auto alloca = ctx.builder->CreateAlloca(v->getType());

    ctx.builder->CreateStore(v, alloca);

    ctx.localVars[id] = { v->getType(), alloca };

    return alloca;
}

llvm::Value* SetExpr::llvmValue(CompileContext& ctx) {
    llvm::Value* ptr = this->ptr->llvmValue(ctx);
    if (!ptr->getType()->isPointerTy())
        error(ERROR_COMPILER,
            "expected pointer type for set expression as first argument");
    
    auto valT = ptr->getType()->getPointerElementType();
    auto val = this->val->llvmValue(ctx);
    auto val1 = tryCast(ctx, val, valT);
    if (!val1)
        error(ERROR_COMPILER,
            "pointer of set instruction is unable to store (expected: "
            + llvmTypeStr(valT) + ", got: " + llvmTypeStr(val->getType())
            + ")");

    return val1;
}

llvm::Value* RefExpr::llvmValue(CompileContext& ctx) {
    ctx.needsRef = true;
    llvm::Value *v = val->llvmValue(ctx);
    ctx.needsRef = false;
    return v;
}

llvm::Value* DerefExpr::llvmValue(CompileContext& ctx) {
    auto ptr = this->ptr->llvmValue(ctx);

    if (!ptr->getType()->isPointerTy())
        error(ERROR_COMPILER, "expected pointer type for deref expression");

    return ctx.builder->CreateLoad(
        ptr->getType()->getPointerElementType(), ptr);
}

llvm::Value* HeGetExpr::llvmValue(CompileContext& ctx) {
    auto ptr = this->ptr->llvmValue(ctx);

    auto t = ptr->getType();
    if (!(t->isPointerTy() && t->getPointerElementType()->isPointerTy()))
        error(ERROR_COMPILER,
            "expected doubled pointer type for"
            "heget expression as second argument");
    
    auto idxT = llvm::Type::getInt64Ty(ctx.mod->getContext());
    auto idx = tryCast(ctx, this->idx->llvmValue(ctx), idxT);
    if (!idx)
        error(ERROR_COMPILER,
            "expected integer type fot heget expression as third argument");

    t = type->llvmType(ctx.mod->getContext())->getPointerTo();
    
    ptr = ctx.builder->CreateGEP(ptr, idx);
    ptr = ctx.builder->CreatePointerCast(
        ptr, t->getPointerTo()->getPointerTo());
    ptr = ctx.builder->CreateLoad(t->getPointerTo(), ptr);

    return ctx.builder->CreateLoad(t, ptr);
}

llvm::Value* CastExpr::llvmValue(CompileContext& ctx) {
    return cast(ctx, expr->llvmValue(ctx),
        type->llvmType(ctx.mod->getContext()));
}

llvm::Value* Function::llvmValue(CompileContext& ctx) {
    std::vector<llvm::Type*> ftArgs;
    for (auto& arg : args)
        ftArgs.push_back(arg.first->llvmType(ctx.mod->getContext()));

    auto f = ctx.mod->getFunction(id);

    if (f) {
        if (ftArgs.size() != f->arg_size())
            error(ERROR_COMPILER,
                "invalid redefenition of function '" + id + "'");

        for (size_t i = 0; i < ftArgs.size(); i++) {
            bool b = ftArgs.at(i)->getPointerTo()
                != f->getArg(i)->getType()->getPointerTo();
            if (b)
                error(ERROR_COMPILER,
                    "invalid redefenition of function '" + id + "'");
        }
                
    } else {
        auto ft = llvm::FunctionType::get(
            retType->llvmType(ctx.mod->getContext()), ftArgs, false);

        f = llvm::Function::Create(
            ft, llvm::Function::ExternalLinkage, id, ctx.mod);
    }

    if (body.size() <= 0) {
        size_t i = 0;
        for (auto& arg : f->args()) arg.setName(args[i].second);
        return f;
    }

    ctx.builder->SetInsertPoint(
        llvm::BasicBlock::Create(ctx.mod->getContext(), "", f));

    size_t i = 0;
    for (auto& arg : f->args()) {
        if (args[i].second.size() <= 0)
            error(ERROR_COMPILER, 
                "function definiton with body must have named arguments");

        arg.setName(args[i].second);

        auto alloca = createAlloca(f, ftArgs[i]);

        ctx.builder->CreateStore(&arg, alloca);

        ctx.localVars[args[i++].second] = { arg.getType(), alloca };
    }

    for (size_t i = 0; i < body.size() - 1; i++)
        body[i]->llvmValue(ctx);

    ctx.builder->CreateRet(
        cast(ctx, body[body.size() - 1]->llvmValue(ctx), f->getReturnType()));

    ctx.localVars.clear();

    if (llvm::verifyFunction(*f)) f->print(llvm::errs());

    if (llvm::verifyFunction(*f))
        error(ERROR_COMPILER, "error in function '" + id + "'");

    return f;
}

llvm::Value* CallExpr::llvmValue(CompileContext& ctx) {
    IdExpr *id = nullptr;
    if (callee->isIdExpr()) id = (IdExpr*) callee;

    if (!id || ctx.isVar(id->getVal())) {
        if (args.size() != 1)
            error(ERROR_COMPILER,
                "expected exactly 1 argument for pointer-index-call");

        auto ptr = callee->llvmValue(ctx);
        if (!ptr->getType()->isPointerTy())
            error(ERROR_COMPILER,
                "pointer-index-calls only work with pointers");

        auto idxT = llvm::Type::getInt64Ty(ctx.mod->getContext());
        auto idx = tryCast(ctx, args[0]->llvmValue(ctx), idxT);
        if (!idx)
            error(ERROR_COMPILER, "argument in pointer-index-call "
                "must be convertable to an integer");

        llvm::Value *v = ctx.builder->CreateGEP(ptr, idx);

        if (ctx.needsRef) return v;
        return ctx.builder->CreateLoad(
            v->getType()->getPointerElementType(), v);
    }

    auto f = ctx.mod->getFunction(id->getVal());

    if (!f) error(ERROR_COMPILER,
        "undefined reference to '" + id->getVal() + "'");

    if (args.size() > f->arg_size())
        error(ERROR_COMPILER, "too many arguments for function '"
            + id->getVal() + "'");
    else if (args.size() < f->arg_size())
        error(ERROR_COMPILER, "too few arguments for function '"
            + id->getVal() + "'");

    std::vector<llvm::Value*> callArgs;

    size_t i = 0;
    for (auto& arg : f->args()) {
        auto v = args[i++]->llvmValue(ctx);
        auto v1 = tryCast(ctx, v, arg.getType());
        if (!v1)
            error(ERROR_COMPILER, "invalid argument type for function '"
                + id->getVal() + "' (expected: '"
                + llvmTypeStr(arg.getType()) + "', got: '"
                + llvmTypeStr(v->getType()) + "')");
        callArgs.push_back(v1);
    }

    auto call = ctx.builder->CreateCall(f, callArgs);

    return call;
}
