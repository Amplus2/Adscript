#include "ast.hh"
#include "compiler.hh"
#include "utils.hh"

#include <iostream>

#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/Utils/Cloning.h>

using namespace Adscript;

// * STRING GENERATION METHODS

std::u32string AST::PrimType::str() {
  switch (type) {
  case TYPE_I8:
    return U"i8";
  case TYPE_I16:
    return U"i16";
  case TYPE_I32:
    return U"i32";
  case TYPE_I64:
    return U"i64";
  case TYPE_FLOAT:
    return U"float";
  case TYPE_DOUBLE:
    return U"double";
  default:
    return U"err";
  }
}

std::u32string AST::PointerType::str() {
  return std::u32string() + U"PointerType: " + type->str();
}

std::u32string AST::StructType::str() {
  return std::u32string() + U"StructType: { " + U"attrs: " +
         attrMapToStr(attrs) + U" }";
}

std::u32string AST::IdentifierType::str() {
  return std::u32string() + U"IdentifierType: { " + U"id: " + std::stou32(id) +
         U" }";
}

std::u32string AST::Int::str() { return std::stou32(std::to_string(val)); }

std::u32string AST::Float::str() { return std::stou32(std::to_string(val)); }

std::u32string AST::Char::str() { return U"\\" + std::u32string(1, (char)val); }

std::u32string AST::Identifier::str() { return std::stou32(val); }

std::u32string AST::String::str() { return val; }

std::u32string AST::UExpr::str() {
  return std::u32string() + U"UExpr: { " + U"op: " + AST::betToStr(type) +
         U", expr: " + expr->str() + U" }";
}

std::u32string AST::BinExpr::str() {
  return std::u32string() + U"BinExpr: { " + U"op: " + AST::betToStr(type) +
         U", left: " + left->str() + U", right: " + right->str() + U" }";
}

std::u32string AST::If::str() {
  return std::u32string() + U"If: {" + U"cond: " + cond->str() +
         U", exprTrue: " + exprTrue->str() + U", exprFalse: " +
         exprFalse->str() + U" }";
}

std::u32string AST::HoArray::str() {
  return std::u32string() + U"HoArray: {" + U"size: " +
         std::stou32(std::to_string(exprs.size())) + U", exprs: " +
         AST::exprVectorToStr(exprs) + U" }";
}

std::u32string AST::HeArray::str() {
  return std::u32string() + U"HeArray: {" + U"size: " +
         std::stou32(std::to_string(exprs.size())) + U", exprs: " +
         AST::exprVectorToStr(exprs) + U" }";
}

std::u32string AST::Deft::str() {
  return std::u32string() + U"Deft: {" + U"id: '" + std::stou32(id) + U"'";
  +U", type: " + type->str() + U" }";
}

std::u32string AST::Let::str() {
  return std::u32string() + U"Def: {" + U"id: '" + std::stou32(id) + U"'";
  +U", val: " + val->str() + U" }";
}

std::u32string AST::Var::str() {
  return std::u32string() + U"Var: {" + U"id: '" + std::stou32(id) + U"'";
  +U", val: " + val->str() + U" }";
}

std::u32string AST::Set::str() {
  return std::u32string() + U"Set: {" + U"ptr: " + ptr->str();
  +U", val: " + val->str() + U" }";
}

std::u32string AST::SetPtr::str() {
  return std::u32string() + U"SetPtr: {" + U"ptr: " + ptr->str();
  +U", val: " + val->str() + U" }";
}

std::u32string AST::Ref::str() {
  return std::u32string() + U"Ref: {" + U"val: " + val->str() + U" }";
}

std::u32string AST::Deref::str() {
  return std::u32string() + U"Deref: {" + U"ptr: " + ptr->str() + U" }";
}

std::u32string AST::HeGet::str() {
  return std::u32string() + U"HeGet: {" + U"type: " + type->str() + U", ptr: " +
         ptr->str() + U", idx: " + idx->str() + U" }";
}

std::u32string AST::Cast::str() {
  return std::u32string() + U"Cast { " + U"type: " + type->str() + U", expr: " +
         expr->str() + U" }";
}

