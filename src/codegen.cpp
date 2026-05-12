#include "codegen.h"

#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace uzpp {

CodeGen::CodeGen()
    : indentLevel_(0),
      namespaceDepth_(0),
      lineStart_(true) {}

std::string CodeGen::generate(const Program* program, const std::string& sourceName, bool testMode, bool benchMode) {
    if (program == nullptr) {
        throw std::runtime_error("Bo'sh AST dan kod generatsiya qilib bo'lmaydi.");
    }

    reset();
    testMode_ = testMode;
    benchMode_ = benchMode;
    writePreamble(sourceName);
    emitNodes(program->getChildren());

    if (testMode_) {
        emitNewline();
        emitRawToken("int main(int argc, char* argv[]) {");
        emitNewline();
        indentMore();
        emitRawToken("uzpp::Sinov::SinovlarToplami runner;");
        emitNewline();
        for (const auto& func : testFunctions_) {
            emitRawToken("runner.testQoshish(\"" + func + "\", " + func + ");");
            emitNewline();
        }
        emitRawToken("runner.ishgaTushirish();");
        emitNewline();
        emitRawToken("return 0;");
        emitNewline();
        indentLess();
        emitRawToken("}");
        emitNewline();
    }

    if (benchMode_) {
        emitNewline();
        emitRawToken("int main(int argc, char* argv[]) {");
        emitNewline();
        indentMore();
        emitRawToken("uzpp::Sinov::BenchToplami runner;");
        emitNewline();
        for (const auto& func : benchFunctions_) {
            emitRawToken("runner.benchQoshish(\"" + func + "\", " + func + ");");
            emitNewline();
        }
        emitRawToken("runner.ishgaTushirish();");
        emitNewline();
        emitRawToken("return 0;");
        emitNewline();
        indentLess();
        emitRawToken("}");
        emitNewline();
    }

    if (!testMode_ && !benchMode_ && hasUserMain_) {
        emitNewline();
        emitRawToken("int main(int argc, char* argv[]) {");
        emitNewline();
        indentMore();
        emitRawToken("uzpp::Xatoliklar::xavfsizlikni_yoqish();");
        emitNewline();
        emitRawToken("try {");
        emitNewline();
        indentMore();
        if (userMainHasArgs_) {
            emitRawToken("std::vector<std::string> _uzpp_args;");
            emitNewline();
            emitRawToken("for(int i=0; i<argc; ++i) _uzpp_args.push_back(argv[i]);");
            emitNewline();
            emitRawToken("_uzpp_user_main(_uzpp_args);");
        } else {
            emitRawToken("_uzpp_user_main();");
        }
        emitNewline();
        emitRawToken("return 0;");
        emitNewline();
        indentLess();
        emitRawToken("} catch (const std::exception& e) {");
        emitNewline();
        indentMore();
        emitRawToken("std::cerr << \"\\n[UZ++ XATOLIK]: Kutilmagan istisno (exception)!\\nSabab: \" << e.what() << \"\\n\";");
        emitNewline();
        emitRawToken("return 1;");
        emitNewline();
        indentLess();
        emitRawToken("} catch (...) {");
        emitNewline();
        indentMore();
        emitRawToken("std::cerr << \"\\n[UZ++ XATOLIK]: Noma'lum halokat yuz berdi!\\n\";");
        emitNewline();
        emitRawToken("return 1;");
        emitNewline();
        indentLess();
        emitRawToken("}");
        emitNewline();
        indentLess();
        emitRawToken("}");
        emitNewline();
    }

    if (!lineStart_) {
        emitNewline();
    }

    return output_.str();
}

void CodeGen::reset() {
    output_.str("");
    output_.clear();
    indentLevel_ = 0;
    namespaceDepth_ = 0;
    lastToken_.clear();
    lineStart_ = true;
    testMode_ = false;
    benchMode_ = false;
    testFunctions_.clear();
    benchFunctions_.clear();
    hasUserMain_ = false;
    userMainHasArgs_ = false;
}

void CodeGen::writePreamble(const std::string& sourceName) {
    output_ << "#include <iostream>\n";
    output_ << "#include <string>\n";
    output_ << "#include <vector>\n";
    output_ << "#include <expected>\n";
    output_ << "#include <memory>\n";
    output_ << "#include <format>\n";
    output_ << "#include <ranges>\n";
    output_ << "#include <future>\n";
    output_ << "#include <unordered_map>\n";
    output_ << "#include <utility>\n";
    // Stdlib headers are resolved via -I<stdlib_dir> passed to g++ from
    // main.cpp::collectIncludeDirs(), so emitting just the bare name lets the
    // generated .cpp live anywhere on disk (random CWD, %TEMP%, network share).
    // The previous "../stdlib/" prefix only worked when the .cpp ended up in
    // <repo>/build/ next to a sibling stdlib/ folder.
    output_ << "#include \"uzpp_runtime.hpp\"\n\n";
    output_ << "using namespace std;\n\n";
    output_ << "#line 1 \"" << escapeForLineDirective(sourceName) << "\"\n";
    lineStart_ = true;
    lastToken_.clear();
}

std::string CodeGen::escapeForLineDirective(const std::string& sourceName) const {
    std::string escaped;
    escaped.reserve(sourceName.size());

    for (const char value : sourceName) {
        if (value == '\\' || value == '"') {
            escaped.push_back('\\');
        }
        escaped.push_back(value);
    }

    return escaped;
}

void CodeGen::emitNodes(const std::vector<std::unique_ptr<ASTNode>>& nodes) {
    for (std::size_t index = 0; index < nodes.size(); ++index) {
        const ASTNode* nextNode = index + 1 < nodes.size() ? nodes[index + 1].get() : nullptr;
        emitNode(nodes[index].get(), nextNode);
    }
}

void CodeGen::emitNode(const ASTNode* node, const ASTNode* nextNode) {
    if (node == nullptr) {
        return;
    }

    switch (node->getType()) {
        case ASTNodeType::Token:
            emitTokenNode(static_cast<const TokenNode*>(node), nextNode);
            break;
        case ASTNodeType::Group:
            emitGroupNode(static_cast<const GroupNode*>(node));
            break;
        case ASTNodeType::Program:
            emitNodes(static_cast<const Program*>(node)->getChildren());
            break;
        case ASTNodeType::NamespaceDeclaration:
            visitNamespaceDeclaration(static_cast<const NamespaceDeclaration*>(node));
            break;
        case ASTNodeType::FunctionDeclaration:
            visitFunctionDeclaration(static_cast<const FunctionDeclaration*>(node));
            break;
        case ASTNodeType::ClassDeclaration:
            visitClassDeclaration(static_cast<const ClassDeclaration*>(node));
            break;
        case ASTNodeType::InterfaceDeclaration:
            visitInterfaceDeclaration(static_cast<const InterfaceDeclaration*>(node));
            break;
        case ASTNodeType::TypeAlias:
            visitTypeAlias(static_cast<const TypeAlias*>(node));
            break;
        case ASTNodeType::EnumDeclaration:
            visitEnumDeclaration(static_cast<const EnumDeclaration*>(node));
            break;
        default:
            // Could be a Statement or Expression
            if (dynamic_cast<const Statement*>(node) != nullptr) {
                visitStatement(static_cast<const Statement*>(node));
            } else {
                throw std::runtime_error("Noma'lum AST tuguni turi emitNode'da.");
            }
            break;
    }
}

