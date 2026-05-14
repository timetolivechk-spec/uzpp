#pragma once

#include "lexer.h"

#include <memory>
#include <string>
#include <vector>

namespace uzpp {

// ===== LEGACY TOKEN & GROUP TREE (MAINTAINED FOR COMPATIBILITY) =====
enum class ASTNodeType {
    // Legacy nodes
    Program,
    Token,
    Group,
    
    // Semantic expression nodes
    LiteralExpression,
    IdentifierExpression,
    BinaryExpression,
    UnaryExpression,
    FunctionCall,
    MemberAccess,
    SubscriptAccess,
    AssignmentExpression,
    
    // Semantic statement nodes
    VariableDeclaration,
    FunctionDeclaration,
    ClassDeclaration,
    IfStatement,
    WhileStatement,
    ForStatement,
    ReturnStatement,
    BreakStatement,
    ContinueStatement,
    ExpressionStatement,
    Block,
    MatchStatement,
    TryStatement,
    ThrowExpression,
    
    // Special nodes
    Namespace,
    NamespaceDeclaration,
    IncludeStatement,
    ExportModuleStatement,
    LinkStatement,
    EnumDeclaration,
    TemplateDeclaration,
    InterfaceDeclaration,
    AwaitExpression,
    // Phase 26: New language features
    PipelineExpression,
    TypeAlias,
    LambdaExpression,
    TernaryExpression,
    StatementList,
    TokenStatement
};

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual ASTNodeType getType() const = 0;
};

// ===== LEGACY NODES (MAINTAINED FOR BACKWARD COMPATIBILITY) =====

class TokenNode final : public ASTNode {
public:
    explicit TokenNode(Token token) : token_(std::move(token)) {}

    ASTNodeType getType() const override {
        return ASTNodeType::Token;
    }

    const Token& getToken() const {
        return token_;
    }

private:
    Token token_;
};

class GroupNode final : public ASTNode {
public:
    GroupNode(Token openingToken, Token closingToken, std::vector<std::unique_ptr<ASTNode>> children)
        : openingToken_(std::move(openingToken)),
          closingToken_(std::move(closingToken)),
          children_(std::move(children)) {}

    ASTNodeType getType() const override {
        return ASTNodeType::Group;
    }

    const Token& getOpeningToken() const {
        return openingToken_;
    }

    const Token& getClosingToken() const {
        return closingToken_;
    }

    const std::vector<std::unique_ptr<ASTNode>>& getChildren() const {
        return children_;
    }

private:
    Token openingToken_;
    Token closingToken_;
    std::vector<std::unique_ptr<ASTNode>> children_;
};

class Program final : public ASTNode {
public:
    explicit Program(std::vector<std::unique_ptr<ASTNode>> children) : children_(std::move(children)) {}

    ASTNodeType getType() const override {
        return ASTNodeType::Program;
    }

    const std::vector<std::unique_ptr<ASTNode>>& getChildren() const {
        return children_;
    }

private:
    std::vector<std::unique_ptr<ASTNode>> children_;
};

// ===== SEMANTIC EXPRESSION NODES =====

class Expression : public ASTNode {
public:
    virtual ~Expression() = default;
};

class LiteralExpression final : public Expression {
public:
    enum class LiteralType { Integer, Float, String, FormatString, Character, Boolean };
    
    LiteralExpression(LiteralType type, const std::string& value, Token sourceToken)
        : type_(type), value_(value), sourceToken_(std::move(sourceToken)) {}
    
    ASTNodeType getType() const override {
        return ASTNodeType::LiteralExpression;
    }
    
    LiteralType getLiteralType() const { return type_; }
    const std::string& getValue() const { return value_; }
    const Token& getSourceToken() const { return sourceToken_; }
    
private:
    LiteralType type_;
    std::string value_;
    Token sourceToken_;
};

class IdentifierExpression final : public Expression {
public:
    IdentifierExpression(const std::string& name, Token sourceToken)
        : name_(name), sourceToken_(std::move(sourceToken)) {}
    
    ASTNodeType getType() const override {
        return ASTNodeType::IdentifierExpression;
    }
    
    const std::string& getName() const { return name_; }
    const Token& getSourceToken() const { return sourceToken_; }
    
private:
    std::string name_;
    Token sourceToken_;
};

