#pragma once

#include "ast.h"
#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <cctype>

namespace uzpp {

struct SemanticError {
    std::string message;
    int line;
    int column;
};

// Uch-holatli tur tasviri ("tri-state" — halol tur xulosasi uchun).
// Phase 2: kompozit turlar (ko'rsatkich, havola, shablon) + interning.
//
//   - Aniq(nomi)         — konkret tur ("butun", "vektor<matn>", "Foo")
//   - Nomalum            — xulosa qilolmadik
//   - Polimorf(param)    — shablon tur parametri (T, U)
//   - Korsatkich(base)   — ko'rsatkich turi (butun*)
//   - Havola(base,const) — havola turi (butun&, const matn&)
//   - Shablon(nomi,args) — shablon misoli (vektor<butun>, lug'at<matn,T>)
//
// isAniq() rekursiv: Korsatkich(Polimorf) → yolg'on, Shablon(vektor<T>) → yolg'on.
// Bu vektor<T> yoki Foo<T>* kabi kompozit turlarda diagnostikani jim qiladi.
//
// Interning: Type::aniq("butun") har doim bir xil obyektga ishora qiladi.
struct Type {
    enum class Kind : unsigned char {
        Aniq,       // konkret nomli tur
        Nomalum,    // noma'lum
        Polimorf,   // shablon parametri
        Korsatkich, // ko'rsatkich (T*)
        Havola,     // havola (T&, const T&)
        Shablon     // shablon misoli (vektor<T>)
    };

    Kind kind = Kind::Nomalum;
    std::string name;           // Aniq/Polimorf/Shablon: nom; Korsatkich/Havola/Nomalum: bo'sh
    const Type* baseType = nullptr;  // Korsatkich/Havola: asos turi
    bool isConstRef = false;        // Havola: const T& yoki T&
    std::vector<Type> templateArgs; // Shablon: argument turlari

    // ---- fabrikalar ----
    static Type aniq(std::string n)     { Type t{Kind::Aniq}; t.name = std::move(n); return t; }
    static Type nomalum()                { return Type{Kind::Nomalum}; }
    static Type polimorf(std::string p) { Type t{Kind::Polimorf}; t.name = std::move(p); return t; }
    static Type korsatkich(Type base)   { Type t{Kind::Korsatkich}; t.baseType = intern(base); return t; }
    static Type havola(Type base, bool konst = false) {
        Type t{Kind::Havola}; t.baseType = intern(base); t.isConstRef = konst; return t;
    }
    static Type shablon(std::string nom, std::vector<Type> args) {
        Type t{Kind::Shablon}; t.name = std::move(nom); t.templateArgs = std::move(args); return t;
    }

    // ---- so'rov metodlari ----
    bool isAniq() const {
        switch (kind) {
        case Kind::Aniq: return true;
        case Kind::Korsatkich:
        case Kind::Havola: return baseType && baseType->isAniq();
        case Kind::Shablon:
            for (const auto& a : templateArgs) if (!a.isAniq()) return false;
            return true;
        default: return false;
        }
    }
    bool isNomalum()  const { return kind == Kind::Nomalum; }
    bool isPolimorf() const { return kind == Kind::Polimorf; }

    // Accessor: aniq tur nomi (rekursiv to'liq nom qaytaradi)
    std::string aniqNomi() const {
        switch (kind) {
        case Kind::Aniq: return name;
        case Kind::Polimorf: return name;
        case Kind::Korsatkich: return baseType ? baseType->aniqNomi() + "*" : "noma'lum*";
        case Kind::Havola: {
            std::string s = baseType ? baseType->aniqNomi() : "noma'lum";
            return isConstRef ? "const " + s + "&" : s + "&";
        }
        case Kind::Shablon: {
            std::string s = name + "<";
            for (std::size_t i = 0; i < templateArgs.size(); ++i) {
                if (i > 0) s += ", ";
                s += templateArgs[i].aniqNomi();
            }
            s += ">";
            return s;
        }
        default: return "noma'lum";
        }
    }

