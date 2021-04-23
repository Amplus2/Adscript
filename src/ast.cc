#include "ast.hh"
#include"utils.hh"

std::string IntExpr::toStr() {
    return std::to_string(val);
}

std::string FloatExpr::toStr() {
    return std::to_string(val);
}

std::string IdExpr::toStr() {
    return val;
}

std::string Function::toStr() {
    return std::string() + "{"
        + "\n\tFunction: {"
        + "\n\t\targs: " + strVectorToStr(args)
        + "\n\t\tbody: " + exprVectorToStr(body)
        + "\n\t}\n}";
}

std::string FunctionCall::toStr() {
    return std::string() + "{"
        + "\n\tFunctionCall: {"
        + "\n\t\tid: " + calleeId
        + "\n\t\targs: " + exprVectorToStr(args)
        + "\n\t}\n}";
}


std::string IdExpr::getVal() {
    return val;
}


// TODO: create llvm IR code generation methods