std::u32string AST::Function::str() {
  return std::u32string() + U"Function: { " + U"id: '" + std::stou32(id) +
         U"'" + U", args: " + AST::argVectorToStr(args) + U", type: " +
         retType->str() + U", body: " + AST::exprVectorToStr(body) + U" }";
}

std::u32string AST::Lambda::str() {
  return std::u32string() + U"Lambda: { " + U"args: " +
         AST::argVectorToStr(args) + U", type: " + retType->str() +
         U", body: " + AST::exprVectorToStr(body) + U" }";
}

std::u32string AST::Call::str() {
  return std::u32string() + U"FunctionCall: { " + U"calle: " + callee->str() +
         U", args: " + AST::exprVectorToStr(args) + U" }";
}

// * LLVM METHODS

llvm::Type *AST::PrimType::llvmType(Compiler::Context &ctx) {
  switch (type) {
  case TYPE_I8:
    return llvm::Type::getInt8Ty(ctx.mod->getContext());
  case TYPE_I16:
    return llvm::Type::getInt16Ty(ctx.mod->getContext());
  case TYPE_I32:
    return llvm::Type::getInt32Ty(ctx.mod->getContext());
  case TYPE_I64:
    return llvm::Type::getInt64Ty(ctx.mod->getContext());
  case TYPE_FLOAT:
    return llvm::Type::getFloatTy(ctx.mod->getContext());
  case TYPE_DOUBLE:
    return llvm::Type::getDoubleTy(ctx.mod->getContext());
  default:
    Error::compiler(U"unknown type name");
  }
  return nullptr;
}

llvm::Type *AST::PointerType::llvmType(Compiler::Context &ctx) {
  if (quantity == 0)
    Error::compiler(U"quantity of pointer type cannot be zero");
  auto t = type->llvmType(ctx)->getPointerTo();
  for (int i = 1; i < quantity; i++)
    t = t->getPointerTo();
  return t;
}

llvm::Type *AST::StructType::llvmType(Compiler::Context &ctx) {
  std::vector<llvm::Type *> llvmAttrs;

  for (auto &attr : attrs)
    llvmAttrs.push_back(attr.second->llvmType(ctx));

  return llvm::StructType::get(ctx.mod->getContext(), llvmAttrs);
}

llvm::Type *AST::IdentifierType::llvmType(Compiler::Context &ctx) {
  if (!ctx.isType(id))
    Error::compiler(U"undefined reference to '" + std::stou32(id) + U"'");

  return ctx.types[id];
}

llvm::Value *AST::Int::llvmValue(Compiler::Context &ctx) {
  return Compiler::constInt(ctx, val);
}

llvm::Value *AST::Float::llvmValue(Compiler::Context &ctx) {
  return Compiler::constFP(ctx, val);
}

llvm::Value *AST::Char::llvmValue(Compiler::Context &ctx) {
  return llvm::ConstantInt::get(llvm::Type::getInt8Ty(ctx.mod->getContext()),
                                val);
}

llvm::Value *AST::Identifier::llvmValue(Compiler::Context &ctx) {
  // get var out of context
  if (ctx.isVar(val)) {
    auto var = ctx.varScope[val];

    // return alloca if reference is needed
    if (ctx.needsRef)
      return var.second;

    // return load to alloca
    return ctx.builder->CreateLoad(var.first, var.second);
  } else if (ctx.isFinal(val)) {
    return ctx.needsRef ? ctx.finalScope[val].second
                        : ctx.builder->CreateLoad(ctx.finalScope[val].first,
                                                  ctx.finalScope[val].second);
  } else if (ctx.isFunction(val)) {
    return ctx.mod->getFunction(val);
  }

  Error::compiler(U"undefined reference to '" + std::stou32(val) + U"'");

  return nullptr;
}

