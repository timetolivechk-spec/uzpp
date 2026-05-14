#pragma once

#include "ast.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cctype>

namespace uzpp {

struct SemanticError {
    std::string message;
    int line;
    int column;
};

class TypeChecker {
private:
    struct ClassInfo {
        std::unordered_map<std::string, std::string> fields;
        std::unordered_map<std::string, std::string> methodReturns;
        std::unordered_map<std::string, std::vector<std::string>> methodParams;
        std::string baseClass;
        std::vector<std::string> interfaces;
    };

    struct VarInfo {
        std::string type;
        Token declToken;
        bool used;
    };

    std::vector<std::unordered_map<std::string, VarInfo>> scopes_;
    std::vector<SemanticError> errors_;
    std::vector<SemanticError> warnings_;
    std::unordered_map<std::string, std::string> functionReturns_;
    std::unordered_map<std::string, std::vector<std::string>> functionParams_;
    // Standart qiymatli parametrlarni inobatga olib, minimal argument soni
    std::unordered_map<std::string, std::size_t> functionMinArgs_;
    std::unordered_map<std::string, ClassInfo> classes_;
    std::unordered_map<std::string, std::string> typeAliases_; // tur X = Y
    std::unordered_set<std::string> templateFunctions_;
    std::string currentReturnType_ = "";
    bool reachable_ = true;
    bool reportedUnreachable_ = false;
    bool currentFunctionIsAsync_ = false;
    int loopDepth_ = 0;

    void reportError(const std::string& msg, const Token& token) {
        errors_.push_back({msg, token.line, token.column});
    }

    void reportWarning(const std::string& msg, const Token& token) {
        warnings_.push_back({msg, token.line, token.column});
    }

    Token getTokenForNode(const ASTNode* node) {
        if (!node) return Token{TokenType::Identifier, "", 0, 0};
        if (auto var = dynamic_cast<const VariableDeclaration*>(node)) return var->getDeclToken();
        if (auto func = dynamic_cast<const FunctionDeclaration*>(node)) return func->getFunctionToken();
        if (auto id = dynamic_cast<const IdentifierExpression*>(node)) return id->getSourceToken();
        if (auto bin = dynamic_cast<const BinaryExpression*>(node)) return bin->getOperatorToken();
        if (auto call = dynamic_cast<const FunctionCall*>(node)) return call->getCallToken();
        if (auto asgn = dynamic_cast<const AssignmentExpression*>(node)) return asgn->getOperatorToken();
        if (auto lit = dynamic_cast<const LiteralExpression*>(node)) return lit->getSourceToken();
        if (auto ret = dynamic_cast<const ReturnStatement*>(node)) return ret->getReturnToken();
        if (auto aw = dynamic_cast<const AwaitExpression*>(node)) return aw->getAwaitToken();
        if (auto tern = dynamic_cast<const TernaryExpression*>(node)) return tern->getTernaryToken();
        if (auto lam = dynamic_cast<const LambdaExpression*>(node)) return lam->getLambdaToken();
        if (auto pipe = dynamic_cast<const PipelineExpression*>(node)) return pipe->getPipeToken();
        if (auto sub = dynamic_cast<const SubscriptAccess*>(node)) return sub->getBracketToken();
        return Token{TokenType::Identifier, "", 0, 0};
    }

