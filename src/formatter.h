#pragma once

#include "ast.h"
#include <string>
#include <vector>
#include <memory>
#include <sstream>

namespace uzpp {

class Formatter {
public:
    std::string format(const Program* program);

private:
    std::ostringstream output_;
    int indentLevel_ = 0;
    bool lineStart_ = true;
    std::string lastToken_;

    // Dispatch
    void formatNodes(const std::vector<std::unique_ptr<ASTNode>>& nodes);
    void formatNode(const ASTNode* node);

    // Legacy nodes
    void emitTokenNode(const TokenNode* node);
    void emitGroupNode(const GroupNode* node);

    // Declarations
    void formatFunctionDeclaration(const FunctionDeclaration* decl);
    void formatClassDeclaration(const ClassDeclaration* decl);
    void formatNamespaceDeclaration(const NamespaceDeclaration* decl);
    void formatInterfaceDeclaration(const InterfaceDeclaration* decl);
    void formatTypeAlias(const TypeAlias* decl);
    void formatIncludeStatement(const IncludeStatement* stmt);
    void formatLinkStatement(const LinkStatement* stmt);

    // Statements
    void formatStatement(const Statement* stmt);
    void formatBlock(const Block* block);
    void formatIfStatement(const IfStatement* stmt);
    void formatWhileStatement(const WhileStatement* stmt);
    void formatForStatement(const ForStatement* stmt);
    void formatReturnStatement(const ReturnStatement* stmt);
    void formatExpressionStatement(const ExpressionStatement* stmt);
    void formatVariableDeclaration(const VariableDeclaration* decl);
    void formatMatchStatement(const MatchStatement* stmt);

    // Expressions
    void formatExpression(const Expression* expr);

    // Utilities
    void writeIndent();
    void emitRaw(const std::string& text);
    void emitRawToken(const std::string& token);
    void emitNewline();
    bool needsSpaceBefore(const std::string& previous, const std::string& current) const;
};

} // namespace uzpp