class BinaryExpression final : public Expression {
public:
    BinaryExpression(std::unique_ptr<Expression> left, const std::string& op,
                    std::unique_ptr<Expression> right, Token opToken)
        : left_(std::move(left)), op_(op), right_(std::move(right)), opToken_(std::move(opToken)) {}
    
    ASTNodeType getType() const override {
        return ASTNodeType::BinaryExpression;
    }
    
    Expression* getLeft() const { return left_.get(); }
    const std::string& getOperator() const { return op_; }
    Expression* getRight() const { return right_.get(); }
    const Token& getOperatorToken() const { return opToken_; }
    
private:
    std::unique_ptr<Expression> left_;
    std::string op_;
    std::unique_ptr<Expression> right_;
    Token opToken_;
};

class UnaryExpression final : public Expression {
public:
    enum class UnaryOp { Plus, Minus, LogicalNot, BitwiseNot, PreIncrement, PreDecrement,
                         PostIncrement, PostDecrement, AddressOf, Dereference,
                         New, Delete };
    
    UnaryExpression(UnaryOp op, std::unique_ptr<Expression> expr, Token opToken, bool isPrefix = true)
        : op_(op), expr_(std::move(expr)), opToken_(std::move(opToken)), isPrefix_(isPrefix) {}
    
    ASTNodeType getType() const override {
        return ASTNodeType::UnaryExpression;
    }
    
    UnaryOp getOperator() const { return op_; }
    Expression* getExpression() const { return expr_.get(); }
    const Token& getOperatorToken() const { return opToken_; }
    bool isPrefix() const { return isPrefix_; }
    
private:
    UnaryOp op_;
    std::unique_ptr<Expression> expr_;
    Token opToken_;
    bool isPrefix_;
};

class FunctionCall final : public Expression {
public:
    FunctionCall(std::unique_ptr<Expression> callee, std::vector<std::unique_ptr<Expression>> args, Token callToken)
        : callee_(std::move(callee)), args_(std::move(args)), callToken_(std::move(callToken)) {}
    
    ASTNodeType getType() const override {
        return ASTNodeType::FunctionCall;
    }
    
    Expression* getCallee() const { return callee_.get(); }
    const std::vector<std::unique_ptr<Expression>>& getArguments() const { return args_; }
    const Token& getCallToken() const { return callToken_; }
    
private:
    std::unique_ptr<Expression> callee_;
    std::vector<std::unique_ptr<Expression>> args_;
    Token callToken_;
};

class AwaitExpression final : public Expression {
public:
    AwaitExpression(std::unique_ptr<Expression> expr, Token awaitToken, bool isYield = false)
        : expr_(std::move(expr)), awaitToken_(std::move(awaitToken)), isYield_(isYield) {}

    ASTNodeType getType() const override {
        return ASTNodeType::AwaitExpression;
    }

    Expression* getExpression() const { return expr_.get(); }
    const Token& getAwaitToken() const { return awaitToken_; }
    bool isYield() const { return isYield_; }

private:
    std::unique_ptr<Expression> expr_;
    Token awaitToken_;
    bool isYield_;
};

class ThrowExpression final : public Expression {
public:
    ThrowExpression(std::unique_ptr<Expression> expr, Token throwToken)
        : expr_(std::move(expr)), throwToken_(std::move(throwToken)) {}
    
    ASTNodeType getType() const override {
        return ASTNodeType::ThrowExpression;
    }
    
    Expression* getExpression() const { return expr_.get(); }
    const Token& getThrowToken() const { return throwToken_; }
    
private:
    std::unique_ptr<Expression> expr_;
    Token throwToken_;
};

class MemberAccess final : public Expression {
public:
    enum class AccessType { Dot, Arrow };
    
    MemberAccess(std::unique_ptr<Expression> object, AccessType accessType,
                const std::string& memberName, Token accessToken)
        : object_(std::move(object)), accessType_(accessType), memberName_(memberName),
          accessToken_(std::move(accessToken)) {}
    
    ASTNodeType getType() const override {
        return ASTNodeType::MemberAccess;
    }
    
    Expression* getObject() const { return object_.get(); }
    AccessType getAccessType() const { return accessType_; }
    const std::string& getMemberName() const { return memberName_; }
    const Token& getAccessToken() const { return accessToken_; }
    
private:
    std::unique_ptr<Expression> object_;
    AccessType accessType_;
    std::string memberName_;
    Token accessToken_;
};