    // Eski string-asosli kod yo'llari uchun moslashuvchi konvertatsiya
    std::string toLegacyString() const { return isAniq() ? aniqNomi() : "noma'lum"; }

private:
    // Tur interningi — bir xil turlar bir xil ko'rsatkichga ega bo'lishini ta'minlaydi.
    //
    // Ikki strategiya:
    //   1) Aniq turlar uchun — nom-bo'yicha kanonik xarita (Type::aniq("butun")
    //      har doim bir xil ko'rsatkich qaytaradi).
    //   2) Kompozit turlar uchun — structural kalit (kind+aniqNomi) bo'yicha
    //      kanonik xarita. Bu Korsatkich(Aniq("butun")) ni har gal bir xil
    //      obyektga ishora qildiradi va xotira o'sishini ham cheklaydi.
    //
    // Saqlash idishlari: std::deque va std::unordered_map — har ikkisi ham
    // element ko'rsatkichlari uchun kafolatlangan barqaror (push/insert da
    // ko'chmaydi). std::vector bu yerda noto'g'ri tanlov edi — reallokatsiya
    // oldindan berilgan ko'rsatkichlarni invalidatsiya qiladi.
    static const Type* intern(const Type& t) {
        if (t.kind == Kind::Aniq && t.baseType == nullptr && t.templateArgs.empty()) {
            static std::unordered_map<std::string, Type> canon;
            auto [it, _] = canon.try_emplace(t.name, t);
            return &it->second;
        }
        // Kompozit: kind + to'liq nom bo'yicha structural intern.
        // Kalit shakli: "K:nom" — K bir belgi (P/H/S — Korsatkich/Havola/Shablon).
        char k = (t.kind == Kind::Korsatkich) ? 'P'
               : (t.kind == Kind::Havola)     ? 'H'
               : (t.kind == Kind::Shablon)    ? 'S'
                                              : '?';
        std::string key;
        key.reserve(t.aniqNomi().size() + 2);
        key += k; key += ':'; key += t.aniqNomi();
        static std::unordered_map<std::string, Type> compositeCanon;
        auto [it, _] = compositeCanon.try_emplace(key, t);
        return &it->second;
    }
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
    // Hozir tekshirilayotgan shablon funksiyasining tur parametrlari
    // (`shablon<tur T, tur U>` dan T, U). Tan ichida T turidagi identifikator
    // Aniq("T") emas, Polimorf("T") sifatida xulosalanadi — diagnostika jim qoladi.
    std::unordered_set<std::string> currentTemplateParams_;
    // For LSP inlay hints: maps `o'zgaruvchan x = ...` declarations to the
    // inferred type of the initializer (only set when inference succeeded).
    std::unordered_map<const VariableDeclaration*, std::string> inferredAutoTypes_;
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

    // Treat aliased type names as one type so we don't emit noisy
    // "Tur nomutanosibligi" warnings for surface-equivalent pairs.
    // Keep this list tight — only true aliases that compile to identical
    // C++ types belong here.
    static bool typesEquivalent(const std::string& a, const std::string& b) {
        if (a == b) return true;
        // bool: mantiq == mantiqiy
        if ((a == "mantiq" && b == "mantiqiy") || (a == "mantiqiy" && b == "mantiq")) return true;
        // double: haqiqiy == ikkilangan
        if ((a == "haqiqiy" && b == "ikkilangan") || (a == "ikkilangan" && b == "haqiqiy")) return true;
        return false;
    }

