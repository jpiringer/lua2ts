#include "lexer.hpp"
#include <unordered_map>
#include <cctype>
#include <stdexcept>

namespace lua2ts {

std::string_view token_type_to_string(TokenType type) {
    switch (type) {
        case TokenType::Eof: return "EOF";
        case TokenType::Invalid: return "INVALID";
        case TokenType::KwAnd: return "and";
        case TokenType::KwBreak: return "break";
        case TokenType::KwDo: return "do";
        case TokenType::KwElse: return "else";
        case TokenType::KwElseif: return "elseif";
        case TokenType::KwEnd: return "end";
        case TokenType::KwFalse: return "false";
        case TokenType::KwFor: return "for";
        case TokenType::KwFunction: return "function";
        case TokenType::KwGoto: return "goto";
        case TokenType::KwIf: return "if";
        case TokenType::KwIn: return "in";
        case TokenType::KwLocal: return "local";
        case TokenType::KwNil: return "nil";
        case TokenType::KwNot: return "not";
        case TokenType::KwOr: return "or";
        case TokenType::KwRepeat: return "repeat";
        case TokenType::KwReturn: return "return";
        case TokenType::KwThen: return "then";
        case TokenType::KwTrue: return "true";
        case TokenType::KwUntil: return "until";
        case TokenType::KwWhile: return "while";
        case TokenType::Identifier: return "IDENTIFIER";
        case TokenType::Number: return "NUMBER";
        case TokenType::String: return "STRING";
        case TokenType::Plus: return "+";
        case TokenType::Minus: return "-";
        case TokenType::Star: return "*";
        case TokenType::Slash: return "/";
        case TokenType::DoubleSlash: return "//";
        case TokenType::Percent: return "%";
        case TokenType::Caret: return "^";
        case TokenType::Hash: return "#";
        case TokenType::Ampersand: return "&";
        case TokenType::Tilde: return "~";
        case TokenType::Pipe: return "|";
        case TokenType::ShiftLeft: return "<<";
        case TokenType::ShiftRight: return ">>";
        case TokenType::Concat: return "..";
        case TokenType::Vararg: return "...";
        case TokenType::EqualEqual: return "==";
        case TokenType::TildeEqual: return "~=";
        case TokenType::LessEqual: return "<=";
        case TokenType::GreaterEqual: return ">=";
        case TokenType::Less: return "<";
        case TokenType::Greater: return ">";
        case TokenType::Equal: return "=";
        case TokenType::LParen: return "(";
        case TokenType::RParen: return ")";
        case TokenType::LBrace: return "{";
        case TokenType::RBrace: return "}";
        case TokenType::LBracket: return "[";
        case TokenType::RBracket: return "]";
        case TokenType::DoubleColon: return "::";
        case TokenType::Semicolon: return ";";
        case TokenType::Colon: return ":";
        case TokenType::Comma: return ",";
        case TokenType::Dot: return ".";
    }
    return "UNKNOWN";
}

static const std::unordered_map<std::string, TokenType> kKeywords = {
    {"and", TokenType::KwAnd},
    {"break", TokenType::KwBreak},
    {"do", TokenType::KwDo},
    {"else", TokenType::KwElse},
    {"elseif", TokenType::KwElseif},
    {"end", TokenType::KwEnd},
    {"false", TokenType::KwFalse},
    {"for", TokenType::KwFor},
    {"function", TokenType::KwFunction},
    {"goto", TokenType::KwGoto},
    {"if", TokenType::KwIf},
    {"in", TokenType::KwIn},
    {"local", TokenType::KwLocal},
    {"nil", TokenType::KwNil},
    {"not", TokenType::KwNot},
    {"or", TokenType::KwOr},
    {"repeat", TokenType::KwRepeat},
    {"return", TokenType::KwReturn},
    {"then", TokenType::KwThen},
    {"true", TokenType::KwTrue},
    {"until", TokenType::KwUntil},
    {"while", TokenType::KwWhile}
};

Lexer::Lexer(std::string source) : source_(std::move(source)) {}

char Lexer::peek() const {
    if (is_at_end()) return '\0';
    return source_[current_];
}

char Lexer::peek_next() const {
    if (current_ + 1 >= source_.size()) return '\0';
    return source_[current_ + 1];
}

char Lexer::advance() {
    char c = source_[current_++];
    col_++;
    return c;
}

bool Lexer::is_at_end() const {
    return current_ >= source_.size();
}

bool Lexer::match(char expected) {
    if (is_at_end() || source_[current_] != expected) return false;
    current_++;
    col_++;
    return true;
}

void Lexer::add_token(TokenType type) {
    std::string text = source_.substr(start_, current_ - start_);
    tokens_.emplace_back(type, std::move(text), line_, token_start_col_);
}

void Lexer::add_token(TokenType type, std::string lexeme) {
    tokens_.emplace_back(type, std::move(lexeme), line_, token_start_col_);
}

std::vector<Token> Lexer::tokenize() {
    tokens_.clear();
    start_ = 0;
    current_ = 0;
    line_ = 1;
    col_ = 1;

    while (!is_at_end()) {
        start_ = current_;
        token_start_col_ = col_;
        scan_token();
    }

    tokens_.emplace_back(TokenType::Eof, "", line_, col_);
    return tokens_;
}

void Lexer::scan_token() {
    char c = advance();
    switch (c) {
        case ' ':
        case '\r':
        case '\t':
            // Ignore whitespace
            break;
        case '\n':
            line_++;
            col_ = 1;
            break;
        case '+': add_token(TokenType::Plus); break;
        case '*': add_token(TokenType::Star); break;
        case '%': add_token(TokenType::Percent); break;
        case '^': add_token(TokenType::Caret); break;
        case '#': add_token(TokenType::Hash); break;
        case '&': add_token(TokenType::Ampersand); break;
        case '|': add_token(TokenType::Pipe); break;
        case '(': add_token(TokenType::LParen); break;
        case ')': add_token(TokenType::RParen); break;
        case '{': add_token(TokenType::LBrace); break;
        case '}': add_token(TokenType::RBrace); break;
        case ']': add_token(TokenType::RBracket); break;
        case ';': add_token(TokenType::Semicolon); break;
        case ',': add_token(TokenType::Comma); break;

        case '-':
            if (match('-')) {
                scan_comment();
            } else {
                add_token(TokenType::Minus);
            }
            break;

        case '/':
            if (match('/')) {
                add_token(TokenType::DoubleSlash);
            } else {
                add_token(TokenType::Slash);
            }
            break;

        case '~':
            if (match('=')) {
                add_token(TokenType::TildeEqual);
            } else {
                add_token(TokenType::Tilde);
            }
            break;

        case '=':
            if (match('=')) {
                add_token(TokenType::EqualEqual);
            } else {
                add_token(TokenType::Equal);
            }
            break;

        case '<':
            if (match('=')) {
                add_token(TokenType::LessEqual);
            } else if (match('<')) {
                add_token(TokenType::ShiftLeft);
            } else {
                add_token(TokenType::Less);
            }
            break;

        case '>':
            if (match('=')) {
                add_token(TokenType::GreaterEqual);
            } else if (match('>')) {
                add_token(TokenType::ShiftRight);
            } else {
                add_token(TokenType::Greater);
            }
            break;

        case ':':
            if (match(':')) {
                add_token(TokenType::DoubleColon);
            } else {
                add_token(TokenType::Colon);
            }
            break;

        case '.':
            if (match('.')) {
                if (match('.')) {
                    add_token(TokenType::Vararg);
                } else {
                    add_token(TokenType::Concat);
                }
            } else if (std::isdigit(peek())) {
                scan_number();
            } else {
                add_token(TokenType::Dot);
            }
            break;

        case '[':
            if (match('[')) {
                scan_multiline_string();
            } else {
                add_token(TokenType::LBracket);
            }
            break;

        case '"':
        case '\'':
            scan_string(c);
            break;

        default:
            if (std::isalpha(c) || c == '_') {
                scan_identifier_or_keyword();
            } else if (std::isdigit(c)) {
                scan_number();
            } else {
                add_token(TokenType::Invalid);
            }
            break;
    }
}

void Lexer::scan_comment() {
    // Check for multiline comment: --[[ ... ]]
    if (peek() == '[' && peek_next() == '[') {
        advance(); // consume [
        advance(); // consume [
        while (!is_at_end()) {
            if (peek() == ']' && peek_next() == ']') {
                advance();
                advance();
                return;
            }
            if (peek() == '\n') {
                line_++;
                col_ = 0;
            }
            advance();
        }
    }
    else {
        // Single line comment
        while (!is_at_end() && peek() != '\n') {
            advance();
        }
    }
}

void Lexer::scan_identifier_or_keyword() {
    while (std::isalnum(peek()) || peek() == '_') {
        advance();
    }
    std::string text = source_.substr(start_, current_ - start_);
    auto it = kKeywords.find(text);
    if (it != kKeywords.end()) {
        add_token(it->second, std::move(text));
    }
    else {
        add_token(TokenType::Identifier, std::move(text));
    }
}

void Lexer::scan_number() {
    // Handles 0x... and floats like 12.34e+5
    if (source_[start_] == '0' && (peek() == 'x' || peek() == 'X')) {
        advance(); // consume 'x'
        while (std::isxdigit(peek()) || peek() == '.') {
            advance();
        }
        if (peek() == 'p' || peek() == 'P') {
            advance();
            if (peek() == '+' || peek() == '-') advance();
            while (std::isdigit(peek())) advance();
        }
    }
    else {
        while (std::isdigit(peek())) {
            advance();
        }
        if (peek() == '.' && std::isdigit(peek_next())) {
            advance(); // consume '.'
            while (std::isdigit(peek())) {
                advance();
            }
        }
        if (peek() == 'e' || peek() == 'E') {
            advance();
            if (peek() == '+' || peek() == '-') advance();
            while (std::isdigit(peek())) advance();
        }
    }
    std::string text = source_.substr(start_, current_ - start_);
    add_token(TokenType::Number, std::move(text));
}

void Lexer::scan_string(char quote) {
    std::string result;
    while (!is_at_end() && peek() != quote) {
        if (peek() == '\n') {
            line_++;
            col_ = 0;
        }
        if (peek() == '\\') {
            advance(); // consume '\'
            if (is_at_end()) break;
            char esc = advance();
            switch (esc) {
                case 'n': result.push_back('\n'); break;
                case 't': result.push_back('\t'); break;
                case 'r': result.push_back('\r'); break;
                case '\\': result.push_back('\\'); break;
                case '\"': result.push_back('\"'); break;
                case '\'': result.push_back('\''); break;
                case '0': result.push_back('\0'); break;
                default:
                    result.push_back('\\');
                    result.push_back(esc);
                    break;
            }
        }
        else {
            result.push_back(advance());
        }
    }

    if (!is_at_end()) {
        advance(); // consume closing quote
    }
    add_token(TokenType::String, std::move(result));
}

void Lexer::scan_multiline_string() {
    std::string result;
    // Skip newline right after [[ if any (standard Lua behavior)
    if (peek() == '\n') {
        advance();
        line_++;
        col_ = 1;
    }
    while (!is_at_end()) {
        if (peek() == ']' && peek_next() == ']') {
            advance();
            advance();
            break;
        }
        if (peek() == '\n') {
            line_++;
            col_ = 0;
        }
        result.push_back(advance());
    }
    add_token(TokenType::String, std::move(result));
}

} // namespace lua2ts
