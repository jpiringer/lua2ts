#include "emitter.hpp"
#include <iomanip>

namespace lua2ts {

TypeScriptEmitter::TypeScriptEmitter(EmitterOptions options)
    : options_(options) {}

void TypeScriptEmitter::indent() {
    buffer_ << get_indent_string();
}

std::string TypeScriptEmitter::get_indent_string() const {
    return std::string(indent_level_ * options_.indent_size, ' ');
}

std::string TypeScriptEmitter::emit(Node& node) {
    buffer_.str("");
    buffer_.clear();
    indent_level_ = 0;
    node.accept(*this);
    return buffer_.str();
}

std::string TypeScriptEmitter::emit_expr(Expr& expr) {
    TypeScriptEmitter sub_emitter(options_);
    sub_emitter.indent_level_ = indent_level_;
    expr.accept(sub_emitter);
    return sub_emitter.buffer_.str();
}

// ==================== Expression Visitors ====================

void TypeScriptEmitter::visit(NilLiteral& /*node*/) {
    buffer_ << "null";
}

void TypeScriptEmitter::visit(BoolLiteral& node) {
    buffer_ << (node.value ? "true" : "false");
}

void TypeScriptEmitter::visit(NumberLiteral& node) {
    buffer_ << node.value;
}

static std::string escape_ts_string(const std::string& str) {
    std::string out;
    out.reserve(str.size() + 2);
    out.push_back('"');
    for (char c : str) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out.push_back(c); break;
        }
    }
    out.push_back('"');
    return out;
}

void TypeScriptEmitter::visit(StringLiteral& node) {
    buffer_ << escape_ts_string(node.value);
}

void TypeScriptEmitter::visit(VarargExpr& /*node*/) {
    buffer_ << "...args";
}

void TypeScriptEmitter::visit(IdentifierExpr& node) {
    buffer_ << node.name;
}

void TypeScriptEmitter::visit(UnaryExpr& node) {
    bool needs_parens = dynamic_cast<BinaryExpr*>(node.expr.get()) != nullptr;
    std::string inner = emit_expr(*node.expr);
    std::string operand = needs_parens ? "(" + inner + ")" : inner;

    switch (node.op) {
        case TokenType::KwNot:
            buffer_ << "!" << operand;
            break;
        case TokenType::Minus:
            buffer_ << "-" << operand;
            break;
        case TokenType::Tilde:
            buffer_ << "~" << operand;
            break;
        case TokenType::Hash:
            // Length operator: expr.length
            buffer_ << "(" << inner << ").length";
            break;
        default:
            buffer_ << operand;
            break;
    }
}

void TypeScriptEmitter::visit(BinaryExpr& node) {
    std::string left = emit_expr(*node.left);
    std::string right = emit_expr(*node.right);

    switch (node.op) {
        case TokenType::DoubleSlash:
            // Integer division
            buffer_ << "Math.floor((" << left << ") / (" << right << "))";
            break;
        case TokenType::Caret:
            // Exponentiation
            buffer_ << "Math.pow(" << left << ", " << right << ")";
            break;
        case TokenType::Concat:
            // String concatenation
            buffer_ << "(" << left << " + " << right << ")";
            break;
        case TokenType::EqualEqual:
            buffer_ << "(" << left << " === " << right << ")";
            break;
        case TokenType::TildeEqual:
            buffer_ << "(" << left << " !== " << right << ")";
            break;
        case TokenType::KwAnd:
            buffer_ << "(" << left << " && " << right << ")";
            break;
        case TokenType::KwOr:
            buffer_ << "(" << left << " || " << right << ")";
            break;
        case TokenType::Plus:
            buffer_ << "(" << left << " + " << right << ")";
            break;
        case TokenType::Minus:
            buffer_ << "(" << left << " - " << right << ")";
            break;
        case TokenType::Star:
            buffer_ << "(" << left << " * " << right << ")";
            break;
        case TokenType::Slash:
            buffer_ << "(" << left << " / " << right << ")";
            break;
        case TokenType::Percent:
            buffer_ << "(" << left << " % " << right << ")";
            break;
        case TokenType::Less:
            buffer_ << "(" << left << " < " << right << ")";
            break;
        case TokenType::LessEqual:
            buffer_ << "(" << left << " <= " << right << ")";
            break;
        case TokenType::Greater:
            buffer_ << "(" << left << " > " << right << ")";
            break;
        case TokenType::GreaterEqual:
            buffer_ << "(" << left << " >= " << right << ")";
            break;
        case TokenType::Ampersand:
            buffer_ << "(" << left << " & " << right << ")";
            break;
        case TokenType::Pipe:
            buffer_ << "(" << left << " | " << right << ")";
            break;
        case TokenType::Tilde:
            buffer_ << "(" << left << " ^ " << right << ")";
            break;
        case TokenType::ShiftLeft:
            buffer_ << "(" << left << " << " << right << ")";
            break;
        case TokenType::ShiftRight:
            buffer_ << "(" << left << " >> " << right << ")";
            break;
        default:
            buffer_ << "(" << left << " " << token_type_to_string(node.op) << " " << right << ")";
            break;
    }
}