llvm::Value *AST::String::llvmValue(Compiler::Context &ctx) {
  auto charT = Utils::isAscii(val)
                   ? llvm::Type::getInt8Ty(ctx.mod->getContext())
                   : llvm::Type::getInt32Ty(ctx.mod->getContext());

  // create llvm value vector for array elements
  std::vector<llvm::Constant *> chars;

  // add characters to the 'elements' vector
  for (auto &c : val)
    chars.push_back(llvm::ConstantInt::get(charT, c));

  // add NULL terminator to chars
  chars.push_back(llvm::ConstantInt::get(charT, 0));

  // create array type using the element type
  auto arrT = llvm::ArrayType::get(charT, chars.size());

  auto initializer = llvm::ConstantArray::get(arrT, chars);

  auto arr = new llvm::GlobalVariable(
      *(ctx.mod), arrT, false, llvm::GlobalValue::PrivateLinkage, initializer);

  // assign 'arr' to the pointer to the first element of the string
  auto zero = Compiler::constInt(ctx, 0);
  return ctx.builder->CreateGEP(arr, {zero, zero});
}

llvm::Value *AST::UExpr::llvmValue(Compiler::Context &ctx) {
  // get llvm value for expr
  auto v = expr->llvmValue(ctx);

  switch (type) {
  // do nothing if the unary expression type is '+'
  case BINEXPR_ADD:
    return v;
  case BINEXPR_SUB: {
    // create 0 - v if v is of type int
    if (v->getType()->isIntegerTy())
      return ctx.builder->CreateSub(
          cast(ctx, Compiler::constInt(ctx, 0), v->getType()), v);
    // create 0.0 - v if v is of type float
    else if (v->getType()->isFloatingPointTy())
      return ctx.builder->CreateFSub(
          cast(ctx, Compiler::constFP(ctx, 0), v->getType()), v);
  }
  case BINEXPR_LNOT: {
    // create a logical value for 'v'
    v = createLogicalVal(ctx, v);
  }
  case BINEXPR_NOT: {
    // create not
    return ctx.builder->CreateNot(v);
  }
  default:;
  }

  Error::compiler(U"unknown type name in unary expression");

  return nullptr;
}

llvm::Value *AST::BinExpr::llvmValue(Compiler::Context &ctx) {
  // get llvm values
  auto lv = left->llvmValue(ctx);
  auto rv = right->llvmValue(ctx);

  // handle logical operators
  if (type >= BINEXPR_LOR && type <= BINEXPR_LXOR) {
    // create logical values for both operands
    lv = createLogicalVal(ctx, lv);
    rv = createLogicalVal(ctx, rv);

    // create instruction
    switch (type) {
    case BINEXPR_LOR:
      return ctx.builder->CreateOr(lv, rv);
    case BINEXPR_LAND:
      return ctx.builder->CreateAnd(lv, rv);
    case BINEXPR_LXOR:
      return ctx.builder->CreateXor(lv, rv);
    default:;
    }
  } else {
    auto lvT = lv->getType();
    auto rvT = rv->getType();

    if (Compiler::isNumTy(lvT) && Compiler::isNumTy(rvT)) {
      if (lvT->isFloatingPointTy() || rvT->isFloatingPointTy()) {
        // get the data type used for the binary expression
        auto calcType = llvm::Type::getDoubleTy(ctx.mod->getContext());

        // cast operands to the correct type
        lv = cast(ctx, lv, calcType);
        rv = cast(ctx, rv, calcType);

        // create instruction
        switch (type) {
        case BINEXPR_ADD:
          return ctx.builder->CreateFAdd(lv, rv);
        case BINEXPR_SUB:
          return ctx.builder->CreateFSub(lv, rv);
        case BINEXPR_MUL:
          return ctx.builder->CreateFMul(lv, rv);
        case BINEXPR_DIV:
          return ctx.builder->CreateFDiv(lv, rv);
        case BINEXPR_MOD:
          return ctx.builder->CreateFRem(lv, rv);

        case BINEXPR_EQ:
          return ctx.builder->CreateFCmpUEQ(lv, rv);
        case BINEXPR_LT:
          return ctx.builder->CreateFCmpULT(lv, rv);
        case BINEXPR_GT:
          return ctx.builder->CreateFCmpUGT(lv, rv);
        case BINEXPR_LTEQ:
          return ctx.builder->CreateFCmpULE(lv, rv);
        case BINEXPR_GTEQ:
          return ctx.builder->CreateFCmpUGE(lv, rv);
        default:;
        }
      } else {
        // get the data type used for the binary expression
        auto calcType = llvm::Type::getInt64Ty(ctx.mod->getContext());

        // cast operands to the correct type
        lv = cast(ctx, lv, calcType);
        rv = cast(ctx, rv, calcType);

        // create instruction
        switch (type) {
        case BINEXPR_ADD:
          return ctx.builder->CreateAdd(lv, rv);
        case BINEXPR_SUB:
          return ctx.builder->CreateSub(lv, rv);
        case BINEXPR_MUL:
          return ctx.builder->CreateMul(lv, rv);
        case BINEXPR_DIV:
          return ctx.builder->CreateSDiv(lv, rv);
        case BINEXPR_MOD:
          return ctx.builder->CreateSRem(lv, rv);

        case BINEXPR_EQ:
          return ctx.builder->CreateICmpEQ(lv, rv);
        case BINEXPR_LT:
          return ctx.builder->CreateICmpULT(lv, rv);
        case BINEXPR_GT:
          return ctx.builder->CreateICmpUGT(lv, rv);
        case BINEXPR_LTEQ:
          return ctx.builder->CreateICmpULE(lv, rv);
        case BINEXPR_GTEQ:
          return ctx.builder->CreateICmpUGE(lv, rv);
        default:;
        }
      }
    } else {
      // error if operand types are incompatible with another
      Error::compiler(U"incompatible operand types (left: '" +
                      Compiler::llvmTypeStr(lvT) + U"', right: '" +
                      Compiler::llvmTypeStr(rvT) + U"')");
    }
  }

  // handle bitwise operators
  switch (type) {
  case BINEXPR_OR:
    return ctx.builder->CreateOr(lv, rv);
  case BINEXPR_AND:
    return ctx.builder->CreateAnd(lv, rv);
  case BINEXPR_XOR:
    return ctx.builder->CreateXor(lv, rv);
  default:;
  }

  // error if no valid operator was provided
  Error::compiler(U"unknown type name in binary expression");

  // return anything
  return nullptr;
}

