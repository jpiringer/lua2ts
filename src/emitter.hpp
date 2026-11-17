#pragma once

#include <string>
#include <sstream>
#include "ast.hpp"

namespace lua2ts {

enum class IndexAdjustment {
    None,         // Keep as-is: t[i]
    Literals,     // Convert integer literals: t[1] -> t[0] (default)
    AllNumeric    // Convert all numeric indices: t[(i) - 1]
};

struct EmitterOptions {
    bool include_runtime_helpers{true};
    bool emit_types{true};
    int indent_size{2};
    IndexAdjustment index_adjustment{IndexAdjustment::Literals};
};

class TypeScriptEmitter : public ASTVisitor {
public:
    explicit TypeScriptEmitter(EmitterOptions options = EmitterOptions{});

    std::string emit(Node& node);

    // Visitor methods
    void visit(NilLiteral& node) override;
    void visit(BoolLiteral& node) override;
    void visit(NumberLiteral& node) override;
    void visit(StringLiteral& node) override;
    void visit(VarargExpr& node) override;
    void visit(IdentifierExpr& node) override;
    void visit(UnaryExpr& node) override;
    void visit(BinaryExpr& node) override;
    void visit(TableConstructorExpr& node) override;
    void visit(IndexExpr& node) override;
    void visit(MemberExpr& node) override;
    void visit(CallExpr& node) override;
    void visit(FunctionExpr& node) override;

    void visit(BlockStmt& node) override;
    void visit(ExprStmt& node) override;
    void visit(LocalVarDeclStmt& node) override;
    void visit(AssignStmt& node) override;
    void visit(FunctionDefStmt& node) override;
    void visit(IfStmt& node) override;
    void visit(WhileStmt& node) override;
    void visit(RepeatStmt& node) override;
    void visit(ForNumericStmt& node) override;
    void visit(ForGenericStmt& node) override;
    void visit(ReturnStmt& node) override;
    void visit(BreakStmt& node) override;

private:
    void indent();
    std::string get_indent_string() const;
    std::string emit_expr(Expr& expr);

    bool try_emit_special_call(CallExpr& node, std::ostringstream& out);

    EmitterOptions options_;
    int indent_level_{0};
    std::ostringstream buffer_;
};

} // namespace lua2ts
