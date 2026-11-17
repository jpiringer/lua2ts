#pragma once

#include <string>
#include <vector>
#include <string_view>
#include <ostream>

namespace lua2ts {

enum class TokenType {
    // End of file / Invalid
    Eof,
    Invalid,

    // Keywords
    KwAnd, KwBreak, KwDo, KwElse, KwElseif, KwEnd,
    KwFalse, KwFor, KwFunction, KwGoto, KwIf, KwIn,
    KwLocal, KwNil, KwNot, KwOr, KwRepeat, KwReturn,
    KwThen, KwTrue, KwUntil, KwWhile,

    // Identifiers and Literals
    Identifier,
    Number,
    String,

    // Operators and Punctuations
    Plus,           // +
    Minus,          // -
    Star,           // *
    Slash,          // /
    DoubleSlash,    // //
    Percent,        // %
    Caret,          // ^
    Hash,           // #
    Ampersand,      // &
    Tilde,          // ~
    Pipe,           // |
    ShiftLeft,      // <<
    ShiftRight,     // >>
    Concat,         // ..
    Vararg,         // ...
    EqualEqual,     // ==
    TildeEqual,     // ~=
    LessEqual,      // <=
    GreaterEqual,   // >=
    Less,           // <
    Greater,        // >
    Equal,          // =
    LParen,         // (
    RParen,         // )
    LBrace,         // {
    RBrace,         // }
    LBracket,       // [
    RBracket,       // ]
    DoubleColon,    // ::
    Semicolon,      // ;
    Colon,          // :
    Comma,          // ,
    Dot             // .
};

std::string_view token_type_to_string(TokenType type);

struct Token {
    TokenType type{TokenType::Invalid};
    std::string lexeme;
    size_t line{1};
    size_t col{1};

    Token() = default;
    Token(TokenType t, std::string lex, size_t l, size_t c)
        : type(t), lexeme(std::move(lex)), line(l), col(c) {}
};

class Lexer {
public:
    explicit Lexer(std::string source);

    std::vector<Token> tokenize();

private:
    char peek() const;
    char peek_next() const;
    char advance();
    bool is_at_end() const;
    bool match(char expected);

    void scan_token();
    void scan_identifier_or_keyword();
    void scan_number();
    void scan_string(char quote);
    void scan_multiline_string();
    void scan_comment();

    void add_token(TokenType type);
    void add_token(TokenType type, std::string lexeme);

    std::string source_;
    size_t start_{0};
    size_t current_{0};
    size_t line_{1};
    size_t col_{1};
    size_t token_start_col_{1};
    std::vector<Token> tokens_;
};

} // namespace lua2ts