class SubscriptAccess final : public Expression {
public:
    SubscriptAccess(std::unique_ptr<Expression> array, std::unique_ptr<Expression> index, Token bracketToken)
        : array_(std::move(array)), index_(std::move(index)), bracketToken_(std::move(bracketToken)) {}

    ASTNodeType getType() const override {
        return ASTNodeType::SubscriptAccess;
    }

    Expression* getArray() const { return array_.get(); }
    Expression* getIndex() const { return index_.get(); }
    const Token& getBracketToken() const { return bracketToken_; }

    // C++23 multidimensional subscript: arr[i, j, k]
    const std::vector<std::unique_ptr<Expression>>& getExtraIndices() const { return extraIndices_; }
    void addExtraIndex(std::unique_ptr<Expression> idx) { extraIndices_.push_back(std::move(idx)); }

private:
    std::unique_ptr<Expression> array_;
    std::unique_ptr<Expression> index_;
    Token bracketToken_;
    std::vector<std::unique_ptr<Expression>> extraIndices_;
};

class AssignmentExpression final : public Expression {
public:
    AssignmentExpression(std::unique_ptr<Expression> target, const std::string& op,
                        std::unique_ptr<Expression> value, Token opToken)
        : target_(std::move(target)), op_(op), value_(std::move(value)), opToken_(std::move(opToken)) {}
    
    ASTNodeType getType() const override {
        return ASTNodeType::AssignmentExpression;
    }
    
    Expression* getTarget() const { return target_.get(); }
    const std::string& getOperator() const { return op_; }
    Expression* getValue() const { return value_.get(); }
    const Token& getOperatorToken() const { return opToken_; }
    
private:
    std::unique_ptr<Expression> target_;
    std::string op_;
    std::unique_ptr<Expression> value_;
    Token opToken_;
};

// ===== SEMANTIC STATEMENT NODES =====

class Statement : public ASTNode {
public:
    virtual ~Statement() = default;
};

class Block final : public Statement {
public:
    explicit Block(std::vector<std::unique_ptr<Statement>> statements, Token openBrace = Token())
        : statements_(std::move(statements)), openBrace_(std::move(openBrace)) {}
    
    ASTNodeType getType() const override {
        return ASTNodeType::Block;
    }
    
    const std::vector<std::unique_ptr<Statement>>& getStatements() const { return statements_; }
    
private:
    std::vector<std::unique_ptr<Statement>> statements_;
    Token openBrace_;
};

// Flat statement sequence — emitted without surrounding braces (used for comma-declarations)
class StatementList final : public Statement {
public:
    explicit StatementList(std::vector<std::unique_ptr<Statement>> statements)
        : statements_(std::move(statements)) {}

    ASTNodeType getType() const override { return ASTNodeType::StatementList; }
    const std::vector<std::unique_ptr<Statement>>& getStatements() const { return statements_; }

private:
    std::vector<std::unique_ptr<Statement>> statements_;
};

class TokenStatement final : public Statement {
public:
    explicit TokenStatement(Token token) : token_(std::move(token)) {}

    ASTNodeType getType() const override {
        return ASTNodeType::TokenStatement;
    }

    const Token& getToken() const {
        return token_;
    }

private:
    Token token_;
};

class ExpressionStatement final : public Statement {
public:
    explicit ExpressionStatement(std::unique_ptr<Expression> expr)
        : expr_(std::move(expr)) {}
    
    ASTNodeType getType() const override {
        return ASTNodeType::ExpressionStatement;
    }
    
    Expression* getExpression() const { return expr_.get(); }
    
private:
    std::unique_ptr<Expression> expr_;
};

class VariableDeclaration final : public Statement {
public:
    VariableDeclaration(const std::string& name, const std::string& typeName,
                       std::unique_ptr<Expression> initializer, Token declToken,
                       bool isConst = false, bool isMutable = false,
                       bool isConstExpr = false, bool isConstEval = false, bool isConstInit = false)
        : name_(name), typeName_(typeName), initializer_(std::move(initializer)),
          declToken_(std::move(declToken)), isConst_(isConst), isMutable_(isMutable),
          isConstExpr_(isConstExpr), isConstEval_(isConstEval), isConstInit_(isConstInit) {}
    
    ASTNodeType getType() const override {
        return ASTNodeType::VariableDeclaration;
    }
    