llvm::Value *AST::If::llvmValue(Compiler::Context &ctx) {
  // get condition llvm value
  auto condV = createLogicalVal(ctx, cond->llvmValue(ctx));

  auto f = ctx.builder->GetInsertBlock()->getParent();

  auto ifBB = llvm::BasicBlock::Create(ctx.mod->getContext(), "", f);
  auto elseBB = llvm::BasicBlock::Create(ctx.mod->getContext(), "", f);
  auto mergeBB = llvm::BasicBlock::Create(ctx.mod->getContext(), "", f);

  ctx.builder->CreateCondBr(condV, ifBB, elseBB);

  ctx.builder->SetInsertPoint(ifBB);
  auto trueV = exprTrue->llvmValue(ctx);

  ctx.builder->CreateBr(mergeBB);
  ifBB = ctx.builder->GetInsertBlock();

  ctx.builder->SetInsertPoint(elseBB);
  auto falseV = exprFalse->llvmValue(ctx);

  ctx.builder->CreateBr(mergeBB);
  elseBB = ctx.builder->GetInsertBlock();

  ctx.builder->SetInsertPoint(mergeBB);

  auto phiNode = ctx.builder->CreatePHI(trueV->getType(), 2);

  phiNode->addIncoming(trueV, ifBB);
  phiNode->addIncoming(falseV, elseBB);

  // error if condition types do not match
  if (trueV->getType() != falseV->getType())
    Error::compiler(U"conditional expression operand types do not match");

  // create and return llvm value
  return phiNode;
}

