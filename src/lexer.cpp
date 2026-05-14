#include "lexer.h"

#include <cctype>
#include <stdexcept>
#include <string>
#include <vector>

namespace uzpp {

namespace {

bool isIdentifierSuffix(char value) {
    return std::isalnum(static_cast<unsigned char>(value)) != 0 || value == '_' || value == '\'' || value == '`';
}

} // namespace

Lexer::Lexer(std::string source)
    : source_(std::move(source)),
      position_(0),
      line_(1),
      column_(1) {}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (!isAtEnd()) {
        skipWhitespaceAndComments();
        if (isAtEnd()) {
            break;
        }
        const std::size_t sizeBefore = tokens.size();

        // RAII helper: at scope exit, attach any pending comments to the
        // most recently produced token. Works for all scan branches below.
        struct AttachAtEnd {
            std::vector<Token>& tokens;
            std::size_t sizeBefore;
            Lexer& self;
            ~AttachAtEnd() {
                if (tokens.size() > sizeBefore && !self.pendingComments_.empty()) {
                    tokens[sizeBefore].leadingComments = self.takeQueuedComments();
                }
            }
        } attach{tokens, sizeBefore, *this};

        const char current = peek();
        if (current == 'f' && peek(1) == '"') {
            tokens.push_back(scanFormatString());
            continue;
        }

        // C++11 raw string literal: R"(...)" or R"delim(...)delim"
        // We emit the whole literal (including R-prefix and delimiters) so
        // codegen can pass it straight through to g++.
        if (current == 'R' && peek(1) == '"') {
            const std::size_t start = position_;
            const int startLine = line_;
            const int startColumn = column_;
            advance(); advance(); // R"
            std::string delim;
            while (!isAtEnd() && peek() != '(') {
                delim += peek();
                advance();
            }
            if (!isAtEnd()) advance(); // '('
            std::string closeSeq = ")" + delim + "\"";
            while (!isAtEnd()) {
                bool matches = true;
                for (std::size_t i = 0; i < closeSeq.size(); ++i) {
                    if (peek(i) != closeSeq[i]) { matches = false; break; }
                }
                if (matches) {
                    for (std::size_t i = 0; i < closeSeq.size(); ++i) advance();
                    tokens.push_back(makeToken(TokenType::StringLiteral, start, startLine, startColumn));
                    goto rawDone;
                }
                advance();
            }
            throw std::runtime_error("Yopilmagan raw string qator: " + std::to_string(startLine));
            rawDone: continue;
        }

        if (isIdentifierStart(current)) {
            tokens.push_back(scanIdentifier());
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(current)) != 0) {
            tokens.push_back(scanNumber());
            continue;
        }

        if (current == '"') {
            // Triple-quoted multiline string: """..."""
            if (peek(1) == '"' && peek(2) == '"') {
                tokens.push_back(scanTripleString());
            } else {
                tokens.push_back(scanString());
            }
            continue;
        }

        if (current == '\'') {
            tokens.push_back(scanCharacter());
            continue;
        }

        tokens.push_back(scanSymbol());
    }

    Token eof{TokenType::EndOfFile, "", line_, column_, takeQueuedComments()};
    tokens.push_back(std::move(eof));
    return tokens;
}

bool Lexer::isAtEnd() const {
    return position_ >= source_.size();
}

char Lexer::peek(std::size_t lookahead) const {
    const std::size_t index = position_ + lookahead;
    if (index >= source_.size()) {
        return '\0';
    }
    return source_[index];
}

char Lexer::advance() {
    const char current = source_[position_++];
    if (current == '\n') {
        ++line_;
        column_ = 1;
    } else {
        ++column_;
    }
    return current;
}

bool Lexer::match(char expected) {
    if (isAtEnd() || source_[position_] != expected) {
        return false;
    }

    ++position_;
    ++column_;
    return true;
}

