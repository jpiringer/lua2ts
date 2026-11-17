#pragma once

#include <vector>
#include <memory>
#include "lexer.hpp"
#include "ast.hpp"

namespace lua2ts {

class ParseError : public std::runtime_error {
public:
    size_t line;
    size_t col;
    ParseError(const std::string& msg, size_t l, size_t c)
        : std::runtime_error(msg + " at line " + std::to_string(l) + ":" + std::to_string(c)),
          line(l), col(c) {}
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    std::unique_ptr<BlockStmt> parse();

private:
    const Token& peek() const;
    const Token& peek_next() const;
    const Token& previous() const;
    bool is_at_end() const;
    bool check(TokenType type) const;
    bool match(TokenType type);
    const Token& advance();
    const Token& consume(TokenType type, const std::string& message);

    // Statements
    std::unique_ptr<BlockStmt> parse_block();
    std::unique_ptr<Stmt> parse_statement();
    std::unique_ptr<Stmt> parse_local();
    std::unique_ptr<Stmt> parse_function_stmt();
    std::unique_ptr<Stmt> parse_if_stmt();
    std::unique_ptr<Stmt> parse_while_stmt();
    std::unique_ptr<Stmt> parse_repeat_stmt();
    std::unique_ptr<Stmt> parse_for_stmt();
    std::unique_ptr<Stmt> parse_return_stmt();
    std::unique_ptr<Stmt> parse_expr_or_assign();

    // Expressions
    std::unique_ptr<Expr> parse_expression();
    std::unique_ptr<Expr> parse_sub_expr(int min_precedence);
    std::unique_ptr<Expr> parse_unary_expr();
    std::unique_ptr<Expr> parse_primary_expr();
    std::unique_ptr<Expr> parse_table_constructor();
    std::unique_ptr<Expr> parse_function_expr();

    // Helpers
    std::vector<std::unique_ptr<Expr>> parse_expr_list();
    std::vector<std::string> parse_param_list(bool& is_vararg);

    std::vector<Token> tokens_;
    size_t current_{0};
};

} // namespace lua2ts