    const std::string& getName() const { return name_; }
    const std::string& getTypeName() const { return typeName_; }
    Expression* getInitializer() const { return initializer_.get(); }
    const Token& getDeclToken() const { return declToken_; }
    bool isConst() const { return isConst_; }
    bool isMutable() const { return isMutable_; }
    bool isConstExpr() const { return isConstExpr_; }
    bool isConstEval() const { return isConstEval_; }
    bool isConstInit() const { return isConstInit_; }
    const std::string& getArraySize() const { return arraySize_; }
    void setArraySize(const std::string& s) { arraySize_ = s; }
    void setConstExpr(bool value) { isConstExpr_ = value; }
    void setConstInit(bool value) { isConstInit_ = value; }
    void setMutable(bool value) { isMutable_ = value; }
    bool isThreadLocal() const { return isThreadLocal_; }
    void setThreadLocal(bool value) { isThreadLocal_ = value; }
    bool isStaticLocal() const { return isStaticLocal_; }
    void setStaticLocal(bool value) { isStaticLocal_ = value; }

private:
    std::string name_;
    std::string typeName_;
    std::unique_ptr<Expression> initializer_;
    Token declToken_;
    bool isConst_;
    bool isMutable_;
    bool isConstExpr_;
    bool isConstEval_;
    bool isConstInit_;
    std::string arraySize_;
    bool isThreadLocal_ = false;
    bool isStaticLocal_ = false;
};

class ReturnStatement final : public Statement {
public:
    explicit ReturnStatement(std::unique_ptr<Expression> value = nullptr, Token returnToken = Token())
        : value_(std::move(value)), returnToken_(std::move(returnToken)) {}
    
    ASTNodeType getType() const override {
        return ASTNodeType::ReturnStatement;
    }
    
    Expression* getValue() const { return value_.get(); }
    const Token& getReturnToken() const { return returnToken_; }
    
private:
    std::unique_ptr<Expression> value_;
    Token returnToken_;
};

class BreakStatement final : public Statement {
public:
    explicit BreakStatement(Token breakToken = Token())
        : breakToken_(std::move(breakToken)) {}
    
    ASTNodeType getType() const override {
        return ASTNodeType::BreakStatement;
    }
    
    const Token& getBreakToken() const { return breakToken_; }
    
private:
    Token breakToken_;
};

class ContinueStatement final : public Statement {
public:
    explicit ContinueStatement(Token continueToken = Token())
        : continueToken_(std::move(continueToken)) {}
    
    ASTNodeType getType() const override {
        return ASTNodeType::ContinueStatement;
    }
    
    const Token& getContinueToken() const { return continueToken_; }
    
private:
    Token continueToken_;
};

class MatchStatement final : public Statement {
public:
    struct MatchCase {
        std::unique_ptr<Expression> pattern; // if null, it's the 'boshqa' (default) case
        std::unique_ptr<Statement> body;
        Token caseToken;
    };
    
    MatchStatement(std::unique_ptr<Expression> condition, std::vector<std::unique_ptr<MatchCase>> cases,
                   Token matchToken = Token())
        : condition_(std::move(condition)), cases_(std::move(cases)), matchToken_(std::move(matchToken)) {}
    
    ASTNodeType getType() const override {
        return ASTNodeType::MatchStatement;
    }
    
    Expression* getCondition() const { return condition_.get(); }
    const std::vector<std::unique_ptr<MatchCase>>& getCases() const { return cases_; }
    const Token& getMatchToken() const { return matchToken_; }
    
private:
    std::unique_ptr<Expression> condition_;
    std::vector<std::unique_ptr<MatchCase>> cases_;
    Token matchToken_;
};

class TryStatement final : public Statement {
public:
    struct CatchClause {
        Token catchToken;
        std::string exceptionDecl; // "std::exception& e" or similar
        std::unique_ptr<Block> block;
    };
    
    TryStatement(std::unique_ptr<Block> tryBlock, std::vector<std::unique_ptr<CatchClause>> catchClauses, Token tryToken = Token())
        : tryBlock_(std::move(tryBlock)), catchClauses_(std::move(catchClauses)), tryToken_(std::move(tryToken)) {}
    
    ASTNodeType getType() const override {
        return ASTNodeType::TryStatement;
    }
    
    Block* getTryBlock() const { return tryBlock_.get(); }
    const std::vector<std::unique_ptr<CatchClause>>& getCatchClauses() const { return catchClauses_; }
    const Token& getTryToken() const { return tryToken_; }
    
private:
    std::unique_ptr<Block> tryBlock_;
    std::vector<std::unique_ptr<CatchClause>> catchClauses_;
    Token tryToken_;
};