void CodeGen::emitTokenNode(const TokenNode* node, const ASTNode* nextNode) {
    if (node->getToken().type == TokenType::FormatStringLiteral) {
        std::string raw = node->getToken().value; // e.g. f"Hello {name}"
        std::string cppFormatStr = "\"";
        std::vector<std::string> args;
        
        bool inBracket = false;
        std::string currentVar;
        
        for (size_t i = 2; i < raw.length() - 1; ++i) {
            char c = raw[i];
            if (c == '{') {
                inBracket = true;
                cppFormatStr += "{}";
            } else if (c == '}') {
                inBracket = false;
                if (!currentVar.empty()) {
                    args.push_back(currentVar);
                    currentVar.clear();
                }
            } else if (inBracket) {
                currentVar += c;
            } else {
                cppFormatStr += c;
            }
        }
        cppFormatStr += "\"";
        
        emitRawToken("std::format");
        emitRawToken("(");
        emitRawToken(cppFormatStr);
        for (const auto& arg : args) {
            emitRawToken(",");
            emitRawToken(arg);
        }
        emitRawToken(")");
        return;
    }

    const std::string translated = translateToken(node->getToken(), nextNode);
    if (translated.empty()) {
        return;
    }

    emitRawToken(translated);

    if (translated == ";" || translated == "public:" || translated == "private:" || translated == "protected:") {
        emitNewline();
    }
}

void CodeGen::emitGroupNode(const GroupNode* node) {
    const std::string opening = node->getOpeningToken().value;
    const std::string closing = node->getClosingToken().value;

    if (opening == "{") {
        emitRawToken("{");
        emitNewline();
        ++indentLevel_;
        emitNodes(node->getChildren());
        if (!lineStart_) {
            emitNewline();
        }
        --indentLevel_;
        writeIndentIfNeeded();
        output_ << closing;
        lineStart_ = false;
        lastToken_ = closing;
        return;
    }

    emitRawToken(opening);
    emitNodes(node->getChildren());
    emitRawToken(closing);
}

std::string CodeGen::translateToken(const Token& token, const ASTNode* nextNode) const {
    static const std::unordered_map<std::string, std::string> identifierTranslations = {
        {"agar", "if"},
        {"aqlli_korsatkich", "std::shared_ptr"},
        {"asosiy", "main"},
        {"belgi", "char"},
        {"bosh", "void"},
        {"butun", "int"},
        {"chiqarish", "std::cout"},
        {"filtr", "std::views::filter"},
        {"filter", "std::views::filter"},
        {"map", "std::views::transform"},
        {"xaritalash", "std::views::transform"},
        {"yigish", "std::ranges::to"},
        {"enum_class", "enum class"},
        {"fayl_oqish", "std::ifstream"},
        {"fayl_yozish", "std::ofstream"},
        {"hesh_jadval", "std::unordered_map"},
        {"hesh_xarita", "std::unordered_map"},
        {"himoyalangan", "protected"},
        {"ikkilangan", "double"},
        {"haqiqiy", "double"},
        {"irgitish", "throw"},
        {"kasr", "float"},
        {"kiritish", "std::cin"},
        {"ko'chirish", "std::move"},
        {"lug'at", "std::unordered_map"},
        {"mantiq", "bool"},
        {"mantiqiy", "bool"},
        {"matn", "std::string"},
        {"mavhum", "virtual"},
        {"meros", ": public"},
        {"nomlar_fazosi", "namespace"},
        {"o'n", "std::deque"},
        {"o'zgaruvchan", "auto"},
        {"o'zgarmas", "const"},
        {"ochiq", "public"},
        {"olish", "std::cin"},
        {"oqim", "uzpp::Oqim"},
        {"oqim_boshla", "std::async"},
        {"kelajak", "std::future"},
        {"ozgaruvchan", "auto"},
        {"ozgarmas", "const"},
        {"qator_oxiri", "std::endl"},
        {"qaytarish", "return"},
        {"qulf", "std::mutex"},
        {"qulflangan_guard", "std::lock_guard"},
        {"rost", "true"},
        {"sanab_olish", "enum"},
        {"sinf", "class"},
        {"straktura", "struct"},
        {"tartiblangan_xarita", "std::map"},
        {"to'plam", "std::vector"},
        {"toki", "while"},
        {"tuzilma", "struct"},
        {"uchun", "for"},
        {"urinish", "try"},
        {"ushlash", "catch"},
        {"ustidan_yozish", "override"},
        {"vektor", "std::vector"},
        {"yagona_korsatkich", "std::unique_ptr"},
        {"yoki", "else"},
        {"yolg'on", "false"},
        {"yozish", "std::cout"},
        {"yopiq", "private"},
        {"o'ziga_xos", "std::set"},
        {"to'xtatish", "break"},
        {"davom_etish", "continue"},
        // Yangi turlar va funksiyalar
        {"Natija", "uzpp::Natija"},
        {"Tanlov", "uzpp::Tanlov"},
        {"muvaffaqiyat", "uzpp::muvaffaqiyat"},
        {"bor", "uzpp::bor"},
        {"ixtiyoriy", "std::optional"},
        {"juftlik", "std::pair"},
        {"uchlik", "std::tuple"},
        {"massiv_turg'un", "std::array"},
        {"variant", "std::variant"},
        {"funksiya_tur", "std::function"},
        {"kuchsiz_korsatkich", "std::weak_ptr"},
        {"nozik_qulf", "std::shared_mutex"},
        {"vada", "std::promise"},
        {"to'plam_noyob", "std::unordered_set"},
        {"tartib_to'plam", "std::set"},
        {"yangi_yagona", "std::make_unique"},
        {"yangi_umumiy", "std::make_shared"},
        {"tartibla", "std::sort"},
        {"qidirish", "std::find"},
        {"saralash", "std::sort"},
        {"teskari", "std::reverse"},
        // Casting operators
        {"statik_otkazish",   "static_cast"},
        {"dinamik_otkazish",  "dynamic_cast"},
        {"sabit_otkazish",    "const_cast"},
        {"qayta_otkazish",    "reinterpret_cast"},
        // Memory management
        {"yangi",             "new"},
        {"o'chirish",         "delete"},
        {"ochirish",          "delete"},
    };

    if (token.type == TokenType::Identifier) {
        if (token.value == "ochiq" || token.value == "yopiq" || token.value == "himoyalangan") {
            const auto it = identifierTranslations.find(token.value);
            const std::string translated = it != identifierTranslations.end() ? it->second : token.value;

            const TokenNode* nextTokenNode = nextNode != nullptr && nextNode->getType() == ASTNodeType::Token
                                                 ? static_cast<const TokenNode*>(nextNode)
                                                 : nullptr;
            if (nextTokenNode == nullptr || nextTokenNode->getToken().value != ":") {
                return translated + ":";
            }
            return translated;
        }

        // Template funksiyalarni tarjima qilish uchun (masalan: yigish<vektor<butun>>)
        if (token.value.find('<') != std::string::npos) {
            return getCppType(token.value);
        }

        const auto it = identifierTranslations.find(token.value);
        if (it != identifierTranslations.end()) {
            return it->second;
        }
        // O'zbek maxsus harflari: o' va g' (apostrof) C++ identifikatorida ruxsat etilmagan.
        // Ularni xavfsiz '_' belgisiga almashtirish.
        if (token.value.find('\'') != std::string::npos) {
            std::string safe = token.value;
            for (char& c : safe) if (c == '\'') c = '_';
            return safe;
        }
    } else if (token.type == TokenType::Symbol) {
        if (token.value == "??") {
            return "^";
        }
    }

    return token.value;
}

