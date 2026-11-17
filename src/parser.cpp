#include "parser.hpp"
#include <stdexcept>

namespace lua2ts {

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

const Token& Parser::peek() const {
    return tokens_[current_];
}

const Token& Parser::peek_next() const {
    if (current_ + 1 >= tokens_.size()) return tokens_.back();
    return tokens_[current_ + 1];
}

const Token& Parser::previous() const {
    return tokens_[current_ - 1];
}

bool Parser::is_at_end() const {
    return peek().type == TokenType::Eof;
}

bool Parser::check(TokenType type) const {
    if (is_at_end()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

const Token& Parser::advance() {
    if (!is_at_end()) current_++;
    return previous();
}

const Token& Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw ParseError(message + " (got '" + std::string(token_type_to_string(peek().type)) + "')",
                     peek().line, peek().col);
}

std::unique_ptr<BlockStmt> Parser::parse() {
    auto block = parse_block();
    if (!is_at_end()) {
        throw ParseError("Unexpected token at end of chunk: " + peek().lexeme, peek().line, peek().col);
    }
    return block;
}

static bool is_block_end(TokenType type) {
    return type == TokenType::KwEnd ||
           type == TokenType::KwElse ||
           type == TokenType::KwElseif ||
           type == TokenType::KwUntil ||
           type == TokenType::Eof;
}

std::unique_ptr<BlockStmt> Parser::parse_block() {
    auto block = std::make_unique<BlockStmt>();

    while (!is_at_end() && !is_block_end(peek().type)) {
        if (match(TokenType::Semicolon)) {
            continue; // Empty statement
        }
        auto stmt = parse_statement();
        if (stmt) {
            block->statements.push_back(std::move(stmt));
        }
    }

    return block;
}

std::unique_ptr<Stmt> Parser::parse_statement() {
    if (match(TokenType::KwLocal)) {
        return parse_local();
    }
    if (match(TokenType::KwFunction)) {
        return parse_function_stmt();
    }
    if (match(TokenType::KwIf)) {
        return parse_if_stmt();
    }
    if (match(TokenType::KwWhile)) {
        return parse_while_stmt();
    }
    if (match(TokenType::KwRepeat)) {
        return parse_repeat_stmt();
    }
    if (match(TokenType::KwFor)) {
        return parse_for_stmt();
    }
    if (match(TokenType::KwDo)) {
        auto body = parse_block();
        consume(TokenType::KwEnd, "Expected 'end' after do block");
        return body;
    }
    if (match(TokenType::KwBreak)) {
        return std::make_unique<BreakStmt>();
    }
    if (match(TokenType::KwReturn)) {
        return parse_return_stmt();
    }

    return parse_expr_or_assign();
}

std::unique_ptr<Stmt> Parser::parse_local() {
    if (match(TokenType::KwFunction)) {
        // local function name(...) ... end
        const Token& name_tok = consume(TokenType::Identifier, "Expected function name after 'local function'");
        std::vector<std::string> path = {name_tok.lexeme};

        consume(TokenType::LParen, "Expected '(' after function name");
        bool is_vararg = false;
        auto params = parse_param_list(is_vararg);
        consume(TokenType::RParen, "Expected ')' after parameters");

        auto body = parse_block();
        consume(TokenType::KwEnd, "Expected 'end' to close function body");

        return std::make_unique<FunctionDefStmt>(std::move(path), "", false, true,
                                                std::move(params), is_vararg, std::move(body));
    }

    // local var1, var2 = e1, e2
    std::vector<std::string> names;
    names.push_back(consume(TokenType::Identifier, "Expected variable name after 'local'").lexeme);
    while (match(TokenType::Comma)) {
        names.push_back(consume(TokenType::Identifier, "Expected variable name after ','").lexeme);
    }

    std::vector<std::unique_ptr<Expr>> inits;
    if (match(TokenType::Equal)) {
        inits = parse_expr_list();
    }

    return std::make_unique<LocalVarDeclStmt>(std::move(names), std::move(inits));
}

std::unique_ptr<Stmt> Parser::parse_function_stmt() {
    std::vector<std::string> path;
    path.push_back(consume(TokenType::Identifier, "Expected function name").lexeme);

    while (match(TokenType::Dot)) {
        path.push_back(consume(TokenType::Identifier, "Expected property name after '.'").lexeme);
    }

    std::string method_name;
    bool is_method = false;
    if (match(TokenType::Colon)) {
        is_method = true;
        method_name = consume(TokenType::Identifier, "Expected method name after ':'").lexeme;
    }

    consume(TokenType::LParen, "Expected '(' after function name");
    bool is_vararg = false;
    auto params = parse_param_list(is_vararg);
    consume(TokenType::RParen, "Expected ')' after parameters");

    auto body = parse_block();
    consume(TokenType::KwEnd, "Expected 'end' to close function body");

    return std::make_unique<FunctionDefStmt>(std::move(path), std::move(method_name),
                                            is_method, false, std::move(params), is_vararg, std::move(body));
}

std::unique_ptr<Stmt> Parser::parse_if_stmt() {
    std::vector<IfBranch> branches;

    auto cond = parse_expression();
    consume(TokenType::KwThen, "Expected 'then' after if condition");
    auto body = parse_block();
    branches.push_back(IfBranch{std::move(cond), std::move(body)});

    while (match(TokenType::KwElseif)) {
        auto elif_cond = parse_expression();
        consume(TokenType::KwThen, "Expected 'then' after elseif condition");
        auto elif_body = parse_block();
        branches.push_back(IfBranch{std::move(elif_cond), std::move(elif_body)});
    }

    std::unique_ptr<BlockStmt> else_body = nullptr;
    if (match(TokenType::KwElse)) {
        else_body = parse_block();
    }

    consume(TokenType::KwEnd, "Expected 'end' to close if statement");
    return std::make_unique<IfStmt>(std::move(branches), std::move(else_body));
}

std::unique_ptr<Stmt> Parser::parse_while_stmt() {
    auto cond = parse_expression();
    consume(TokenType::KwDo, "Expected 'do' after while condition");
    auto body = parse_block();
    consume(TokenType::KwEnd, "Expected 'end' to close while statement");
    return std::make_unique<WhileStmt>(std::move(cond), std::move(body));
}

std::unique_ptr<Stmt> Parser::parse_repeat_stmt() {
    auto body = parse_block();
    consume(TokenType::KwUntil, "Expected 'until' after repeat body");
    auto cond = parse_expression();
    return std::make_unique<RepeatStmt>(std::move(cond), std::move(body));
}

std::unique_ptr<Stmt> Parser::parse_for_stmt() {
    const Token& first_var = consume(TokenType::Identifier, "Expected variable name after 'for'");

    if (match(TokenType::Equal)) {
        // Numeric for: for i = start, end [, step] do ... end
        auto start = parse_expression();
        consume(TokenType::Comma, "Expected ',' after for start expression");
        auto end = parse_expression();
        std::unique_ptr<Expr> step = nullptr;
        if (match(TokenType::Comma)) {
            step = parse_expression();
        }
        consume(TokenType::KwDo, "Expected 'do' in for loop");
        auto body = parse_block();
        consume(TokenType::KwEnd, "Expected 'end' to close for loop");
        return std::make_unique<ForNumericStmt>(first_var.lexeme, std::move(start),
                                               std::move(end), std::move(step), std::move(body));
    }
    else {
        // Generic for: for k, v in iter do ... end
        std::vector<std::string> names = {first_var.lexeme};
        while (match(TokenType::Comma)) {
            names.push_back(consume(TokenType::Identifier, "Expected variable name after ','").lexeme);
        }
        consume(TokenType::KwIn, "Expected 'in' or '=' in for statement");
        auto iters = parse_expr_list();
        consume(TokenType::KwDo, "Expected 'do' in for loop");
        auto body = parse_block();
        consume(TokenType::KwEnd, "Expected 'end' to close for loop");
        return std::make_unique<ForGenericStmt>(std::move(names), std::move(iters), std::move(body));
    }
}

std::unique_ptr<Stmt> Parser::parse_return_stmt() {
    std::vector<std::unique_ptr<Expr>> values;
    if (!is_at_end() && !is_block_end(peek().type) && peek().type != TokenType::Semicolon) {
        values = parse_expr_list();
    }
    match(TokenType::Semicolon); // Optional semicolon
    return std::make_unique<ReturnStmt>(std::move(values));
}

std::unique_ptr<Stmt> Parser::parse_expr_or_assign() {
    auto first = parse_primary_expr();

    // Check if this is an assignment
    if (check(TokenType::Comma) || check(TokenType::Equal)) {
        std::vector<std::unique_ptr<Expr>> lhs;
        lhs.push_back(std::move(first));
        while (match(TokenType::Comma)) {
            lhs.push_back(parse_primary_expr());
        }
        consume(TokenType::Equal, "Expected '=' in assignment");
        auto rhs = parse_expr_list();
        return std::make_unique<AssignStmt>(std::move(lhs), std::move(rhs));
    }

    // Otherwise it must be a function call or valid expression statement
    return std::make_unique<ExprStmt>(std::move(first));
}

// ==================== Expression Parsing ====================

static bool get_binary_precedence(TokenType type, int& prec, bool& right_assoc) {
    right_assoc = false;
    switch (type) {
        case TokenType::KwOr:        prec = 1; return true;
        case TokenType::KwAnd:       prec = 2; return true;
        case TokenType::Less:
        case TokenType::Greater:
        case TokenType::LessEqual:
        case TokenType::GreaterEqual:
        case TokenType::TildeEqual:
        case TokenType::EqualEqual:  prec = 3; return true;
        case TokenType::Pipe:        prec = 4; return true;
        case TokenType::Tilde:       prec = 5; return true;
        case TokenType::Ampersand:   prec = 6; return true;
        case TokenType::ShiftLeft:
        case TokenType::ShiftRight:  prec = 7; return true;
        case TokenType::Concat:      prec = 8; right_assoc = true; return true;
        case TokenType::Plus:
        case TokenType::Minus:       prec = 9; return true;
        case TokenType::Star:
        case TokenType::Slash:
        case TokenType::DoubleSlash:
        case TokenType::Percent:     prec = 10; return true;
        case TokenType::Caret:       prec = 12; right_assoc = true; return true;
        default: return false;
    }
}

std::unique_ptr<Expr> Parser::parse_expression() {
    return parse_sub_expr(0);
}

std::unique_ptr<Expr> Parser::parse_sub_expr(int min_precedence) {
    auto left = parse_unary_expr();

    while (true) {
        int prec = 0;
        bool right_assoc = false;
        if (!get_binary_precedence(peek().type, prec, right_assoc) || prec < min_precedence) {
            break;
        }

        TokenType op = advance().type;
        int next_min_prec = right_assoc ? prec : prec + 1;
        auto right = parse_sub_expr(next_min_prec);
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
    }

    return left;
}

std::unique_ptr<Expr> Parser::parse_unary_expr() {
    if (check(TokenType::KwNot) || check(TokenType::Hash) ||
        check(TokenType::Minus) || check(TokenType::Tilde)) {
        TokenType op = advance().type;
        auto sub = parse_sub_expr(11); // Unary precedence is 11
        return std::make_unique<UnaryExpr>(op, std::move(sub));
    }
    return parse_primary_expr();
}

std::unique_ptr<Expr> Parser::parse_primary_expr() {
    std::unique_ptr<Expr> expr;

    // Prefix expression root
    if (match(TokenType::KwNil)) {
        expr = std::make_unique<NilLiteral>();
    }
    else if (match(TokenType::KwTrue)) {
        expr = std::make_unique<BoolLiteral>(true);
    }
    else if (match(TokenType::KwFalse)) {
        expr = std::make_unique<BoolLiteral>(false);
    }
    else if (match(TokenType::Number)) {
        expr = std::make_unique<NumberLiteral>(previous().lexeme);
    }
    else if (match(TokenType::String)) {
        expr = std::make_unique<StringLiteral>(previous().lexeme);
    }
    else if (match(TokenType::Vararg)) {
        expr = std::make_unique<VarargExpr>();
    }
    else if (match(TokenType::Identifier)) {
        expr = std::make_unique<IdentifierExpr>(previous().lexeme);
    }
    else if (match(TokenType::LParen)) {
        expr = parse_expression();
        consume(TokenType::RParen, "Expected ')' after parenthesized expression");
    }
    else if (check(TokenType::LBrace)) {
        expr = parse_table_constructor();
    }
    else if (match(TokenType::KwFunction)) {
        expr = parse_function_expr();
    }
    else {
        throw ParseError("Expected expression", peek().line, peek().col);
    }

    // Postfix operations: ., [], (), :method(), table arg, string arg
    while (true) {
        if (match(TokenType::Dot)) {
            const Token& name = consume(TokenType::Identifier, "Expected property name after '.'");
            expr = std::make_unique<MemberExpr>(std::move(expr), name.lexeme);
        }
        else if (match(TokenType::LBracket)) {
            auto idx = parse_expression();
            consume(TokenType::RBracket, "Expected ']' after index");
            expr = std::make_unique<IndexExpr>(std::move(expr), std::move(idx));
        }
        else if (match(TokenType::Colon)) {
            const Token& method = consume(TokenType::Identifier, "Expected method name after ':'");
            std::vector<std::unique_ptr<Expr>> args;
            if (match(TokenType::LParen)) {
                if (!check(TokenType::RParen)) {
                    args = parse_expr_list();
                }
                consume(TokenType::RParen, "Expected ')' after arguments");
            } else if (check(TokenType::LBrace)) {
                args.push_back(parse_table_constructor());
            } else if (match(TokenType::String)) {
                args.push_back(std::make_unique<StringLiteral>(previous().lexeme));
            } else {
                throw ParseError("Expected argument list or literal after method call", peek().line, peek().col);
            }
            expr = std::make_unique<CallExpr>(std::move(expr), method.lexeme, std::move(args));
        }
        else if (match(TokenType::LParen)) {
            std::vector<std::unique_ptr<Expr>> args;
            if (!check(TokenType::RParen)) {
                args = parse_expr_list();
            }
            consume(TokenType::RParen, "Expected ')' after arguments");
            expr = std::make_unique<CallExpr>(std::move(expr), std::move(args));
        }
        else if (check(TokenType::LBrace)) {
            // Function call with single table arg: f{ ... }
            std::vector<std::unique_ptr<Expr>> args;
            args.push_back(parse_table_constructor());
            expr = std::make_unique<CallExpr>(std::move(expr), std::move(args));
        }
        else if (match(TokenType::String)) {
            // Function call with single string arg: f "hello"
            std::vector<std::unique_ptr<Expr>> args;
            args.push_back(std::make_unique<StringLiteral>(previous().lexeme));
            expr = std::make_unique<CallExpr>(std::move(expr), std::move(args));
        }
        else {
            break;
        }
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parse_table_constructor() {
    consume(TokenType::LBrace, "Expected '{'");
    auto table = std::make_unique<TableConstructorExpr>();

    while (!check(TokenType::RBrace) && !is_at_end()) {
        TableField field;
        if (match(TokenType::LBracket)) {
            // [exp] = exp
            field.key = parse_expression();
            consume(TokenType::RBracket, "Expected ']' after key");
            consume(TokenType::Equal, "Expected '=' after field key");
            field.value = parse_expression();
            field.is_implicit_key = false;
        }
        else if (check(TokenType::Identifier) && peek_next().type == TokenType::Equal) {
            // ident = exp
            const Token& name = advance();
            consume(TokenType::Equal, "Expected '='");
            field.key = std::make_unique<StringLiteral>(name.lexeme);
            field.value = parse_expression();
            field.is_implicit_key = false;
        }
        else {
            // implicit key exp
            field.value = parse_expression();
            field.is_implicit_key = true;
        }

        table->fields.push_back(std::move(field));

        if (!match(TokenType::Comma) && !match(TokenType::Semicolon)) {
            break;
        }
    }

    consume(TokenType::RBrace, "Expected '}' to close table constructor");
    return table;
}

std::unique_ptr<Expr> Parser::parse_function_expr() {
    consume(TokenType::LParen, "Expected '(' after 'function'");
    bool is_vararg = false;
    auto params = parse_param_list(is_vararg);
    consume(TokenType::RParen, "Expected ')' after parameters");

    auto body = parse_block();
    consume(TokenType::KwEnd, "Expected 'end' to close function expression");

    return std::make_unique<FunctionExpr>(std::move(params), is_vararg, std::move(body));
}

std::vector<std::unique_ptr<Expr>> Parser::parse_expr_list() {
    std::vector<std::unique_ptr<Expr>> list;
    list.push_back(parse_expression());
    while (match(TokenType::Comma)) {
        list.push_back(parse_expression());
    }
    return list;
}

std::vector<std::string> Parser::parse_param_list(bool& is_vararg) {
    std::vector<std::string> params;
    is_vararg = false;

    if (check(TokenType::RParen)) {
        return params;
    }

    while (true) {
        if (match(TokenType::Vararg)) {
            is_vararg = true;
            break;
        }
        params.push_back(consume(TokenType::Identifier, "Expected parameter name").lexeme);
        if (!match(TokenType::Comma)) {
            break;
        }
    }

    return params;
}

} // namespace lua2ts