class IfStatement final : public Statement {
public:
    IfStatement(std::unique_ptr<Expression> condition, std::unique_ptr<Statement> thenBranch,
               std::unique_ptr<Statement> elseBranch = nullptr, Token ifToken = Token())
        : condition_(std::move(condition)), thenBranch_(std::move(thenBranch)),
          elseBranch_(std::move(elseBranch)), ifToken_(std::move(ifToken)) {}
    
    ASTNodeType getType() const override {
        return ASTNodeType::IfStatement;
    }
    
    Expression* getCondition() const { return condition_.get(); }
    Statement* getThenBranch() const { return thenBranch_.get(); }
    Statement* getElseBranch() const { return elseBranch_.get(); }
    const Token& getIfToken() const { return ifToken_; }
    
    bool isConstExpr() const { return isConstExpr_; }
    void setConstExpr(bool val) { isConstExpr_ = val; }
    // C++23 `if consteval { ... }` — no condition, true in constant context
    bool isConsteval() const { return isConsteval_; }
    void setConsteval(bool val) { isConsteval_ = val; }

    // C++20 branch-prediction hints on the then-branch
    bool isThenLikely() const { return isThenLikely_; }
    bool isThenUnlikely() const { return isThenUnlikely_; }
    void setThenLikely(bool v) { isThenLikely_ = v; }
    void setThenUnlikely(bool v) { isThenUnlikely_ = v; }
    // ...and on the else-branch
    bool isElseLikely() const { return isElseLikely_; }
    bool isElseUnlikely() const { return isElseUnlikely_; }
    void setElseLikely(bool v) { isElseLikely_ = v; }
    void setElseUnlikely(bool v) { isElseUnlikely_ = v; }

private:
    std::unique_ptr<Expression> condition_;
    std::unique_ptr<Statement> thenBranch_;
    std::unique_ptr<Statement> elseBranch_;
    Token ifToken_;
    bool isConstExpr_ = false;
    bool isConsteval_ = false;
    bool isThenLikely_ = false;
    bool isThenUnlikely_ = false;
    bool isElseLikely_ = false;
    bool isElseUnlikely_ = false;
};

class WhileStatement final : public Statement {
public:
    WhileStatement(std::unique_ptr<Expression> condition, std::unique_ptr<Statement> body,
                  Token whileToken = Token())
        : condition_(std::move(condition)), body_(std::move(body)), whileToken_(std::move(whileToken)) {}
    
    ASTNodeType getType() const override {
        return ASTNodeType::WhileStatement;
    }
    
    Expression* getCondition() const { return condition_.get(); }
    Statement* getBody() const { return body_.get(); }
    const Token& getWhileToken() const { return whileToken_; }
    
private:
    std::unique_ptr<Expression> condition_;
    std::unique_ptr<Statement> body_;
    Token whileToken_;
};

class ForStatement final : public Statement {
public:
    ForStatement(std::unique_ptr<Statement> init, std::unique_ptr<Expression> condition,
                std::unique_ptr<Expression> increment, std::unique_ptr<Statement> body,
                Token forToken = Token(), bool isRangeBased = false)
        : init_(std::move(init)), condition_(std::move(condition)),
          increment_(std::move(increment)), body_(std::move(body)),
          forToken_(std::move(forToken)), isRangeBased_(isRangeBased) {}

    ASTNodeType getType() const override {
        return ASTNodeType::ForStatement;
    }

    Statement* getInit() const { return init_.get(); }
    Expression* getCondition() const { return condition_.get(); }
    Expression* getIncrement() const { return increment_.get(); }
    Statement* getBody() const { return body_.get(); }
    const Token& getForToken() const { return forToken_; }
    bool isRangeBased() const { return isRangeBased_; }

private:
    std::unique_ptr<Statement> init_;
    std::unique_ptr<Expression> condition_;
    std::unique_ptr<Expression> increment_;
    std::unique_ptr<Statement> body_;
    Token forToken_;
    bool isRangeBased_ = false;
};

// ===== SEMANTIC DECLARATION NODES =====

class Declaration : public ASTNode {
public:
    virtual ~Declaration() = default;
};

class FunctionDeclaration final : public Declaration {
public:
    struct Parameter {
        std::string name;
        std::string type;
        std::string defaultValue; // Standart qiymat, masalan: "0", "\"\"", "rost"
        bool isConst = false;
        bool isExplicitObject = false; // C++23 explicit object parameter (this)
        Token token;
    };
    