bool CodeGen::needsSpaceBefore(const std::string& previous, const std::string& current) const {
    static const std::unordered_set<std::string> noSpaceBefore = {
        ")", "]", "}", ";", ",", ".", "::", "->", ":", "(", "["
    };
    static const std::unordered_set<std::string> noSpaceAfter = {
        "(", "[", "{", ".", "::", "->"
    };

    if (previous.empty()) {
        return false;
    }

    if (noSpaceBefore.contains(current)) {
        return false;
    }

    if (noSpaceAfter.contains(previous)) {
        return false;
    }

    return true;
}

void CodeGen::writeIndentIfNeeded() {
    if (!lineStart_) {
        return;
    }

    for (int index = 0; index < indentLevel_; ++index) {
        output_ << "    ";
    }
    lineStart_ = false;
}

void CodeGen::emitRawToken(const std::string& token) {
    if (token.empty()) {
        return;
    }

    writeIndentIfNeeded();
    if (needsSpaceBefore(lastToken_, token)) {
        output_ << ' ';
    }
    output_ << token;
    lastToken_ = token;
}

void CodeGen::emitNewline() {
    output_ << '\n';
    lineStart_ = true;
    lastToken_.clear();
}

// ===== SEMANTIC VISITOR METHODS (PHASE 11-C) =====

void CodeGen::indentMore() {
    ++indentLevel_;
}

void CodeGen::indentLess() {
    if (indentLevel_ > 0) {
        --indentLevel_;
    }
}

std::string CodeGen::getOperatorSymbol(const std::string& op) const {
    // Most operators pass through, but some Uzbek keywords need translation
    if (op == "yoki") return "||";
    if (op == "va") return "&&";
    return op;
}

std::string CodeGen::translateDefaultValue(const std::string& val) const {
    // Uz++ kalit so'zlarini C++ ga tarjima qilish
    if (val == "rost" || val == "to'g'ri") return "true";
    if (val == "yolg'on" || val == "yolgon" || val == "noto'g'ri") return "false";
    if (val == "bosh" || val == "nullptr") return "nullptr";
    // Boolean literals already translated above; numbers and strings pass through.
    // Translate keyword identifiers inside the value string using translateToken.
    Token t; t.type = TokenType::Identifier; t.value = val;
    std::string translated = translateToken(t, nullptr);
    return translated;
}

std::string CodeGen::getCppType(const std::string& uzppType, int depth) const {
    if (depth > 50) {
        return "void /* XATO: Shablon rekursiyasi chegaradan oshdi! */";
    }
    if (uzppType.starts_with("template ")) {
        return uzppType;
    }

    // o'zgarmas TYPE yoki ozgarmas TYPE -> const TYPE
    auto stripConstPrefix = [&](const std::string& prefix) -> std::string {
        if (uzppType.starts_with(prefix + " ")) {
            return "const " + getCppType(uzppType.substr(prefix.size() + 1), depth + 1);
        }
        return "";
    };
    for (const char* kw : {"o'zgarmas", "ozgarmas"}) {
        auto result = stripConstPrefix(kw);
        if (!result.empty()) return result;
    }

    // Strip trailing reference/pointer qualifiers, translate base, reattach
    {
        std::string suffix;
        std::string base = uzppType;
        // Handle &&, &, * suffixes
        while (!base.empty()) {
            if (base.size() >= 2 && base.substr(base.size() - 2) == "&&") {
                suffix = "&&" + suffix;
                base = base.substr(0, base.size() - 2);
            } else if (base.back() == '&' || base.back() == '*') {
                suffix = std::string(1, base.back()) + suffix;
                base.pop_back();
            } else {
                break;
            }
        }
        // Trim spaces between base and suffix
        while (!base.empty() && base.back() == ' ') base.pop_back();
        if (!suffix.empty() && base != uzppType) {
            return getCppType(base, depth + 1) + suffix;
        }
    }

    size_t templateStart = uzppType.find('<');
    if (templateStart != std::string::npos) {
        std::string baseType = uzppType.substr(0, templateStart);
        std::string result = getCppType(baseType, depth + 1) + "<";
        
        size_t templateEnd = uzppType.rfind('>');
        if (templateEnd != std::string::npos && templateEnd > templateStart) {
            std::string inner = uzppType.substr(templateStart + 1, templateEnd - templateStart - 1);
            
            auto trim = [](const std::string& s) -> std::string {
                size_t start = s.find_first_not_of(" \t\r\n");
                if (start == std::string::npos) return std::string("");
                size_t end = s.find_last_not_of(" \t\r\n");
                return s.substr(start, end - start + 1);
            };
            
            std::string currentArg;
            int innerDepth = 0;
            for (char c : inner) {
                if (c == '<') innerDepth++;
                else if (c == '>') innerDepth--;
                else if (c == ',' && innerDepth == 0) {
                    result += getCppType(trim(currentArg), depth + 1) + ", ";
                    currentArg.clear();
                    continue;
                }
                currentArg += c;
            }
            if (!currentArg.empty()) {
                result += getCppType(trim(currentArg), depth + 1);
            }
            
            result += ">";
            result += uzppType.substr(templateEnd + 1);
            return result;
        }
    }

    static const std::unordered_map<std::string, std::string> typeMap = {
        {"bosh", "void"},
        {"butun", "int"},
        {"ozgaruvchan", "auto"},
        {"o'zgaruvchan", "auto"},
        {"ozgarmas", "const auto"},
        {"o'zgarmas", "const auto"},
        {"kasr", "float"},
        {"ikkilangan", "double"},
        {"haqiqiy", "double"},
        {"belgi", "char"},
        {"mantiq", "bool"},
        {"mantiqiy", "bool"},
        {"matn", "std::string"},
        {"to'plam", "std::vector"},
        {"vektor", "std::vector"},
        {"o'n", "std::deque"},
        {"lug'at", "std::unordered_map"},
        {"hesh_jadval", "std::unordered_map"},
        {"hesh_xarita", "std::unordered_map"},
        {"o'ziga_xos", "std::set"},
        {"to'plam_noyob", "std::unordered_set"},
        {"yigish", "std::ranges::to"},
        {"to'plash", "std::ranges::to"},
        {"tartiblangan_xarita", "std::map"},
        {"tartib_to'plam", "std::set"},
        {"yagona", "std::unique_ptr"},
        {"yagona_korsatkich", "std::unique_ptr"},
        {"umumiy", "std::shared_ptr"},
        {"aqlli_korsatkich", "std::shared_ptr"},
        {"kuchsiz_korsatkich", "std::weak_ptr"},
        {"oqim", "uzpp::Oqim"},
        {"qulf", "std::mutex"},
        {"qulflangan_guard", "std::lock_guard"},
        {"nozik_qulf", "std::shared_mutex"},
        {"kelajak", "std::future"},
        {"vada", "std::promise"},
        {"oqim_boshla", "std::async"},
        {"fayl_oqish", "std::ifstream"},
        {"fayl_yozish", "std::ofstream"},
        {"filtr", "std::views::filter"},
        {"filter", "std::views::filter"},
        {"xaritalash", "std::views::transform"},
        {"ko'chirish", "std::move"},
        // Yangi turlar
        {"Natija", "uzpp::Natija"},
        {"Tanlov", "uzpp::Tanlov"},
        {"ixtiyoriy", "std::optional"},
        {"juftlik", "std::pair"},
        {"uchlik", "std::tuple"},
        {"massiv", "std::array"},
        {"variant", "std::variant"},
        {"funksiya", "std::function"},
        // Xotira boshqaruv funksiyalari (template argumentli chaqiruvlar uchun)
        {"yangi_yagona", "std::make_unique"},
        {"yangi_umumiy", "std::make_shared"},
        {"tartibla", "std::sort"},
        {"saralash", "std::sort"},
        {"qidirish", "std::find"},
        {"teskari", "std::reverse"},
    };
    
    const auto it = typeMap.find(uzppType);
    if (it != typeMap.end()) return it->second;
    // O'zbek harflari (o', g') turlar nomida bo'lishi mumkin — C++ uchun xavfsiz qilish
    return safeIdent(uzppType);
}