llvm::Value *AST::HoArray::llvmValue(Compiler::Context &ctx) {
  // create llvm value vector for array elements
  std::vector<llvm::Constant *> constants;
  std::vector<std::pair<size_t, llvm::Value *>> values;

  llvm::Type *elementT = nullptr;

  // add llvm values to the 'elements' vector
  for (size_t i = 0; i < exprs.size(); i++) {
    auto val = exprs[i]->llvmValue(ctx);

    if (!elementT)
      elementT = val->getType();

    val = Compiler::tryCast(ctx, val, elementT);

    if (!val) {
      Error::compiler(U"element types do not match in homogenous array");
    }

    if (llvm::isa<llvm::Constant>(val)) {
      constants.push_back((llvm::Constant *)val);
    } else {
      constants.push_back(llvm::UndefValue::get(elementT));
      values.push_back({i, val});
    }
  }

  if (!elementT)
    elementT = llvm::Type::getInt8Ty(ctx.mod->getContext());

  auto arrT = llvm::ArrayType::get(elementT, constants.size());

  auto initializer = llvm::ConstantArray::get(arrT, constants);

  auto arr = new llvm::GlobalVariable(
      *(ctx.mod), arrT, false, llvm::GlobalValue::PrivateLinkage, initializer);

  auto zero = Compiler::constInt(ctx, 0);
  for (auto &pair : values) {
    auto idx = Compiler::constInt(ctx, pair.first);
    auto ptr = ctx.builder->CreateInBoundsGEP(arr, {zero, idx});

    ctx.builder->CreateStore(pair.second, ptr);
  }

  return ctx.builder->CreateInBoundsGEP(arr, {zero, zero});
}

llvm::Value *AST::HeArray::llvmValue(Compiler::Context &ctx) {
  // create llvm value vector for array elements
  std::vector<llvm::Value *> elements;

  for (auto &expr : exprs) {
    // get llvm value for the element
    auto v = expr->llvmValue(ctx);

    // create pointer for storing the element's data
    auto ptr = ctx.builder->CreateAlloca(v->getType());

    // store the element's data into the pointer
    ctx.builder->CreateStore(v, ptr);

    // add the pointer to the 'elements' vector
    elements.push_back(ptr);
  }

  // create the array's element type (void*)
  auto t = llvm::Type::getVoidTy(ctx.mod->getContext())->getPointerTo();

  // create the array's type
  auto arrT = llvm::ArrayType::get(t, elements.size());

  // create the array
  auto arr = (llvm::Value *)ctx.builder->CreateAlloca(arrT);

  // assign 'arr' to the pointer to the first element of the array
  arr = ctx.builder->CreateGEP(
      arr, {Compiler::constInt(ctx, 0), Compiler::constInt(ctx, 0)});

  // store all the elements into the array
  for (size_t i = 0; i < elements.size(); i++) {
    auto ptr = ctx.builder->CreateGEP(arr, Compiler::constInt(ctx, i));

    ctx.builder->CreateStore(cast(ctx, elements[i], t), ptr);
  }

  // return the array
  return arr;
}

llvm::Value *AST::Deft::llvmValue(Compiler::Context &ctx) {
  // error if variable is already defined
  if (ctx.isType(id))
    Error::warning(U"data type '" + std::stou32(id) + U"' already defined");
  else if (ctx.isVar(id))
    Error::warning(U"'" + std::stou32(id) + U"' already defined as variable");
  else if (ctx.isFinal(id))
    Error::warning(U"'" + std::stou32(id) + U"' already defined as constant");
  else if (ctx.isFunction(id))
    Error::warning(U"'" + std::stou32(id) + U"' already defined as function");

  // get llvm value for the stored value
  auto t = type->llvmType(ctx);

  // add the alloca to the 'vars' map
  ctx.types[id] = t;

  // return the alloca
  return constInt(ctx, 0);
}

llvm::Value *AST::Let::llvmValue(Compiler::Context &ctx) {
  // error if variable is already defined
  if (ctx.isFinal(id))
    Error::warning(U"constant '" + std::stou32(id) + U"' already defined");
  else if (ctx.isType(id))
    Error::warning(U"'" + std::stou32(id) + U"' already defined as data type");
  else if (ctx.isVar(id))
    Error::warning(U"'" + std::stou32(id) + U"' already defined as variable");
  else if (ctx.isFunction(id))
    Error::warning(U"'" + std::stou32(id) + U"' already defined as function");

  auto v = val->llvmValue(ctx);

  // add the alloca to the 'vars' map
  ctx.finalScope[id] = {v->getType(), v};

  // return the alloca
  return constInt(ctx, 0);
}