    FunctionDeclaration(const std::string& name, const std::string& returnType,
                       std::vector<Parameter> params, std::unique_ptr<Block> body,
                       Token funcToken = Token(), bool isAsync = false, bool isTest = false, bool isBench = false,
                       bool isConstExpr = false, bool isConstEval = false)
        : name_(name), returnType_(returnType), params_(std::move(params)),
          body_(std::move(body)), funcToken_(std::move(funcToken)), isAsync_(isAsync), isTest_(isTest), isBench_(isBench),
          isConstExpr_(isConstExpr), isConstEval_(isConstEval), isNoDiscard_(false), isDeprecated_(false) {}

    ASTNodeType getType() const override {
        return ASTNodeType::FunctionDeclaration;
    }

    const std::string& getName() const { return name_; }
    const std::string& getReturnType() const { return returnType_; }
    const std::vector<Parameter>& getParameters() const { return params_; }
    Block* getBody() const { return body_.get(); }
    const Token& getFunctionToken() const { return funcToken_; }
    bool isAsync() const { return isAsync_; }
    void setAsync(bool async) { isAsync_ = async; }
    bool isTest() const { return isTest_; }
    void setTest(bool test) { isTest_ = test; }
    bool isBench() const { return isBench_; }
    void setBench(bool bench) { isBench_ = bench; }
    bool isConstExpr() const { return isConstExpr_; }
    void setConstExpr(bool value) { isConstExpr_ = value; }
    bool isConstEval() const { return isConstEval_; }
    void setConstEval(bool value) { isConstEval_ = value; }
    bool isNoDiscard() const { return isNoDiscard_; }
    void setNoDiscard(bool value) { isNoDiscard_ = value; }
    bool isDeprecated() const { return isDeprecated_; }
    void setDeprecated(bool value) { isDeprecated_ = value; }
    bool isNoExcept() const { return isNoExcept_; }
    void setNoExcept(bool value) { isNoExcept_ = value; }
    bool hasTrailingReturn() const { return hasTrailingReturn_; }
    void setTrailingReturn(bool value) { hasTrailingReturn_ = value; }
    const std::string& getRequiresClause() const { return requiresClause_; }
    void setRequiresClause(const std::string& clause) { requiresClause_ = clause; }

private:
    std::string name_;
    std::string returnType_;
    std::vector<Parameter> params_;
    std::unique_ptr<Block> body_;
    Token funcToken_;
    bool isAsync_;
    bool isTest_;
    bool isBench_;
    bool isConstExpr_;
    bool isConstEval_;
    bool isNoDiscard_;
    bool isDeprecated_;
    bool isNoExcept_ = false;
    bool hasTrailingReturn_ = false;
    std::string requiresClause_;
};

class ClassDeclaration final : public Declaration {
public:
    struct Member {
        std::string name;
        std::string type;
        std::string accessSpecifier; // ochiq, yopiq, protected
        std::string arraySize; // For C-style arrays: "10" for int data[10], empty if not array
        std::string bitWidth;  // C++ bitfield: "4" for `int x : 4`. Empty if not bitfield.
        bool isVolatile = false;
        bool isThreadLocal = false;
        bool hasNoUniqueAddress = false;  // C++20 [[no_unique_address]]
        Token token;
    };
    
    struct Method {
        std::string name;
        std::string returnType;
        std::vector<FunctionDeclaration::Parameter> params;
        std::unique_ptr<Block> body;
        std::string accessSpecifier;
        bool isVirtual = false;       // override in derived class
        bool isPureVirtual = false;   // mavhum: virtual ... = 0
        bool isConstMethod = false;   // o'zgarmas: void foo() const
        bool isStatic = false;
        bool isNoExcept = false;
        bool isDefaulted = false;     // = default  (e.g. for operator<=>)
        bool isDeleted = false;       // = delete
        std::string refQualifier;     // "&" or "&&" — ref-qualified method
        Token token;
        std::string initializerList; // ": field1(val1), field2(val2)" for constructors
    };
    
    ClassDeclaration(const std::string& name, const std::string& baseClass,
                     std::vector<std::string> interfaces,
                    std::vector<Member> members, std::vector<std::unique_ptr<Method>> methods,
                    Token classToken = Token())
        : name_(name), baseClass_(baseClass), interfaces_(std::move(interfaces)), members_(std::move(members)),
          methods_(std::move(methods)), classToken_(std::move(classToken)), kind_("class") {}