// Expression visitors
void CodeGen::visitExpression(const Expression* expr) {
    if (expr == nullptr) return;
    
    switch (expr->getType()) {
        case ASTNodeType::LiteralExpression:
            visitLiteralExpression(static_cast<const LiteralExpression*>(expr));
            break;
        case ASTNodeType::IdentifierExpression:
            visitIdentifierExpression(static_cast<const IdentifierExpression*>(expr));
            break;
        case ASTNodeType::BinaryExpression:
            visitBinaryExpression(static_cast<const BinaryExpression*>(expr));
            break;
        case ASTNodeType::UnaryExpression:
            visitUnaryExpression(static_cast<const UnaryExpression*>(expr));
            break;
        case ASTNodeType::FunctionCall:
            visitFunctionCall(static_cast<const FunctionCall*>(expr));
            break;
        case ASTNodeType::MemberAccess:
            visitMemberAccess(static_cast<const MemberAccess*>(expr));
            break;
        case ASTNodeType::SubscriptAccess:
            visitSubscriptAccess(static_cast<const SubscriptAccess*>(expr));
            break;
        case ASTNodeType::AssignmentExpression:
            visitAssignmentExpression(static_cast<const AssignmentExpression*>(expr));
            break;
        case ASTNodeType::AwaitExpression:
            visitAwaitExpression(static_cast<const AwaitExpression*>(expr));
            break;
        case ASTNodeType::PipelineExpression:
            visitPipelineExpression(static_cast<const PipelineExpression*>(expr));
            break;
        case ASTNodeType::LambdaExpression:
            visitLambdaExpression(static_cast<const LambdaExpression*>(expr));
            break;
        case ASTNodeType::TernaryExpression:
            visitTernaryExpression(static_cast<const TernaryExpression*>(expr));
            break;
        default:
            break;
    }
}

void CodeGen::visitBinaryExpression(const BinaryExpression* expr) {
    if (expr == nullptr || expr->getLeft() == nullptr || expr->getRight() == nullptr) return;
    
    if (expr->getOperator() == "??") {
        emitRawToken("[&]()");
        emitRawToken("{");
        emitRawToken("auto&& _val =");
        visitExpression(expr->getLeft());
        emitRawToken(";");
        emitRawToken("return _val.has_value() ? _val.value() :");
        visitExpression(expr->getRight());
        emitRawToken(";");
        emitRawToken("}()");
        return;
    }
    
    emitRawToken("(");
    visitExpression(expr->getLeft());
    emitRawToken(getOperatorSymbol(expr->getOperator()));
    visitExpression(expr->getRight());
    emitRawToken(")");
}

void CodeGen::visitUnaryExpression(const UnaryExpression* expr) {
    if (expr == nullptr) return;
    
    if (expr->isPrefix()) {
        switch (expr->getOperator()) {
            case UnaryExpression::UnaryOp::Plus:
                emitRawToken("+");
                break;
            case UnaryExpression::UnaryOp::Minus:
                emitRawToken("-");
                break;
            case UnaryExpression::UnaryOp::LogicalNot:
                emitRawToken("!");
                break;
            case UnaryExpression::UnaryOp::BitwiseNot:
                emitRawToken("~");
                break;
            case UnaryExpression::UnaryOp::PreIncrement:
                emitRawToken("++");
                break;
            case UnaryExpression::UnaryOp::PreDecrement:
                emitRawToken("--");
                break;
            case UnaryExpression::UnaryOp::AddressOf:
                emitRawToken("&");
                break;
            case UnaryExpression::UnaryOp::Dereference:
                emitRawToken("*");
                break;
            case UnaryExpression::UnaryOp::New:
                emitRawToken("new");
                break;
            case UnaryExpression::UnaryOp::Delete:
                emitRawToken("delete");
                break;
            default:
                break;
        }
    }

    visitExpression(expr->getExpression());

    if (!expr->isPrefix()) {
        switch (expr->getOperator()) {
            case UnaryExpression::UnaryOp::PostIncrement:
                emitRawToken("++");
                break;
            case UnaryExpression::UnaryOp::PostDecrement:
                emitRawToken("--");
                break;
            default:
                break;
        }
    }
}

void CodeGen::visitLiteralExpression(const LiteralExpression* expr) {
    if (expr == nullptr) return;
    
    if (expr->getLiteralType() == LiteralExpression::LiteralType::FormatString) {
        std::string raw = expr->getValue(); // e.g. "Hello {name}"
        std::string cppFormatStr = "\"";
        std::vector<std::string> args;
        
        bool inBracket = false;
        std::string currentVar;
        
        for (size_t i = 2; i < raw.length() - 1; ++i) {
            char c = raw[i];
            if (c == '{') {
                inBracket = true;
                cppFormatStr += "{}";
            } else if (c == '}') {
                inBracket = false;
                if (!currentVar.empty()) {
                    args.push_back(currentVar);
                    currentVar.clear();
                }
            } else if (inBracket) {
                currentVar += c;
            } else {
                cppFormatStr += c;
            }
        }
        cppFormatStr += "\"";
        
        emitRawToken("std::format");
        emitRawToken("(");
        emitRawToken(cppFormatStr);
        for (const auto& arg : args) {
            emitRawToken(",");
            emitRawToken(arg);
        }
        emitRawToken(")");
        return;
    }
    
    // Triple-quoted multiline string: """...""" → regular C++ string with \n
    if (expr->getLiteralType() == LiteralExpression::LiteralType::String) {
        const std::string& v = expr->getValue();
        if (v.size() >= 6 && v.substr(0, 3) == "\"\"\"") {
            // Strip leading/trailing """ and convert real newlines to \n
            std::string inner = v.substr(3, v.size() - 6);
            // Strip optional leading newline
            if (!inner.empty() && inner[0] == '\n') inner = inner.substr(1);
            std::string result = "\"";
            for (char c : inner) {
                if (c == '\n') result += "\\n";
                else if (c == '\r') { /* skip */ }
                else if (c == '"') result += "\\\"";
                else if (c == '\\') result += "\\\\";
                else result += c;
            }
            result += "\"";
            emitRawToken(result);
            return;
        }
    }

    // Mantiqiy qiymatlarni (rost/yolg'on) C++ ga tarjima qilish
    if (expr->getLiteralType() == LiteralExpression::LiteralType::Boolean) {
        const std::string& v = expr->getValue();
        if (v == "rost" || v == "to'g'ri" || v == "true") { emitRawToken("true"); return; }
        if (v == "yolg'on" || v == "yolgon" || v == "noto'g'ri" || v == "false") { emitRawToken("false"); return; }
    }
    emitRawToken(expr->getValue());
}

void CodeGen::visitIdentifierExpression(const IdentifierExpression* expr) {
    if (expr == nullptr) return;
    Token dummyToken;
    dummyToken.type = TokenType::Identifier;
    dummyToken.value = expr->getName();
    emitRawToken(translateToken(dummyToken, nullptr));
}

