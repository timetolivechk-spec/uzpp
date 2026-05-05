#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace uzpp {

enum class TokenType {
    Identifier,
    IntegerLiteral,
    FloatLiteral,
    StringLiteral,
    FormatStringLiteral,
    CharLiteral,
    Symbol,
    EndOfFile
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
};

class Lexer {
public:
    explicit Lexer(std::string source);

    std::vector<Token> tokenize();

private:
    std::string source_;
    std::size_t position_;
    int line_;
    int column_;

    bool isAtEnd() const;
    char peek(std::size_t lookahead = 0) const;
    char advance();
    bool match(char expected);

    void skipWhitespaceAndComments();
    bool isIdentifierStart(char value) const;
    bool isIdentifierPart(char value) const;

    Token makeToken(TokenType type, std::size_t start, int line, int column) const;
    Token scanIdentifier();
    Token scanNumber();
    Token scanString();
    Token scanTripleString();
    Token scanFormatString();
    Token scanCharacter();
    Token scanSymbol();
};

} // namespace uzpp
