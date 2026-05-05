#include "formatter.h"
#include <unordered_set>
#include <stdexcept>
#include <sstream>

namespace uzpp {

std::string Formatter::format(const Program* program) {
    if (!program) return "";
    output_.str("");
    output_.clear();
    indentLevel_ = 0;
    lineStart_ = true;
    lastToken_.clear();

    formatNodes(program->getChildren());
    if (!lineStart_) emitNewline();

    return output_.str();
}

// ===== DISPATCH =====

void Formatter::formatNodes(const std::vector<std::unique_ptr<ASTNode>>& nodes) {
    for (const auto& node : nodes) {
        formatNode(node.get());
    }
}

void Formatter::formatNode(const ASTNode* node) {
    if (!node) return;

    switch (node->getType()) {
        // Legacy token/group nodes
        case ASTNodeType::Token:
            emitTokenNode(static_cast<const TokenNode*>(node));
            return;
        case ASTNodeType::Group:
            emitGroupNode(static_cast<const GroupNode*>(node));
            return;
        case ASTNodeType::Program:
            formatNodes(static_cast<const Program*>(node)->getChildren());
            return;

        // Declarations
        case ASTNodeType::FunctionDeclaration:
            formatFunctionDeclaration(static_cast<const FunctionDeclaration*>(node));
            return;
        case ASTNodeType::ClassDeclaration:
            formatClassDeclaration(static_cast<const ClassDeclaration*>(node));
            return;
        case ASTNodeType::NamespaceDeclaration:
            formatNamespaceDeclaration(static_cast<const NamespaceDeclaration*>(node));
            return;
        case ASTNodeType::InterfaceDeclaration:
            formatInterfaceDeclaration(static_cast<const InterfaceDeclaration*>(node));
            return;
        case ASTNodeType::TypeAlias:
            formatTypeAlias(static_cast<const TypeAlias*>(node));
            return;
        case ASTNodeType::IncludeStatement:
            formatIncludeStatement(static_cast<const IncludeStatement*>(node));
            return;
        case ASTNodeType::LinkStatement:
            formatLinkStatement(static_cast<const LinkStatement*>(node));
            return;

        // Statements
        default:
            if (const auto* stmt = dynamic_cast<const Statement*>(node)) {
                formatStatement(stmt);
            }
    }
}

// ===== DECLARATIONS =====

void Formatter::formatFunctionDeclaration(const FunctionDeclaration* decl) {
    if (!decl) return;
    if (decl->isAsync()) { emitRaw("asinxron"); emitRaw(" "); }
    if (decl->isTest())  { emitRaw("@sinov"); emitNewline(); }
    if (decl->isBench()) { emitRaw("@bench"); emitNewline(); }
    writeIndent();
    emitRaw(decl->getReturnType());
    emitRaw(" ");
    emitRaw(decl->getName());
    emitRaw("(");
    const auto& params = decl->getParameters();
    for (std::size_t i = 0; i < params.size(); ++i) {
        if (i > 0) emitRaw(", ");
        if (params[i].isConst) emitRaw("ozgarmas ");
        emitRaw(params[i].type);
        emitRaw(" ");
        emitRaw(params[i].name);
    }
    emitRaw(")");
    emitNewline();
    formatBlock(decl->getBody());
    emitNewline();
}

void Formatter::formatClassDeclaration(const ClassDeclaration* decl) {
    if (!decl) return;
    writeIndent();
    emitRaw("sinf ");
    emitRaw(decl->getName());
    if (!decl->getBaseClass().empty()) {
        emitRaw(" meros ");
        emitRaw(decl->getBaseClass());
    }
    emitRaw(" {");
    emitNewline();
    indentLevel_++;

    for (const auto& m : decl->getMembers()) {
        writeIndent();
        emitRaw(m.type);
        emitRaw(" ");
        emitRaw(m.name);
        emitRaw(";");
        emitNewline();
    }
    for (const auto& m : decl->getMethods()) {
        writeIndent();
        if (!m->returnType.empty()) { emitRaw(m->returnType); emitRaw(" "); }
        emitRaw(m->name);
        emitRaw("(");
        for (std::size_t i = 0; i < m->params.size(); ++i) {
            if (i > 0) emitRaw(", ");
            if (m->params[i].isConst) emitRaw("ozgarmas ");
            emitRaw(m->params[i].type);
            emitRaw(" ");
            emitRaw(m->params[i].name);
        }
        emitRaw(")");
        emitNewline();
        if (m->body) formatBlock(m->body.get());
        emitNewline();
    }

    indentLevel_--;
    writeIndent();
    emitRaw("}");
    emitNewline();
}

void Formatter::formatNamespaceDeclaration(const NamespaceDeclaration* decl) {
    if (!decl) return;
    writeIndent();
    emitRaw("nomlar_fazosi ");
    emitRaw(decl->getName());
    emitRaw(" {");
    emitNewline();
    indentLevel_++;
    formatNodes(decl->getChildren());
    indentLevel_--;
    writeIndent();
    emitRaw("}");
    emitNewline();
}

void Formatter::formatInterfaceDeclaration(const InterfaceDeclaration* decl) {
    if (!decl) return;
    writeIndent();
    emitRaw("shartnoma ");
    emitRaw(decl->getName());
    emitRaw(" {");
    emitNewline();
    indentLevel_++;
    for (const auto& m : decl->getMethods()) {
        writeIndent();
        emitRaw(m->returnType);
        emitRaw(" ");
        emitRaw(m->name);
        emitRaw("(");
        for (std::size_t i = 0; i < m->params.size(); ++i) {
            if (i > 0) emitRaw(", ");
            emitRaw(m->params[i].type);
            emitRaw(" ");
            emitRaw(m->params[i].name);
        }
        emitRaw(");");
        emitNewline();
    }
    indentLevel_--;
    writeIndent();
    emitRaw("}");
    emitNewline();
}

void Formatter::formatTypeAlias(const TypeAlias* decl) {
    if (!decl) return;
    writeIndent();
    emitRaw("tur ");
    emitRaw(decl->getAlias());
    emitRaw(" = ");
    emitRaw(decl->getTarget());
    emitRaw(";");
    emitNewline();
}

void Formatter::formatIncludeStatement(const IncludeStatement* stmt) {
    if (!stmt) return;
    writeIndent();
    emitRaw("ulash \"");
    emitRaw(stmt->getModuleName());
    emitRaw("\";");
    emitNewline();
}

void Formatter::formatLinkStatement(const LinkStatement* stmt) {
    if (!stmt) return;
    writeIndent();
    emitRaw("ulash_kutubxona \"");
    emitRaw(stmt->getLibName());
    emitRaw("\";");
    emitNewline();
}

// ===== STATEMENTS =====

void Formatter::formatStatement(const Statement* stmt) {
    if (!stmt) return;

    switch (stmt->getType()) {
        case ASTNodeType::Block:
            formatBlock(static_cast<const Block*>(stmt));
            break;
        case ASTNodeType::IfStatement:
            formatIfStatement(static_cast<const IfStatement*>(stmt));
            break;
        case ASTNodeType::WhileStatement:
            formatWhileStatement(static_cast<const WhileStatement*>(stmt));
            break;
        case ASTNodeType::ForStatement:
            formatForStatement(static_cast<const ForStatement*>(stmt));
            break;
        case ASTNodeType::ReturnStatement:
            formatReturnStatement(static_cast<const ReturnStatement*>(stmt));
            break;
        case ASTNodeType::BreakStatement:
            writeIndent(); emitRaw("to'xtatish;"); emitNewline();
            break;
        case ASTNodeType::ContinueStatement:
            writeIndent(); emitRaw("davom_etish;"); emitNewline();
            break;
        case ASTNodeType::ExpressionStatement:
            formatExpressionStatement(static_cast<const ExpressionStatement*>(stmt));
            break;
        case ASTNodeType::VariableDeclaration:
            formatVariableDeclaration(static_cast<const VariableDeclaration*>(stmt));
            break;
        case ASTNodeType::MatchStatement:
            formatMatchStatement(static_cast<const MatchStatement*>(stmt));
            break;
        default:
            break;
    }
}

void Formatter::formatBlock(const Block* block) {
    if (!block) return;
    writeIndent();
    emitRaw("{");
    emitNewline();
    indentLevel_++;
    for (const auto& s : block->getStatements()) {
        formatStatement(s.get());
    }
    indentLevel_--;
    writeIndent();
    emitRaw("}");
    emitNewline();
}

void Formatter::formatIfStatement(const IfStatement* stmt) {
    if (!stmt) return;
    writeIndent();
    emitRaw("agar (");
    formatExpression(stmt->getCondition());
    emitRaw(")");
    emitNewline();
    indentLevel_++;
    formatStatement(stmt->getThenBranch());
    indentLevel_--;
    if (stmt->getElseBranch()) {
        writeIndent();
        emitRaw("aks");
        emitNewline();
        indentLevel_++;
        formatStatement(stmt->getElseBranch());
        indentLevel_--;
    }
}

void Formatter::formatWhileStatement(const WhileStatement* stmt) {
    if (!stmt) return;
    writeIndent();
    emitRaw("toki (");
    formatExpression(stmt->getCondition());
    emitRaw(")");
    emitNewline();
    indentLevel_++;
    formatStatement(stmt->getBody());
    indentLevel_--;
}

void Formatter::formatForStatement(const ForStatement* stmt) {
    if (!stmt) return;
    writeIndent();
    emitRaw("uchun (");
    if (stmt->isRangeBased()) {
        if (stmt->getInit()) {
            if (stmt->getInit()->getType() == ASTNodeType::VariableDeclaration) {
                const auto* v = static_cast<const VariableDeclaration*>(stmt->getInit());
                emitRaw(v->getTypeName());
                emitRaw(" ");
                emitRaw(v->getName());
            }
        }
        emitRaw(" : ");
        if (stmt->getCondition()) formatExpression(stmt->getCondition());
    } else {
        if (stmt->getInit()) {
            if (stmt->getInit()->getType() == ASTNodeType::VariableDeclaration) {
                const auto* v = static_cast<const VariableDeclaration*>(stmt->getInit());
                emitRaw(v->getTypeName());
                emitRaw(" ");
                emitRaw(v->getName());
                if (v->getInitializer()) { emitRaw(" = "); formatExpression(v->getInitializer()); }
            }
        }
        emitRaw("; ");
        if (stmt->getCondition()) formatExpression(stmt->getCondition());
        emitRaw("; ");
        if (stmt->getIncrement()) formatExpression(stmt->getIncrement());
    }
    emitRaw(")");
    emitNewline();
    indentLevel_++;
    formatStatement(stmt->getBody());
    indentLevel_--;
}

void Formatter::formatReturnStatement(const ReturnStatement* stmt) {
    if (!stmt) return;
    writeIndent();
    emitRaw("qaytarish");
    if (stmt->getValue()) {
        emitRaw(" ");
        formatExpression(stmt->getValue());
    }
    emitRaw(";");
    emitNewline();
}

void Formatter::formatExpressionStatement(const ExpressionStatement* stmt) {
    if (!stmt) return;
    writeIndent();
    formatExpression(stmt->getExpression());
    emitRaw(";");
    emitNewline();
}

void Formatter::formatVariableDeclaration(const VariableDeclaration* decl) {
    if (!decl) return;
    writeIndent();
    if (decl->isConst()) emitRaw("ozgarmas ");
    emitRaw(decl->getTypeName());
    emitRaw(" ");
    emitRaw(decl->getName());
    if (decl->getInitializer()) {
        emitRaw(" = ");
        formatExpression(decl->getInitializer());
    }
    emitRaw(";");
    emitNewline();
}

void Formatter::formatMatchStatement(const MatchStatement* stmt) {
    if (!stmt) return;
    writeIndent();
    emitRaw("moslash (");
    formatExpression(stmt->getCondition());
    emitRaw(") {");
    emitNewline();
    indentLevel_++;
    for (const auto& c : stmt->getCases()) {
        writeIndent();
        if (c->pattern) {
            emitRaw("holat ");
            formatExpression(c->pattern.get());
            emitRaw(":");
        } else {
            emitRaw("boshqa:");
        }
        emitNewline();
        indentLevel_++;
        formatStatement(c->body.get());
        indentLevel_--;
    }
    indentLevel_--;
    writeIndent();
    emitRaw("}");
    emitNewline();
}

// ===== EXPRESSIONS =====

void Formatter::formatExpression(const Expression* expr) {
    if (!expr) return;
    switch (expr->getType()) {
        case ASTNodeType::LiteralExpression: {
            const auto* lit = static_cast<const LiteralExpression*>(expr);
            emitRaw(lit->getValue());
            break;
        }
        case ASTNodeType::IdentifierExpression: {
            const auto* id = static_cast<const IdentifierExpression*>(expr);
            emitRaw(id->getName());
            break;
        }
        case ASTNodeType::BinaryExpression: {
            const auto* bin = static_cast<const BinaryExpression*>(expr);
            emitRaw("(");
            formatExpression(bin->getLeft());
            emitRaw(" ");
            emitRaw(bin->getOperator());
            emitRaw(" ");
            formatExpression(bin->getRight());
            emitRaw(")");
            break;
        }
        case ASTNodeType::UnaryExpression: {
            const auto* un = static_cast<const UnaryExpression*>(expr);
            if (un->isPrefix()) {
                std::string op;
                switch (un->getOperator()) {
                    case UnaryExpression::UnaryOp::Minus:      op = "-";  break;
                    case UnaryExpression::UnaryOp::LogicalNot: op = "!";  break;
                    case UnaryExpression::UnaryOp::BitwiseNot: op = "~";  break;
                    case UnaryExpression::UnaryOp::PreIncrement: op = "++"; break;
                    case UnaryExpression::UnaryOp::PreDecrement: op = "--"; break;
                    default: break;
                }
                emitRaw(op);
                formatExpression(un->getExpression());
            } else {
                formatExpression(un->getExpression());
                std::string op;
                switch (un->getOperator()) {
                    case UnaryExpression::UnaryOp::PostIncrement: op = "++"; break;
                    case UnaryExpression::UnaryOp::PostDecrement: op = "--"; break;
                    default: break;
                }
                emitRaw(op);
            }
            break;
        }
        case ASTNodeType::FunctionCall: {
            const auto* call = static_cast<const FunctionCall*>(expr);
            formatExpression(call->getCallee());
            emitRaw("(");
            const auto& args = call->getArguments();
            for (std::size_t i = 0; i < args.size(); ++i) {
                if (i > 0) emitRaw(", ");
                formatExpression(args[i].get());
            }
            emitRaw(")");
            break;
        }
        case ASTNodeType::MemberAccess: {
            const auto* ma = static_cast<const MemberAccess*>(expr);
            formatExpression(ma->getObject());
            emitRaw(ma->getAccessType() == MemberAccess::AccessType::Dot ? "." : "->");
            emitRaw(ma->getMemberName());
            break;
        }
        case ASTNodeType::SubscriptAccess: {
            const auto* sa = static_cast<const SubscriptAccess*>(expr);
            formatExpression(sa->getArray());
            emitRaw("[");
            formatExpression(sa->getIndex());
            emitRaw("]");
            break;
        }
        case ASTNodeType::AssignmentExpression: {
            const auto* as = static_cast<const AssignmentExpression*>(expr);
            formatExpression(as->getTarget());
            emitRaw(" ");
            emitRaw(as->getOperator());
            emitRaw(" ");
            formatExpression(as->getValue());
            break;
        }
        case ASTNodeType::AwaitExpression: {
            const auto* aw = static_cast<const AwaitExpression*>(expr);
            emitRaw("kutish ");
            formatExpression(aw->getExpression());
            break;
        }
        case ASTNodeType::PipelineExpression: {
            const auto* pipe = static_cast<const PipelineExpression*>(expr);
            formatExpression(pipe->getLeft());
            emitRaw(" |> ");
            formatExpression(pipe->getRight());
            break;
        }
        case ASTNodeType::TernaryExpression: {
            const auto* tern = static_cast<const TernaryExpression*>(expr);
            formatExpression(tern->getCondition());
            emitRaw(" ? ");
            formatExpression(tern->getThenExpr());
            emitRaw(" : ");
            formatExpression(tern->getElseExpr());
            break;
        }
        case ASTNodeType::LambdaExpression: {
            const auto* lam = static_cast<const LambdaExpression*>(expr);
            emitRaw("[");
            bool first = true;
            for (const auto& cap : lam->getCaptures()) {
                if (!first) emitRaw(", ");
                first = false;
                if (cap.byRef && cap.name == "&") emitRaw("&");
                else if (cap.byRef) { emitRaw("&"); emitRaw(cap.name); }
                else emitRaw(cap.name);
            }
            emitRaw("](");
            first = true;
            for (const auto& p : lam->getParams()) {
                if (!first) emitRaw(", ");
                first = false;
                if (!p.type.empty()) { emitRaw(p.type); emitRaw(" "); }
                emitRaw(p.name);
            }
            emitRaw(")");
            if (lam->getBody()) {
                emitNewline();
                formatStatement(lam->getBody());
            }
            break;
        }
        default:
            break;
    }
}

// ===== LEGACY NODES =====

void Formatter::emitTokenNode(const TokenNode* node) {
    const std::string& val = node->getToken().value;
    if (val.empty() || node->getToken().type == TokenType::EndOfFile) return;
    emitRawToken(val);
    if (val == ";" || val == "ochiq:" || val == "yopiq:" || val == "himoyalangan:") {
        emitNewline();
    }
}

void Formatter::emitGroupNode(const GroupNode* node) {
    const std::string opening = node->getOpeningToken().value;
    const std::string closing = node->getClosingToken().value;

    if (opening == "{") {
        emitRawToken("{");
        emitNewline();
        ++indentLevel_;
        for (const auto& child : node->getChildren()) formatNode(child.get());
        if (!lineStart_) emitNewline();
        --indentLevel_;
        writeIndent();
        output_ << closing;
        lineStart_ = false;
        lastToken_ = closing;
        return;
    }

    emitRawToken(opening);
    for (const auto& child : node->getChildren()) formatNode(child.get());
    emitRawToken(closing);
}

// ===== UTILITIES =====

bool Formatter::needsSpaceBefore(const std::string& previous, const std::string& current) const {
    static const std::unordered_set<std::string> noSpaceBefore = {
        ")", "]", "}", ";", ",", ".", "::", "->", ":", "(", "["
    };
    static const std::unordered_set<std::string> noSpaceAfter = {
        "(", "[", "{", ".", "::", "->", "@"
    };
    if (previous.empty() || noSpaceBefore.contains(current) || noSpaceAfter.contains(previous))
        return false;
    return true;
}

void Formatter::writeIndent() {
    if (!lineStart_) return;
    for (int i = 0; i < indentLevel_; ++i) output_ << "    ";
    lineStart_ = false;
}

void Formatter::emitRaw(const std::string& text) {
    output_ << text;
    lastToken_ = text;
    lineStart_ = false;
}

void Formatter::emitRawToken(const std::string& token) {
    if (token.empty()) return;
    writeIndent();
    if (needsSpaceBefore(lastToken_, token)) output_ << ' ';
    output_ << token;
    lastToken_ = token;
    lineStart_ = false;
}

void Formatter::emitNewline() {
    output_ << '\n';
    lineStart_ = true;
    lastToken_.clear();
}

} // namespace uzpp