void CodeGen::visitFunctionCall(const FunctionCall* expr) {
    if (expr == nullptr || expr->getCallee() == nullptr) return;
    
    if (expr->getCallee()->getType() == ASTNodeType::IdentifierExpression) {
        const auto* idExpr = static_cast<const IdentifierExpression*>(expr->getCallee());
        if (idExpr->getName() == "__uzpp_array") {
            const auto& args = expr->getArguments();
            // If elements are string literals, emit explicit type to avoid const char* deduction
            bool hasStringLiterals = !args.empty() && args[0]->getType() == ASTNodeType::LiteralExpression &&
                static_cast<const LiteralExpression*>(args[0].get())->getLiteralType() == LiteralExpression::LiteralType::String;
            if (hasStringLiterals) {
                emitRawToken("std::vector<std::string>");
            } else {
                emitRawToken("std::vector");
            }
            emitRawToken("{");
            for (std::size_t i = 0; i < args.size(); ++i) {
                if (i > 0) emitRawToken(",");
                visitExpression(args[i].get());
            }
            emitRawToken("}");
            return;
        }
        if (idExpr->getName() == "__uzpp_brace") {
            emitRawToken("{");
            const auto& args = expr->getArguments();
            for (std::size_t i = 0; i < args.size(); ++i) {
                if (i > 0) emitRawToken(",");
                visitExpression(args[i].get());
            }
            emitRawToken("}");
            return;
        }
        if (idExpr->getName() == "__uzpp_dict") {
            emitRawToken("std::unordered_map");
            emitRawToken("{");
            const auto& args = expr->getArguments();
            for (std::size_t i = 0; i < args.size(); i += 2) {
                if (i > 0) emitRawToken(",");
                emitRawToken("std::pair");
                emitRawToken("{");
                visitExpression(args[i].get());
                emitRawToken(",");
                if (i + 1 < args.size()) {
                    visitExpression(args[i+1].get());
                }
                emitRawToken("}");
            }
            emitRawToken("}");
            return;
        }
        if (idExpr->getName() == "__uzpp_throw") {
            emitRawToken("throw");
            visitExpression(expr->getArguments()[0].get());
            return;
        }

        // Cast operators: callee name is like "statik_otkazish<butun>"
        // The template arg is embedded in the identifier name
        {
            static const std::unordered_map<std::string, std::string> castMap = {
                {"statik_otkazish",  "static_cast"},
                {"dinamik_otkazish", "dynamic_cast"},
                {"sabit_otkazish",   "const_cast"},
                {"qayta_otkazish",   "reinterpret_cast"},
            };
            const std::string& fullName = idExpr->getName();
            for (const auto& [uzName, cppName] : castMap) {
                if (fullName == uzName || fullName.substr(0, uzName.size() + 1) == uzName + "<") {
                    // Rebuild: cppName<type>(args)
                    std::string typeArg;
                    if (fullName.size() > uzName.size() + 1) {
                        // Extract type between < and >
                        typeArg = fullName.substr(uzName.size() + 1);
                        if (!typeArg.empty() && typeArg.back() == '>') typeArg.pop_back();
                        typeArg = getCppType(typeArg);
                    }
                    emitRawToken(cppName);
                    if (!typeArg.empty()) {
                        emitRawToken("<");
                        emitRawToken(typeArg);
                        emitRawToken(">");
                    }
                    emitRawToken("(");
                    const auto& args = expr->getArguments();
                    for (std::size_t i = 0; i < args.size(); ++i) {
                        if (i > 0) emitRawToken(",");
                        visitExpression(args[i].get());
                    }
                    emitRawToken(")");
                    return;
                }
            }
        }
    }

    visitExpression(expr->getCallee());
    emitRawToken("(");
    
    const auto& args = expr->getArguments();
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (i > 0) {
            emitRawToken(",");
        }
        visitExpression(args[i].get());
    }
    
    emitRawToken(")");
}

void CodeGen::visitMemberAccess(const MemberAccess* expr) {
    if (expr == nullptr || expr->getObject() == nullptr) return;
    
    visitExpression(expr->getObject());
    
    if (expr->getAccessType() == MemberAccess::AccessType::Dot) {
        emitRawToken(".");
    } else {
        emitRawToken("->");
    }
    
    emitRawToken(safeIdent(expr->getMemberName()));
}

void CodeGen::visitSubscriptAccess(const SubscriptAccess* expr) {
    if (expr == nullptr || expr->getArray() == nullptr || expr->getIndex() == nullptr) return;

    // Wrap complex array expressions in parens to avoid precedence issues
    // e.g. (*ptr)[idx] not *ptr[idx]
    bool needParens = false;
    auto arrType = expr->getArray()->getType();
    if (arrType == ASTNodeType::UnaryExpression ||
        arrType == ASTNodeType::BinaryExpression ||
        arrType == ASTNodeType::AssignmentExpression) {
        needParens = true;
    }
    if (needParens) emitRawToken("(");
    visitExpression(expr->getArray());
    if (needParens) emitRawToken(")");
    emitRawToken("[");
    visitExpression(expr->getIndex());
    emitRawToken("]");
}

void CodeGen::visitAssignmentExpression(const AssignmentExpression* expr) {
    if (expr == nullptr || expr->getTarget() == nullptr || expr->getValue() == nullptr) return;
    
    if (expr->getOperator() == "??" "=") {
        emitRawToken("[&]()");
        emitRawToken("->");
        emitRawToken("auto&");
        emitRawToken("{");
        emitRawToken("auto&& _ref =");
        visitExpression(expr->getTarget());
        emitRawToken(";");
        emitRawToken("if(!_ref.has_value())");
        emitRawToken("{");
        emitRawToken("_ref =");
        visitExpression(expr->getValue());
        emitRawToken(";");
        emitRawToken("}");
        emitRawToken("return _ref;");
        emitRawToken("}()");
        return;
    }

    visitExpression(expr->getTarget());
    emitRawToken(expr->getOperator());
    visitExpression(expr->getValue());
}

// Statement visitors
void CodeGen::visitStatement(const Statement* stmt) {
    if (stmt == nullptr) return;
    
    switch (stmt->getType()) {
        case ASTNodeType::IfStatement:
            visitIfStatement(static_cast<const IfStatement*>(stmt));
            break;
        case ASTNodeType::WhileStatement:
            visitWhileStatement(static_cast<const WhileStatement*>(stmt));
            break;
        case ASTNodeType::ForStatement:
            visitForStatement(static_cast<const ForStatement*>(stmt));
            break;
        case ASTNodeType::Block:
            visitBlock(static_cast<const Block*>(stmt));
            break;
        case ASTNodeType::StatementList:
            for (const auto& s : static_cast<const StatementList*>(stmt)->getStatements())
                visitStatement(s.get());
            break;
        case ASTNodeType::MatchStatement:
            visitMatchStatement(static_cast<const MatchStatement*>(stmt));
            break;
        case ASTNodeType::ReturnStatement:
            visitReturnStatement(static_cast<const ReturnStatement*>(stmt));
            break;
        case ASTNodeType::BreakStatement:
            visitBreakStatement(static_cast<const BreakStatement*>(stmt));
            break;
        case ASTNodeType::ContinueStatement:
            visitContinueStatement(static_cast<const ContinueStatement*>(stmt));
            break;
        case ASTNodeType::ExpressionStatement:
            visitExpressionStatement(static_cast<const ExpressionStatement*>(stmt));
            break;
        case ASTNodeType::VariableDeclaration:
            visitVariableDeclaration(static_cast<const VariableDeclaration*>(stmt));
            break;
        case ASTNodeType::IncludeStatement:
            visitIncludeStatement(static_cast<const IncludeStatement*>(stmt));
            break;
        case ASTNodeType::LinkStatement:
            visitLinkStatement(static_cast<const LinkStatement*>(stmt));
            break;
        case ASTNodeType::ExportModuleStatement:
            visitExportModuleStatement(static_cast<const ExportModuleStatement*>(stmt));
            break;
        default:
            break;
    }
}