void TypeScriptEmitter::visit(TableConstructorExpr& node) {
    if (node.fields.empty()) {
        buffer_ << "{}";
        return;
    }

    bool all_implicit = true;
    for (const auto& field : node.fields) {
        if (!field.is_implicit_key) {
            all_implicit = false;
            break;
        }
    }

    if (all_implicit) {
        // Output as JS Array [e1, e2, ...]
        buffer_ << "[";
        for (size_t i = 0; i < node.fields.size(); ++i) {
            if (i > 0) buffer_ << ", ";
            buffer_ << emit_expr(*node.fields[i].value);
        }
        buffer_ << "]";
    }
    else {
        // Output as JS Object { k1: v1, ... }
        buffer_ << "{ ";
        for (size_t i = 0; i < node.fields.size(); ++i) {
            if (i > 0) buffer_ << ", ";
            const auto& field = node.fields[i];
            if (field.is_implicit_key) {
                // Numbered key (1-based index in Lua)
                buffer_ << (i + 1) << ": " << emit_expr(*field.value);
            } else {
                if (auto str_lit = dynamic_cast<StringLiteral*>(field.key.get())) {
                    buffer_ << str_lit->value << ": " << emit_expr(*field.value);
                } else {
                    buffer_ << "[" << emit_expr(*field.key) << "]: " << emit_expr(*field.value);
                }
            }
        }
        buffer_ << " }";
    }
}

void TypeScriptEmitter::visit(IndexExpr& node) {
    std::string tbl = emit_expr(*node.table);

    if (options_.index_adjustment != IndexAdjustment::None) {
        if (auto num = dynamic_cast<NumberLiteral*>(node.index.get())) {
            try {
                // Only adjust if it's a positive integer without a decimal point
                if (num->value.find('.') == std::string::npos &&
                    num->value.find('e') == std::string::npos &&
                    num->value.find('E') == std::string::npos) {
                    long long val = std::stoll(num->value);
                    if (val >= 1) {
                        buffer_ << tbl << "[" << (val - 1) << "]";
                        return;
                    }
                }
            } catch (...) {}
        }
        else if (options_.index_adjustment == IndexAdjustment::AllNumeric) {
            // If the key is not explicitly a string literal, adjust by -1
            if (dynamic_cast<StringLiteral*>(node.index.get()) == nullptr) {
                buffer_ << tbl << "[(" << emit_expr(*node.index) << ") - 1]";
                return;
            }
        }
    }

    buffer_ << tbl << "[" << emit_expr(*node.index) << "]";
}