    // Evristika: nom shablon tur parametriga o'xshaydimi (T, U, K, TKey, ...)?
    // Bosh harfdan boshlanadi, faqat alfanumerik/underscore, ma'lum konkret
    // turlar ro'yxatida emas. Bir-ikkita yolg'on musbat (masalan, foydalanuvchi
    // o'zining T degan sinfini yozsa) — diagnostikani jim qoladi, halokat emas.
    bool looksLikeTemplateParam(const std::string& name) const {
        if (name.empty()) return false;
        unsigned char c0 = static_cast<unsigned char>(name[0]);
        if (!(c0 >= 'A' && c0 <= 'Z')) return false;
        for (char c : name) {
            unsigned char u = static_cast<unsigned char>(c);
            if (!((u >= 'A' && u <= 'Z') || (u >= 'a' && u <= 'z') ||
                  (u >= '0' && u <= '9') || u == '_')) return false;
        }
        if (classes_.contains(name)) return false;
        if (typeAliases_.contains(name)) return false;
        // Stdlib-da ma'lum bo'lgan bosh harfli turlar — shablon parametri emas
        static const std::unordered_set<std::string> wellKnownTypes = {
            "Json", "Natija", "Tanlov", "Matn", "RegEx", "Matematika",
            "Vektor2", "Vektor3", "Matritsa2x2", "Qiymat", "OqimHovuz",
            "VazifaJavob", "TestTo'plami",
            "INT_MAX", "INT_MIN", "UINT_MAX", "SIZE_MAX",
            "DBL_MAX", "FLT_MAX", "EXIT_SUCCESS", "EXIT_FAILURE",
            "EOF", "NULL", "Grafika", "Hodisalar", "Vidjetlar"
        };
        if (wellKnownTypes.contains(name)) return false;
        return true;
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

    // Tur xulosasi (type inference) ning asosiy yo'li. Uch-holatli Type qaytaradi:
    //   Aniq(nomi)  — aniq tur belgilandi
    //   Nomalum     — xulosa qilolmadik (diagnostikada jim)
    //   Polimorf    — hozircha bu yerda chiqarilmaydi; shablon tanasi xulosasiga qoldirilgan
    //
    // Diagnostikani turga bog'lab tekshiradigan barcha joylar nomlarni
    // solishtirgunga qadar .isAniq() ni tekshirishi kerak. Eskirib qolgan
    // inferType() o'rovi oddiy string qaytaradi — eski kontraktga muhtoj kod uchun.
    Type inferTypeT(const Expression* expr) {
        if (!expr) return Type::nomalum();
        switch (expr->getType()) {
            case ASTNodeType::LiteralExpression: {
                auto lit = static_cast<const LiteralExpression*>(expr);
                switch(lit->getLiteralType()) {
                    case LiteralExpression::LiteralType::Integer: return Type::aniq("butun");
                    case LiteralExpression::LiteralType::Float: return Type::aniq("haqiqiy");
                    case LiteralExpression::LiteralType::String:
                    case LiteralExpression::LiteralType::FormatString: return Type::aniq("matn");
                    case LiteralExpression::LiteralType::Character: return Type::aniq("belgi");
                    case LiteralExpression::LiteralType::Boolean: return Type::aniq("mantiqiy");
                }
                break;
            }
            case ASTNodeType::IdentifierExpression: {
                auto id = static_cast<const IdentifierExpression*>(expr);
                std::string name = id->getName();
                for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
                    if (it->contains(name)) {
                        (*it)[name].used = true;
                        const std::string& t = (*it)[name].type;
                        if (t.empty() || t == "noma'lum") return Type::nomalum();
                        // Joriy shablon parametriga teng bo'lsa — Polimorf,
                        // aks holda Aniq. Polimorf diagnostikada jim qabul qilinadi.
                        if (currentTemplateParams_.contains(t)) return Type::polimorf(t);
                        return Type::aniq(t);
                    }
                }
                break;
            }
            case ASTNodeType::BinaryExpression: {
                auto bin = static_cast<const BinaryExpression*>(expr);
                std::string op = bin->getOperator();
                if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=" || op == "&&" || op == "||" || op == "va" || op == "yoki") {
                    return Type::aniq("mantiqiy");
                }
                Type leftType = inferTypeT(bin->getLeft());
                Type rightType = inferTypeT(bin->getRight());
                if (leftType.isAniq() && rightType.isAniq()) {
                    const std::string& ln = leftType.aniqNomi();
                    const std::string& rn = rightType.aniqNomi();
                    if (ln == "haqiqiy" || rn == "haqiqiy") return Type::aniq("haqiqiy");
                    if (ln == "matn" || rn == "matn") return Type::aniq("matn");
                    return leftType;
                }
                if (leftType.isAniq()) return leftType;
                if (rightType.isAniq()) return rightType;
                return Type::nomalum();
            }
            case ASTNodeType::MemberAccess: {
                auto mac = static_cast<const MemberAccess*>(expr);
                Type objType = inferTypeT(mac->getObject());
                if (objType.isAniq() && classes_.contains(objType.aniqNomi())) {
                    std::string ret = classMethodReturn(objType.aniqNomi(), mac->getMemberName());
                    if (ret != "noma'lum") return Type::aniq(ret);
                }
                break;
            }
            case ASTNodeType::SubscriptAccess: {
                auto sub = static_cast<const SubscriptAccess*>(expr);
                Type arrType = inferTypeT(sub->getArray());
                // Phase 2: shablon turi orqali element turini chiqaramiz
                if (arrType.isAniq() && arrType.kind == Type::Kind::Shablon &&
                    arrType.name == "vektor" && arrType.templateArgs.size() == 1) {
                    return arrType.templateArgs[0];
                }
                // Legacy string-based fallback
                const std::string& an = arrType.aniqNomi();
                if (arrType.isAniq() && an.starts_with("vektor<") && an.back() == '>') {
                    return Type::aniq(an.substr(7, an.length() - 8));
                }
                break;
            }
            case ASTNodeType::FunctionCall: {
                auto call = static_cast<const FunctionCall*>(expr);
                if (call->getCallee()->getType() == ASTNodeType::IdentifierExpression) {
                    std::string name = static_cast<const IdentifierExpression*>(call->getCallee())->getName();
                    if (name == "__uzpp_array") {
                        if (!call->getArguments().empty()) {
                            Type elem = inferTypeT(call->getArguments()[0].get());
                            return Type::shablon("vektor", {elem});
                        }
                        return Type::aniq("vektor<noma'lum>");
                    }
                    if (name == "__uzpp_dict") {
                        return Type::aniq("lug'at<matn, noma'lum>");
                    }
                    if (functionReturns_.contains(name)) {
                        const std::string& r = functionReturns_[name];
                        return r.empty() || r == "noma'lum" ? Type::nomalum() : Type::aniq(r);
                    }
                    if (classes_.contains(name)) return Type::aniq(name);
                } else if (call->getCallee()->getType() == ASTNodeType::MemberAccess) {
                    auto mac = static_cast<const MemberAccess*>(call->getCallee());
                    Type objType = inferTypeT(mac->getObject());
                    if (objType.isAniq() && classes_.contains(objType.aniqNomi())) {
                        std::string ret = classMethodReturn(objType.aniqNomi(), mac->getMemberName());
                        if (ret != "noma'lum") return Type::aniq(ret);
                    }
                }
                break;
            }
            case ASTNodeType::UnaryExpression: {
                auto un = static_cast<const UnaryExpression*>(expr);
                Type inner = inferTypeT(un->getExpression());
                switch (un->getOperator()) {
                    case UnaryExpression::UnaryOp::LogicalNot:
                        return Type::aniq("mantiqiy");
                    case UnaryExpression::UnaryOp::AddressOf:
                        if (inner.isAniq()) return Type::korsatkich(inner);
                        return Type::nomalum();
                    case UnaryExpression::UnaryOp::Dereference:
                        if (inner.isAniq() && inner.kind == Type::Kind::Korsatkich && inner.baseType) {
                            return *inner.baseType;
                        }
                        // Legacy string-based fallback
                        if (inner.isAniq()) {
                            const std::string& an = inner.aniqNomi();
                            if (!an.empty() && an.back() == '*')
                                return Type::aniq(an.substr(0, an.size() - 1));
                        }
                        return Type::nomalum();
                    case UnaryExpression::UnaryOp::New:
                        if (inner.isAniq()) return Type::korsatkich(inner);
                        return Type::nomalum();
                    case UnaryExpression::UnaryOp::Delete:
                        return Type::aniq("bosh");
                    case UnaryExpression::UnaryOp::Plus:
                    case UnaryExpression::UnaryOp::Minus:
                    case UnaryExpression::UnaryOp::BitwiseNot:
                    case UnaryExpression::UnaryOp::PreIncrement:
                    case UnaryExpression::UnaryOp::PreDecrement:
                    case UnaryExpression::UnaryOp::PostIncrement:
                    case UnaryExpression::UnaryOp::PostDecrement:
                        return inner; // Aniq/Nomalum holati operanddan saqlanadi
                }
                return Type::nomalum();
            }
            case ASTNodeType::TernaryExpression: {
                auto tern = static_cast<const TernaryExpression*>(expr);
                Type t = inferTypeT(tern->getThenExpr());
                Type e = inferTypeT(tern->getElseExpr());
                if (t.isAniq() && e.isAniq()) {
                    if (typesEquivalent(t.aniqNomi(), e.aniqNomi())) return t;
                    // Numeric promotion: butun + haqiqiy → haqiqiy
                    const std::string& tn = t.aniqNomi();
                    const std::string& en = e.aniqNomi();
                    if ((tn == "haqiqiy" && en == "butun") ||
                        (tn == "butun" && en == "haqiqiy")) return Type::aniq("haqiqiy");
                    return Type::nomalum();
                }
                if (t.isAniq()) return t;
                if (e.isAniq()) return e;
                return Type::nomalum();
            }
            case ASTNodeType::AssignmentExpression: {
                // C++ assignment expression evaluates to the value (or the target lvalue).
                // Prefer the value side; fall back to target.
                auto asgn = static_cast<const AssignmentExpression*>(expr);
                Type v = inferTypeT(asgn->getValue());
                if (v.isAniq()) return v;
                return inferTypeT(asgn->getTarget());
            }
            case ASTNodeType::AwaitExpression: {
                // kutish expr / chiqar_qadam expr — operand's "co_await result type"
                // awaitable-trait kuzatishni talab qiladi — hozircha yo'q. Halol: Nomalum.
                return Type::nomalum();
            }
            case ASTNodeType::ThrowExpression: {
                // `throw expr` has type `void` in C++ but is a "never" in practice.
                // Nomalum deb belgilash — qiymatini ishlatish allaqachon kategoriya xatosi.
                return Type::nomalum();
            }
            case ASTNodeType::LambdaExpression: {
                // Lambdas have anonymous closure types. We can later represent these
                // keyinchalik "@lambda<ret(args...)>" sintetik turi bilan ko'rsatsa bo'ladi, hozircha: Nomalum.
                return Type::nomalum();
            }
            case ASTNodeType::PipelineExpression: {
                // `left |> right` desugars to `right(left)` — right's return type
                // could be inferred from functionReturns_, but the pipeline syntax
                // ko'pincha std::ranges adapterlari bilan ishlatiladi. Halol: Nomalum.
                return Type::nomalum();
            }
            default: break;
        }
        return Type::nomalum();
    }

