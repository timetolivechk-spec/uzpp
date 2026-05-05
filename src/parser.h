#pragma once

#include "ast.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace uzpp {

class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string& message) : std::runtime_error(message) {}
};

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    std::unique_ptr<Program> parse();

private:
    const std::vector<Token>& tokens_;
    std::size_t current_;

    const Token& peek() const;
    const Token& previous() const;
    bool isAtEnd() const;
    const Token& advance();

    bool isOpeningGroup(const Token& token) const;
    bool isClosingGroup(const Token& token) const;
    std::string matchingCloser(const Token& token) const;
    std::string formatLocation(const Token& token) const;

    // ===== LEGACY TOKEN-BASED PARSING =====
    std::vector<std::unique_ptr<ASTNode>> parseNodesUntil(const std::string& closingSymbol);
    std::unique_ptr<ASTNode> parseNode();
    std::unique_ptr<GroupNode> parseGroup();
    
    // ===== SEMANTIC EXPRESSION PARSING =====
    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parseAssignmentExpression();
    std::unique_ptr<Expression> parseNullCoalescingExpression();
    std::unique_ptr<Expression> parseLogicalOrExpression();
    std::unique_ptr<Expression> parseLogicalAndExpression();
    std::unique_ptr<Expression> parseEqualityExpression();
    std::unique_ptr<Expression> parseRelationalExpression();
    std::unique_ptr<Expression> parseAdditiveExpression();
    std::unique_ptr<Expression> parseMultiplicativeExpression();
    std::unique_ptr<Expression> parseUnaryExpression();
    std::unique_ptr<Expression> parsePostfixExpression();
    std::unique_ptr<Expression> parsePrimaryExpression();
    std::unique_ptr<Expression> parseIdentifierOrCall();
    
    // Helper for operator precedence
    bool isAssignmentOperator(const std::string& text) const;
    bool isNullCoalescingOperator(const std::string& text) const;
    bool isLogicalOrOperator(const std::string& text) const;
    bool isLogicalAndOperator(const std::string& text) const;
    bool isEqualityOperator(const std::string& text) const;
    bool isRelationalOperator(const std::string& text) const;
    bool isAdditiveOperator(const std::string& text) const;
    bool isMultiplicativeOperator(const std::string& text) const;
    bool isUnaryOperator(const std::string& text) const;
    bool isPostfixOperator(const std::string& text) const;
    
    // ===== SEMANTIC STATEMENT PARSING =====
    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<Block> parseBlock();
    std::unique_ptr<MatchStatement> parseMatchStatement();
    std::unique_ptr<IfStatement> parseIfStatement();
    std::unique_ptr<WhileStatement> parseWhileStatement();
    std::unique_ptr<ForStatement> parseForStatement();
    std::unique_ptr<ReturnStatement> parseReturnStatement();
    std::unique_ptr<BreakStatement> parseBreakStatement();
    std::unique_ptr<ContinueStatement> parseContinueStatement();
    std::unique_ptr<Statement> parseDeclarationOrExpressionStatement();
    
    // ===== SEMANTIC DECLARATION PARSING =====
    std::unique_ptr<ASTNode> parseGlobalDeclaration();
    std::unique_ptr<NamespaceDeclaration> parseNamespaceDeclaration();
    std::unique_ptr<IncludeStatement> parseIncludeStatement();
    std::unique_ptr<LinkStatement> parseLinkStatement();
    std::unique_ptr<VariableDeclaration> parseVariableDeclaration(const std::string& typeName, const std::string& varName);
    std::unique_ptr<FunctionDeclaration> parseFunctionDeclaration(const std::string& returnType, const std::string& funcName);
    std::unique_ptr<FunctionDeclaration> parseLegacyFunctionDeclaration();
    std::unique_ptr<ClassDeclaration> parseClassDeclaration();
    std::unique_ptr<InterfaceDeclaration> parseInterfaceDeclaration();
    std::vector<FunctionDeclaration::Parameter> parseFunctionParameters();
    
    // ===== HELPER METHODS =====
    bool checkKeyword(const std::string& keyword) const;
    bool matchKeyword(const std::string& keyword);
    bool isUzbekKeyword(const std::string& text) const;
    std::string getKeywordTranslation(const std::string& text) const;
};

} // namespace uzpp