void TypeScriptEmitter::visit(MemberExpr& node) {
    // Check for math.pi, math.huge
    if (auto id = dynamic_cast<IdentifierExpr*>(node.table.get())) {
        if (id->name == "math") {
            if (node.member == "pi") {
                buffer_ << "Math.PI";
                return;
            }
            if (node.member == "huge") {
                buffer_ << "Infinity";
                return;
            }
        }
    }
    buffer_ << emit_expr(*node.table) << "." << node.member;
}

bool TypeScriptEmitter::try_emit_special_call(CallExpr& node, std::ostringstream& out) {
    // 1. Check for global function calls: print, type, tostring, tonumber
    if (auto id = dynamic_cast<IdentifierExpr*>(node.callee.get())) {
        if (id->name == "print") {
            out << "console.log(";
            for (size_t i = 0; i < node.args.size(); ++i) {
                if (i > 0) out << ", ";
                out << emit_expr(*node.args[i]);
            }
            out << ")";
            return true;
        }
        if (id->name == "type" && node.args.size() == 1) {
            out << "typeof (" << emit_expr(*node.args[0]) << ")";
            return true;
        }
        if (id->name == "tostring" && node.args.size() == 1) {
            out << "String(" << emit_expr(*node.args[0]) << ")";
            return true;
        }
        if (id->name == "tonumber" && node.args.size() == 1) {
            out << "Number(" << emit_expr(*node.args[0]) << ")";
            return true;
        }
    }

    // 2. Check for math.* and table.* and string.* calls
    if (auto member = dynamic_cast<MemberExpr*>(node.callee.get())) {
        if (auto base_id = dynamic_cast<IdentifierExpr*>(member->table.get())) {
            if (base_id->name == "math") {
                out << "Math." << member->member << "(";
                for (size_t i = 0; i < node.args.size(); ++i) {
                    if (i > 0) out << ", ";
                    out << emit_expr(*node.args[i]);
                }
                out << ")";
                return true;
            }
            if (base_id->name == "table") {
                if (member->member == "insert" && node.args.size() >= 2) {
                    // table.insert(t, val) -> t.push(val)
                    if (node.args.size() == 2) {
                        out << emit_expr(*node.args[0]) << ".push(" << emit_expr(*node.args[1]) << ")";
                        return true;
                    } else if (node.args.size() == 3) {
                        // table.insert(t, pos, val) -> t.splice(pos - 1, 0, val)
                        out << emit_expr(*node.args[0]) << ".splice(" << emit_expr(*node.args[1]) << " - 1, 0, "
                            << emit_expr(*node.args[2]) << ")";
                        return true;
                    }
                }
                if (member->member == "remove" && !node.args.empty()) {
                    if (node.args.size() == 1) {
                        out << emit_expr(*node.args[0]) << ".pop()";
                        return true;
                    } else {
                        out << emit_expr(*node.args[0]) << ".splice(" << emit_expr(*node.args[1]) << " - 1, 1)[0]";
                        return true;
                    }
                }
                if (member->member == "concat" && !node.args.empty()) {
                    out << emit_expr(*node.args[0]) << ".join(";
                    if (node.args.size() >= 2) {
                        out << emit_expr(*node.args[1]);
                    } else {
                        out << "\"\"";
                    }
                    out << ")";
                    return true;
                }
            }
            if (base_id->name == "string") {
                if (member->member == "len" && node.args.size() == 1) {
                    out << emit_expr(*node.args[0]) << ".length";
                    return true;
                }
                if (member->member == "lower" && node.args.size() == 1) {
                    out << emit_expr(*node.args[0]) << ".toLowerCase()";
                    return true;
                }
                if (member->member == "upper" && node.args.size() == 1) {
                    out << emit_expr(*node.args[0]) << ".toUpperCase()";
                    return true;
                }
                if (member->member == "sub" && node.args.size() >= 2) {
                    out << emit_expr(*node.args[0]) << ".slice(" << emit_expr(*node.args[1]) << " - 1";
                    if (node.args.size() >= 3) {
                        out << ", " << emit_expr(*node.args[2]);
                    }
                    out << ")";
                    return true;
                }
            }
        }
    }

    return false;
}