void Lexer::skipWhitespaceAndComments() {
    while (!isAtEnd()) {
        const char current = peek();

        if (current == ' ' || current == '\r' || current == '\t' || current == '\n') {
            advance();
            continue;
        }

        if (current == '/' && peek(1) == '/') {
            const std::size_t start = position_;
            while (!isAtEnd() && peek() != '\n') {
                advance();
            }
            // Capture the line comment (without trailing newline) for round-trip.
            pendingComments_.push_back(source_.substr(start, position_ - start));
            continue;
        }

        if (current == '/' && peek(1) == '*') {
            const std::size_t start = position_;
            advance();
            advance();
            while (!isAtEnd()) {
                if (peek() == '*' && peek(1) == '/') {
                    advance();
                    advance();
                    break;
                }
                advance();
            }
            pendingComments_.push_back(source_.substr(start, position_ - start));
            continue;
        }

        break;
    }
}

std::vector<std::string> Lexer::takeQueuedComments() {
    std::vector<std::string> out;
    out.swap(pendingComments_);
    return out;
}

bool Lexer::isIdentifierStart(char value) const {
    return std::isalpha(static_cast<unsigned char>(value)) != 0 || value == '_';
}

bool Lexer::isIdentifierPart(char value) const {
    return isIdentifierSuffix(value);
}

Token Lexer::makeToken(TokenType type, std::size_t start, int line, int column) const {
    return Token{type, source_.substr(start, position_ - start), line, column};
}

Token Lexer::scanIdentifier() {
    const std::size_t start = position_;
    const int startLine = line_;
    const int startColumn = column_;

    advance();
    while (!isAtEnd() && isIdentifierPart(peek())) {
        advance();
    }

    return makeToken(TokenType::Identifier, start, startLine, startColumn);
}

Token Lexer::scanNumber() {
    const std::size_t start = position_;
    const int startLine = line_;
    const int startColumn = column_;
    bool isFloat = false;

    if (peek() == '0' && (peek(1) == 'x' || peek(1) == 'X')) {
        advance();
        advance();
        while (std::isxdigit(static_cast<unsigned char>(peek())) != 0) {
            advance();
        }
        while (std::isalnum(static_cast<unsigned char>(peek())) != 0 || peek() == '_') {
            advance();
        }
        return makeToken(TokenType::IntegerLiteral, start, startLine, startColumn);
    }

    if (peek() == '0' && (peek(1) == 'b' || peek(1) == 'B')) {
        advance();
        advance();
        while (peek() == '0' || peek() == '1' || peek() == '_') {
            advance();
        }
        return makeToken(TokenType::IntegerLiteral, start, startLine, startColumn);
    }

    if (peek() == '0' && (peek(1) == 'o' || peek(1) == 'O')) {
        advance();
        advance();
        while ((peek() >= '0' && peek() <= '7') || peek() == '_') {
            advance();
        }
        return makeToken(TokenType::IntegerLiteral, start, startLine, startColumn);
    }

    // C++14 numeric separator: 1'000'000 — apostrophe between digits
    while (std::isdigit(static_cast<unsigned char>(peek())) != 0 ||
           (peek() == '\'' && std::isdigit(static_cast<unsigned char>(peek(1))) != 0)) {
        advance();
    }

    if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peek(1))) != 0) {
        isFloat = true;
        advance();
        while (std::isdigit(static_cast<unsigned char>(peek())) != 0 ||
               (peek() == '\'' && std::isdigit(static_cast<unsigned char>(peek(1))) != 0)) {
            advance();
        }
    }

    if (peek() == 'e' || peek() == 'E') {
        isFloat = true;
        advance();
        if (peek() == '+' || peek() == '-') {
            advance();
        }
        while (std::isdigit(static_cast<unsigned char>(peek())) != 0) {
            advance();
        }
    }

    while (std::isalnum(static_cast<unsigned char>(peek())) != 0 || peek() == '_') {
        advance();
    }

    return makeToken(isFloat ? TokenType::FloatLiteral : TokenType::IntegerLiteral, start, startLine, startColumn);
}