llvm::Value *AST::Var::llvmValue(Compiler::Context &ctx) {
  // error if variable is already defined
  if (ctx.isVar(id))
    Error::compiler(U"variable '" + std::stou32(id) + U"' already defined");
  else if (ctx.isFinal(id))
    Error::warning(U"'" + std::stou32(id) + U"' already defined as constant");
  else if (ctx.isType(id))
    Error::warning(U"'" + std::stou32(id) + U"' already defined data type");
  else if (ctx.isFunction(id))
    Error::warning(U"'" + std::stou32(id) + U"' already defined as function");

  // get llvm value for the stored value
  auto v = val->llvmValue(ctx);

  // create alloca for storing the the value
  auto alloca = ctx.builder->CreateAlloca(v->getType());

  // store the value
  ctx.builder->CreateStore(v, alloca);

  // add the alloca to the 'vars' map
  ctx.varScope[id] = {v->getType(), alloca};

  // return the alloca
  return alloca;
}

llvm::Value *AST::Set::llvmValue(Compiler::Context &ctx) {
  if (ptr->isIdentifier()) {
    auto id = ((Identifier *)ptr)->getVal();
    if (ctx.isFinal(id)) {
      Error::compiler(U"unable to assign value to runtime constant");
    } else if (ctx.isVar(id)) {
      auto var = ctx.varScope[id];

      auto llvmVal = cast(ctx, val->llvmValue(ctx), var.first);

      ctx.builder->CreateStore(llvmVal, var.second);

      return llvmVal;
    } else if (ctx.isFunction(id)) {
      Error::compiler(U"'" + std::stou32(id) + U"' is defined as a function");
    }
  } else if (ptr->isPtrElementCall(ctx)) {
    bool b = ctx.needsRef;
    ctx.needsRef = true;

    auto llvmPtr = ptr->llvmValue(ctx);

    ctx.needsRef = b;

    auto llvmVal = cast(ctx, val->llvmValue(ctx),
                        llvmPtr->getType()->getPointerElementType());

    ctx.builder->CreateStore(llvmVal, llvmPtr);

    return llvmVal;
  }

  Error::compiler(U"invalid 'set' expression");

  return nullptr;
}

llvm::Value *AST::SetPtr::llvmValue(Compiler::Context &ctx) {
  // get pointer to store the value in
  llvm::Value *ptr = this->ptr->llvmValue(ctx);

  // error if the value is not going to be stored in a pointer
  if (!ptr->getType()->isPointerTy())
    Error::compiler(
        U"expected pointer type for setptr expression as first argument");

  // get the type of the pointer
  auto valT = ptr->getType()->getPointerElementType();

  // get the value to store
  auto val = this->val->llvmValue(ctx);

  // try casting the value to the pointer's element type
  auto val1 = tryCast(ctx, val, valT);

  // error if casting failed
  if (!val1 || val1->getType() != valT)
    Error::compiler(
        U"pointer of setptr instruction is unable to store (expected: " +
        Compiler::llvmTypeStr(valT) + U", got: " +
        Compiler::llvmTypeStr(val->getType()) + U")");

  ctx.builder->CreateStore(val1, ptr);

  // return stored value
  return val1;
}

llvm::Value *AST::Ref::llvmValue(Compiler::Context &ctx) {
  // get the current 'needsRef' value
  bool b = ctx.needsRef;

  // set the 'needsRef' flag
  ctx.needsRef = true;

  // get the reference value
  llvm::Value *v = val->llvmValue(ctx);

  // error if reference is no reference
  if (!v->getType()->isPointerTy())
    Error::compiler(U"failed to create reference");

  // set 'needsRef' flag to the value stored in 'b'
  ctx.needsRef = b;

  // return reference
  return v;
}

llvm::Value *AST::Deref::llvmValue(Compiler::Context &ctx) {
  auto ptr = this->ptr->llvmValue(ctx);

  if (!ptr->getType()->isPointerTy())
    Error::compiler(U"expected pointer type for deref expression");

  return ctx.builder->CreateLoad(ptr->getType()->getPointerElementType(), ptr);
}