void CodeGen::visitIfStatement(const IfStatement* stmt) {
    if (stmt == nullptr) return;
    
    bool isTry = false;
    bool isCatch = false;
    if (stmt->getCondition()->getType() == ASTNodeType::IdentifierExpression) {
        auto id = static_cast<const IdentifierExpression*>(stmt->getCondition())->getName();
        if (id == "__uzpp_try") isTry = true;
        else if (id.starts_with("__uzpp_catch ")) isCatch = true;
    }

    if (isTry) {
        writeIndentIfNeeded();
        emitRawToken("try");
        emitNewline();
        indentMore();
        visitStatement(stmt->getThenBranch());
        indentLess();
        if (stmt->getElseBranch() != nullptr) visitStatement(stmt->getElseBranch());
        return;
    }
    if (isCatch) {
        auto id = static_cast<const IdentifierExpression*>(stmt->getCondition())->getName();
        writeIndentIfNeeded();
        emitRawToken("catch");
        emitRawToken("("); emitRawToken(id.substr(13)); emitRawToken(")");
        emitNewline();
        indentMore();
        visitStatement(stmt->getThenBranch());
        indentLess();
        if (stmt->getElseBranch() != nullptr) visitStatement(stmt->getElseBranch());
        return;
    }

    writeIndentIfNeeded();
    emitRawToken("if");
    emitRawToken("(");
    visitExpression(stmt->getCondition());
    emitRawToken(")");
    emitNewline();
    
    indentMore();
    visitStatement(stmt->getThenBranch());
    indentLess();
    
    if (stmt->getElseBranch() != nullptr) {
        writeIndentIfNeeded();
        emitRawToken("else");
        emitNewline();
        indentMore();
        visitStatement(stmt->getElseBranch());
        indentLess();
    }
}

void CodeGen::visitWhileStatement(const WhileStatement* stmt) {
    if (stmt == nullptr) return;
    
    writeIndentIfNeeded();
    emitRawToken("while");
    emitRawToken("(");
    visitExpression(stmt->getCondition());
    emitRawToken(")");
    emitNewline();
    
    indentMore();
    visitStatement(stmt->getBody());
    indentLess();
}

void CodeGen::visitForStatement(const ForStatement* stmt) {
    if (stmt == nullptr) return;

    writeIndentIfNeeded();
    emitRawToken("for");
    emitRawToken("(");

    if (stmt->isRangeBased()) {
        // Range-based for: uchun (tur nom : to'plam)
        if (stmt->getInit() != nullptr) {
            if (stmt->getInit()->getType() == ASTNodeType::VariableDeclaration) {
                const auto* varDecl = static_cast<const VariableDeclaration*>(stmt->getInit());
                if (varDecl->isConst()) emitRawToken("const");
                emitRawToken(getCppType(varDecl->getTypeName()));
                emitRawToken(safeIdent(varDecl->getName()));
            } else {
                visitStatement(stmt->getInit());
            }
        }
        emitRawToken(":");
        if (stmt->getCondition() != nullptr) {
            visitExpression(stmt->getCondition());
        }
    } else {
        // Regular C-style for loop
        if (stmt->getInit() != nullptr) {
            if (stmt->getInit()->getType() == ASTNodeType::VariableDeclaration) {
                const auto* varDecl = static_cast<const VariableDeclaration*>(stmt->getInit());
                if (varDecl->isConst()) emitRawToken("const");
                emitRawToken(getCppType(varDecl->getTypeName()));
                emitRawToken(safeIdent(varDecl->getName()));
                if (varDecl->getInitializer() != nullptr) {
                    emitRawToken("=");
                    visitExpression(varDecl->getInitializer());
                }
            } else {
                visitStatement(stmt->getInit());
            }
        }
        emitRawToken(";");

        if (stmt->getCondition() != nullptr) {
            visitExpression(stmt->getCondition());
        }
        emitRawToken(";");

        if (stmt->getIncrement() != nullptr) {
            visitExpression(stmt->getIncrement());
        }
    }

    emitRawToken(")");
    emitNewline();

    indentMore();
    visitStatement(stmt->getBody());
    indentLess();
}

void CodeGen::visitBlock(const Block* stmt) {
    if (stmt == nullptr) return;
    
    writeIndentIfNeeded();
    emitRawToken("{");
    emitNewline();
    
    indentMore();
    for (const auto& s : stmt->getStatements()) {
        visitStatement(s.get());
    }
    indentLess();
    
    writeIndentIfNeeded();
    emitRawToken("}");
    emitNewline();
}

void CodeGen::visitMatchStatement(const MatchStatement* stmt) {
    if (stmt == nullptr) return;
    
    const auto& cases = stmt->getCases();
    if (cases.empty()) return;

    static unsigned long long matchCounter = 0;
    std::string matchVar = "_match_val_" + std::to_string(++matchCounter);
    
    writeIndentIfNeeded();
    emitRawToken("{"); 
    emitNewline();
    indentMore();
    
    writeIndentIfNeeded();
    emitRawToken("auto&&");
    emitRawToken(matchVar);
    emitRawToken("=");
    visitExpression(stmt->getCondition());
    emitRawToken(";");
    emitNewline();
    
    bool isFirst = true;
    for (const auto& matchCase : cases) {
        writeIndentIfNeeded();
        if (matchCase->pattern != nullptr) {
            if (!isFirst) {
                emitRawToken("else");
                emitRawToken("if");
            } else {
                emitRawToken("if");
            }
            emitRawToken("(");
            emitRawToken(matchVar);
            emitRawToken("==");
            visitExpression(matchCase->pattern.get());
            emitRawToken(")");
            emitNewline();
        } else {
            if (!isFirst) {
                emitRawToken("else");
                emitNewline();
            } else {
                emitRawToken("if");
                emitRawToken("(");
                emitRawToken("true");
                emitRawToken(")");
                emitNewline();
            }
        }
        
        indentMore();
        visitStatement(matchCase->body.get());
        indentLess();
        
        isFirst = false;
    }
    
    indentLess();
    writeIndentIfNeeded();
    emitRawToken("}");
    emitNewline();
}

void CodeGen::visitReturnStatement(const ReturnStatement* stmt) {
    if (stmt == nullptr) return;
    
    writeIndentIfNeeded();
    if (currentFunctionIsAsync_) {
        emitRawToken("co_return");
    } else {
        emitRawToken("return");
    }
    
    if (stmt->getValue() != nullptr) {
        visitExpression(stmt->getValue());
    }
    
    emitRawToken(";");
    emitNewline();
}

void CodeGen::visitBreakStatement(const BreakStatement* stmt) {
    if (stmt == nullptr) return;
    writeIndentIfNeeded();
    emitRawToken("break");
    emitRawToken(";");
    emitNewline();
}

void CodeGen::visitContinueStatement(const ContinueStatement* stmt) {
    if (stmt == nullptr) return;
    writeIndentIfNeeded();
    emitRawToken("continue");
    emitRawToken(";");
    emitNewline();
}

void CodeGen::visitExpressionStatement(const ExpressionStatement* stmt) {
    if (stmt == nullptr) return;
    writeIndentIfNeeded();
    visitExpression(stmt->getExpression());
    emitRawToken(";");
    emitNewline();
}

void CodeGen::visitVariableDeclaration(const VariableDeclaration* stmt) {
    if (stmt == nullptr) return;
    writeIndentIfNeeded();
    
    if (indentLevel_ == namespaceDepth_) {
        emitRawToken("inline");
    }
    
    if (stmt->isConst()) emitRawToken("const");
    
    emitRawToken(getCppType(stmt->getTypeName()));
    emitRawToken(safeIdent(stmt->getName()));
    
    if (stmt->getInitializer() != nullptr) {
        emitRawToken("=");
        visitExpression(stmt->getInitializer());
    }
    emitRawToken(";");
    emitNewline();
}