Token Lexer::scanString() {
    const std::size_t start = position_;
    const int startLine = line_;
    const int startColumn = column_;

    advance();
    while (!isAtEnd()) {
        if (peek() == '\\') {
            advance();
            if (!isAtEnd()) {
                advance();
            }
            continue;
        }
        if (peek() == '"') {
            advance();
            return makeToken(TokenType::StringLiteral, start, startLine, startColumn);
        }
        advance();
    }

    throw std::runtime_error("Yopilmagan satr literal qator: " + std::to_string(startLine) +
                             " ustun: " + std::to_string(startColumn));
}

Token Lexer::scanTripleString() {
    const std::size_t start = position_;
    const int startLine = line_;
    const int startColumn = column_;

    // Consume opening """
    advance(); advance(); advance();

    while (!isAtEnd()) {
        if (peek() == '"' && peek(1) == '"' && peek(2) == '"') {
            advance(); advance(); advance(); // consume closing """
            // Token value includes the triple-quote delimiters so codegen can recognize it
            return makeToken(TokenType::StringLiteral, start, startLine, startColumn);
        }
        advance();
    }

    throw std::runtime_error("Yopilmagan uch qo'shtirnoqli satr qator: " + std::to_string(startLine) +
                             " ustun: " + std::to_string(startColumn));
}

Token Lexer::scanFormatString() {
    const std::size_t start = position_;
    const int startLine = line_;
    const int startColumn = column_;

    advance(); // 'f'
    advance(); // '"'
    
    int braceDepth = 0;
    bool inNestedString = false;

    while (!isAtEnd()) {
        char c = peek();
        if (peek() == '\\') {
            advance();
            if (!isAtEnd()) {
                advance();
            }
            continue;
        }
        
        if (c == '{' && !inNestedString) {
            braceDepth++;
        } else if (c == '}' && !inNestedString && braceDepth > 0) {
            braceDepth--;
        } else if (c == '"') {
            if (braceDepth > 0) {
                inNestedString = !inNestedString;
            } else {
                advance();
                return makeToken(TokenType::FormatStringLiteral, start, startLine, startColumn);
            }
        }
        advance();
    }

    throw std::runtime_error("Yopilmagan formatlangan satr literal qator: " + std::to_string(startLine) +
                             " ustun: " + std::to_string(startColumn));
}

Token Lexer::scanCharacter() {
    const std::size_t start = position_;
    const int startLine = line_;
    const int startColumn = column_;

    advance();
    if (peek() == '\\') {
        advance();
        if (!isAtEnd()) {
            advance();
        }
    } else if (!isAtEnd()) {
        advance();
    }

    if (peek() != '\'') {
        throw std::runtime_error("Yopilmagan belgi literal qator: " + std::to_string(startLine) +
                                 " ustun: " + std::to_string(startColumn));
    }

    advance();
    return makeToken(TokenType::CharLiteral, start, startLine, startColumn);
}

Token Lexer::scanSymbol() {
    static const std::vector<std::string> multiCharacterSymbols = {
        "<=>", // C++20 three-way comparison (spaceship) — MUST be before "<=" and ">"
        "...",  // C++ parameter pack ellipsis (variadic templates)
        "::", "->", "++", "--", "<<=", ">>=", "==", "!=", "<=", ">=", "&&", "||", "??" "=", "??",
        "|>", "=>",  // Phase 26: pipeline and arrow operators
        "<<", ">>", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^="
    };

    const std::size_t start = position_;
    const int startLine = line_;
    const int startColumn = column_;

    for (const std::string& symbol : multiCharacterSymbols) {
        bool matches = true;
        for (std::size_t index = 0; index < symbol.size(); ++index) {
            if (peek(index) != symbol[index]) {
                matches = false;
                break;
            }
        }

        if (matches) {
            for (std::size_t index = 0; index < symbol.size(); ++index) {
                advance();
            }
            return makeToken(TokenType::Symbol, start, startLine, startColumn);
        }
    }

    advance();
    return makeToken(TokenType::Symbol, start, startLine, startColumn);
}

} // namespace uzpp