llvm::Value *AST::HeGet::llvmValue(Compiler::Context &ctx) {
  auto ptr = this->ptr->llvmValue(ctx);

  auto t = ptr->getType();
  if (!(t->isPointerTy() && t->getPointerElementType()->isPointerTy()))
    Error::compiler(U"expected doubled pointer type for"
                    "'heget' expression as the second argument");

  auto idxT = llvm::Type::getInt64Ty(ctx.mod->getContext());
  auto idx = tryCast(ctx, this->idx->llvmValue(ctx), idxT);
  if (!idx)
    Error::compiler(
        U"expected integer type fot heget expression as third argument");

  t = type->llvmType(ctx)->getPointerTo();

  ptr = ctx.builder->CreateGEP(ptr, idx);
  ptr = ctx.builder->CreatePointerCast(ptr, t->getPointerTo()->getPointerTo());
  ptr = ctx.builder->CreateLoad(t->getPointerTo(), ptr);

  return ctx.builder->CreateLoad(t, ptr);
}

llvm::Value *AST::Cast::llvmValue(Compiler::Context &ctx) {
  return cast(ctx, expr->llvmValue(ctx), type->llvmType(ctx));
}

llvm::Value *AST::Function::llvmValue(Compiler::Context &ctx) {
  std::vector<llvm::Type *> ftArgs;
  for (auto &arg : args)
    ftArgs.push_back(arg.second->llvmType(ctx));

  auto f = ctx.mod->getFunction(id);

  if (f) {
    if (ftArgs.size() != f->arg_size())
      Error::compiler(U"invalid redefenition of function '" + std::stou32(id) +
                      U"'");

    for (size_t i = 0; i < ftArgs.size(); i++) {
      bool b = ftArgs.at(i)->getPointerTo() !=
               f->getArg(i)->getType()->getPointerTo();
      if (b)
        Error::compiler(U"invalid redefenition of function '" +
                        std::stou32(id) + U"'");
    }

  } else {
    auto ft = llvm::FunctionType::get(retType->llvmType(ctx), ftArgs, varArg);

    f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, id,
                               ctx.mod);
  }

  if (body.size() <= 0) {
    size_t i = 0;
    for (auto &arg : f->args())
      arg.setName(args[i].first);
    return f;
  }

  ctx.builder->SetInsertPoint(
      llvm::BasicBlock::Create(ctx.mod->getContext(), "", f));

  size_t i = 0;
  for (auto &arg : f->args()) {
    if (args[i].first.size() <= 0)
      Error::compiler(
          U"function definiton with body must have named arguments");

    arg.setName(args[i].first);

    auto alloca = Compiler::createAlloca(f, ftArgs[i]);

    ctx.builder->CreateStore(&arg, alloca);

    ctx.varScope[args[i++].first] = {arg.getType(), alloca};
  }

  for (size_t i = 0; i < body.size() - 1; i++)
    body[i]->llvmValue(ctx);

  auto retVal = body[body.size() - 1]->llvmValue(ctx);
  ctx.builder->CreateRet(cast(ctx, retVal, f->getReturnType()));

  ctx.varScope.clear();

  if (llvm::verifyFunction(*f)) {
    // f->print(llvm::errs());
    Error::compiler(U"error in function '" + std::stou32(id) + U"'");
  }

  ctx.runFPM(f);

  return f;
}

llvm::Value *AST::Lambda::llvmValue(Compiler::Context &ctx) {
  if (body.size() <= 0)
    Error::compiler(U"lambda expressions cannot have an empty body");

  std::vector<llvm::Type *> ftArgs;
  for (auto &arg : args)
    ftArgs.push_back(arg.second->llvmType(ctx));

  auto ft = llvm::FunctionType::get(retType->llvmType(ctx), ftArgs, varArg);

  auto f =
      llvm::Function::Create(ft, llvm::Function::PrivateLinkage, "", ctx.mod);

  auto prevBB = ctx.builder->GetInsertBlock();
  auto fnBB = llvm::BasicBlock::Create(ctx.mod->getContext(), "", f);

  ctx.builder->SetInsertPoint(fnBB);

  auto prevVars = ctx.varScope;
  ctx.varScope.clear();

  size_t i = 0;
  for (auto &arg : f->args()) {
    if (args[i].first.size() <= 0)
      Error::compiler(U"lambda expression must have named arguments");

    arg.setName(args[i].first);

    auto alloca = Compiler::createAlloca(f, ftArgs[i]);

    ctx.builder->CreateStore(&arg, alloca);

    ctx.varScope[args[i++].first] = {arg.getType(), alloca};
  }

  for (size_t i = 0; i < body.size() - 1; i++)
    body[i]->llvmValue(ctx);

  auto retVal = body[body.size() - 1]->llvmValue(ctx);
  ctx.builder->CreateRet(cast(ctx, retVal, f->getReturnType()));

  ctx.builder->SetInsertPoint(prevBB);

  ctx.varScope = prevVars;

  if (llvm::verifyFunction(*f)) {
    // f->print(llvm::errs());
    Error::compiler(U"error in lambda expression");
  }

  ctx.runFPM(f);

  return f;
}