void CodeGen::visitFunctionDeclaration(const FunctionDeclaration* decl) {
    if (decl == nullptr) return;
    writeIndentIfNeeded();
    
    if (!testMode_ && decl->getName() != "asosiy" && decl->getName() != "main") {
        emitRawToken("inline");
    }
    
    std::string retType = getCppType(decl->getReturnType());
    if (decl->isAsync()) {
        if (retType == "void") {
            emitRawToken("uzpp::Asinxron::Vazifa<void>");
        } else {
            emitRawToken("uzpp::Asinxron::Vazifa<" + retType + ">");
        }
    } else {
        emitRawToken(retType);
    }
    
    // Translate function name (e.g., asosiy -> main)
    Token funcNameToken;
    funcNameToken.type = TokenType::Identifier;
    funcNameToken.value = decl->getName();
    funcNameToken.line = 0;
    funcNameToken.column = 0;
    std::string cppFuncName = translateToken(funcNameToken, nullptr);
    
    if ((testMode_ || benchMode_) && cppFuncName == "main") {
        cppFuncName = "_uzpp_original_main";
    } else if (!testMode_ && !benchMode_ && cppFuncName == "main") {
        cppFuncName = "_uzpp_user_main";
        hasUserMain_ = true;
        userMainHasArgs_ = !decl->getParameters().empty();
    }
    if (decl->isTest()) {
        testFunctions_.push_back(cppFuncName);
    }
    if (decl->isBench()) {
        benchFunctions_.push_back(cppFuncName);
    }
    
    emitRawToken(cppFuncName);
    emitRawToken("(");
    
    const auto& params = decl->getParameters();
    for (std::size_t i = 0; i < params.size(); ++i) {
        if (i > 0) emitRawToken(",");
        if (params[i].isConst) emitRawToken("const");
        emitRawToken(getCppType(params[i].type));
        emitRawToken(safeIdent(params[i].name));
        if (!params[i].defaultValue.empty()) {
            emitRawToken("=");
            emitRawToken(translateDefaultValue(params[i].defaultValue));
        }
    }

    emitRawToken(")");
    emitNewline();

    bool oldAsyncState = currentFunctionIsAsync_;
    currentFunctionIsAsync_ = decl->isAsync();
    
    visitBlock(decl->getBody());
    
    currentFunctionIsAsync_ = oldAsyncState;
}

void CodeGen::visitInterfaceDeclaration(const InterfaceDeclaration* decl) {
    if (decl == nullptr) return;
    
    writeIndentIfNeeded();
    emitRawToken("struct");
    emitRawToken(safeIdent(decl->getName()));
    emitRawToken("{");
    emitNewline();
    indentMore();
    
    for (const auto& method : decl->getMethods()) {
        writeIndentIfNeeded();
        emitRawToken("virtual");
        emitRawToken(getCppType(method->returnType));
        emitRawToken(safeIdent(method->name));
        emitRawToken("(");
        
        for (std::size_t i = 0; i < method->params.size(); ++i) {
            if (i > 0) emitRawToken(",");
            if (method->params[i].isConst) emitRawToken("const");
            emitRawToken(getCppType(method->params[i].type));
            emitRawToken(safeIdent(method->params[i].name));
        }
        
        emitRawToken(")");
        emitRawToken("=");
        emitRawToken("0");
        emitRawToken(";");
        emitNewline();
    }
    
    writeIndentIfNeeded();
    emitRawToken("virtual");
    emitRawToken("~" + safeIdent(decl->getName()));
    emitRawToken("(");
    emitRawToken(")");
    emitRawToken("=");
    emitRawToken("default");
    emitRawToken(";");
    emitNewline();
    
    indentLess();
    writeIndentIfNeeded();
    emitRawToken("}");
    emitRawToken(";");
    emitNewline();
}

void CodeGen::visitClassDeclaration(const ClassDeclaration* decl) {
    if (decl == nullptr) return;
    
    writeIndentIfNeeded();
    emitRawToken("class");
    emitRawToken(safeIdent(decl->getName()));
    
    bool hasBaseOrInterface = !decl->getBaseClass().empty() || !decl->getInterfaces().empty();
    if (hasBaseOrInterface) {
        emitRawToken(":");
        bool first = true;
        if (!decl->getBaseClass().empty()) {
            emitRawToken("public");
            emitRawToken(decl->getBaseClass());
            first = false;
        }
        for (const auto& iface : decl->getInterfaces()) {
            if (!first) emitRawToken(",");
            emitRawToken("public");
            emitRawToken(iface);
            first = false;
        }
    }
    
    emitRawToken("{");
    emitNewline();
    
    if (!decl->getMethods().empty() || !decl->getMembers().empty()) {
        indentMore();

        // Emit members and methods in declaration order, tracking access specifier changes
        // Items: fields first (as stored in AST), then methods — emit access label on change
        std::string lastAccess;
        auto emitAccessIfChanged = [&](const std::string& spec) {
            std::string access = spec.empty() ? "public" : spec;
            if (access != lastAccess) {
                indentLess();
                writeIndentIfNeeded();
                emitRawToken(access + ":");
                emitNewline();
                indentMore();
                lastAccess = access;
            }
        };

        // Emit class members (fields)
        for (const auto& member : decl->getMembers()) {
            emitAccessIfChanged(member.accessSpecifier);
            writeIndentIfNeeded();
            emitRawToken(getCppType(member.type));
            emitRawToken(safeIdent(member.name));
            emitRawToken(";");
            emitNewline();
        }

        // Emit methods
        for (const auto& method : decl->getMethods()) {
            emitAccessIfChanged(method->accessSpecifier);
            writeIndentIfNeeded();

            // Prefix modifiers
            if (method->isStatic) emitRawToken("static");
            if (method->isPureVirtual) emitRawToken("virtual");

            if (!method->returnType.empty()) {
                // Strip manual "static " prefix from returnType if we handle it via flag
                std::string rt = method->returnType;
                if (rt.substr(0, 7) == "static ") rt = rt.substr(7);
                if (rt.substr(0, 14) == "inline static ") rt = rt.substr(14);
                emitRawToken(getCppType(rt));
            }
            emitRawToken(safeIdent(method->name));
            emitRawToken("(");

            for (std::size_t i = 0; i < method->params.size(); ++i) {
                if (i > 0) emitRawToken(",");
                if (method->params[i].isConst) emitRawToken("const");
                emitRawToken(getCppType(method->params[i].type));
                emitRawToken(safeIdent(method->params[i].name));
                if (!method->params[i].defaultValue.empty()) {
                    emitRawToken("=");
                    emitRawToken(translateDefaultValue(method->params[i].defaultValue));
                }
            }

            emitRawToken(")");

            // Post-params modifiers
            if (method->isConstMethod) emitRawToken("const");
            if (method->isVirtual && !method->isPureVirtual) emitRawToken("override");

            // Emit constructor initializer list before body
            if (!method->initializerList.empty()) {
                output_ << safeIdent(method->initializerList);
            }

            if (method->isPureVirtual && !method->body) {
                // Pure virtual: no body, just = 0;
                emitRawToken("= 0;");
                emitNewline();
            } else {
                emitNewline();
                visitBlock(method->body.get());
            }
        }
        
        indentLess();
    }
    
    writeIndentIfNeeded();
    emitRawToken("}");
    emitRawToken(";");
    emitNewline();
}