    ASTNodeType getType() const override {
        return ASTNodeType::ClassDeclaration;
    }

    const std::string& getName() const { return name_; }
    const std::string& getBaseClass() const { return baseClass_; }
    const std::vector<std::string>& getInterfaces() const { return interfaces_; }
    const std::vector<Member>& getMembers() const { return members_; }
    const std::vector<std::unique_ptr<Method>>& getMethods() const { return methods_; }
    const Token& getClassToken() const { return classToken_; }
    const std::string& getKind() const { return kind_; }
    void setKind(const std::string& kind) { kind_ = kind; }
    const std::vector<std::string>& getFriendDecls() const { return friendDecls_; }
    void addFriendDecl(const std::string& decl) { friendDecls_.push_back(decl); }
    
    const std::string& getAlignment() const { return alignment_; }
    void setAlignment(const std::string& alignment) { alignment_ = alignment; }

private:
    std::string name_;
    std::string baseClass_;
    std::vector<std::string> interfaces_;
    std::vector<Member> members_;
    std::vector<std::unique_ptr<Method>> methods_;
    Token classToken_;
    std::string kind_;  // "class" (default), "struct", or "union"
    std::vector<std::string> friendDecls_;  // raw "friend ..." declarations
    std::string alignment_;  // alignas(N) value
};

class NamespaceDeclaration final : public Declaration {
public:
    NamespaceDeclaration(const std::string& name, std::vector<std::unique_ptr<ASTNode>> children, Token targetToken = Token())
        : name_(name), children_(std::move(children)), targetToken_(std::move(targetToken)) {}

    ASTNodeType getType() const override {
        return ASTNodeType::NamespaceDeclaration;
    }

    const std::string& getName() const { return name_; }
    const std::vector<std::unique_ptr<ASTNode>>& getChildren() const { return children_; }
    const Token& getTargetToken() const { return targetToken_; }

private:
    std::string name_;
    std::vector<std::unique_ptr<ASTNode>> children_;
    Token targetToken_;
};

class InterfaceDeclaration final : public Declaration {
public:
    struct MethodSignature {
        std::string name;
        std::string returnType;
        std::vector<FunctionDeclaration::Parameter> params;
        Token token;
    };
    
    InterfaceDeclaration(const std::string& name, std::vector<std::unique_ptr<MethodSignature>> methods, Token interfaceToken = Token())
        : name_(name), methods_(std::move(methods)), interfaceToken_(std::move(interfaceToken)) {}
    
    ASTNodeType getType() const override {
        return ASTNodeType::InterfaceDeclaration;
    }
    
    const std::string& getName() const { return name_; }
    const std::vector<std::unique_ptr<MethodSignature>>& getMethods() const { return methods_; }
    const Token& getInterfaceToken() const { return interfaceToken_; }
    
private:
    std::string name_;
    std::vector<std::unique_ptr<MethodSignature>> methods_;
    Token interfaceToken_;
};

class IncludeStatement final : public Statement {
public:
    IncludeStatement(const std::string& moduleName, Token includeToken = Token())
        : moduleName_(moduleName), includeToken_(std::move(includeToken)) {}

    ASTNodeType getType() const override {
        return ASTNodeType::IncludeStatement;
    }

    const std::string& getModuleName() const { return moduleName_; }
    const Token& getIncludeToken() const { return includeToken_; }

private:
    std::string moduleName_;
    Token includeToken_;
};

class ExportModuleStatement final : public Statement {
public:
    ExportModuleStatement(const std::string& moduleName, Token token = Token())
        : moduleName_(moduleName), token_(std::move(token)) {}

    ASTNodeType getType() const override {
        return ASTNodeType::ExportModuleStatement;
    }

    const std::string& getModuleName() const { return moduleName_; }
    const Token& getToken() const { return token_; }

private:
    std::string moduleName_;
    Token token_;
};

class EnumDeclaration final : public Declaration {
public:
    struct EnumValue {
        std::string name;
        std::string explicitValue; // ixtiyoriy: "= 5" kabi
    };

    EnumDeclaration(std::string name, std::vector<EnumValue> values, Token token = Token())
        : name_(std::move(name)), values_(std::move(values)), token_(std::move(token)) {}