llvm::Value *AST::Call::llvmValue(Compiler::Context &ctx) {
  Identifier *id = nullptr;
  if (callee->isIdentifier())
    id = (Identifier *)callee;

  llvm::Function *f = nullptr;

  if (callee->isLambda()) {
    f = (llvm::Function *)callee->llvmValue(ctx);
  } else if (!id && ctx.isFunction(id->getVal())) {
    f = (llvm::Function *)ctx.mod->getFunction(id->getVal());
  } else if (!id || ctx.isVar(id->getVal()) || ctx.isFinal(id->getVal())) {
    bool needsRef = ctx.needsRef;
    ctx.needsRef = false;

    auto ptr = callee->llvmValue(ctx);

    if (Compiler::isFunctionTy(ptr->getType()))
      f = (llvm::Function *)ptr;
    else if (!ptr->getType()->isPointerTy())
      Error::compiler(Compiler::llvmTypeStr(ptr->getType()) +
                      U" is not a callable type");
    else {
      if (args.size() != 1)
        Error::compiler(U"expected exactly 1 argument for pointer-index-call");

      auto idxT = llvm::Type::getInt64Ty(ctx.mod->getContext());
      auto idx = tryCast(ctx, args[0]->llvmValue(ctx), idxT);
      if (!idx)
        Error::compiler(U"argument in pointer-index-call "
                        "must be convertable to an integer");

      llvm::Value *v = ctx.builder->CreateGEP(ptr, idx);

      if (needsRef)
        return v;
      return ctx.builder->CreateLoad(v->getType()->getPointerElementType(), v);
    }
  }
  if (!f)
    f = ctx.mod->getFunction(id->getVal());

  if (!f)
    Error::compiler(U"undefined reference to '" + std::stou32(id->getVal()) +
                    U"'");

  if (!f->isVarArg()) {
    if (args.size() > f->arg_size())
      Error::compiler(U"too many arguments for function '" +
                      std::stou32(id->getVal()) + U"'");
    else if (args.size() < f->arg_size())
      Error::compiler(U"too few arguments for function '" +
                      std::stou32(id->getVal()) + U"'");
    else if (args.size() != f->arg_size())
      Error::compiler(U"invalid argument size for function '" +
                      std::stou32(id->getVal()) + U"'");
  }

  std::vector<llvm::Value *> callArgs;

  for (size_t i = 0; i < args.size(); i++) {
    auto v = args[i]->llvmValue(ctx);
    auto v1 = f->isVarArg() && i >= f->arg_size()
                  ? v
                  : tryCast(ctx, v, f->getArg(i)->getType());
    if (!v1)
      Error::compiler(
          U"invalid argument type for function '" + std::stou32(id->getVal()) +
          U"' (expected: '" + Compiler::llvmTypeStr(f->getArg(i)->getType()) +
          U"', got: '" + Compiler::llvmTypeStr(v->getType()) + U"')");
    callArgs.push_back(v1);
  }

  auto call = ctx.builder->CreateCall(f, callArgs);

  return call;
}

bool AST::Call::isPtrElementCall(Compiler::Context &ctx) {
  auto id = callee->isIdentifier() ? (Identifier *)callee : nullptr;
  bool b = callee->isLambda() || (!id && ctx.isFunction(id->getVal()));

  if (!b && (!id || ctx.isVar(id->getVal()) || ctx.isFinal(id->getVal()))) {
    auto tmp = callee->llvmValue(ctx);

    b = !::Adscript::Compiler::isFunctionTy(tmp->getType()) &&
        tmp->getType()->isPointerTy();

    // tmp->deleteValue();

    return b;
  }

  return false;
}