void CodeGen::visitNamespaceDeclaration(const NamespaceDeclaration* decl) {
    if (decl == nullptr) return;
    // "nomlar_fazosi X;" (no children) -> "using namespace X;"
    if (decl->getChildren().empty()) {
        writeIndentIfNeeded();
        emitRawToken("using");
        emitRawToken("namespace");
        emitRawToken(decl->getName());
        emitRawToken(";");
        emitNewline();
        return;
    }

    writeIndentIfNeeded();
    emitRawToken("namespace");
    emitRawToken(decl->getName());
    emitRawToken("{");
    emitNewline();
    indentMore();
    namespaceDepth_++;

    for (const auto& child : decl->getChildren()) {
        emitNode(child.get(), nullptr);
    }

    indentLess();
    namespaceDepth_--;
    writeIndentIfNeeded();
    emitRawToken("}");
    emitNewline();
}

void CodeGen::visitIncludeStatement(const IncludeStatement* stmt) {
    if (stmt == nullptr) return;
    writeIndentIfNeeded();
    std::string mod = stmt->getModuleName();
    // Strip surrounding quotes if present (lexer may include them)
    if (mod.size() >= 2 && mod.front() == '"' && mod.back() == '"') {
        mod = mod.substr(1, mod.size() - 2);
    }
    // Skip uzpp_runtime.hpp — already emitted in preamble
    if (mod == "uzpp_runtime.hpp") {
        return;
    }
    if (mod.find(".hpp") != std::string::npos || mod.find(".h") != std::string::npos) {
        // Emit just the bare name. g++ resolves it via the -I<stdlib_dir>
        // and -I<input.parent_path> flags that main.cpp::collectIncludeDirs()
        // hands the compiler, so this works no matter where the generated
        // .cpp ends up. (Previously we hard-coded "../stdlib/", which broke
        // every time CWD wasn't the repo root.)
        output_ << "#include \"" << mod << "\"\n";
    } else if (!mod.empty() && mod.front() == '<') {
        output_ << "#include " << mod << "\n";
    } else {
        output_ << "import " << mod << ";\n";
    }
    lineStart_ = true;
}

void CodeGen::visitExportModuleStatement(const ExportModuleStatement* stmt) {
    if (stmt == nullptr) return;
    writeIndentIfNeeded();
    output_ << "export module " << stmt->getModuleName() << ";\n";
    lineStart_ = true;
}

void CodeGen::visitLinkStatement(const LinkStatement* stmt) {
    if (stmt == nullptr) return;
    writeIndentIfNeeded();
    output_ << "// UZPP_LINK: " << stmt->getLibName() << "\n";
    lineStart_ = true;
}

void CodeGen::visitAwaitExpression(const AwaitExpression* expr) {
    if (expr == nullptr) return;
    emitRawToken("co_await");
    visitExpression(expr->getExpression());
}

// ===== PHASE 26: NEW LANGUAGE FEATURES CODEGEN =====

// Pipeline: degeri |> funksiya  =>  funksiya(degeri)
void CodeGen::visitPipelineExpression(const PipelineExpression* expr) {
    if (expr == nullptr) return;
    // Elixir/F# style: left |> right  =>  right(left)
    // For chained member access calls: left |> obj.method  =>  obj.method(left)
    visitExpression(expr->getRight());
    emitRawToken("(");
    visitExpression(expr->getLeft());
    emitRawToken(")");
}

// Lambda/Closure: [captures](params) { body } yoki |params| => expr
void CodeGen::visitLambdaExpression(const LambdaExpression* expr) {
    if (expr == nullptr) return;

    // Capture list
    emitRawToken("[");
    bool first = true;
    for (const auto& cap : expr->getCaptures()) {
        if (!first) emitRawToken(",");
        first = false;
        // Special sentinels: "&" = [&], "=" = [=]
        if ((cap.name == "&" && cap.byRef) || cap.name == "=") {
            if (cap.byRef) emitRawToken("&");
            else emitRawToken("=");
        } else {
            if (cap.byRef) emitRawToken("&");
            emitRawToken(cap.name);
        }
    }
    emitRawToken("]");

    // Parameter list
    emitRawToken("(");
    first = true;
    for (const auto& p : expr->getParams()) {
        if (!first) emitRawToken(",");
        first = false;
        emitRawToken(p.type.empty() ? "auto" : getCppType(p.type));
        emitRawToken(safeIdent(p.name));
    }
    emitRawToken(")");

    // Body: if it's a ReturnStatement, emit as expression lambda { return x; }
    if (expr->getBody() != nullptr &&
        expr->getBody()->getType() == ASTNodeType::ReturnStatement) {
        emitRawToken("{");
        emitRawToken("return");
        const auto* ret = static_cast<const ReturnStatement*>(expr->getBody());
        if (ret->getValue() != nullptr) {
            visitExpression(ret->getValue());
        }
        emitRawToken(";");
        emitRawToken("}");
    } else if (expr->getBody() != nullptr) {
        visitStatement(expr->getBody());
    }
}

// Ternary: shart ? ha_qiymat : yoq_qiymat
void CodeGen::visitTernaryExpression(const TernaryExpression* expr) {
    if (expr == nullptr) return;
    emitRawToken("(");
    visitExpression(expr->getCondition());
    emitRawToken("?");
    visitExpression(expr->getThenExpr());
    emitRawToken(":");
    visitExpression(expr->getElseExpr());
    emitRawToken(")");
}

// Type alias: tur Raqam = butun;  =>  using Raqam = int;
void CodeGen::visitTypeAlias(const TypeAlias* decl) {
    if (decl == nullptr) return;
    writeIndentIfNeeded();
    emitRawToken("using");
    emitRawToken(decl->getAlias());
    emitRawToken("=");
    emitRawToken(getCppType(decl->getTarget()));
    emitRawToken(";");
    emitNewline();
}

// Enum: sanab_olish Rang { Qizil, Yashil = 2, Ko'k }
//   =>  enum class Rang { Qizil, Yashil = 2, Ko_k };
//   +   std::string Rang_nomi(Rang v) { ... }
void CodeGen::visitEnumDeclaration(const EnumDeclaration* decl) {
    if (decl == nullptr) return;
    const std::string safeName = safeIdent(decl->getName());

    writeIndentIfNeeded();
    emitRawToken("enum class");
    emitRawToken(safeName);
    emitRawToken("{");
    emitNewline();
    indentMore();

    const auto& vals = decl->getValues();
    for (std::size_t i = 0; i < vals.size(); ++i) {
        writeIndentIfNeeded();
        emitRawToken(safeIdent(vals[i].name));
        if (!vals[i].explicitValue.empty()) {
            emitRawToken("=");
            emitRawToken(vals[i].explicitValue);
        }
        if (i + 1 < vals.size()) emitRawToken(",");
        emitNewline();
    }

    indentLess();
    writeIndentIfNeeded();
    emitRawToken("};");
    emitNewline();
    emitNewline();

    // Helper: EnumNomi_nomi() -> std::string
    writeIndentIfNeeded();
    emitRawToken("inline std::string");
    emitRawToken(safeName + "_nomi");
    emitRawToken("(" + safeName + " _v) {");
    emitNewline();
    indentMore();
    writeIndentIfNeeded();
    emitRawToken("switch (_v) {");
    emitNewline();
    indentMore();
    for (const auto& v : vals) {
        writeIndentIfNeeded();
        emitRawToken("case " + safeName + "::" + safeIdent(v.name) + ":");
        emitRawToken("return \"" + v.name + "\";");
        emitNewline();
    }
    writeIndentIfNeeded();
    emitRawToken("default: return \"?\";");
    emitNewline();
    indentLess();
    writeIndentIfNeeded();
    emitRawToken("}");
    emitNewline();
    indentLess();
    writeIndentIfNeeded();
    emitRawToken("}");
    emitNewline();
}

} // namespace uzpp