    // Eskirib qolgan o'rov: Nomalum va Polimorf uchun "noma'lum" qaytaradi.
    // Existing call sites still go through here; new code should prefer
    // inferTypeT() ga o'tib, nomlarni solishtirgunga qadar isAniq() ni tekshirish kerak.
    std::string inferType(const Expression* expr) {
        return inferTypeT(expr).toLegacyString();
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
            "OqimHovuz", "VazifaJavob",
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

    // Returns the inferred type for an `o'zgaruvchan`/`o'zgarmas` (auto) variable
    // declaration with an initializer, recorded during check(). Returns nullptr
    // if the variable was not declared with auto-type or its initializer's type
    // could not be inferred.
    const std::string* getInferredAutoType(const VariableDeclaration* var) const {
        const auto it = inferredAutoTypes_.find(var);
        return it != inferredAutoTypes_.end() ? &it->second : nullptr;
    }

    const std::unordered_map<const VariableDeclaration*, std::string>& getInferredAutoTypes() const {
        return inferredAutoTypes_;
    }

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
                    Type inferred = inferTypeT(var->getInitializer());

                    if (declaredType == "ozgaruvchan" || declaredType == "o'zgaruvchan" || declaredType == "ozgarmas") {
                        if (inferred.isAniq()) {
                            inferredAutoTypes_[var] = inferred.aniqNomi();
                        }
                        declaredType = inferred.toLegacyString(); // Type inference
                    } else if (inferred.isAniq() && declaredType != "noma'lum" &&
                               !typesEquivalent(inferred.aniqNomi(), declaredType)) {
                        const std::string& inferredType = inferred.aniqNomi();
                        if ((declaredType == "butun" || declaredType == "matn" || declaredType == "mantiq" || declaredType == "mantiqiy" || declaredType == "haqiqiy" || declaredType == "ikkilangan") &&
                            (inferredType == "butun" || inferredType == "matn" || inferredType == "mantiq" || inferredType == "mantiqiy" || inferredType == "haqiqiy" || inferredType == "ikkilangan")) {
                            if (!((declaredType == "haqiqiy" || declaredType == "ikkilangan") && inferredType == "butun")) {
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

                // Shablon funksiyalari uchun: signaturaga nazar tashlab, shablon
                // tur parametri ko'rinishidagi nomlarni currentTemplateParams_ ga
                // qo'shamiz. Bu funksiya tanasi davomida T-li ifodalar Polimorf
                // sifatida xulosalanadi va diagnostika jim qoladi.
                auto savedTemplateParams = currentTemplateParams_;
                bool isTemplate = templateFunctions_.contains(func->getName());
                if (isTemplate) {
                    for (const auto& p : func->getParameters()) {
                        if (looksLikeTemplateParam(p.type)) {
                            currentTemplateParams_.insert(p.type);
                        }
                    }
                    if (looksLikeTemplateParam(func->getReturnType())) {
                        currentTemplateParams_.insert(func->getReturnType());
                    }
                }

                enterScope();
                for (const auto& p : func->getParameters()) {
                    declareVar(p.name, p.type, p.token);
                }
                if (func->getBody()) {
                    for (const auto& s : func->getBody()->getStatements()) checkNode(s.get());
                }
                exitScope();

                currentTemplateParams_ = savedTemplateParams;
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
                Type retInferred = Type::aniq("bosh");
                if (ret->getValue()) {
                    checkExpr(ret->getValue());
                    retInferred = inferTypeT(ret->getValue());
                }

                // Shablon parametri qaytariladigan tur sifatida ko'rsatilgan bo'lsa
                // (`-> T`), aniq nima qaytarilgani ahamiyatga ega emas — instansiyalashda T
                // mos kelishi ham mumkin. Bu yerda ham diagnostikani jim qoldiramiz.
                if (!currentReturnType_.empty() && currentReturnType_ != "ozgaruvchan" && currentReturnType_ != "o'zgaruvchan"
                    && retInferred.isAniq()
                    && !currentTemplateParams_.contains(currentReturnType_)) {
                    std::string expResolved = resolveType(currentReturnType_);
                    std::string gotResolved = resolveType(retInferred.name);
                    if (expResolved != gotResolved) {
                        if (!((expResolved == "haqiqiy" || expResolved == "ikkilangan") && gotResolved == "butun")) {
                            if (!classIsSubtype(gotResolved, expResolved)) {
                                reportWarning("Funksiya '" + currentReturnType_ + "' qaytarishi kerak, lekin '" + retInferred.name + "' qaytarilmoqda.", ret->getReturnToken());
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

                Type target = inferTypeT(asgn->getTarget());
                Type value = inferTypeT(asgn->getValue());

                if (target.isAniq() && value.isAniq() &&
                    !typesEquivalent(target.name, value.name)) {
                    const std::string& targetType = target.name;
                    const std::string& valueType = value.name;
                    if ((targetType == "butun" || targetType == "matn" || targetType == "mantiq" || targetType == "mantiqiy" || targetType == "haqiqiy" || targetType == "ikkilangan") &&
                        (valueType == "butun" || valueType == "matn" || valueType == "mantiq" || valueType == "mantiqiy" || valueType == "haqiqiy" || valueType == "ikkilangan")) {
                        if (!((targetType == "haqiqiy" || targetType == "ikkilangan") && valueType == "butun")) {
                            reportWarning("Tur nomutanosibligi: '" + targetType + "' o'zgaruvchiga '" + valueType + "' qiymat ta'minlanmoqda.", getTokenForNode(expr));
                        }
                    }
                }
                break;
            }
            case ASTNodeType::AwaitExpression: {
                auto aw = static_cast<const AwaitExpression*>(expr);
                // co_yield doesn't require `asinxron` — it just needs a coroutine-return-type
                // (e.g. std::generator<T>). The frontend doesn't track that, so we trust g++.
                if (!aw->isYield() && !currentFunctionIsAsync_) {
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
                Type idx = inferTypeT(sub->getIndex());
                // Allow butun (int) and matn (string) — matn is valid for map/JSON subscript
                if (idx.isAniq() && idx.name != "butun" && idx.name != "matn") {
                    reportWarning("Massiv indeksi 'butun' (int) yoki 'matn' (string) bo'lishi kerak, lekin '" + idx.name + "' berildi.", sub->getBracketToken());
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
                                Type arg = inferTypeT(call->getArguments()[i].get());
                                std::string expBase = stripRef(expectedParams[i]);
                                if (arg.isAniq() && expectedParams[i] != "ozgaruvchan" &&
                                    !typesEquivalent(arg.name, expectedParams[i]) && !typesEquivalent(arg.name, expBase)) {
                                    if (!((expBase == "haqiqiy" || expBase == "ikkilangan") && arg.name == "butun")) {
                                        if (!classIsSubtype(arg.name, expBase)) {
                                            reportWarning("Argument " + std::to_string(i+1) + " turi mos emas: '" + expectedParams[i] + "' kutilgan, lekin '" + arg.name + "' berildi.", getTokenForNode(call->getArguments()[i].get()));
                                        }
                                    }
                                }
                            }
                        }
                    }
                } else if (call->getCallee()->getType() == ASTNodeType::MemberAccess) {
                    auto mac = static_cast<const MemberAccess*>(call->getCallee());
                    Type obj = inferTypeT(mac->getObject());
                    if (obj.isAniq() && classes_.contains(obj.name)) {
                        std::string methodName = mac->getMemberName();
                        if (classes_[obj.name].methodParams.contains(methodName)) {
                            const auto& expectedParams = classes_[obj.name].methodParams[methodName];
                            if (expectedParams.size() != call->getArguments().size()) {
                                reportError("Metod '" + methodName + "' " + std::to_string(expectedParams.size()) + " ta argument kutadi, lekin " + std::to_string(call->getArguments().size()) + " ta berildi.", call->getCallToken());
                            } else {
                                for (size_t i = 0; i < expectedParams.size(); ++i) {
                                    Type arg = inferTypeT(call->getArguments()[i].get());
                                    if (arg.isAniq() && expectedParams[i] != "ozgaruvchan" &&
                                        !typesEquivalent(arg.name, expectedParams[i])) {
                                        if (!((expectedParams[i] == "haqiqiy" || expectedParams[i] == "ikkilangan") && arg.name == "butun")) {
                                            reportWarning("Argument " + std::to_string(i+1) + " turi mos emas: '" + expectedParams[i] + "' kutilgan, lekin '" + arg.name + "' berildi.", getTokenForNode(call->getArguments()[i].get()));
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

                // Kompilyatsiya vaqtida nolga bo'lish / nolga qoldiqni tekshirish
                std::string op = bin->getOperator();
                if ((op == "/" || op == "%" || op == "mod")) {
                    const Expression* right = bin->getRight();
                    if (right->getType() == ASTNodeType::LiteralExpression) {
                        auto lit = static_cast<const LiteralExpression*>(right);
                        if (lit->getLiteralType() == LiteralExpression::LiteralType::Integer && lit->getValue() == "0") {
                            reportError("Nolga bo'lish yoki nolga qoldiq aniqlab bo'lmaydi.", bin->getOperatorToken());
                        }
                    }
                }

                break;
            }
            case ASTNodeType::MemberAccess: {
                auto mac = static_cast<const MemberAccess*>(expr);
                checkExpr(mac->getObject());

                Type obj = inferTypeT(mac->getObject());
                if (obj.isAniq() && classes_.contains(obj.name)) {
                    if (!classHasMember(obj.name, mac->getMemberName())) {
                        reportError("Sinf '" + obj.name + "' da '" + mac->getMemberName() + "' nomli maydon yoki metod topilmadi.", mac->getAccessToken());
                    }
                }

                break;
            }
            default: break;
        }
    }
};

} // namespace uzpp