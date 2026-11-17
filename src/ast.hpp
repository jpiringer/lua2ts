#pragma once

#include <string>
#include <vector>
#include <memory>
#include "lexer.hpp"

namespace lua2ts {

class ASTVisitor;

class Node {
public:
    virtual ~Node() = default;
    virtual void accept(ASTVisitor& visitor) = 0;
};

// ==================== Expressions ====================

class Expr : public Node {};

class NilLiteral : public Expr {
public:
    void accept(ASTVisitor& visitor) override;
};

class BoolLiteral : public Expr {
public:
    bool value;
    explicit BoolLiteral(bool val) : value(val) {}
    void accept(ASTVisitor& visitor) override;
};

class NumberLiteral : public Expr {
public:
    std::string value;
    explicit NumberLiteral(std::string val) : value(std::move(val)) {}
    void accept(ASTVisitor& visitor) override;
};

class StringLiteral : public Expr {
public:
    std::string value;
    explicit StringLiteral(std::string val) : value(std::move(val)) {}
    void accept(ASTVisitor& visitor) override;
};

class VarargExpr : public Expr {
public:
    void accept(ASTVisitor& visitor) override;
};

class IdentifierExpr : public Expr {
public:
    std::string name;
    explicit IdentifierExpr(std::string n) : name(std::move(n)) {}
    void accept(ASTVisitor& visitor) override;
};

class UnaryExpr : public Expr {
public:
    TokenType op;
    std::unique_ptr<Expr> expr;
    UnaryExpr(TokenType op, std::unique_ptr<Expr> e)
        : op(op), expr(std::move(e)) {}
    void accept(ASTVisitor& visitor) override;
};

class BinaryExpr : public Expr {
public:
    TokenType op;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    BinaryExpr(TokenType op, std::unique_ptr<Expr> l, std::unique_ptr<Expr> r)
        : op(op), left(std::move(l)), right(std::move(r)) {}
    void accept(ASTVisitor& visitor) override;
};

struct TableField {
    std::unique_ptr<Expr> key;   // nullptr if implicit array index
    std::unique_ptr<Expr> value;
    bool is_implicit_key{false};
};

class TableConstructorExpr : public Expr {
public:
    std::vector<TableField> fields;
    void accept(ASTVisitor& visitor) override;
};

class IndexExpr : public Expr {
public:
    std::unique_ptr<Expr> table;
    std::unique_ptr<Expr> index;
    IndexExpr(std::unique_ptr<Expr> tbl, std::unique_ptr<Expr> idx)
        : table(std::move(tbl)), index(std::move(idx)) {}
    void accept(ASTVisitor& visitor) override;
};

class MemberExpr : public Expr {
public:
    std::unique_ptr<Expr> table;
    std::string member;
    MemberExpr(std::unique_ptr<Expr> tbl, std::string m)
        : table(std::move(tbl)), member(std::move(m)) {}
    void accept(ASTVisitor& visitor) override;
};

class CallExpr : public Expr {
public:
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> args;
    bool is_method_call{false};
    std::string method_name;

    CallExpr(std::unique_ptr<Expr> c, std::vector<std::unique_ptr<Expr>> a)
        : callee(std::move(c)), args(std::move(a)) {}
    CallExpr(std::unique_ptr<Expr> c, std::string method, std::vector<std::unique_ptr<Expr>> a)
        : callee(std::move(c)), args(std::move(a)), is_method_call(true), method_name(std::move(method)) {}

    void accept(ASTVisitor& visitor) override;
};

class BlockStmt;

class FunctionExpr : public Expr {
public:
    std::vector<std::string> params;
    bool is_vararg{false};
    std::unique_ptr<BlockStmt> body;

    FunctionExpr(std::vector<std::string> p, bool va, std::unique_ptr<BlockStmt> b)
        : params(std::move(p)), is_vararg(va), body(std::move(b)) {}

    void accept(ASTVisitor& visitor) override;
};

// ==================== Statements ====================

class Stmt : public Node {};

class BlockStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Stmt>> statements;
    void accept(ASTVisitor& visitor) override;
};