    std::string inferType(const Expression* expr) {
        if (!expr) return "noma'lum";
        switch (expr->getType()) {
            case ASTNodeType::LiteralExpression: {
                auto lit = static_cast<const LiteralExpression*>(expr);
                switch(lit->getLiteralType()) {
                    case LiteralExpression::LiteralType::Integer: return "butun";
                    case LiteralExpression::LiteralType::Float: return "haqiqiy";
                    case LiteralExpression::LiteralType::String: 
                    case LiteralExpression::LiteralType::FormatString: return "matn";
                    case LiteralExpression::LiteralType::Character: return "belgi";
                    case LiteralExpression::LiteralType::Boolean: return "mantiqiy";
                }
                break;
            }
            case ASTNodeType::IdentifierExpression: {
                auto id = static_cast<const IdentifierExpression*>(expr);
                std::string name = id->getName();
                for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
                    if (it->contains(name)) {
                        (*it)[name].used = true;
                        return (*it)[name].type;
                    }
                }
                break;
            }
            case ASTNodeType::BinaryExpression: {
                // Oddiy tur xulosasi (soddalashtirilgan)
                auto bin = static_cast<const BinaryExpression*>(expr);
                std::string op = bin->getOperator();
                if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=" || op == "&&" || op == "||" || op == "va" || op == "yoki") {
                    return "mantiqiy";
                }
                std::string leftType = inferType(bin->getLeft());
                std::string rightType = inferType(bin->getRight());
                if (leftType == "haqiqiy" || rightType == "haqiqiy") return "haqiqiy";
                if (leftType == "matn" || rightType == "matn") return "matn";
                return leftType;
            }
            case ASTNodeType::MemberAccess: {
                auto mac = static_cast<const MemberAccess*>(expr);
                std::string objType = inferType(mac->getObject());
                if (classes_.contains(objType)) {
                    std::string ret = classMethodReturn(objType, mac->getMemberName());
                    if (ret != "noma'lum") return ret;
                }
                break;
            }
            case ASTNodeType::SubscriptAccess: {
                auto sub = static_cast<const SubscriptAccess*>(expr);
                std::string arrType = inferType(sub->getArray());
                if (arrType.starts_with("vektor<") && arrType.back() == '>') {
                    return arrType.substr(7, arrType.length() - 8);
                }
                break;
            }
            case ASTNodeType::FunctionCall: {
                auto call = static_cast<const FunctionCall*>(expr);
                if (call->getCallee()->getType() == ASTNodeType::IdentifierExpression) {
                    std::string name = static_cast<const IdentifierExpression*>(call->getCallee())->getName();
                    if (name == "__uzpp_array") {
                        if (!call->getArguments().empty()) {
                            return "vektor<" + inferType(call->getArguments()[0].get()) + ">";
                        }
                        return "vektor<noma'lum>";
                    }
                    if (name == "__uzpp_dict") {
                        return "lug'at<matn, noma'lum>";
                    }
                    if (functionReturns_.contains(name)) return functionReturns_[name];
                    if (classes_.contains(name)) return name; // Konstruktor chaqiruvi
                } else if (call->getCallee()->getType() == ASTNodeType::MemberAccess) {
                    auto mac = static_cast<const MemberAccess*>(call->getCallee());
                    std::string objType = inferType(mac->getObject());
                    if (classes_.contains(objType)) {
                        std::string ret = classMethodReturn(objType, mac->getMemberName());
                        if (ret != "noma'lum") return ret;
                    }
                }
                break;
            }
            default: break;
        }
        return "noma'lum";
    }

    void enterScope() {
        scopes_.push_back({});
    }

    void exitScope() {
        if (!scopes_.empty()) {
            for (const auto& [name, info] : scopes_.back()) {
                if (!info.used && !name.empty() && name[0] != '_') {
                    reportWarning("O'zgaruvchi '" + name + "' e'lon qilingan, lekin ishlatilmagan.", info.declToken);
                }
            }
            scopes_.pop_back();
        }
    }

    // Resolve type aliases recursively (up to depth 8)
    std::string resolveType(const std::string& t, int depth = 0) const {
        if (depth >= 8) return t;
        auto it = typeAliases_.find(t);
        if (it != typeAliases_.end()) return resolveType(it->second, depth + 1);
        return t;
    }

    // Returns true if `derived` is a subtype of `base` (same class or inherits from it)
    bool classIsSubtype(const std::string& derived, const std::string& base) const {
        if (derived == base) return true;
        std::string cur = derived;
        int depth = 0;
        while (!cur.empty() && classes_.contains(cur) && depth < 16) {
            cur = classes_.at(cur).baseClass;
            if (cur == base) return true;
            ++depth;
        }
        return false;
    }

    // Walk up inheritance chain looking for a field or method named `member`
    bool classHasMember(const std::string& className, const std::string& member) const {
        std::string cur = className;
        int depth = 0;
        while (!cur.empty() && classes_.contains(cur) && depth < 16) {
            const auto& info = classes_.at(cur);
            if (info.fields.contains(member) || info.methodReturns.contains(member)) return true;
            // Check interfaces too
            for (const auto& iface : info.interfaces) {
                if (classes_.contains(iface)) {
                    const auto& ifInfo = classes_.at(iface);
                    if (ifInfo.fields.contains(member) || ifInfo.methodReturns.contains(member)) return true;
                }
            }
            cur = info.baseClass;
            ++depth;
        }
        return false;
    }

    // Return the method's return type, walking up the inheritance chain
    std::string classMethodReturn(const std::string& className, const std::string& member) const {
        std::string cur = className;
        int depth = 0;
        while (!cur.empty() && classes_.contains(cur) && depth < 16) {
            const auto& info = classes_.at(cur);
            if (info.methodReturns.contains(member)) return info.methodReturns.at(member);
            if (info.fields.contains(member)) return info.fields.at(member);
            for (const auto& iface : info.interfaces) {
                if (classes_.contains(iface)) {
                    const auto& ifInfo = classes_.at(iface);
                    if (ifInfo.methodReturns.contains(member)) return ifInfo.methodReturns.at(member);
                }
            }
            cur = info.baseClass;
            ++depth;
        }
        return "noma'lum";
    }

    void markUsed(const std::string& name) {
        for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
            if (it->contains(name)) { (*it)[name].used = true; return; }
        }
    }

    void declareVar(const std::string& name, const std::string& type, const Token& token) {
        if (scopes_.empty()) enterScope();
        // Allow function overloading: skip duplicate check for "funktsiya" type
        if (scopes_.back().contains(name) && type != "funktsiya") {
            reportError("O'zgaruvchi '" + name + "' ushbu qamrovda allaqachon e'lon qilingan.", token);
        }
        scopes_.back()[name] = VarInfo{type, token, false};
    }

    bool isDeclared(const std::string& name) {
        if (name.starts_with("__uzpp_catch")) return true;

        static const std::unordered_set<std::string> builtins = {
            // I/O va tizim
            "yozish", "qator_oxiri", "olish", "kiritish", "chiqarish",
            "__uzpp_try", "__uzpp_array", "__uzpp_dict", "__uzpp_throw", "__uzpp_brace",
            // Asosiy turlar
            "butun", "kasr", "ikkilangan", "haqiqiy", "belgi",
            "mantiq", "mantiqiy", "matn", "bosh", "belgilar",
            // Konteynerlar
            "vektor", "to'plam", "lug'at", "hesh_jadval", "hesh_xarita",
            "o'ziga_xos", "tartiblangan_xarita", "o'n",
            // Ko'rsatkichlar
            "yagona_korsatkich", "aqlli_korsatkich", "yagona", "umumiy",
            // Oqimlar va sinxronizatsiya
            "oqim", "qulf", "qulflangan_guard", "kelajak", "oqim_boshla",
            "OqimPool", "VazifaJavob",
            // Fayl I/O
            "fayl_oqish", "fayl_yozish",
            // Modifikatorlar
            "ozgaruvchan", "o'zgaruvchan", "ozgarmas", "o'zgarmas", "statik",
            // Range/algoritm
            "filtr", "filter", "xaritalash", "yigish", "to'plash",
            "ko'chirish", "nullptr", "NULL",
            // Mantiqiy qiymatlar
            "rost", "yolg'on", "true", "false",
            // C++ NULL va atipik konstantalar
            "INT_MAX", "INT_MIN", "UINT_MAX", "SIZE_MAX", "DBL_MAX", "FLT_MAX",
            "EXIT_SUCCESS", "EXIT_FAILURE", "EOF",
            // Yangi turlar: Natija, Tanlov (Result, Option)
            "Natija", "Tanlov", "natija", "tanlov",
            "muvaffaqiyat", "xato", "bor", "yoq",
            // JSON
            "Json", "Qiymat", "tahlil", "chiqarish",
            // Matematika
            "Matematika", "Vektor2", "Vektor3", "Matritsa2x2",
            // Matn
            "Matn", "RegEx",
            // Std namespace prefiks
            "std", "uzpp",
            // C++ stdlib to'g'ridan to'g'ri
            "auto", "size_t", "int", "float", "double", "bool", "char",
            "string", "vector", "map", "set", "optional", "variant",
            "shared_ptr", "unique_ptr", "make_shared", "make_unique",
            // to_string va boshqa umumiy funksiyalar
            "to_string", "stoi", "stof", "stod",
            "move", "forward", "swap",
            // Nomlar fazolari
            "Grafika", "Hodisalar", "Vidjetlar",
        };

        if (builtins.contains(name)) return true;

        for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
            if (it->contains(name)) {
                (*it)[name].used = true;
                return true;
            }
        }
        return false;
    }

