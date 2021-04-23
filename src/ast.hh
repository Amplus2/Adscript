#ifndef AST_HH_
#define AST_HH_

#include <string>
#include <vector>

class Expr {
public:
    virtual ~Expr() = default;
    virtual std::string toStr() = 0;
};

class IntExpr : public Expr {
private:
    const int val;
public:
    IntExpr(const int val) : val(val) {}
    std::string toStr() override;
};

class FloatExpr : public Expr {
private:
    const float val;
public:
    FloatExpr(const float val) : val(val) {}
    std::string toStr() override;
};

class IdExpr : public Expr {
private:
    const std::string val;
public:
    IdExpr(const std::string  val) : val(val) {}
    std::string toStr() override;
    std::string getVal();
};

class Function : public Expr {
private:
    const std::vector<std::string> args;
    const std::vector<Expr*> body;
public:
    Function(std::vector<std::string>& args, std::vector<Expr*>& body)
        : args(args), body(body) {}
    std::string toStr() override;
};

class FunctionCall : public Expr {
private:
    const std::string calleeId;
    const std::vector<Expr*> args;
public:
    FunctionCall(const std::string& calleeId, const std::vector<Expr*>& args)
        : calleeId(calleeId), args(args) {}
    std::string toStr() override;
};

#endif