void TypeScriptEmitter::visit(CallExpr& node) {
    std::ostringstream out;
    if (try_emit_special_call(node, out)) {
        buffer_ << out.str();
        return;
    }

    if (node.is_method_call) {
        // Method call: obj:method(args) -> obj.method(args)
        buffer_ << emit_expr(*node.callee) << "." << node.method_name << "(";
    }
    else {
        buffer_ << emit_expr(*node.callee) << "(";
    }

    for (size_t i = 0; i < node.args.size(); ++i) {
        if (i > 0) buffer_ << ", ";
        buffer_ << emit_expr(*node.args[i]);
    }
    buffer_ << ")";
}

void TypeScriptEmitter::visit(FunctionExpr& node) {
    buffer_ << "(";
    for (size_t i = 0; i < node.params.size(); ++i) {
        if (i > 0) buffer_ << ", ";
        buffer_ << node.params[i];
        if (options_.emit_types) buffer_ << ": any";
    }
    if (node.is_vararg) {
        if (!node.params.empty()) buffer_ << ", ";
        buffer_ << "...args: any[]";
    }
    buffer_ << ")";
    if (options_.emit_types) buffer_ << ": any";
    buffer_ << " => {\n";

    indent_level_++;
    for (const auto& stmt : node.body->statements) {
        indent();
        stmt->accept(*this);
        buffer_ << "\n";
    }
    indent_level_--;

    indent();
    buffer_ << "}";
}

// ==================== Statement Visitors ====================

void TypeScriptEmitter::visit(BlockStmt& node) {
    for (const auto& stmt : node.statements) {
        indent();
        stmt->accept(*this);
        buffer_ << "\n";
    }
}

void TypeScriptEmitter::visit(ExprStmt& node) {
    buffer_ << emit_expr(*node.expr) << ";";
}

void TypeScriptEmitter::visit(LocalVarDeclStmt& node) {
    buffer_ << "let ";
    for (size_t i = 0; i < node.names.size(); ++i) {
        if (i > 0) buffer_ << ", ";
        buffer_ << node.names[i];
        if (options_.emit_types) {
            buffer_ << ": any";
        }
        if (i < node.inits.size()) {
            buffer_ << " = " << emit_expr(*node.inits[i]);
        }
    }
    buffer_ << ";";
}

void TypeScriptEmitter::visit(AssignStmt& node) {
    if (node.lhs.size() == 1 && node.rhs.size() == 1) {
        buffer_ << emit_expr(*node.lhs[0]) << " = " << emit_expr(*node.rhs[0]) << ";";
    }
    else {
        // Multi-assignment via destructuring: [a, b] = [x, y];
        buffer_ << "[";
        for (size_t i = 0; i < node.lhs.size(); ++i) {
            if (i > 0) buffer_ << ", ";
            buffer_ << emit_expr(*node.lhs[i]);
        }
        buffer_ << "] = [";
        for (size_t i = 0; i < node.rhs.size(); ++i) {
            if (i > 0) buffer_ << ", ";
            if (i < node.rhs.size()) {
                buffer_ << emit_expr(*node.rhs[i]);
            } else {
                buffer_ << "undefined";
            }
        }
        buffer_ << "];";
    }
}