    ASTNodeType getType() const override { return ASTNodeType::EnumDeclaration; }
    const std::string& getName() const { return name_; }
    const std::vector<EnumValue>& getValues() const { return values_; }
    const Token& getToken() const { return token_; }

private:
    std::string name_;
    std::vector<EnumValue> values_;
    Token token_;
};

class LinkStatement final : public Statement {
public:
    LinkStatement(const std::string& libName, Token linkToken = Token())
        : libName_(libName), linkToken_(std::move(linkToken)) {}

    ASTNodeType getType() const override {
        return ASTNodeType::LinkStatement;
    }

    const std::string& getLibName() const { return libName_; }
    const Token& getLinkToken() const { return linkToken_; }

private:
    std::string libName_;
    Token linkToken_;
};

// ===== PHASE 26: MODERN LANGUAGE FEATURES =====

// Pipeline: a |> f => f(a)  (Elixir / F# style)
class PipelineExpression final : public Expression {
public:
    PipelineExpression(std::unique_ptr<Expression> left,
                       std::unique_ptr<Expression> right,
                       Token pipeToken)
        : left_(std::move(left)), right_(std::move(right)), pipeToken_(std::move(pipeToken)) {}

    ASTNodeType getType() const override { return ASTNodeType::PipelineExpression; }

    Expression* getLeft()  const { return left_.get(); }
    Expression* getRight() const { return right_.get(); }
    const Token& getPipeToken() const { return pipeToken_; }

private:
    std::unique_ptr<Expression> left_;
    std::unique_ptr<Expression> right_;
    Token pipeToken_;
};

// Type alias: tur Raqam = butun;  (TypeScript / Rust type alias)
class TypeAlias final : public Declaration {
public:
    TypeAlias(const std::string& alias, const std::string& target, Token token = Token())
        : alias_(alias), target_(target), token_(std::move(token)) {}

    ASTNodeType getType() const override { return ASTNodeType::TypeAlias; }

    const std::string& getAlias()  const { return alias_; }
    const std::string& getTarget() const { return target_; }
    const Token& getToken() const { return token_; }

private:
    std::string alias_;
    std::string target_;
    Token token_;
};

// Lambda / closure: |x, y| => x + y  (Rust-style)
class LambdaExpression final : public Expression {
public:
    struct Capture {
        std::string name;
        bool byRef = false; // & captures — va& / va=
        std::string initExpr;  // C++14 init-capture: [name = init_expr]
    };
    struct Param {
        std::string name;
        std::string type; // empty = auto
    };

    LambdaExpression(std::vector<Capture> captures,
                     std::vector<Param> params,
                     std::string returnType,
                     std::unique_ptr<Statement> body,
                     Token lambdaToken)
        : captures_(std::move(captures)), params_(std::move(params)),
          returnType_(std::move(returnType)),
          body_(std::move(body)), lambdaToken_(std::move(lambdaToken)) {}

    ASTNodeType getType() const override { return ASTNodeType::LambdaExpression; }

    const std::vector<Capture>& getCaptures() const { return captures_; }
    const std::vector<Param>&   getParams()   const { return params_; }
    const std::string& getReturnType() const { return returnType_; }
    Statement* getBody() const { return body_.get(); }
    const Token& getLambdaToken() const { return lambdaToken_; }

private:
    std::vector<Capture> captures_;
    std::vector<Param>   params_;
    std::string          returnType_;
    std::unique_ptr<Statement> body_;
    Token lambdaToken_;
};

// Ternary: shart ? agar_ha : aks_holda
class TernaryExpression final : public Expression {
public:
    TernaryExpression(std::unique_ptr<Expression> condition,
                      std::unique_ptr<Expression> thenExpr,
                      std::unique_ptr<Expression> elseExpr,
                      Token ternaryToken)
        : condition_(std::move(condition)), thenExpr_(std::move(thenExpr)),
          elseExpr_(std::move(elseExpr)), ternaryToken_(std::move(ternaryToken)) {}

    ASTNodeType getType() const override { return ASTNodeType::TernaryExpression; }

    Expression* getCondition() const { return condition_.get(); }
    Expression* getThenExpr()  const { return thenExpr_.get(); }
    Expression* getElseExpr()  const { return elseExpr_.get(); }
    const Token& getTernaryToken() const { return ternaryToken_; }

private:
    std::unique_ptr<Expression> condition_;
    std::unique_ptr<Expression> thenExpr_;
    std::unique_ptr<Expression> elseExpr_;
    Token ternaryToken_;
};

} // namespace uzpp