class ExprStmt : public Stmt {
public:
    std::unique_ptr<Expr> expr;
    explicit ExprStmt(std::unique_ptr<Expr> e) : expr(std::move(e)) {}
    void accept(ASTVisitor& visitor) override;
};

class LocalVarDeclStmt : public Stmt {
public:
    std::vector<std::string> names;
    std::vector<std::unique_ptr<Expr>> inits;
    LocalVarDeclStmt(std::vector<std::string> n, std::vector<std::unique_ptr<Expr>> i)
        : names(std::move(n)), inits(std::move(i)) {}
    void accept(ASTVisitor& visitor) override;
};

class AssignStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Expr>> lhs;
    std::vector<std::unique_ptr<Expr>> rhs;
    AssignStmt(std::vector<std::unique_ptr<Expr>> l, std::vector<std::unique_ptr<Expr>> r)
        : lhs(std::move(l)), rhs(std::move(r)) {}
    void accept(ASTVisitor& visitor) override;
};

class FunctionDefStmt : public Stmt {
public:
    std::vector<std::string> path; // e.g. ["obj", "sub"] or ["foo"]
    std::string method_name;       // non-empty if defined with ':'
    bool is_method{false};
    bool is_local{false};
    std::vector<std::string> params;
    bool is_vararg{false};
    std::unique_ptr<BlockStmt> body;

    FunctionDefStmt(std::vector<std::string> p, std::string m, bool is_meth, bool is_loc,
                    std::vector<std::string> params, bool va, std::unique_ptr<BlockStmt> b)
        : path(std::move(p)), method_name(std::move(m)), is_method(is_meth), is_local(is_loc),
          params(std::move(params)), is_vararg(va), body(std::move(b)) {}

    void accept(ASTVisitor& visitor) override;
};

struct IfBranch {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<BlockStmt> body;
};

class IfStmt : public Stmt {
public:
    std::vector<IfBranch> branches;
    std::unique_ptr<BlockStmt> else_body; // May be nullptr

    IfStmt(std::vector<IfBranch> b, std::unique_ptr<BlockStmt> eb)
        : branches(std::move(b)), else_body(std::move(eb)) {}

    void accept(ASTVisitor& visitor) override;
};

class WhileStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<BlockStmt> body;

    WhileStmt(std::unique_ptr<Expr> cond, std::unique_ptr<BlockStmt> b)
        : condition(std::move(cond)), body(std::move(b)) {}

    void accept(ASTVisitor& visitor) override;
};

class RepeatStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<BlockStmt> body;

    RepeatStmt(std::unique_ptr<Expr> cond, std::unique_ptr<BlockStmt> b)
        : condition(std::move(cond)), body(std::move(b)) {}

    void accept(ASTVisitor& visitor) override;
};

class ForNumericStmt : public Stmt {
public:
    std::string var_name;
    std::unique_ptr<Expr> start;
    std::unique_ptr<Expr> end;
    std::unique_ptr<Expr> step; // May be nullptr (defaults to 1)
    std::unique_ptr<BlockStmt> body;

    ForNumericStmt(std::string var, std::unique_ptr<Expr> s, std::unique_ptr<Expr> e,
                   std::unique_ptr<Expr> st, std::unique_ptr<BlockStmt> b)
        : var_name(std::move(var)), start(std::move(s)), end(std::move(e)),
          step(std::move(st)), body(std::move(b)) {}

    void accept(ASTVisitor& visitor) override;
};

class ForGenericStmt : public Stmt {
public:
    std::vector<std::string> names;
    std::vector<std::unique_ptr<Expr>> iter_exprs;
    std::unique_ptr<BlockStmt> body;

    ForGenericStmt(std::vector<std::string> n, std::vector<std::unique_ptr<Expr>> iters,
                   std::unique_ptr<BlockStmt> b)
        : names(std::move(n)), iter_exprs(std::move(iters)), body(std::move(b)) {}

    void accept(ASTVisitor& visitor) override;
};

class ReturnStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Expr>> values;
    explicit ReturnStmt(std::vector<std::unique_ptr<Expr>> vals)
        : values(std::move(vals)) {}
    void accept(ASTVisitor& visitor) override;
};

class BreakStmt : public Stmt {
public:
    void accept(ASTVisitor& visitor) override;
};

// ==================== Visitor Interface ====================

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    virtual void visit(NilLiteral& node) = 0;
    virtual void visit(BoolLiteral& node) = 0;
    virtual void visit(NumberLiteral& node) = 0;
    virtual void visit(StringLiteral& node) = 0;
    virtual void visit(VarargExpr& node) = 0;
    virtual void visit(IdentifierExpr& node) = 0;
    virtual void visit(UnaryExpr& node) = 0;
    virtual void visit(BinaryExpr& node) = 0;
    virtual void visit(TableConstructorExpr& node) = 0;
    virtual void visit(IndexExpr& node) = 0;
    virtual void visit(MemberExpr& node) = 0;
    virtual void visit(CallExpr& node) = 0;
    virtual void visit(FunctionExpr& node) = 0;

    virtual void visit(BlockStmt& node) = 0;
    virtual void visit(ExprStmt& node) = 0;
    virtual void visit(LocalVarDeclStmt& node) = 0;
    virtual void visit(AssignStmt& node) = 0;
    virtual void visit(FunctionDefStmt& node) = 0;
    virtual void visit(IfStmt& node) = 0;
    virtual void visit(WhileStmt& node) = 0;
    virtual void visit(RepeatStmt& node) = 0;
    virtual void visit(ForNumericStmt& node) = 0;
    virtual void visit(ForGenericStmt& node) = 0;
    virtual void visit(ReturnStmt& node) = 0;
    virtual void visit(BreakStmt& node) = 0;
};

inline void NilLiteral::accept(ASTVisitor& v) { v.visit(*this); }
inline void BoolLiteral::accept(ASTVisitor& v) { v.visit(*this); }
inline void NumberLiteral::accept(ASTVisitor& v) { v.visit(*this); }
inline void StringLiteral::accept(ASTVisitor& v) { v.visit(*this); }
inline void VarargExpr::accept(ASTVisitor& v) { v.visit(*this); }
inline void IdentifierExpr::accept(ASTVisitor& v) { v.visit(*this); }
inline void UnaryExpr::accept(ASTVisitor& v) { v.visit(*this); }
inline void BinaryExpr::accept(ASTVisitor& v) { v.visit(*this); }
inline void TableConstructorExpr::accept(ASTVisitor& v) { v.visit(*this); }
inline void IndexExpr::accept(ASTVisitor& v) { v.visit(*this); }
inline void MemberExpr::accept(ASTVisitor& v) { v.visit(*this); }
inline void CallExpr::accept(ASTVisitor& v) { v.visit(*this); }
inline void FunctionExpr::accept(ASTVisitor& v) { v.visit(*this); }

inline void BlockStmt::accept(ASTVisitor& v) { v.visit(*this); }
inline void ExprStmt::accept(ASTVisitor& v) { v.visit(*this); }
inline void LocalVarDeclStmt::accept(ASTVisitor& v) { v.visit(*this); }
inline void AssignStmt::accept(ASTVisitor& v) { v.visit(*this); }
inline void FunctionDefStmt::accept(ASTVisitor& v) { v.visit(*this); }
inline void IfStmt::accept(ASTVisitor& v) { v.visit(*this); }
inline void WhileStmt::accept(ASTVisitor& v) { v.visit(*this); }
inline void RepeatStmt::accept(ASTVisitor& v) { v.visit(*this); }
inline void ForNumericStmt::accept(ASTVisitor& v) { v.visit(*this); }
inline void ForGenericStmt::accept(ASTVisitor& v) { v.visit(*this); }
inline void ReturnStmt::accept(ASTVisitor& v) { v.visit(*this); }
inline void BreakStmt::accept(ASTVisitor& v) { v.visit(*this); }

} // namespace lua2ts