void TypeScriptEmitter::visit(FunctionDefStmt& node) {
    // If it's a simple top-level/local function: "function name(...) { ... }"
    if (node.path.size() == 1 && !node.is_method) {
        buffer_ << "function " << node.path[0] << "(";
        for (size_t i = 0; i < node.params.size(); ++i) {
            if (i > 0) buffer_ << ", ";
            buffer_ << node.params[i];
            if (options_.emit_types) buffer_ << ": any";
        }
        if (node.is_vararg) {
            if (!node.params.empty()) buffer_ << ", ";
            buffer_ << "...args: any[]";
        }
        buffer_ << ")";
        if (options_.emit_types) buffer_ << ": any";
        buffer_ << " {\n";

        indent_level_++;
        for (const auto& stmt : node.body->statements) {
            indent();
            stmt->accept(*this);
            buffer_ << "\n";
        }
        indent_level_--;

        indent();
        buffer_ << "}";
    }
    else {
        // Table member or method definition: obj.method = function(this: any, ...)
        for (size_t i = 0; i < node.path.size(); ++i) {
            if (i > 0) buffer_ << ".";
            buffer_ << node.path[i];
        }
        if (node.is_method) {
            buffer_ << "." << node.method_name;
        }
        buffer_ << " = function(";
        bool first_param = true;
        if (node.is_method) {
            buffer_ << "this: any";
            first_param = false;
        }
        for (const auto& p : node.params) {
            if (!first_param) buffer_ << ", ";
            buffer_ << p;
            if (options_.emit_types) buffer_ << ": any";
            first_param = false;
        }
        if (node.is_vararg) {
            if (!first_param) buffer_ << ", ";
            buffer_ << "...args: any[]";
        }
        buffer_ << ")";
        if (options_.emit_types) buffer_ << ": any";
        buffer_ << " {\n";

        indent_level_++;
        if (node.is_method) {
            indent();
            buffer_ << "const self = this;\n";
        }
        for (const auto& stmt : node.body->statements) {
            indent();
            stmt->accept(*this);
            buffer_ << "\n";
        }
        indent_level_--;

        indent();
        buffer_ << "};";
    }
}

void TypeScriptEmitter::visit(IfStmt& node) {
    for (size_t i = 0; i < node.branches.size(); ++i) {
        if (i == 0) {
            buffer_ << "if (" << emit_expr(*node.branches[i].condition) << ") {\n";
        } else {
            indent();
            buffer_ << "} else if (" << emit_expr(*node.branches[i].condition) << ") {\n";
        }
        indent_level_++;
        for (const auto& stmt : node.branches[i].body->statements) {
            indent();
            stmt->accept(*this);
            buffer_ << "\n";
        }
        indent_level_--;
    }

    if (node.else_body) {
        indent();
        buffer_ << "} else {\n";
        indent_level_++;
        for (const auto& stmt : node.else_body->statements) {
            indent();
            stmt->accept(*this);
            buffer_ << "\n";
        }
        indent_level_--;
    }

    indent();
    buffer_ << "}";
}

void TypeScriptEmitter::visit(WhileStmt& node) {
    buffer_ << "while (" << emit_expr(*node.condition) << ") {\n";
    indent_level_++;
    for (const auto& stmt : node.body->statements) {
        indent();
        stmt->accept(*this);
        buffer_ << "\n";
    }
    indent_level_--;
    indent();
    buffer_ << "}";
}

void TypeScriptEmitter::visit(RepeatStmt& node) {
    buffer_ << "do {\n";
    indent_level_++;
    for (const auto& stmt : node.body->statements) {
        indent();
        stmt->accept(*this);
        buffer_ << "\n";
    }
    indent_level_--;
    indent();
    buffer_ << "} while (!(" << emit_expr(*node.condition) << "));";
}