public:
    TypeChecker() {
        enterScope(); // Global qamrov
    }

    const std::vector<SemanticError>& getErrors() const { return errors_; }
    const std::vector<SemanticError>& getWarnings() const { return warnings_; }

    const std::unordered_map<std::string, std::vector<std::string>>& getFunctionParams() const { return functionParams_; }
    const std::unordered_map<std::string, std::string>& getFunctionReturns() const { return functionReturns_; }

    bool check(const Program* program) {
        if (!program) return false;
        for (const auto& node : program->getChildren()) {
            checkNode(node.get());
        }
        return errors_.empty();
    }

private:
    void checkNode(const ASTNode* node) {
        if (!node) return;

        if (!reachable_) {
            if (!reportedUnreachable_) {
                Token t = getTokenForNode(node);
                if (t.line > 0) {
                    reportWarning("Ushbu kodga hech qachon etib kelinmaydi (unreachable code).", t);
                    reportedUnreachable_ = true;
                }
            }
        }

        switch (node->getType()) {
            case ASTNodeType::ExportModuleStatement:
                break; // Semantik tekshiruv shart emas
            case ASTNodeType::VariableDeclaration: {
                auto var = static_cast<const VariableDeclaration*>(node);
                std::string declaredType = var->getTypeName();
                
                if (var->getInitializer()) {
                    checkExpr(var->getInitializer());
                    std::string inferredType = inferType(var->getInitializer());
                    
                    if (declaredType == "ozgaruvchan" || declaredType == "o'zgaruvchan" || declaredType == "ozgarmas") {
                        declaredType = inferredType; // Type inference
                    } else if (inferredType != "noma'lum" && declaredType != "noma'lum" && inferredType != declaredType) {
                        if ((declaredType == "butun" || declaredType == "matn" || declaredType == "mantiqiy" || declaredType == "haqiqiy") &&
                            (inferredType == "butun" || inferredType == "matn" || inferredType == "mantiqiy" || inferredType == "haqiqiy")) {
                            if (!(declaredType == "haqiqiy" && inferredType == "butun")) {
                                reportWarning("Tur nomutanosibligi: '" + declaredType + "' kutilgan, lekin '" + inferredType + "' berildi.", var->getDeclToken());
                            }
                        }
                    }
                }
                if (var->isConst() && !var->getInitializer()) {
                    reportError("O'zgarmas (const) o'zgaruvchi '" + var->getName() + "' e'lon qilinganda qiymatga ega bo'lishi shart.", var->getDeclToken());
                }
                // Structured binding: name like "[a, b]" -> declare each binding individually
                const std::string& vname = var->getName();
                if (!vname.empty() && vname.front() == '[' && vname.back() == ']') {
                    std::string inner = vname.substr(1, vname.size() - 2);
                    std::string cur;
                    for (char c : inner) {
                        if (c == ',' || c == ' ' || c == '\t') {
                            if (!cur.empty()) { declareVar(cur, "noma'lum", var->getDeclToken()); cur.clear(); }
                        } else {
                            cur += c;
                        }
                    }
                    if (!cur.empty()) declareVar(cur, "noma'lum", var->getDeclToken());
                } else {
                    declareVar(vname, declaredType, var->getDeclToken());
                }
                break;
            }
            case ASTNodeType::FunctionDeclaration: {
                auto func = static_cast<const FunctionDeclaration*>(node);
                declareVar(func->getName(), "funktsiya", func->getFunctionToken());
                
                std::vector<std::string> pTypes;
                std::size_t minArgs = 0;
                for (const auto& p : func->getParameters()) {
                    // Skip explicit object parameters (C++23 deducing this)
                    if (p.isExplicitObject) continue;
                    pTypes.push_back(p.type);
                    if (p.defaultValue.empty()) minArgs++;
                }
                functionParams_[func->getName()] = pTypes;
                functionMinArgs_[func->getName()] = minArgs;
                functionReturns_[func->getName()] = func->getReturnType();
                
                std::string prevRet = currentReturnType_;
                currentReturnType_ = func->getReturnType();
                
                bool oldAsync = currentFunctionIsAsync_;
                currentFunctionIsAsync_ = func->isAsync();
                
                bool savedReachable = reachable_;
                bool savedReported = reportedUnreachable_;
                reachable_ = true;
                reportedUnreachable_ = false;

                enterScope();
                for (const auto& p : func->getParameters()) {
                    declareVar(p.name, p.type, p.token);
                }
                if (func->getBody()) {
                    for (const auto& s : func->getBody()->getStatements()) checkNode(s.get());
                }
                exitScope();

                reachable_ = savedReachable;
                reportedUnreachable_ = savedReported;
                currentReturnType_ = prevRet;
                currentFunctionIsAsync_ = oldAsync;
                break;
            }
            case ASTNodeType::ClassDeclaration: {
                auto cls = static_cast<const ClassDeclaration*>(node);
                ClassInfo info;
                info.baseClass = cls->getBaseClass();
                info.interfaces = cls->getInterfaces();
                for (const auto& member : cls->getMembers()) {
                    info.fields[member.name] = member.type;
                }
                for (const auto& method : cls->getMethods()) {
                    std::vector<std::string> pTypes;
                    for (const auto& p : method->params) {
                        // Skip explicit object parameters (C++23 deducing this)
                        if (p.isExplicitObject) continue;
                        pTypes.push_back(p.type);
                    }
                    info.methodParams[method->name] = pTypes;
                    info.methodReturns[method->name] = method->returnType.empty() ? cls->getName() : method->returnType;

                    if (method->name == cls->getName()) {
                        functionParams_[cls->getName()] = pTypes;
                        functionReturns_[cls->getName()] = cls->getName();
                    }
                }
                classes_[cls->getName()] = info;

                for (const auto& method : cls->getMethods()) {
                    std::string prevRet = currentReturnType_;
                    currentReturnType_ = method->returnType.empty() ? "bosh" : method->returnType;
                    
                    bool oldAsync = currentFunctionIsAsync_;
                    currentFunctionIsAsync_ = false; // Hozircha sinf metodlari parserda asinxron qilinmagan
                    
                    bool savedReachable = reachable_;
                    bool savedReported = reportedUnreachable_;
                    reachable_ = true;
                    reportedUnreachable_ = false;

                    enterScope();
                    // 'joriy' — implicit this pointer; pre-mark as used to suppress unused warning
                    declareVar("joriy", cls->getName() + "*", method->token);
                    scopes_.back()["joriy"].used = true;
                    // Expose own class fields+methods in scope (direct access without `joriy->`)
                    for (const auto& [fieldName, fieldType] : classes_[cls->getName()].fields) {
                        scopes_.back()[fieldName] = VarInfo{fieldType, method->token, true};
                    }
                    for (const auto& [mName, mRet] : classes_[cls->getName()].methodReturns) {
                        scopes_.back()[mName] = VarInfo{mRet, method->token, true};
                    }
                    // Walk full inheritance chain: expose all ancestor fields+methods
                    {
                        std::string base = cls->getBaseClass();
                        int depth = 0;
                        while (!base.empty() && classes_.contains(base) && depth < 16) {
                            for (const auto& [fieldName, fieldType] : classes_[base].fields) {
                                if (!scopes_.back().contains(fieldName))
                                    scopes_.back()[fieldName] = VarInfo{fieldType, method->token, true};
                            }
                            for (const auto& [mName, mRet] : classes_[base].methodReturns) {
                                if (!scopes_.back().contains(mName))
                                    scopes_.back()[mName] = VarInfo{mRet, method->token, true};
                            }
                            base = classes_[base].baseClass;
                            ++depth;
                        }
                    }
                    bool isConstructor = (method->name == cls->getName());
                    for (const auto& p : method->params) {
                        declareVar(p.name, p.type, p.token);
                        // Constructor params are used in the initializer list (not tracked by TypeChecker)
                        if (isConstructor) scopes_.back()[p.name].used = true;
                    }
                    if (method->body) {
                        for (const auto& s : method->body->getStatements()) checkNode(s.get());
                    }
                    exitScope();

                    reachable_ = savedReachable;
                    reportedUnreachable_ = savedReported;
                    currentReturnType_ = prevRet;
                    currentFunctionIsAsync_ = oldAsync;
                }
                break;
            }
            case ASTNodeType::StatementList: {
                for (const auto& s : static_cast<const StatementList*>(node)->getStatements())
                    checkNode(s.get());
                break;
            }
            case ASTNodeType::Block: {
                auto block = static_cast<const Block*>(node);
                enterScope();
                for (const auto& s : block->getStatements()) checkNode(s.get());
                exitScope();
                break;
            }
            case ASTNodeType::IfStatement: {
                auto ifs = static_cast<const IfStatement*>(node);
                checkExpr(ifs->getCondition());

                // Detect __uzpp_catch and declare catch variable in then-branch scope
                bool isCatchBlock = false;
                std::string catchVarName;
                if (auto idExpr = dynamic_cast<const IdentifierExpression*>(ifs->getCondition())) {
                    const std::string& cond = idExpr->getName();
                    if (cond.starts_with("__uzpp_catch ")) {
                        isCatchBlock = true;
                        // Extract variable name: last whitespace-delimited token
                        std::string decl = cond.substr(13); // strip "__uzpp_catch "
                        auto pos = decl.rfind(' ');
                        catchVarName = (pos != std::string::npos) ? decl.substr(pos + 1) : decl;
                    }
                }

                bool initialReachable = reachable_;
                if (isCatchBlock && !catchVarName.empty()) {
                    enterScope();
                    Token tok = getTokenForNode(ifs->getCondition());
                    declareVar(catchVarName, "std::exception", tok);
                    scopes_.back()[catchVarName].used = true;
                    checkNode(ifs->getThenBranch());
                    exitScope();
                } else {
                    checkNode(ifs->getThenBranch());
                }
                bool thenReachable = reachable_;

                reachable_ = initialReachable;
                reportedUnreachable_ = false;
                if (ifs->getElseBranch()) {
                    checkNode(ifs->getElseBranch());
                }

                if (!ifs->getElseBranch()) {
                    reachable_ = initialReachable;
                } else {
                    reachable_ = thenReachable || reachable_;
                }
                reportedUnreachable_ = false;
                break;
            }
            case ASTNodeType::ForStatement: {
                auto fors = static_cast<const ForStatement*>(node);
                enterScope();
                loopDepth_++;
                if (fors->getInit()) checkNode(fors->getInit());
                if (fors->getCondition()) checkExpr(fors->getCondition());
                if (fors->getIncrement()) checkExpr(fors->getIncrement());
                
                bool initialReachable = reachable_;
                if (fors->getBody()) checkNode(fors->getBody());
                
                reachable_ = initialReachable;
                reportedUnreachable_ = false;
                loopDepth_--;
                exitScope();
                break;
            }
            case ASTNodeType::WhileStatement: {
                auto whiles = static_cast<const WhileStatement*>(node);
                if (whiles->getCondition()) checkExpr(whiles->getCondition());
                loopDepth_++;
                bool initialReachable = reachable_;
                if (whiles->getBody()) checkNode(whiles->getBody());
                reachable_ = initialReachable;
                reportedUnreachable_ = false;
                loopDepth_--;
                break;
            }
            case ASTNodeType::ReturnStatement: {
                auto ret = static_cast<const ReturnStatement*>(node);
                std::string retType = "bosh";
                if (ret->getValue()) {
                    checkExpr(ret->getValue());
                    retType = inferType(ret->getValue());
                }
                
                if (!currentReturnType_.empty() && currentReturnType_ != "ozgaruvchan" && currentReturnType_ != "o'zgaruvchan") {
                    std::string expResolved = resolveType(currentReturnType_);
                    std::string gotResolved = resolveType(retType);
                    if (expResolved != gotResolved && retType != "noma'lum") {
                        if (!((expResolved == "haqiqiy" || expResolved == "ikkilangan") && gotResolved == "butun")) {
                            if (!classIsSubtype(gotResolved, expResolved)) {
                                reportWarning("Funksiya '" + currentReturnType_ + "' qaytarishi kerak, lekin '" + retType + "' qaytarilmoqda.", ret->getReturnToken());
                            }
                        }
                    }
                }
                reachable_ = false;
                reportedUnreachable_ = false;
                break;
            }
            case ASTNodeType::BreakStatement:
            case ASTNodeType::ContinueStatement: {
                if (loopDepth_ == 0) {
                    reportError("'to'xtatish' yoki 'davom_etish' faqat sikl ichida ishlatilishi mumkin.", getTokenForNode(node));
                }
                reachable_ = false;
                reportedUnreachable_ = false;
                break;
            }
            case ASTNodeType::ExpressionStatement: {
                checkExpr(static_cast<const ExpressionStatement*>(node)->getExpression());
                break;
            }
            case ASTNodeType::MatchStatement: {
                auto ms = static_cast<const MatchStatement*>(node);
                checkExpr(ms->getCondition());
                bool savedReachable = reachable_;
                bool savedReported = reportedUnreachable_;
                for (const auto& mc : ms->getCases()) {
                    if (mc->pattern) checkExpr(mc->pattern.get());
                    // Each case arm is an independent branch — reset reachability
                    reachable_ = true;
                    reportedUnreachable_ = false;
                    if (mc->body) checkNode(mc->body.get());
                }
                reachable_ = savedReachable;
                reportedUnreachable_ = savedReported;
                break;
            }
            case ASTNodeType::Group: {
                // shablon funksiyalari va sinflarini GroupNode ichida o'rab keladi
                auto grp = static_cast<const GroupNode*>(node);
                for (const auto& child : grp->getChildren()) {
                    if (child->getType() == ASTNodeType::FunctionDeclaration) {
                        auto fn = static_cast<const FunctionDeclaration*>(child.get());
                        templateFunctions_.insert(fn->getName());
                    }
                    checkNode(child.get());
                }
                break;
            }
            case ASTNodeType::TypeAlias: {
                auto ta = static_cast<const TypeAlias*>(node);
                typeAliases_[ta->getAlias()] = ta->getTarget();
                break;
            }
            case ASTNodeType::EnumDeclaration: {
                auto en = static_cast<const EnumDeclaration*>(node);
                // Register enum type so "EnumName::Value" expressions don't trigger warnings
                ClassInfo info;
                for (const auto& v : en->getValues()) {
                    info.fields[v.name] = en->getName();
                }
                // Helper function registered as built-in so TypeChecker doesn't complain
                functionParams_[en->getName() + "_nomi"] = {en->getName()};
                functionReturns_[en->getName() + "_nomi"] = "matn";
                classes_[en->getName()] = info;
                break;
            }
            default: break;
        }
    }

    // F-string ichidagi o'zgaruvchi nomlarini ajratib olish
    static std::vector<std::string> extractFStringVars(const std::string& s) {
        std::vector<std::string> vars;
        size_t i = 0;
        while (i < s.size()) {
            if (s[i] == '{' && i + 1 < s.size() && s[i+1] != '{') {
                size_t end = s.find('}', i + 1);
                if (end != std::string::npos) {
                    std::string inner = s.substr(i + 1, end - i - 1);
                    // Only simple identifiers (no dots, no spaces at start)
                    if (!inner.empty() && (std::isalpha(inner[0]) || inner[0] == '_')) {
                        // Trim any format spec after ':'
                        size_t colon = inner.find(':');
                        if (colon != std::string::npos) inner = inner.substr(0, colon);
                        vars.push_back(inner);
                    }
                    i = end + 1;
                    continue;
                }
            }
            i++;
        }
        return vars;
    }

    void checkExpr(const Expression* expr) {
        if (!expr) return;
        switch (expr->getType()) {
            case ASTNodeType::LiteralExpression: {
                auto lit = static_cast<const LiteralExpression*>(expr);
                if (lit->getLiteralType() == LiteralExpression::LiteralType::FormatString) {
                    // F-string ichidagi o'zgaruvchilarni "ishlatilgan" deb belgilash
                    for (const auto& varName : extractFStringVars(lit->getValue())) {
                        markUsed(varName);
                    }
                }
                break;
            }
            case ASTNodeType::IdentifierExpression: {
                auto id = static_cast<const IdentifierExpression*>(expr);
                std::string name = id->getName();
                // Shablon argumenti bo'lgan nomlar (<...> ichida) — standart funksiyalar
                if (name.find('<') != std::string::npos) break;
                // Pack-expansion / fold-expression ellipsis — kompilyator C++ ga so'zma-so'z chiqaradi
                if (name == "...") break;
                // C++ built-in operators — not variables
                static const std::unordered_set<std::string> cppOperators = {
                    "sizeof", "sizeof...", "typeid", "alignof", "alignas", "decltype",
                    "noexcept", "static_assert", "throw"
                };
                if (cppOperators.contains(name)) break;
                // Ogohlantirish faqat kichik harf bilan boshlangan va "::" qatnashmagan noma'lum o'zgaruvchilarga
                if (!name.empty() && std::islower(name[0]) && name.find("::") == std::string::npos && !isDeclared(name)) {
                    reportWarning("Noma'lum o'zgaruvchi ishlatilmoqda -> " + name, id->getSourceToken());
                }
                break;
            }
            case ASTNodeType::AssignmentExpression: {
                auto asgn = static_cast<const AssignmentExpression*>(expr);
                checkExpr(asgn->getTarget());
                checkExpr(asgn->getValue());
                
                std::string targetType = inferType(asgn->getTarget());
                std::string valueType = inferType(asgn->getValue());
                
                if (targetType != "noma'lum" && valueType != "noma'lum" && targetType != valueType) {
                    if ((targetType == "butun" || targetType == "matn" || targetType == "mantiqiy" || targetType == "haqiqiy") &&
                        (valueType == "butun" || valueType == "matn" || valueType == "mantiqiy" || valueType == "haqiqiy")) {
                        if (!(targetType == "haqiqiy" && valueType == "butun")) {
                            reportWarning("Tur nomutanosibligi: '" + targetType + "' o'zgaruvchiga '" + valueType + "' qiymat ta'minlanmoqda.", getTokenForNode(expr));
                        }
                    }
                }
                break;
            }
            case ASTNodeType::AwaitExpression: {
                auto aw = static_cast<const AwaitExpression*>(expr);
                if (!currentFunctionIsAsync_) {
                    reportError("'kutish' (await) operatori faqat 'asinxron' funksiyalar ichida ishlatilishi mumkin.", aw->getAwaitToken());
                }
                checkExpr(aw->getExpression());
                break;
            }
            case ASTNodeType::TernaryExpression: {
                auto tern = static_cast<const TernaryExpression*>(expr);
                checkExpr(tern->getCondition());
                checkExpr(tern->getThenExpr());
                checkExpr(tern->getElseExpr());
                break;
            }
            case ASTNodeType::PipelineExpression: {
                auto pipe = static_cast<const PipelineExpression*>(expr);
                checkExpr(pipe->getLeft());
                checkExpr(pipe->getRight());
                break;
            }
            case ASTNodeType::SubscriptAccess: {
                auto sub = static_cast<const SubscriptAccess*>(expr);
                checkExpr(sub->getArray());
                checkExpr(sub->getIndex());
                std::string indexType = inferType(sub->getIndex());
                // Allow butun (int) and matn (string) — matn is valid for map/JSON subscript
                if (indexType != "noma'lum" && indexType != "butun" && indexType != "matn") {
                    reportWarning("Massiv indeksi 'butun' (int) yoki 'matn' (string) bo'lishi kerak, lekin '" + indexType + "' berildi.", sub->getBracketToken());
                }
                // C++23 multidim: also check extra indices
                for (const auto& extra : sub->getExtraIndices()) {
                    checkExpr(extra.get());
                }
                break;
            }
            case ASTNodeType::LambdaExpression: {
                auto lam = static_cast<const LambdaExpression*>(expr);
                // Save outer function state — lambda has its own reachability and return type
                bool savedReachable = reachable_;
                bool savedReported = reportedUnreachable_;
                std::string savedReturn = currentReturnType_;
                reachable_ = true;
                reportedUnreachable_ = false;
                currentReturnType_ = lam->getReturnType().empty() ? "ozgaruvchan" : lam->getReturnType();
                enterScope();
                for (const auto& p : lam->getParams()) {
                    declareVar(p.name, p.type.empty() ? "ozgaruvchan" : p.type, lam->getLambdaToken());
                }
                if (lam->getBody()) checkNode(lam->getBody());
                exitScope();
                // Restore outer state — lambda doesn't affect outer reachability
                reachable_ = savedReachable;
                reportedUnreachable_ = savedReported;
                currentReturnType_ = savedReturn;
                break;
            }
            case ASTNodeType::FunctionCall: {
                auto call = static_cast<const FunctionCall*>(expr);
                checkExpr(call->getCallee());
                
                if (call->getCallee()->getType() == ASTNodeType::IdentifierExpression) {
                    std::string name = static_cast<const IdentifierExpression*>(call->getCallee())->getName();
                    if (functionParams_.contains(name) && !templateFunctions_.contains(name)) {
                        const auto& expectedParams = functionParams_[name];
                        std::size_t minArgs = functionMinArgs_.contains(name) ? functionMinArgs_[name] : expectedParams.size();
                        std::size_t gotArgs = call->getArguments().size();
                        if (gotArgs < minArgs || gotArgs > expectedParams.size()) {
                            std::string expected = minArgs == expectedParams.size()
                                ? std::to_string(expectedParams.size())
                                : std::to_string(minArgs) + ".." + std::to_string(expectedParams.size());
                            reportError("Funksiya '" + name + "' " + expected + " ta argument kutadi, lekin " + std::to_string(gotArgs) + " ta berildi.", call->getCallToken());
                        } else {
                            auto stripRef = [](std::string t) {
                                while (!t.empty() && (t.back() == '&' || t.back() == '*')) t.pop_back();
                                return t;
                            };
                            for (size_t i = 0; i < gotArgs; ++i) {
                                std::string argType = inferType(call->getArguments()[i].get());
                                std::string expBase = stripRef(expectedParams[i]);
                                if (argType != "noma'lum" && expectedParams[i] != "ozgaruvchan" && argType != expectedParams[i] && argType != expBase) {
                                    if (!((expBase == "haqiqiy" || expBase == "ikkilangan") && argType == "butun")) {
                                        // Suppress if argType is a subclass of expBase
                                        if (!classIsSubtype(argType, expBase)) {
                                            reportWarning("Argument " + std::to_string(i+1) + " turi mos emas: '" + expectedParams[i] + "' kutilgan, lekin '" + argType + "' berildi.", getTokenForNode(call->getArguments()[i].get()));
                                        }
                                    }
                                }
                            }
                        }
                    }
                } else if (call->getCallee()->getType() == ASTNodeType::MemberAccess) {
                    auto mac = static_cast<const MemberAccess*>(call->getCallee());
                    std::string objType = inferType(mac->getObject());
                    if (objType != "noma'lum" && classes_.contains(objType)) {
                        std::string methodName = mac->getMemberName();
                        if (classes_[objType].methodParams.contains(methodName)) {
                            const auto& expectedParams = classes_[objType].methodParams[methodName];
                            if (expectedParams.size() != call->getArguments().size()) {
                                reportError("Metod '" + methodName + "' " + std::to_string(expectedParams.size()) + " ta argument kutadi, lekin " + std::to_string(call->getArguments().size()) + " ta berildi.", call->getCallToken());
                            } else {
                                for (size_t i = 0; i < expectedParams.size(); ++i) {
                                    std::string argType = inferType(call->getArguments()[i].get());
                                    if (argType != "noma'lum" && expectedParams[i] != "ozgaruvchan" && argType != expectedParams[i]) {
                                        if (!((expectedParams[i] == "haqiqiy" || expectedParams[i] == "ikkilangan") && argType == "butun")) {
                                            reportWarning("Argument " + std::to_string(i+1) + " turi mos emas: '" + expectedParams[i] + "' kutilgan, lekin '" + argType + "' berildi.", getTokenForNode(call->getArguments()[i].get()));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                
                for (const auto& arg : call->getArguments()) checkExpr(arg.get());
                break;
            }
            case ASTNodeType::UnaryExpression: {
                auto un = static_cast<const UnaryExpression*>(expr);
                checkExpr(un->getExpression());
                break;
            }
            case ASTNodeType::BinaryExpression: {
                auto bin = static_cast<const BinaryExpression*>(expr);
                checkExpr(bin->getLeft());
                checkExpr(bin->getRight());
                break;
            }
            case ASTNodeType::MemberAccess: {
                auto mac = static_cast<const MemberAccess*>(expr);
                checkExpr(mac->getObject());

                std::string objType = inferType(mac->getObject());
                if (objType != "noma'lum" && classes_.contains(objType)) {
                    if (!classHasMember(objType, mac->getMemberName())) {
                        reportError("Sinf '" + objType + "' da '" + mac->getMemberName() + "' nomli maydon yoki metod topilmadi.", mac->getAccessToken());
                    }
                }

                break;
            }
            default: break;
        }
    }
};

} // namespace uzpp