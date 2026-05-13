#pragma once

#include "ast.h"

#include <sstream>
#include <string>

namespace uzpp {

// Forward declarations
class Expression;
class Statement;
class BinaryExpression;
class UnaryExpression;
class LiteralExpression;
class IdentifierExpression;
class FunctionCall;
class MemberAccess;
class SubscriptAccess;
class AssignmentExpression;
class IfStatement;
class WhileStatement;
class ForStatement;
class Block;
class MatchStatement;
class ReturnStatement;
class BreakStatement;
class ContinueStatement;
class ExpressionStatement;
class VariableDeclaration;
class FunctionDeclaration;
class ClassDeclaration;
// Phase 26
class PipelineExpression;
class TypeAlias;
class LambdaExpression;
class TernaryExpression;

class CodeGen {
public:
    CodeGen();

    // Legacy token-based generation
    std::string generate(const Program* program, const std::string& sourceName = "input.uzpp", bool testMode = false, bool benchMode = false);

private:
    std::ostringstream output_;
    int indentLevel_;
    std::string lastToken_;
    bool lineStart_;
    int namespaceDepth_ = 0;
    bool testMode_ = false;
    bool benchMode_ = false;
    std::vector<std::string> testFunctions_;
    std::vector<std::string> benchFunctions_;
    bool hasUserMain_ = false;
    bool userMainHasArgs_ = false;

    void reset();
    void writePreamble(const std::string& sourceName);
    std::string escapeForLineDirective(const std::string& sourceName) const;

    // Legacy token-based code generation
    void emitNodes(const std::vector<std::unique_ptr<ASTNode>>& nodes);
    void emitNode(const ASTNode* node, const ASTNode* nextNode);
    void emitTokenNode(const TokenNode* node, const ASTNode* nextNode);
    void emitGroupNode(const GroupNode* node);

    std::string translateToken(const Token& token, const ASTNode* nextNode) const;
    // Standart qiymatni (uz++ sintaksisida) C++ ga tarjima qilish
    std::string translateDefaultValue(const std::string& val) const;
    bool needsSpaceBefore(const std::string& previous, const std::string& current) const;
    void writeIndentIfNeeded();
    void emitRawToken(const std::string& token);
    // O'zbek identifikatorlaridagi apostrof (o', g') ni C++ uchun xavfsiz qilish
    static std::string safeIdent(const std::string& name) {
        if (name.find('\'') == std::string::npos) return name;
        std::string safe = name;
        for (char& c : safe) if (c == '\'') c = '_';
        return safe;
    }
    void emitNewline();
    
    // ===== SEMANTIC VISITOR METHODS (PHASE 11-C) =====
    
    // Expression visitors
    void visitExpression(const Expression* expr);
    void visitBinaryExpression(const BinaryExpression* expr);
    void visitUnaryExpression(const UnaryExpression* expr);
    void visitLiteralExpression(const LiteralExpression* expr);
    void visitIdentifierExpression(const IdentifierExpression* expr);
    void visitFunctionCall(const FunctionCall* expr);
    void visitMemberAccess(const MemberAccess* expr);
    void visitSubscriptAccess(const SubscriptAccess* expr);
    void visitAssignmentExpression(const AssignmentExpression* expr);
    void visitAwaitExpression(const AwaitExpression* expr);
    void visitPipelineExpression(const PipelineExpression* expr);
    void visitLambdaExpression(const LambdaExpression* expr);
    void visitTernaryExpression(const TernaryExpression* expr);
    void visitThrowExpression(const ThrowExpression* expr);
    
    // Statement visitors
    void visitStatement(const Statement* stmt);
    void visitIfStatement(const IfStatement* stmt);
    void visitTryStatement(const TryStatement* stmt);
    void visitWhileStatement(const WhileStatement* stmt);
    void visitForStatement(const ForStatement* stmt);
    void visitBlock(const Block* stmt);
    void visitMatchStatement(const MatchStatement* stmt);
    void visitReturnStatement(const ReturnStatement* stmt);
    void visitBreakStatement(const BreakStatement* stmt);
    void visitContinueStatement(const ContinueStatement* stmt);
    void visitExpressionStatement(const ExpressionStatement* stmt);
    void visitVariableDeclaration(const VariableDeclaration* stmt);
    
    // Declaration visitors
    void visitFunctionDeclaration(const FunctionDeclaration* decl);
    void visitClassDeclaration(const ClassDeclaration* decl);
    void visitInterfaceDeclaration(const InterfaceDeclaration* decl);
    void visitNamespaceDeclaration(const NamespaceDeclaration* decl);
    void visitIncludeStatement(const IncludeStatement* stmt);
    void visitExportModuleStatement(const ExportModuleStatement* stmt);
    void visitLinkStatement(const LinkStatement* stmt);
    void visitTypeAlias(const TypeAlias* decl);
    void visitEnumDeclaration(const EnumDeclaration* decl);
    
    // Helper methods
    std::string getOperatorSymbol(const std::string& op) const;
    std::string getCppType(const std::string& uzppType, int depth = 0) const;
    void indentMore();
    void indentLess();
    bool currentFunctionIsAsync_ = false;
};

} // namespace uzpp