void TypeScriptEmitter::visit(ForNumericStmt& node) {
    std::string start_str = emit_expr(*node.start);
    std::string end_str = emit_expr(*node.end);

    if (node.step) {
        std::string step_str = emit_expr(*node.step);
        // Check if step is a simple positive or negative constant
        if (step_str == "1") {
            buffer_ << "for (let " << node.var_name << " = " << start_str << "; "
                    << node.var_name << " <= " << end_str << "; " << node.var_name << "++) {\n";
        }
        else if (step_str == "-1") {
            buffer_ << "for (let " << node.var_name << " = " << start_str << "; "
                    << node.var_name << " >= " << end_str << "; " << node.var_name << "--) {\n";
        }
        else {
            buffer_ << "for (let " << node.var_name << " = " << start_str << ", _step = " << step_str << "; "
                    << "_step >= 0 ? " << node.var_name << " <= " << end_str << " : "
                    << node.var_name << " >= " << end_str << "; "
                    << node.var_name << " += _step) {\n";
        }
    }
    else {
        // Default step is 1
        buffer_ << "for (let " << node.var_name << " = " << start_str << "; "
                << node.var_name << " <= " << end_str << "; " << node.var_name << "++) {\n";
    }

    indent_level_++;
    for (const auto& stmt : node.body->statements) {
        indent();
        stmt->accept(*this);
        buffer_ << "\n";
    }
    indent_level_--;
    indent();
    buffer_ << "}";
}

void TypeScriptEmitter::visit(ForGenericStmt& node) {
    // Check for pairs(t) or ipairs(t)
    if (!node.iter_exprs.empty()) {
        if (auto call = dynamic_cast<CallExpr*>(node.iter_exprs[0].get())) {
            if (auto id = dynamic_cast<IdentifierExpr*>(call->callee.get())) {
                if (id->name == "pairs" && call->args.size() == 1 && node.names.size() >= 2) {
                    std::string tbl = emit_expr(*call->args[0]);
                    buffer_ << "for (const [" << node.names[0] << ", " << node.names[1]
                            << "] of Object.entries(" << tbl << " as Record<string, any>)) {\n";
                    indent_level_++;
                    for (const auto& stmt : node.body->statements) {
                        indent();
                        stmt->accept(*this);
                        buffer_ << "\n";
                    }
                    indent_level_--;
                    indent();
                    buffer_ << "}";
                    return;
                }
                if (id->name == "ipairs" && call->args.size() == 1 && node.names.size() >= 2) {
                    std::string tbl = emit_expr(*call->args[0]);
                    std::string idx_tmp = "_" + node.names[0] + "_idx";
                    buffer_ << "for (const [" << idx_tmp << ", " << node.names[1]
                            << "] of (" << tbl << " as any[]).entries()) {\n";
                    indent_level_++;
                    // Emit 1-based index assignment to match Lua semantics
                    indent();
                    buffer_ << "const " << node.names[0] << " = " << idx_tmp << " + 1;\n";
                    for (const auto& stmt : node.body->statements) {
                        indent();
                        stmt->accept(*this);
                        buffer_ << "\n";
                    }
                    indent_level_--;
                    indent();
                    buffer_ << "}";
                    return;
                }
            }
        }
    }

    // Generic fallback for custom iterators
    buffer_ << "for (const [";
    for (size_t i = 0; i < node.names.size(); ++i) {
        if (i > 0) buffer_ << ", ";
        buffer_ << node.names[i];
    }
    buffer_ << "] of " << emit_expr(*node.iter_exprs[0]) << ") {\n";
    indent_level_++;
    for (const auto& stmt : node.body->statements) {
        indent();
        stmt->accept(*this);
        buffer_ << "\n";
    }
    indent_level_--;
    indent();
    buffer_ << "}";
}

void TypeScriptEmitter::visit(ReturnStmt& node) {
    if (node.values.empty()) {
        buffer_ << "return;";
    }
    else if (node.values.size() == 1) {
        buffer_ << "return " << emit_expr(*node.values[0]) << ";";
    }
    else {
        // Multiple returns in Lua -> array in TypeScript
        buffer_ << "return [";
        for (size_t i = 0; i < node.values.size(); ++i) {
            if (i > 0) buffer_ << ", ";
            buffer_ << emit_expr(*node.values[i]);
        }
        buffer_ << "];";
    }
}

void TypeScriptEmitter::visit(BreakStmt& /*node*/) {
    buffer_ << "break;";
}

} // namespace lua2ts
