class Expr {
public:
    virtual ~Expr() = default;
};

class IntExpr : public Expr {
public:
    const int val;
    IntExpr(const int val) : val(val) {}
};

class FloatExpr : public Expr {
public:
    const float val;
    FloatExpr(const float val) : val(val) {}
};

class VarExpr : public Expr {
public:
    const std::string id;
    VarExpr(const std::string& id) : id(id) {}
};

class Function : public Expr {
private:
    const std::vector<std::string> args;
    const std::vector<Expr*> body;
public:
    Function(std::vector<std::string>& args, std::vector<Expr*>& body)
        : args(args), body(body) {}
};

class FunctionCall : public Expr {
private:
    const std::string calleeId;
    const std::vector<Expr*> args;
public:
    FunctionCall(const std::string& calleeId, const std::vector<Expr*>& args)
        : calleeId(calleeId), args(args) {}
};
