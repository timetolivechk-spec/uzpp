#include "parser.h"

#include <iostream>
#include <map>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace uzpp {

namespace {
    bool looksLikeDeclHelper(const std::vector<Token>& tokens, std::size_t current) {
        std::size_t i = current;
        if (i >= tokens.size() || tokens[i].type != TokenType::Identifier) return false;
        // Special case: decltype(...) / tur_baholash(...) acts as a type.
        // Skip past the balanced parentheses, then expect identifier.
        if (tokens[i].value == "decltype" || tokens[i].value == "tur_baholash") {
            i++;
            if (i >= tokens.size() || tokens[i].value != "(") return false;
            i++;
            int depth = 1;
            while (i < tokens.size() && depth > 0) {
                if (tokens[i].value == "(") depth++;
                else if (tokens[i].value == ")") depth--;
                i++;
            }
            // Optional &/*/&& after decltype(...)
            while (i < tokens.size() && tokens[i].type == TokenType::Symbol &&
                   (tokens[i].value == "&" || tokens[i].value == "*" || tokens[i].value == "&&")) {
                i++;
            }
            return (i < tokens.size() && tokens[i].type == TokenType::Identifier);
        }
        i++;
        while (i < tokens.size() && tokens[i].type == TokenType::Symbol && tokens[i].value == "::") {
            i++;
            if (i >= tokens.size() || tokens[i].type != TokenType::Identifier) return false;
            i++;
        }
        if (i < tokens.size() && tokens[i].type == TokenType::Symbol && tokens[i].value == "<") {
            i++;
            int depth = 1;
            while (i < tokens.size() && depth > 0) {
                if (tokens[i].value == "<") depth++;
                else if (tokens[i].value == ">") depth--;
                else if (tokens[i].value == ">>") depth -= 2;
                i++;
            }
        }
        while (i < tokens.size() && tokens[i].type == TokenType::Symbol &&
               (tokens[i].value == "&" || tokens[i].value == "*" || tokens[i].value == "&&")) {
            i++;
        }
        return (i < tokens.size() && tokens[i].type == TokenType::Identifier);
    }
} // namespace

Parser::Parser(const std::vector<Token>& tokens)
    : tokens_(tokens),
      current_(0) {}

std::unique_ptr<Program> Parser::parse() {
    std::vector<std::unique_ptr<ASTNode>> globals;
    std::size_t lastPos = current_;
    while (!isAtEnd()) {
        try {
            globals.push_back(parseGlobalDeclaration());
            lastPos = current_;
        } catch (const ParseError& e) {
            // Phase 12: Xatolikni yig'amiz va sinxronizatsiya qilamiz
            recordError(e.what(), peek());
            synchronize();
            // Agar sinxronizatsiya oldinga siljimagan bo'lsa, qo'lda
            // bitta token o'tkazamiz — cheksiz sikldan qochish uchun
            if (current_ <= lastPos && !isAtEnd()) {
                advance();
            }
            lastPos = current_;
            if (isAtEnd()) break;
        }
    }
    if (!errors_.empty()) {
        // Xatoliklar to'plandi — chaqiruvchi (main.cpp) ularni ko'rsatadi.
    }
    return std::make_unique<Program>(std::move(globals));
}

const Token& Parser::peek() const {
    return tokens_[current_];
}

const Token& Parser::previous() const {
    return tokens_[current_ - 1];
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::EndOfFile;
}

const Token& Parser::advance() {
    if (!isAtEnd()) {
        ++current_;
    }
    return previous();
}

bool Parser::isOpeningGroup(const Token& token) const {
    return token.type == TokenType::Symbol &&
           (token.value == "(" || token.value == "{" || token.value == "[");
}

bool Parser::isClosingGroup(const Token& token) const {
    return token.type == TokenType::Symbol &&
           (token.value == ")" || token.value == "}" || token.value == "]");
}

std::string Parser::matchingCloser(const Token& token) const {
    if (token.value == "(") {
        return ")";
    }
    if (token.value == "{") {
        return "}";
    }
    if (token.value == "[") {
        return "]";
    }
    throw ParseError("Noma'lum guruh boshlanishi " + formatLocation(token));
}

std::string Parser::formatLocation(const Token& token) const {
    std::ostringstream builder;
    builder << "qator: " << token.line << " ustun: " << token.column;
    return builder.str();
}

// ===== TYPE STRING PARSING =====

// Phase 2.5: Eskirgan tur sinonimi — `parseTypeString` ichida tekshiriladi.
// Bu funksiya `checkDeprecatedSynonym()` ga delegatsiya qiladi.

std::string Parser::parseTypeString() {
    // Phase 2.5: Eskirgan sinonim ishlatilgan bo'lsa, yordamchi xato.
    // Tur kontekstida: tur sinonimlari + kalit so'z sinonimlari (`mantiq`,
    // `ikkilangan` va boshq.).
    checkDeprecatedTypeSynonym();

    // C++11 decltype(expr) — type deduction. Uzbek alias: tur_baholash(expr).
    // Collect everything inside (...) as raw text, emit as `decltype(...)`.
    if (!isAtEnd() && peek().type == TokenType::Identifier &&
        (peek().value == "decltype" || peek().value == "tur_baholash")) {
        advance(); // consume keyword
        if (isAtEnd() || peek().value != "(") {
            throw ParseError("Kutilgan '(' decltype/tur_baholash dan keyin");
        }
        advance(); // '('
        std::string inner;
        int depth = 1;
        while (!isAtEnd() && depth > 0) {
            const std::string& v = peek().value;
            if (v == "(") depth++;
            else if (v == ")") { depth--; if (depth == 0) break; }
            inner += peek().value + " ";
            advance();
        }
        if (!isAtEnd()) advance(); // ')'
        std::string base = "decltype(" + inner + ")";
        // Allow trailing &/&&/*/... qualifiers on decltype(...) too
        while (!isAtEnd() && peek().type == TokenType::Symbol &&
               (peek().value == "&" || peek().value == "*" || peek().value == "&&" || peek().value == "...")) {
            base += advance().value;
        }
        return base;
    }

    // Base name (may include namespace like uzpp::Tanlov)
    std::string typeStr = advance().value;
    while (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "::") {
        typeStr += advance().value; // "::"
        if (!isAtEnd()) typeStr += advance().value; // next segment
    }

    // Template arguments: handle nested >> correctly
    if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "<") {
        typeStr += advance().value; // consume "<"
        int depth = 1;
        while (!isAtEnd() && depth > 0) {
            const std::string& val = peek().value;
            if (val == "<") {
                depth++;
                typeStr += advance().value;
            } else if (val == ">") {
                depth--;
                typeStr += advance().value;
            } else if (val == ">>") {
                // Split ">>" into two ">" — handles vektor<vektor<T>>
                advance(); // consume the ">>" token
                depth -= 2;
                if (depth >= 0) {
                    typeStr += ">>";
                } else {
                    // depth was 1, so only one > belongs here
                    typeStr += ">";
                    depth = 0;
                }
            } else {
                typeStr += advance().value;
            }
        }
    }

    // Check for function call-like syntax: decltype(...) or tur_baholash(...)
    // Only handle if we just parsed those identifiers
    if ((typeStr == "decltype" || typeStr == "tur_baholash") &&
        !isAtEnd() && peek().type == TokenType::Symbol && peek().value == "(") {
        typeStr += advance().value; // consume "("
        int depth = 1;
        while (!isAtEnd() && depth > 0) {
            const std::string& val = peek().value;
            if (val == "(") {
                depth++;
                typeStr += advance().value;
            } else if (val == ")") {
                depth--;
                typeStr += advance().value;
            } else {
                typeStr += advance().value;
            }
        }
    }

    // Trailing reference/pointer qualifiers, plus variadic ellipsis (Args...)
    while (!isAtEnd() && peek().type == TokenType::Symbol &&
           (peek().value == "&" || peek().value == "*" || peek().value == "&&" || peek().value == "...")) {
        typeStr += advance().value;
    }

    // O'zgarmas ko'rsatkich: `butun* o'zgarmas p` → `int* const p`.
    // (Ko'rsatkichning O'ZI o'zgarmas; `o'zgarmas butun* p` esa —
    //  ko'rsatilayotgan QIYMAT o'zgarmas.) Faqat `*` dan keyin qabul
    //  qilamiz — aks holda `butun x; o'zgarmas butun y;` ketma-ketligi
    //  bilan chalkashib ketardi.
    while (!typeStr.empty() && typeStr.back() == '*' &&
           !isAtEnd() && peek().type == TokenType::Identifier && peek().value == "o'zgarmas") {
        advance();
        typeStr += " o'zgarmas";
        while (!isAtEnd() && peek().type == TokenType::Symbol &&
               (peek().value == "*" || peek().value == "&" || peek().value == "&&")) {
            typeStr += advance().value;
        }
    }

    return typeStr;
}

// ===== LEGACY TOKEN-BASED PARSING (MAINTAINED FOR COMPATIBILITY) =====

std::vector<std::unique_ptr<ASTNode>> Parser::parseNodesUntil(const std::string& closingSymbol) {
    std::vector<std::unique_ptr<ASTNode>> children;

    while (!isAtEnd()) {
        if (!closingSymbol.empty() && peek().type == TokenType::Symbol && peek().value == closingSymbol) {
            break;
        }

        if (isClosingGroup(peek())) {
            throw ParseError("Mos kelmaydigan yopuvchi belgi '" + peek().value + "' " + formatLocation(peek()));
        }

        children.push_back(parseNode());
    }

    return children;
}

std::unique_ptr<ASTNode> Parser::parseNode() {
    if (isOpeningGroup(peek())) {
        return parseGroup();
    }

    return std::make_unique<TokenNode>(advance());
}

std::unique_ptr<GroupNode> Parser::parseGroup() {
    const Token openingToken = advance();
    const std::string closingSymbol = matchingCloser(openingToken);
    auto children = parseNodesUntil(closingSymbol);

    if (isAtEnd()) {
        throw ParseError("Yopuvchi '" + closingSymbol + "' topilmadi " + formatLocation(openingToken));
    }

    const Token closingToken = advance();
    return std::make_unique<GroupNode>(openingToken, closingToken, std::move(children));
}

// ===== HELPER METHODS FOR SEMANTIC PARSING =====

bool Parser::isUzbekKeyword(const std::string& text) const {
    static const std::vector<std::string> uzbekKeywords{
        // Asosiy boshqarish oqimi — har C++ tushunchasi uchun bitta uz++ so'z
        "agar", "aks_holda", "uchun", "toki", "bajar", "qaytarish",
        "to'xtatish", "davom_etish",
        // O'zgaruvchilar (faqat kanonik apostrofli shakllar)
        "o'zgaruvchan", "o'zgarmas",
        // Sinf va funksiyalar
        "sinf", "tuzilma", "funksiya", "mavhum", "meros", "amalga_oshirish",
        "shartnoma", "shablon", "statik",
        // Modullar
        "ulash", "eksport", "import", "modul", "nomlar_fazosi",
        // Istisno boshqaruvi
        "urinish", "ushlash", "irgitish",
        // Asinxron
        "asinxron", "kutish",
        // Turlar — `bekor` ilgari `bosh` (void) sinonimi sifatida edi,
        // Phase 2.5 da olib tashlandi (bitta C++ tushunchasi = bitta uz++ so'z).
        "bosh", "yangi",
        // Mantiq — har bir C++ tushunchasi uchun bitta uz++ so'z:
        // `rost` = true (yagona), `yolg'on` = false (yagona).
        // `to'g'ri`/`noto'g'ri`/`yolgon` ilgari sinonim sifatida ishlatilardi —
        // endi oddiy identifikator (foydalanuvchi o'zgaruvchi nomi sifatida olishi mumkin).
        "va", "yoki", "rost", "yolg'on",
        // Moslash
        "moslash", "holat", "boshqa",
        // Boshqalar — strukturali kalit so'zlar.
        // O'chirildi (Phase 2.5): `o'n`/`yagona`/`umumiy` typeMap aliaslari
        // (deque/unique_ptr/shared_ptr) — gotcha #13 ga ko'ra ularni lokal
        // o'zgaruvchi nomi sifatida ishlatish mumkin bo'lishi shart, codegen
        // localScopes_ orqali alias tarjimasini boshqaradi.
        // Ham o'chirildi: `xotira`/`fayl`/`kutilish`/`hammasi`/`yoxud`/
        // `nomlari`/`vazifi`/`yayin` — kompilyatorda hech qaerda ishlatilmagan
        // o'lik reservatsiyalar. Foydalanuvchi kodida oddiy nom sifatida ishlatilishi mumkin.
        "sanab_olish", "tushuncha", "shart", "makro", "ulash_kutubxona",
        // `sikldan` o'chirildi (Phase 2.5) — kompilyatorda ishlatilmagan o'lik reservatsiya.
        "asosiy", "ustidan_yozish", "null",
        // Casting
        "statik_otkazish", "dinamik_otkazish", "o'zgarmas_otkazish", "qayta_otkazish",
        // Type deduction (C++11+)
        "decltype", "tur_baholash",
        // Const method modifier
        "o'zgarmas",
        // Compile-time modifiers (Stage 1): constexpr / consteval / constinit
        "sobit_ifoda", "sobit_baholash", "sobit_boshlangich",
        // Union (Stage 6)
        "birlashma",
        // Task 2 and 3
        "statik_tasdiqlash", "xato_tashlamaydi",
        // Storage modifiers (mutable, thread_local)
        "ozgaruvchi_o'zgartirish", "mutable",
        "oqim_mahalliy", "thread_local",
        // friend declarations
        "dust", "friend",
        // extern "C" linkage
        "tashqi", "extern",
        // co_yield generators
        "chiqar_qadam",
        // decltype(expr)
        "tur_baholash", "decltype",
        // inline variables (C++17)
        "qator_ichi",
        // lambda this-capture
        "bu",
        // initializer_list type alias
        "boshlovchi_royxat"
    };
    
    for (const auto& kw : uzbekKeywords) {
        if (text == kw) {
            return true;
        }
    }
    return false;
}

bool Parser::checkKeyword(const std::string& keyword) const {
    if (isAtEnd()) return false;
    const Token& current = peek();
    return current.type == TokenType::Identifier && current.value == keyword;
}

bool Parser::matchKeyword(const std::string& keyword) {
    if (checkKeyword(keyword)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::isAssignmentOperator(const std::string& text) const {
    return text == "=" || text == "+=" || text == "-=" || text == "*=" || 
           text == "/=" || text == "%=" || text == "&=" || text == "|=" || text == "^=" ||
           text == "<<=" || text == ">>=" || text == "?" "?=";
}

bool Parser::isNullCoalescingOperator(const std::string& text) const {
    return text == "??";
}

bool Parser::isLogicalOrOperator(const std::string& text) const {
    return text == "||" || text == "yoki";
}

bool Parser::isLogicalAndOperator(const std::string& text) const {
    return text == "&&" || text == "va";
}

bool Parser::isEqualityOperator(const std::string& text) const {
    return text == "==" || text == "!=";
}

bool Parser::isRelationalOperator(const std::string& text) const {
    return text == "<" || text == ">" || text == "<=" || text == ">=" || text == "<=>";
}

// C++ da siljitish (`<<`, `>>`) qo'shishdan PAST, taqqoslashdan YUQORI
// darajada turadi. Ilgari ular `isAdditiveOperator` ichida edi — natijada
// `yozish << a + b` noto'g'ri `(yozish << a) + b` bo'lib tahlil qilinardi.
bool Parser::isShiftOperator(const std::string& text) const {
    return text == "<<" || text == ">>";
}

bool Parser::isAdditiveOperator(const std::string& text) const {
    return text == "+" || text == "-";
}

bool Parser::isMultiplicativeOperator(const std::string& text) const {
    return text == "*" || text == "/" || text == "%";
}

bool Parser::isUnaryOperator(const std::string& text) const {
    return text == "!" || text == "~" || text == "-" || text == "+" || 
           text == "++" || text == "--" || text == "&" || text == "*";
}

bool Parser::isPostfixOperator(const std::string& text) const {
    return text == "++" || text == "--" || text == "(" || text == "[" || text == ".";
}

// ===== SEMANTIC EXPRESSION PARSING =====

std::unique_ptr<Expression> Parser::parseExpression() {
    return parseAssignmentExpression();
}

std::unique_ptr<Expression> Parser::parseAssignmentExpression() {
    auto left = parseNullCoalescingExpression();
    
    if (!isAtEnd() && peek().type == TokenType::Symbol && isAssignmentOperator(peek().value)) {
        const Token opToken = advance();
        auto right = parseAssignmentExpression();
        return std::make_unique<AssignmentExpression>(std::move(left), opToken.value, std::move(right), opToken);
    }
    
    // Pipeline operator: qiymat |> funksiya
    while (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "|>") {
        const Token pipeToken = advance();
        auto right = parseNullCoalescingExpression();
        left = std::make_unique<PipelineExpression>(std::move(left), std::move(right), pipeToken);
    }
    
    // Ternary operator: shart ? ha : yoq
    if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "?") {
        const Token ternaryToken = advance(); // consume '?'
        auto thenExpr = parseNullCoalescingExpression();
        if (isAtEnd() || peek().value != ":") {
            throw ParseError("Kutilgan ':' ternary operatorida " + formatLocation(peek()));
        }
        advance(); // consume ':'
        auto elseExpr = parseNullCoalescingExpression();
        left = std::make_unique<TernaryExpression>(std::move(left), std::move(thenExpr), std::move(elseExpr), ternaryToken);
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parseNullCoalescingExpression() {
    auto left = parseLogicalOrExpression();
    
    while (!isAtEnd() && peek().type == TokenType::Symbol && isNullCoalescingOperator(peek().value)) {
        const Token opToken = advance();
        auto right = parseLogicalOrExpression();
        left = std::make_unique<BinaryExpression>(std::move(left), opToken.value, std::move(right), opToken);
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parseLogicalOrExpression() {
    auto left = parseLogicalAndExpression();
    
    while (!isAtEnd() && isLogicalOrOperator(peek().value)) {
        const Token opToken = advance();
        auto right = parseLogicalAndExpression();
        left = std::make_unique<BinaryExpression>(std::move(left), opToken.value, std::move(right), opToken);
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parseLogicalAndExpression() {
    auto left = parseBitwiseOrExpression();

    while (!isAtEnd() && isLogicalAndOperator(peek().value)) {
        const Token opToken = advance();
        auto right = parseBitwiseOrExpression();
        left = std::make_unique<BinaryExpression>(std::move(left), opToken.value, std::move(right), opToken);
    }

    return left;
}

// C-precedence bitwise-OR / XOR / AND, slotted between logical-AND and equality.
// `&` is the trickiest — it doubles as address-of (unary), so we only consume
// it here when the LHS is already complete (postfix returned an expression).
std::unique_ptr<Expression> Parser::parseBitwiseOrExpression() {
    auto left = parseBitwiseXorExpression();

    while (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "|") {
        const Token opToken = advance();
        auto right = parseBitwiseXorExpression();
        left = std::make_unique<BinaryExpression>(std::move(left), opToken.value, std::move(right), opToken);
    }

    return left;
}

std::unique_ptr<Expression> Parser::parseBitwiseXorExpression() {
    auto left = parseBitwiseAndExpression();

    while (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "^") {
        const Token opToken = advance();
        auto right = parseBitwiseAndExpression();
        left = std::make_unique<BinaryExpression>(std::move(left), opToken.value, std::move(right), opToken);
    }

    return left;
}

std::unique_ptr<Expression> Parser::parseBitwiseAndExpression() {
    auto left = parseEqualityExpression();

    while (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "&") {
        const Token opToken = advance();
        auto right = parseEqualityExpression();
        left = std::make_unique<BinaryExpression>(std::move(left), opToken.value, std::move(right), opToken);
    }

    return left;
}

std::unique_ptr<Expression> Parser::parseEqualityExpression() {
    auto left = parseRelationalExpression();
    
    while (!isAtEnd() && peek().type == TokenType::Symbol && isEqualityOperator(peek().value)) {
        const Token opToken = advance();
        auto right = parseRelationalExpression();
        left = std::make_unique<BinaryExpression>(std::move(left), opToken.value, std::move(right), opToken);
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parseRelationalExpression() {
    auto left = parseShiftExpression();
    
    while (!isAtEnd() && peek().type == TokenType::Symbol && isRelationalOperator(peek().value)) {
        const Token opToken = advance();
        auto right = parseShiftExpression();
        left = std::make_unique<BinaryExpression>(std::move(left), opToken.value, std::move(right), opToken);
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parseShiftExpression() {
    auto left = parseAdditiveExpression();

    while (!isAtEnd() && peek().type == TokenType::Symbol && isShiftOperator(peek().value)) {
        const Token opToken = advance();
        auto right = parseAdditiveExpression();
        left = std::make_unique<BinaryExpression>(std::move(left), opToken.value, std::move(right), opToken);
    }

    return left;
}

std::unique_ptr<Expression> Parser::parseAdditiveExpression() {
    auto left = parseMultiplicativeExpression();
    
    while (!isAtEnd() && peek().type == TokenType::Symbol && isAdditiveOperator(peek().value)) {
        const Token opToken = advance();
        auto right = parseMultiplicativeExpression();
        left = std::make_unique<BinaryExpression>(std::move(left), opToken.value, std::move(right), opToken);
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parseMultiplicativeExpression() {
    auto left = parseUnaryExpression();
    
    while (!isAtEnd() && peek().type == TokenType::Symbol && isMultiplicativeOperator(peek().value)) {
        const Token opToken = advance();
        auto right = parseUnaryExpression();
        left = std::make_unique<BinaryExpression>(std::move(left), opToken.value, std::move(right), opToken);
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parseUnaryExpression() {
    if (!isAtEnd() && peek().type == TokenType::Identifier && peek().value == "kutish") {
        const Token opToken = advance(); // consume 'kutish'
        auto expr = parseUnaryExpression();
        return std::make_unique<AwaitExpression>(std::move(expr), opToken);
    }

    // C++20 co_yield generators: `chiqar_qadam expr` → `co_yield expr`
    if (!isAtEnd() && peek().type == TokenType::Identifier && peek().value == "chiqar_qadam") {
        const Token opToken = advance(); // consume 'chiqar_qadam'
        auto expr = parseUnaryExpression();
        return std::make_unique<AwaitExpression>(std::move(expr), opToken, /*isYield=*/true);
    }

    // yangi Tur(args)  →  new Type(args). `yangi` doubles as a valid Uzbek
    // identifier ("new" in the everyday sense), so only consume it as the
    // `new` operator when the following token can start a type expression —
    // an identifier (possibly qualified) or `(` for placement-new. Otherwise
    // fall through and let parsePrimary treat `yangi` as a regular name.
    if (!isAtEnd() && peek().type == TokenType::Identifier && peek().value == "yangi" &&
        current_ + 1 < tokens_.size()) {
        const Token& nxt = tokens_[current_ + 1];
        const bool looksLikeNew =
            nxt.type == TokenType::Identifier ||
            (nxt.type == TokenType::Symbol && nxt.value == "(");
        if (looksLikeNew) {
            const Token opToken = advance(); // consume 'yangi'

            // `yangi Tur{a, b}` — qavsli initsializatsiya. Turdan keyin `{`
            // kelsa, uni shu yerda o'zimiz o'qiymiz: `parsePostfixExpression`
            // umumiy holda `Identifier {` ni tutolmaydi (bu `agar (x) {` bilan
            // chalkashadi), shuning uchun faqat `yangi` kontekstida.
            if (peek().type == TokenType::Identifier) {
                const std::size_t save = current_;
                const Token typeToken = peek();
                std::string typeName = parseTypeString();
                if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "{") {
                    const Token braceToken = advance(); // '{'
                    std::vector<std::unique_ptr<Expression>> args;
                    while (!isAtEnd() && peek().value != "}") {
                        args.push_back(parseExpression());
                        if (!isAtEnd() && peek().value == ",") {
                            advance();
                            continue;
                        }
                        break;
                    }
                    if (isAtEnd() || peek().value != "}") {
                        throw ParseError("Kutilgan '}' `yangi` qavsli initsializatsiyasida " + formatLocation(peek()));
                    }
                    advance(); // '}'

                    auto callee = std::make_unique<IdentifierExpression>(typeName, typeToken);
                    auto call = std::make_unique<FunctionCall>(std::move(callee), std::move(args), braceToken);
                    call->setBraceInit(true);
                    return std::make_unique<UnaryExpression>(UnaryExpression::UnaryOp::New, std::move(call), opToken, true);
                }
                current_ = save; // `{` yo'q — odatdagi yo'lga qaytamiz
            }

            auto expr = parseUnaryExpression();
            return std::make_unique<UnaryExpression>(UnaryExpression::UnaryOp::New, std::move(expr), opToken, true);
        }
    }

    // o'chirish ptr  →  delete ptr. Same context-sensitivity as above —
    // accept only when followed by something that could be an expression
    // (identifier, `*`, `(`).
    if (!isAtEnd() && peek().type == TokenType::Identifier &&
        peek().value == "o'chirish" &&
        current_ + 1 < tokens_.size()) {
        const Token& nxt = tokens_[current_ + 1];
        const bool looksLikeDelete =
            nxt.type == TokenType::Identifier ||
            (nxt.type == TokenType::Symbol && (nxt.value == "*" || nxt.value == "(" || nxt.value == "["));
        if (looksLikeDelete) {
            const Token opToken = advance(); // consume 'o'chirish'
            auto expr = parseUnaryExpression();
            return std::make_unique<UnaryExpression>(UnaryExpression::UnaryOp::Delete, std::move(expr), opToken, true);
        }
    }
    
    if (!isAtEnd() && peek().type == TokenType::Symbol && isUnaryOperator(peek().value)) {
        const Token opToken = advance();
        auto expr = parseUnaryExpression();
        
        UnaryExpression::UnaryOp op;
        if (opToken.value == "!") op = UnaryExpression::UnaryOp::LogicalNot;
        else if (opToken.value == "~") op = UnaryExpression::UnaryOp::BitwiseNot;
        else if (opToken.value == "-") op = UnaryExpression::UnaryOp::Minus;
        else if (opToken.value == "+") op = UnaryExpression::UnaryOp::Plus;
        else if (opToken.value == "++") op = UnaryExpression::UnaryOp::PreIncrement;
        else if (opToken.value == "--") op = UnaryExpression::UnaryOp::PreDecrement;
        else if (opToken.value == "&") op = UnaryExpression::UnaryOp::AddressOf;
        else if (opToken.value == "*") op = UnaryExpression::UnaryOp::Dereference;
        else op = UnaryExpression::UnaryOp::Plus;
        
        return std::make_unique<UnaryExpression>(op, std::move(expr), opToken, true);
    }
    
    return parsePostfixExpression();
}

std::unique_ptr<Expression> Parser::parsePostfixExpression() {
    auto expr = parsePrimaryExpression();
    
    while (!isAtEnd()) {
        if (peek().type == TokenType::Symbol && peek().value == "(") {
            // Function call
            advance(); // consume '('
            std::vector<std::unique_ptr<Expression>> args;
            
            while (!isAtEnd() && !(peek().type == TokenType::Symbol && peek().value == ")")) {
                args.push_back(parseExpression());
                
                if (peek().type == TokenType::Symbol && peek().value == ",") {
                    advance();
                } else if (!(peek().type == TokenType::Symbol && peek().value == ")")) {
                    throw ParseError("Kutilgan ',' yoki ')' " + formatLocation(peek()));
                }
            }
            
            if (isAtEnd() || peek().value != ")") {
                throw ParseError("Kutilgan ')' " + formatLocation(peek()));
            }
            
            const Token callToken = advance();
            expr = std::make_unique<FunctionCall>(std::move(expr), std::move(args), callToken);
        }
        else if (peek().type == TokenType::Symbol && peek().value == "[") {
            // Subscript — supports C++23 multidim: arr[i, j, k]
            const Token bracketToken = advance();
            auto index = parseExpression();
            auto sub = std::make_unique<SubscriptAccess>(std::move(expr), std::move(index), bracketToken);

            while (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ",") {
                advance(); // consume ','
                sub->addExtraIndex(parseExpression());
            }

            if (isAtEnd() || peek().value != "]") {
                throw ParseError("Kutilgan ']' " + formatLocation(peek()));
            }

            advance(); // consume ']'
            expr = std::move(sub);
        }
        else if (peek().type == TokenType::Symbol && (peek().value == "." || peek().value == "->")) {
            // Member access
            const Token accessToken = advance();
            MemberAccess::AccessType accessType = (accessToken.value == ".") ? 
                MemberAccess::AccessType::Dot : MemberAccess::AccessType::Arrow;
            
            if (isAtEnd() || peek().type != TokenType::Identifier) {
                throw ParseError("Kutilgan etiket " + formatLocation(peek()));
            }
            
            const std::string memberName = peek().value;
            advance();
            expr = std::make_unique<MemberAccess>(std::move(expr), accessType, memberName, accessToken);
        }
        else if (peek().type == TokenType::Symbol && (peek().value == "++" || peek().value == "--")) {
            // Postfix increment/decrement
            const Token opToken = advance();
            UnaryExpression::UnaryOp op = (opToken.value == "++") ?
                UnaryExpression::UnaryOp::PostIncrement : UnaryExpression::UnaryOp::PostDecrement;
            expr = std::make_unique<UnaryExpression>(op, std::move(expr), opToken, false);
        }
        else if (peek().type == TokenType::Symbol && peek().value == "...") {
            // Pack expansion: args... -> emit as binary "..." with empty rhs
            const Token dotsToken = advance();
            auto dummy = std::make_unique<LiteralExpression>(
                LiteralExpression::LiteralType::String, "", dotsToken);
            expr = std::make_unique<BinaryExpression>(std::move(expr), "...", std::move(dummy), dotsToken);
        }
        else {
            break;
        }
    }

    return expr;
}

std::unique_ptr<Expression> Parser::parsePrimaryExpression() {
    if (isAtEnd()) {
        throw ParseError("Kutilgan ifoda, ammo faylning oxiri keldi");
    }

    // Phase 2.5: Faqat kalit so'z sinonimlari (`chiqarish`, `olish` va boshq.).
    // Tur sinonimlari (`umumiy`, `to'plam`) bu yerda tekshirilmaydi — chunki
    // ular foydalanuvchi o'zgaruvchi nomi sifatida ishlatilishi mumkin.
    checkDeprecatedSynonym();

    const Token& current = peek();

    // Ellipsis as an operand — supports C++17 fold expressions like (args + ...)
    if (current.type == TokenType::Symbol && current.value == "...") {
        const Token token = advance();
        return std::make_unique<IdentifierExpression>("...", token);
    }

    // Numeric literal
    if (current.type == TokenType::IntegerLiteral) {
        const Token token = advance();
        return std::make_unique<LiteralExpression>(
            LiteralExpression::LiteralType::Integer, token.value, token);
    }
    
    // Float literal
    if (current.type == TokenType::FloatLiteral) {
        const Token token = advance();
        return std::make_unique<LiteralExpression>(
            LiteralExpression::LiteralType::Float, token.value, token);
    }
    
    // String literal
    if (current.type == TokenType::StringLiteral) {
        const Token token = advance();
        return std::make_unique<LiteralExpression>(
            LiteralExpression::LiteralType::String, token.value, token);
    }
    
    // Format String literal
    if (current.type == TokenType::FormatStringLiteral) {
        const Token token = advance();
        return std::make_unique<LiteralExpression>(
            LiteralExpression::LiteralType::FormatString, token.value, token);
    }
    
    // Character literal
    if (current.type == TokenType::CharLiteral) {
        const Token token = advance();
        return std::make_unique<LiteralExpression>(
            LiteralExpression::LiteralType::Character, token.value, token);
    }
    
    // Boolean literal — yagona shakl: rost (true), yolg'on (false).
    if (current.type == TokenType::Identifier && (current.value == "rost" || current.value == "yolg'on")) {
        const Token token = advance();
        return std::make_unique<LiteralExpression>(
            LiteralExpression::LiteralType::Boolean, token.value, token);
    }
    
    // Lambda yoki Massiv: [captures](params) { body } yoki [1, 2, 3]
    if (current.type == TokenType::Symbol && current.value == "[") {
        // Lambda ekanligini aniqlash: [...]( ... ) { ... }
        // Lookahead: ']' topilgandan keyin '(' bo'lsa, bu lambda
        std::size_t lookahead = current_;
        int bracketDepth = 0;
        bool mightBeLambda = false;
        lookahead++; // skip '['
        while (lookahead < tokens_.size()) {
            if (tokens_[lookahead].value == "[") bracketDepth++;
            else if (tokens_[lookahead].value == "]") {
                if (bracketDepth == 0) {
                    if (lookahead + 1 < tokens_.size()) {
                        const std::string& nxt = tokens_[lookahead + 1].value;
                        // C++20 lambda template prefix: []shablon<T>(args){...}
                        if (nxt == "(" || nxt == "shablon") {
                            mightBeLambda = true;
                        }
                    }
                    break;
                }
                bracketDepth--;
            }
            lookahead++;
        }

        if (mightBeLambda) {
            const Token lambdaToken = advance(); // consume '['
            std::vector<LambdaExpression::Capture> captures;

            while (!isAtEnd() && peek().value != "]") {
                LambdaExpression::Capture cap;
                // C++17 *this capture (by value): [*bu] or [*this]
                if (peek().value == "*") {
                    advance(); // '*'
                    if (!isAtEnd() && peek().type == TokenType::Identifier &&
                        (peek().value == "bu" || peek().value == "this")) {
                        advance();
                        cap.name = "*this";
                        cap.byRef = false;
                    }
                } else if (peek().value == "&") {
                    cap.byRef = true;
                    advance(); // consume '&'
                    if (!isAtEnd() && peek().type == TokenType::Identifier && peek().value != "]" && peek().value != ",") {
                        cap.name = advance().value;
                    } else {
                        cap.name = "&"; // capture-all by ref
                    }
                } else if (peek().value == "=") {
                    advance(); // capture-all by value
                    cap.name = "=";
                    cap.byRef = false;
                } else if (peek().type == TokenType::Identifier) {
                    std::string n = advance().value;
                    // [bu] / [this] — pointer-this capture
                    if (n == "bu" || n == "this") {
                        cap.name = "this";
                    } else {
                        cap.name = n;
                    }
                    cap.byRef = false;
                }
                // C++14 init-capture: [name = expr]
                if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "=" && !cap.name.empty() && cap.name != "=") {
                    advance(); // '='
                    int depth = 0;
                    static const std::unordered_map<std::string, std::string> rawTokenMap = {
                        {"rost", "true"}, {"yolg'on", "false"},
                        {"ko'chirish", "std::move"},
                        {"null", "nullptr"}
                    };
                    while (!isAtEnd()) {
                        const std::string& v = peek().value;
                        if (depth == 0 && (v == "," || v == "]")) break;
                        if (v == "(" || v == "[" || v == "{") depth++;
                        else if (v == ")" || v == "]" || v == "}") depth--;
                        auto it = rawTokenMap.find(v);
                        cap.initExpr += (it != rawTokenMap.end() ? it->second : v) + " ";
                        advance();
                    }
                }
                captures.push_back(std::move(cap));
                if (!isAtEnd() && peek().value == ",") advance();
            }
            if (!isAtEnd()) advance(); // consume ']'

            // Parse optional template parameters: []shablon<tur T, tur U>(...) {...}
            // Translate Uzbek `tur` → C++ `typename` per existing shablon convention.
            std::string templateParams;
            if (checkKeyword("shablon")) {
                advance(); // consume 'shablon'
                if (!isAtEnd() && peek().value == "<") {
                    advance(); // consume '<'
                    templateParams = "<";
                    bool first = true;
                    while (!isAtEnd() && peek().value != ">") {
                        if (!first && peek().value == ",") {
                            templateParams += ", ";
                            advance();
                            continue;
                        }
                        if (checkKeyword("tur")) {
                            templateParams += "typename ";
                            advance();
                        } else if (peek().type == TokenType::Identifier) {
                            templateParams += advance().value + " ";
                        } else {
                            templateParams += advance().value;
                        }
                        first = false;
                    }
                    if (!isAtEnd()) advance(); // consume '>'
                    templateParams += ">";
                }
            }

            // Parse parameter list
            std::vector<LambdaExpression::Param> params;
            if (!isAtEnd() && peek().value == "(") {
                advance(); // consume '('
                while (!isAtEnd() && peek().value != ")") {
                    LambdaExpression::Param p;
                    if (looksLikeDeclHelper(tokens_, current_)) {
                        // Parse full type: base + namespace + template args + &/*
                        std::string typeStr = advance().value;
                        while (!isAtEnd() && peek().value == "::") {
                            typeStr += advance().value;
                            if (!isAtEnd()) typeStr += advance().value;
                        }
                        if (!isAtEnd() && peek().value == "<") {
                            typeStr += advance().value;
                            int d = 1;
                            while (!isAtEnd() && d > 0) {
                                if (peek().value == "<") d++;
                                else if (peek().value == ">") d--;
                                typeStr += advance().value;
                            }
                        }
                        while (!isAtEnd() && (peek().value == "&" || peek().value == "*" || peek().value == "&&")) {
                            typeStr += advance().value;
                        }
                        p.type = typeStr;
                        if (!isAtEnd() && peek().type == TokenType::Identifier) {
                            p.name = advance().value;
                        } else {
                            p.name = "_p" + std::to_string(params.size());
                        }
                    } else if (peek().type == TokenType::Identifier) {
                        p.name = advance().value;
                    } else {
                        advance(); // skip unrecognized token to avoid infinite loop
                    }
                    params.push_back(std::move(p));
                    if (!isAtEnd() && peek().value == ",") advance();
                }
                if (!isAtEnd()) advance(); // consume ')'
            }

            // Optional: -> ReturnType (consume and keep for later use)
            std::string lambdaReturnType;
            if (!isAtEnd() && peek().value == "->") {
                advance(); // consume '->'
                if (!isAtEnd() && peek().value != "{" && peek().value != "=>") {
                    lambdaReturnType = parseTypeString(); // consume return type annotation
                }
            }

            // Body: block or single expression (=> expr)
            std::unique_ptr<Statement> body;
            if (!isAtEnd() && peek().value == "{") {
                body = parseBlock();
            } else if (!isAtEnd() && peek().value == "=>") {
                advance(); // consume '=>'
                auto retExpr = parseExpression();
                Token retTok = lambdaToken;
                body = std::make_unique<ReturnStatement>(std::move(retExpr), retTok);
            }

            auto lambdaNode = std::make_unique<LambdaExpression>(std::move(captures), std::move(params), lambdaReturnType,
                                                      std::move(body), lambdaToken);
            lambdaNode->setTemplateParams(templateParams);
            return lambdaNode;
        }

        // Array Literal: [1, 2, 3]
        const Token bracketToken = advance(); // consume '['
        std::vector<std::unique_ptr<Expression>> elements;

        while (!isAtEnd() && !(peek().type == TokenType::Symbol && peek().value == "]")) {
            elements.push_back(parseExpression());

            if (peek().type == TokenType::Symbol && peek().value == ",") {
                advance();
            } else if (!(peek().type == TokenType::Symbol && peek().value == "]")) {
                throw ParseError("Kutilgan ',' yoki ']' massiv ichida " + formatLocation(peek()));
            }
        }

        if (isAtEnd() || peek().value != "]") {
            throw ParseError("Kutilgan ']' massiv oxirida " + formatLocation(peek()));
        }
        advance(); // consume ']'

        Token builtinToken = bracketToken;
        builtinToken.type = TokenType::Identifier;
        builtinToken.value = "__uzpp_array";
        auto callee = std::make_unique<IdentifierExpression>("__uzpp_array", builtinToken);
        return std::make_unique<FunctionCall>(std::move(callee), std::move(elements), bracketToken);
    }
    
    // Brace literal: bo'sh `{}`, lug'at `{"k": v}`, yoki brace-init `{a, b, c}`
    if (current.type == TokenType::Symbol && current.value == "{") {
        const Token braceToken = advance(); // consume '{'

        // Bo'sh {} - brace initialization
        if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "}") {
            advance(); // consume '}'
            Token builtinToken = braceToken;
            builtinToken.type = TokenType::Identifier;
            builtinToken.value = "__uzpp_brace";
            auto callee = std::make_unique<IdentifierExpression>("__uzpp_brace", builtinToken);
            return std::make_unique<FunctionCall>(std::move(callee),
                std::vector<std::unique_ptr<Expression>>{}, braceToken);
        }

        // Designated initializer: {.x = 1, .y = 2}
        if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ".") {
            std::string designatedText = "{";
            bool first = true;
            while (!isAtEnd() && peek().value != "}") {
                if (!first) designatedText += ", ";
                if (peek().value != ".") {
                    throw ParseError("Kutilgan '.' designated initializer'da " + formatLocation(peek()));
                }
                advance(); // '.'
                if (isAtEnd() || peek().type != TokenType::Identifier) {
                    throw ParseError("Kutilgan maydon nomi designated initializer'da");
                }
                std::string field = advance().value;
                if (isAtEnd() || peek().value != "=") {
                    throw ParseError("Kutilgan '=' designated initializer maydonidan keyin");
                }
                advance(); // '='
                // Collect value as text up to ',' or '}' at current nesting level.
                // Translate common Uzbek literals/types since this text bypasses codegen.
                static const std::unordered_map<std::string, std::string> rawTokenMap = {
                    {"rost", "true"},
                    {"yolg'on", "false"},
                    {"butun", "int"}, {"haqiqiy", "double"}, {"kasr", "float"},
                    {"belgi", "char"}, {"mantiqiy", "bool"}, {"bosh", "void"},
                    {"matn", "std::string"}, {"vektor", "std::vector"},
                    {"null", "nullptr"}
                };
                std::string valueText;
                int depth = 0;
                while (!isAtEnd()) {
                    const std::string& v = peek().value;
                    if (depth == 0 && (v == "," || v == "}")) break;
                    if (v == "{" || v == "(" || v == "[") depth++;
                    else if (v == "}" || v == ")" || v == "]") depth--;
                    auto it = rawTokenMap.find(v);
                    valueText += (it != rawTokenMap.end() ? it->second : v) + " ";
                    advance();
                }
                designatedText += "." + field + " = " + valueText;
                first = false;
                if (!isAtEnd() && peek().value == ",") advance();
            }
            if (!isAtEnd()) advance(); // '}'
            designatedText += "}";
            // Emit as an identifier expression carrying the literal C++ text
            Token litTok = braceToken;
            litTok.value = designatedText;
            return std::make_unique<IdentifierExpression>(designatedText, litTok);
        }

        // Birinchi elementni parse qilib, ':' yoki ',' ga qarab tur aniqlanadi
        auto firstExpr = parseExpression();

        if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ":") {
            // Lug'at (dictionary): {"kalit": "qiymat", ...}
            advance(); // consume ':'
            std::vector<std::unique_ptr<Expression>> elements;
            elements.push_back(std::move(firstExpr));
            elements.push_back(parseExpression()); // Value

            while (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ",") {
                advance(); // consume ','
                if (!isAtEnd() && peek().value == "}") break;
                elements.push_back(parseExpression()); // Key
                if (isAtEnd() || peek().value != ":") {
                    throw ParseError("Kutilgan ':' lug'at kalitidan keyin " + formatLocation(peek()));
                }
                advance(); // consume ':'
                elements.push_back(parseExpression()); // Value
            }

            if (isAtEnd() || peek().value != "}") {
                throw ParseError("Kutilgan '}' lug'at oxirida " + formatLocation(peek()));
            }
            advance(); // consume '}'

            Token builtinToken = braceToken;
            builtinToken.type = TokenType::Identifier;
            builtinToken.value = "__uzpp_dict";
            auto callee = std::make_unique<IdentifierExpression>("__uzpp_dict", builtinToken);
            return std::make_unique<FunctionCall>(std::move(callee), std::move(elements), braceToken);
        } else {
            // Brace-init ro'yxat: {a, b, c}
            std::vector<std::unique_ptr<Expression>> elements;
            elements.push_back(std::move(firstExpr));

            while (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ",") {
                advance(); // consume ','
                if (!isAtEnd() && peek().value == "}") break;
                elements.push_back(parseExpression());
            }

            if (isAtEnd() || peek().value != "}") {
                throw ParseError("Kutilgan '}' brace-init oxirida " + formatLocation(peek()));
            }
            advance(); // consume '}'

            Token builtinToken = braceToken;
            builtinToken.type = TokenType::Identifier;
            builtinToken.value = "__uzpp_brace";
            auto callee = std::make_unique<IdentifierExpression>("__uzpp_brace", builtinToken);
            return std::make_unique<FunctionCall>(std::move(callee), std::move(elements), braceToken);
        }
    }

    // irgitish (Throw Expression)
    if (current.type == TokenType::Identifier && current.value == "irgitish") {
        Token throwToken = advance(); // consume 'irgitish'
        auto expr = parseExpression();
        return std::make_unique<ThrowExpression>(std::move(expr), throwToken);
    }

    // Identifier or keyword
    if (current.type == TokenType::Identifier) {
        return parseIdentifierOrCall();
    }
    
    // Grouped expression
    if (current.type == TokenType::Symbol && current.value == "(") {
        advance(); // consume '('
        auto expr = parseExpression();

        if (isAtEnd() || peek().value != ")") {
            throw ParseError("Kutilgan ')' " + formatLocation(peek()));
        }

        advance(); // consume ')'
        return expr;
    }

    // Rust-style lambda: |x, y| => x + y  yoki  |x| { body }
    if (current.type == TokenType::Symbol && current.value == "|") {
        const Token lambdaToken = advance(); // consume '|'
        std::vector<LambdaExpression::Param> params;

        while (!isAtEnd() && peek().value != "|") {
            LambdaExpression::Param p;
            if (looksLikeDeclHelper(tokens_, current_)) {
                p.type = advance().value;
                if (!isAtEnd() && peek().type == TokenType::Identifier) {
                    p.name = advance().value;
                } else {
                    p.name = "_p" + std::to_string(params.size());
                }
            } else if (peek().type == TokenType::Identifier) {
                p.name = advance().value;
            }
            params.push_back(std::move(p));
            if (!isAtEnd() && peek().value == ",") advance();
        }
        if (!isAtEnd()) advance(); // consume closing '|'

        std::unique_ptr<Statement> body;
        if (!isAtEnd() && peek().value == "{") {
            body = parseBlock();
        } else {
            if (!isAtEnd() && (peek().value == "=>" || peek().value == "->")) {
                advance();
            }
            auto retExpr = parseExpression();
            Token retTok = lambdaToken;
            body = std::make_unique<ReturnStatement>(std::move(retExpr), retTok);
        }

        std::vector<LambdaExpression::Capture> captures;
        LambdaExpression::Capture allRef;
        allRef.name = "&";
        allRef.byRef = true;
        captures.push_back(std::move(allRef));

        return std::make_unique<LambdaExpression>(std::move(captures), std::move(params),
                                                  "", std::move(body), lambdaToken);
    }

    throw ParseError("Noto'g'ri ifoda " + formatLocation(current));
}

std::unique_ptr<Expression> Parser::parseIdentifierOrCall() {
    Token token = advance();
    
    // Nomlar fazosi va Sanab o'tish (Enum) qiymatlarini o'qish (masalan: Kunlar::Dushanba)
    while (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "::") {
        token.value += advance().value; // "::"
        if (!isAtEnd() && peek().type == TokenType::Identifier) {
            token.value += advance().value; // Identifikator
        } else {
            throw ParseError("Kutilgan identifikator '::' dan keyin " + formatLocation(peek()));
        }
    }

    // Generik turlarni identifikator ichiga yig'ish (masalan: vektor<butun>)
    if (!isAtEnd() && peek().value == "<") {
        // current_ points TO '<'; start lookahead one past it
        std::size_t i = current_ + 1;
        int depth = 1;
        bool isTemplate = false;
        while (i < tokens_.size() && depth > 0) {
            if (tokens_[i].value == "<") depth++;
            else if (tokens_[i].value == ">") {
                depth--;
                if (depth == 0) { isTemplate = true; break; }
            }
            else if (tokens_[i].value == ">>") {
                depth -= 2;
                if (depth <= 0) { isTemplate = true; break; }
            }
            else if (tokens_[i].value == ";" || tokens_[i].value == "{" || tokens_[i].value == "}") {
                break;
            }
            else if (depth == 1 && (tokens_[i].value == "||" || tokens_[i].value == "&&" ||
                                    tokens_[i].value == "!=" || tokens_[i].value == "yoki" ||
                                    tokens_[i].value == "va")) {
                break;
            }
            i++;
        }

        if (isTemplate) {
            token.value += advance().value; // '<'
            int tDepth = 1;
            // Phase 18: Shablon argumentlari orasidagi bo'shliqlarni saqlash.
            // `static_cast<unsigned char>` → `static_cast<unsigned char>`
            // bo'lishi kerak — `unsignedchar` emas.
            bool prevWasIdent = false;
            while (!isAtEnd() && tDepth > 0) {
                const std::string& tv = peek().value;
                if (tv == "<") { tDepth++; token.value += advance().value; prevWasIdent = false; }
                else if (tv == ">") { tDepth--; token.value += advance().value; prevWasIdent = false; }
                else if (tv == ">>") {
                    advance();
                    tDepth -= 2;
                    if (tDepth >= 0) { token.value += ">>"; }
                    else { token.value += ">"; tDepth = 0; }
                    prevWasIdent = false;
                }
                else if (tv == ",") {
                    token.value += advance().value; // ','
                    token.value += ' ';              // argument ajratuvchi bo'shliq
                    prevWasIdent = false;
                }
                else {
                    // Identifikatorlar orasiga bo'shliq qo'shamiz (unsigned char holati)
                    bool isIdent = (peek().type == TokenType::Identifier);
                    if (prevWasIdent && isIdent)
                        token.value += ' ';
                    token.value += advance().value;
                    prevWasIdent = isIdent;
                }
            }

            // After template params, handle any trailing ::Segment chains
            // e.g. uzpp::Natija<butun>::xato(...)
            while (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "::") {
                token.value += advance().value; // "::"
                if (!isAtEnd() && peek().type == TokenType::Identifier) {
                    token.value += advance().value; // segment name
                }
                // Handle nested templates: Foo<A>::Bar<B>::baz
                if (!isAtEnd() && peek().value == "<") {
                    std::size_t j = current_;
                    int d2 = 1;
                    bool isTpl2 = false;
                    while (j < tokens_.size() && d2 > 0) {
                        if (tokens_[j].value == "<") d2++;
                        else if (tokens_[j].value == ">") { d2--; if (d2 == 0) { isTpl2 = true; break; } }
                        else if (tokens_[j].value == ";" || tokens_[j].value == "{" || tokens_[j].value == "}") break;
                        j++;
                    }
                    if (isTpl2) {
                        token.value += advance().value; // '<'
                        while (peek().value != ">") token.value += advance().value;
                        token.value += advance().value; // '>'
                    }
                }
            }
        }
    }

    auto expr = std::make_unique<IdentifierExpression>(token.value, token);
    return expr;
}

// ===== SEMANTIC STATEMENT PARSING =====

std::unique_ptr<Statement> Parser::parseStatement() {
    // C++23 [[assume(cond)]]; — optimisation hint as a standalone statement.
    // Syntax: `@taxmin(cond);` — collect tokens inside () as raw text.
    if (peek().type == TokenType::Symbol && peek().value == "@" &&
        current_ + 1 < tokens_.size() && tokens_[current_+1].value == "taxmin") {
        const Token atTok = advance(); // '@'
        advance(); // 'taxmin'
        if (peek().value != "(") throw ParseError("Kutilgan '(' taxmindan keyin");
        advance(); // '('
        std::string cond;
        int depth = 1;
        while (!isAtEnd() && depth > 0) {
            if (peek().value == "(") depth++;
            else if (peek().value == ")") { depth--; if (depth == 0) break; }
            cond += peek().value + " ";
            advance();
        }
        if (!isAtEnd()) advance(); // ')'
        if (!isAtEnd() && peek().value == ";") advance();
        Token t = atTok;
        t.value = "[[assume(" + cond + ")]];\n";
        return std::make_unique<TokenStatement>(t);
    }

    if (checkKeyword("agar")) {
        return parseIfStatement();
    }

    if (checkKeyword("statik_tasdiqlash")) {
        // Parse as a regular function call expression — Uzbek identifier names
        // inside (e.g. `butun`, `haqiqiy`) will be translated by codegen normally.
        // Codegen recognises "statik_tasdiqlash" and emits "static_assert".
        auto e = parseExpression();
        if (!isAtEnd() && peek().value == ";") advance();
        return std::make_unique<ExpressionStatement>(std::move(e));
    }

    if (checkKeyword("moslash")) {
        return parseMatchStatement();
    }

    if (checkKeyword("urinish")) {
        const Token tryToken = advance(); // consume 'urinish'
        auto tryBlock = parseBlock();
        
        std::vector<std::unique_ptr<TryStatement::CatchClause>> catches;
        
        while (checkKeyword("ushlash")) {
            auto cb = std::make_unique<TryStatement::CatchClause>();
            cb->catchToken = advance(); // 'ushlash'
            if (isAtEnd() || peek().value != "(") throw ParseError("Kutilgan '(' ushlashdan keyin " + formatLocation(peek()));
            advance(); // '('
            while (!isAtEnd() && peek().value != ")") {
                cb->exceptionDecl += advance().value;
                if (peek().value != ")") cb->exceptionDecl += " ";
            }
            advance(); // ')'
            cb->block = parseBlock();
            catches.push_back(std::move(cb));
        }
        
        return std::make_unique<TryStatement>(std::move(tryBlock), std::move(catches), tryToken);
    }
    
    if (checkKeyword("toki")) {
        return parseWhileStatement();
    }

    if (checkKeyword("bajar")) {
        return parseDoWhileStatement();
    }

    // Bo'sh gap: `;` yolg'iz o'zi (masalan `uchun (...);` — tanasiz sikl).
    if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ";") {
        advance();
        return std::make_unique<Block>(std::vector<std::unique_ptr<Statement>>{});
    }
    
    if (checkKeyword("uchun")) {
        return parseForStatement();
    }
    
    if (checkKeyword("qaytarish")) {
        return parseReturnStatement();
    }
    
    if (checkKeyword("to'xtatish")) {
        return parseBreakStatement();
    }
    
    if (checkKeyword("davom_etish")) {
        return parseContinueStatement();
    }
    
    if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "{") {
        return parseBlock();
    }
    
    auto stmt = parseDeclarationOrExpressionStatement();
    
    // Consume the trailing semicolon if present
    if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ";") {
        advance();
    }
    
    return stmt;
}

std::unique_ptr<Block> Parser::parseBlock() {
    if (isAtEnd() || peek().value != "{") {
        throw ParseError("Kutilgan '{' " + formatLocation(peek()));
    }
    
    const Token openBrace = advance();
    std::vector<std::unique_ptr<Statement>> statements;
    
    while (!isAtEnd() && !(peek().type == TokenType::Symbol && peek().value == "}")) {
        statements.push_back(parseStatement());
    }
    
    if (isAtEnd() || peek().value != "}") {
        throw ParseError("Kutilgan '}' " + formatLocation(peek()));
    }
    
    advance(); // consume '}'
    return std::make_unique<Block>(std::move(statements), openBrace);
}

std::unique_ptr<MatchStatement> Parser::parseMatchStatement() {
    const Token matchToken = advance(); // consume 'moslash'
    
    if (isAtEnd() || peek().value != "(") {
        throw ParseError("Kutilgan '(' moslashdan keyin " + formatLocation(peek()));
    }
    advance(); // consume '('
    
    auto condition = parseExpression();
    
    if (isAtEnd() || peek().value != ")") {
        throw ParseError("Kutilgan ')' moslash shartidan keyin " + formatLocation(peek()));
    }
    advance(); // consume ')'
    
    if (isAtEnd() || peek().value != "{") {
        throw ParseError("Kutilgan '{' moslash tanasidan oldin " + formatLocation(peek()));
    }
    advance(); // consume '{'
    
    std::vector<std::unique_ptr<MatchStatement::MatchCase>> cases;
    
    while (!isAtEnd() && !(peek().type == TokenType::Symbol && peek().value == "}")) {
        auto matchCase = std::make_unique<MatchStatement::MatchCase>();
        matchCase->caseToken = peek();
        
        if (checkKeyword("holat")) {
            advance(); // consume 'holat'
            matchCase->pattern = parseExpression();
            // `holat 1, 2, 3:` — bir nechta qiymat bitta tanaga.
            while (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ",") {
                advance(); // ','
                matchCase->extraPatterns.push_back(parseExpression());
            }
            if (isAtEnd() || peek().value != ":") {
                throw ParseError("Kutilgan ':' holatdan keyin " + formatLocation(peek()));
            }
            advance(); // consume ':'
        } else if (checkKeyword("boshqa")) {
            advance(); // consume 'boshqa'
            if (isAtEnd() || peek().value != ":") {
                throw ParseError("Kutilgan ':' boshqadan keyin " + formatLocation(peek()));
            }
            advance(); // consume ':'
            matchCase->pattern = nullptr;
        } else {
            throw ParseError("Kutilgan 'holat' yoki 'boshqa' " + formatLocation(peek()));
        }
        
        // Collect statements until next holat/boshqa/} — wrap in implicit Block
        {
            Token blkTok = peek();
            std::vector<std::unique_ptr<Statement>> stmts;
            while (!isAtEnd() && peek().value != "}" &&
                   !checkKeyword("holat") && !checkKeyword("boshqa")) {
                stmts.push_back(parseStatement());
            }
            if (stmts.size() == 1 && stmts[0]->getType() == ASTNodeType::Block) {
                matchCase->body.reset(static_cast<Block*>(stmts[0].release()));
            } else {
                matchCase->body = std::make_unique<Block>(std::move(stmts), blkTok);
            }
        }
        cases.push_back(std::move(matchCase));
    }
    
    if (cases.empty()) {
        throw ParseError("Moslash kamida bitta 'holat' yoki 'boshqa' talab qiladi " + formatLocation(matchToken));
    }

    // C uslubidagi guruhlangan yorliqlar:
    //     holat 12:
    //     holat 1:
    //     holat 2:
    //         yozish << "Qish" << qator_oxiri;
    // Tanasi bo'sh `holat` o'zidan keyingi holatning muqobil naqshiga
    // aylanadi — `switch` dagi "fallthrough" bilan bir xil natija.
    {
        std::vector<std::unique_ptr<MatchStatement::MatchCase>> merged;
        std::vector<std::unique_ptr<Expression>> pending;
        for (auto& mc : cases) {
            const bool emptyBody =
                mc->body == nullptr ||
                (mc->body->getType() == ASTNodeType::Block &&
                 static_cast<const Block*>(mc->body.get())->getStatements().empty());
            const bool isLast = (mc.get() == cases.back().get());

            if (mc->pattern != nullptr && emptyBody && !isLast) {
                pending.push_back(std::move(mc->pattern));
                for (auto& ep : mc->extraPatterns) pending.push_back(std::move(ep));
                continue;
            }

            if (!pending.empty() && mc->pattern != nullptr) {
                // Yig'ilgan naqshlar birinchi bo'ladi, joriy naqsh qo'shimcha.
                auto first = std::move(pending.front());
                std::vector<std::unique_ptr<Expression>> extras;
                for (std::size_t i = 1; i < pending.size(); ++i) extras.push_back(std::move(pending[i]));
                extras.push_back(std::move(mc->pattern));
                for (auto& ep : mc->extraPatterns) extras.push_back(std::move(ep));
                mc->pattern = std::move(first);
                mc->extraPatterns = std::move(extras);
                pending.clear();
            }
            merged.push_back(std::move(mc));
        }
        // Oxirida osilib qolgan bo'sh yorliqlar — o'z holicha qoldiramiz.
        for (auto& pat : pending) {
            auto mc = std::make_unique<MatchStatement::MatchCase>();
            mc->pattern = std::move(pat);
            mc->body = std::make_unique<Block>(std::vector<std::unique_ptr<Statement>>{});
            merged.push_back(std::move(mc));
        }
        cases = std::move(merged);
    }
    
    if (isAtEnd() || peek().value != "}") {
        throw ParseError("Kutilgan '}' moslash oxirida " + formatLocation(peek()));
    }
    advance(); // consume '}'
    
    return std::make_unique<MatchStatement>(std::move(condition), std::move(cases), matchToken);
}

std::unique_ptr<IfStatement> Parser::parseIfStatement() {
    const Token ifToken = advance(); // consume 'agar'

    bool isConstExpr = false;
    bool isConsteval = false;
    if (checkKeyword("sobit_ifoda")) {
        isConstExpr = true;
        advance();
    } else if (checkKeyword("sobit_baholash")) {
        // C++23 `if consteval { ... }` — no condition!
        isConsteval = true;
        advance();
    }

    // `if consteval` has no parenthesised condition — go straight to body.
    std::unique_ptr<Expression> condition;
    if (isConsteval) {
        // Build a dummy "true" literal as a placeholder condition.
        Token dummyTrue = ifToken; dummyTrue.value = "true";
        condition = std::make_unique<LiteralExpression>(
            LiteralExpression::LiteralType::Boolean, "rost", dummyTrue);
    } else {
        if (isAtEnd() || peek().value != "(") {
            throw ParseError("Kutilgan '(' agar ifodasidan keyin " + formatLocation(peek()));
        }
        advance(); // consume '('
        condition = parseExpression();
        if (isAtEnd() || peek().value != ")") {
            throw ParseError("Kutilgan ')' " + formatLocation(peek()));
        }
        advance(); // consume ')'
    }

    // C++20 branch hints on the then-branch: agar (cond) @bashqarib { ... }
    // (@bashqarib = "odatda" → [[likely]], @kamdan_kam = "kamdan-kam" → [[unlikely]])
    auto parseBranchHint = [&](bool& likely, bool& unlikely) {
        if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "@") {
            std::size_t saved = current_;
            advance(); // '@'
            if (!isAtEnd() && peek().type == TokenType::Identifier &&
                (peek().value == "bashqarib" || peek().value == "kamdan_kam")) {
                if (peek().value == "bashqarib") likely = true;
                else unlikely = true;
                advance(); // consume the hint identifier
            } else {
                current_ = saved; // not a branch hint — back off
            }
        }
    };

    bool thenLikely = false, thenUnlikely = false;
    parseBranchHint(thenLikely, thenUnlikely);

    auto thenBranch = parseStatement();

    std::unique_ptr<Statement> elseBranch;
    bool elseLikely = false, elseUnlikely = false;
    // `yoki` doubles as a logical-OR operator, but in this position (right
    // after an if's then-branch) it can only be `else`. We accept it here so
    // users writing `agar (...) { ... } yoki { ... }` get the natural meaning
    // instead of an `else; { ... }` (empty-then-orphan-block) emit.
    if (matchKeyword("aks_holda") || matchKeyword("yoki")) {
        // else-if: aks_holda agar (...)
        parseBranchHint(elseLikely, elseUnlikely);
        if (checkKeyword("agar")) {
            elseBranch = parseIfStatement();
        } else {
            elseBranch = parseStatement();
        }
    } else if (checkKeyword("boshqa") && !isAtEnd()) {
        // boshqa { ... } — else clause (not switch default which uses boshqa:)
        size_t next = current_ + 1;
        bool isElse = next < tokens_.size() && tokens_[next].value == "{";
        if (isElse) {
            advance(); // consume 'boshqa'
            parseBranchHint(elseLikely, elseUnlikely);
            elseBranch = parseStatement();
        }
    }

    auto ifStmt = std::make_unique<IfStatement>(std::move(condition), std::move(thenBranch),
                                        std::move(elseBranch), ifToken);
    ifStmt->setConstExpr(isConstExpr);
    ifStmt->setConsteval(isConsteval);
    if (thenLikely) ifStmt->setThenLikely(true);
    if (thenUnlikely) ifStmt->setThenUnlikely(true);
    if (elseLikely) ifStmt->setElseLikely(true);
    if (elseUnlikely) ifStmt->setElseUnlikely(true);
    return ifStmt;
}

std::unique_ptr<WhileStatement> Parser::parseWhileStatement() {
    const Token whileToken = advance(); // consume 'toki'
    
    if (isAtEnd() || peek().value != "(") {
        throw ParseError("Kutilgan '(' toki ifodasidan keyin " + formatLocation(peek()));
    }
    
    advance(); // consume '('
    auto condition = parseExpression();
    
    if (isAtEnd() || peek().value != ")") {
        throw ParseError("Kutilgan ')' " + formatLocation(peek()));
    }
    
    advance(); // consume ')'
    auto body = parseStatement();
    
    return std::make_unique<WhileStatement>(std::move(condition), std::move(body), whileToken);
}

std::unique_ptr<WhileStatement> Parser::parseDoWhileStatement() {
    const Token doToken = advance(); // 'bajar'

    auto body = parseStatement();

    if (!checkKeyword("toki")) {
        throw ParseError("Kutilgan 'toki' `bajar` tanasidan keyin " + formatLocation(peek()));
    }
    advance(); // 'toki'

    if (isAtEnd() || peek().value != "(") {
        throw ParseError("Kutilgan '(' toki ifodasidan keyin " + formatLocation(peek()));
    }
    advance(); // '('
    auto condition = parseExpression();

    if (isAtEnd() || peek().value != ")") {
        throw ParseError("Kutilgan ')' " + formatLocation(peek()));
    }
    advance(); // ')'

    if (!isAtEnd() && peek().value == ";") advance(); // ixtiyoriy ';'

    auto stmt = std::make_unique<WhileStatement>(std::move(condition), std::move(body), doToken);
    stmt->setDoWhile(true);
    return stmt;
}

std::unique_ptr<Statement> Parser::parseForStatement() {
    const Token forToken = advance(); // consume 'uchun'
    
    if (isAtEnd() || peek().value != "(") {
        throw ParseError("Kutilgan '(' uchun siklidan keyin " + formatLocation(peek()));
    }
    
    advance(); // consume '('

    // Parse init
    std::unique_ptr<Statement> init;
    if (!(peek().type == TokenType::Symbol && peek().value == ";")) {
        init = parseDeclarationOrExpressionStatement();
    }

    // Range-based for: uchun (tur nom : to'plam) { ... }
    if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ":") {
        advance(); // consume ':'
        auto rangeExpr = parseExpression();

        if (isAtEnd() || peek().value != ")") {
            throw ParseError("Kutilgan ')' " + formatLocation(peek()));
        }
        advance(); // consume ')'

        auto body = parseStatement();
        return std::make_unique<ForStatement>(std::move(init), std::move(rangeExpr),
                                             nullptr, std::move(body), forToken, true);
    }

    // C++20 range-for with init: uchun (DECL1; DECL2 : EXPR) { ... }
    // We detect this when after init we see `;` followed by a decl-then-`:`.
    if (!isAtEnd() && peek().value == ";") {
        // Lookahead — find `:` or second `;` first
        std::size_t scan = current_ + 1;
        int depth = 0;
        bool foundColon = false;
        while (scan < tokens_.size()) {
            const std::string& v = tokens_[scan].value;
            if (v == "(" || v == "[" || v == "{") depth++;
            else if (v == ")" || v == "]" || v == "}") {
                if (depth == 0) break;
                depth--;
            } else if (depth == 0 && v == ";") break;
            else if (depth == 0 && v == ":") { foundColon = true; break; }
            scan++;
        }
        if (foundColon) {
            advance(); // consume ';' between init and range-decl
            // Parse range-declaration
            auto rangeDecl = parseDeclarationOrExpressionStatement();
            if (peek().value != ":") throw ParseError("Kutilgan ':' range-for ichida");
            advance(); // ':'
            auto rangeExpr = parseExpression();
            if (peek().value != ")") throw ParseError("Kutilgan ')'");
            advance();
            auto body = parseStatement();
            // C++20 range-for-with-init: rewrite as
            //   { init; for (rangedecl : rangeexpr) body }
            std::vector<std::unique_ptr<Statement>> blockStmts;
            if (init) blockStmts.push_back(std::move(init));
            auto rangeFor = std::make_unique<ForStatement>(std::move(rangeDecl),
                std::move(rangeExpr), nullptr, std::move(body), forToken, true);
            blockStmts.push_back(std::move(rangeFor));
            return std::make_unique<Block>(std::move(blockStmts));
        }
    }

    if (isAtEnd() || peek().value != ";") {
        throw ParseError("Kutilgan ';' siklda " + formatLocation(peek()));
    }

    advance(); // consume first ';'
    
    // Parse condition
    std::unique_ptr<Expression> condition;
    if (!(peek().type == TokenType::Symbol && peek().value == ";")) {
        condition = parseExpression();
    }
    
    if (isAtEnd() || peek().value != ";") {
        throw ParseError("Kutilgan ';' siklda " + formatLocation(peek()));
    }
    
    advance(); // consume second ';'
    
    // Parse increment
    std::unique_ptr<Expression> increment;
    if (!(peek().type == TokenType::Symbol && peek().value == ")")) {
        increment = parseExpression();
    }
    
    if (isAtEnd() || peek().value != ")") {
        throw ParseError("Kutilgan ')' " + formatLocation(peek()));
    }
    
    advance(); // consume ')'
    
    auto body = parseStatement();
    
    return std::make_unique<ForStatement>(std::move(init), std::move(condition), 
                                         std::move(increment), std::move(body), forToken);
}

std::unique_ptr<ReturnStatement> Parser::parseReturnStatement() {
    const Token returnToken = advance(); // consume 'qaytish' or 'qaytarish'
    
    std::unique_ptr<Expression> value;
    if (!(peek().type == TokenType::Symbol && peek().value == ";") &&
        !(peek().type == TokenType::Symbol && peek().value == "}")) {
        value = parseExpression();
    }
    
    if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ";") {
        advance();
    }
    return std::make_unique<ReturnStatement>(std::move(value), returnToken);
}

std::unique_ptr<BreakStatement> Parser::parseBreakStatement() {
    const Token breakToken = advance(); // consume 'to'xtatish'
    if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ";") {
        advance();
    }
    return std::make_unique<BreakStatement>(breakToken);
}

std::unique_ptr<ContinueStatement> Parser::parseContinueStatement() {
    const Token continueToken = advance(); // consume 'davom_etish'
    if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ";") {
        advance();
    }
    return std::make_unique<ContinueStatement>(continueToken);
}

std::unique_ptr<Statement> Parser::parseDeclarationOrExpressionStatement() {
    // Phase 2.5: Eskirgan sinonim ishlatilgan bo'lsa, yordamchi xato.
    checkDeprecatedSynonym();

    bool isConstExpr = false;
    bool isConstEval = false;
    bool isConstInit = false;

    bool isMutable = false;
    bool isThreadLocal = false;
    bool isStaticLocal = false;
    bool isInlineVar = false;
    while (!isAtEnd()) {
        if (checkKeyword("o'zgarmas_ifoda") || checkKeyword("sobit_ifoda")) {
            isConstExpr = true;
            advance();
        } else if (checkKeyword("o'zgarmas_baholash") || checkKeyword("sobit_baholash")) {
            isConstEval = true;
            advance();
        } else if (checkKeyword("o'zgarmas_boshlangich") || checkKeyword("sobit_boshlangich")) {
            isConstInit = true;
            advance();
        } else if (checkKeyword("ozgaruvchi_o'zgartirish") || checkKeyword("mutable")) {
            // mutable — for caching in const methods
            isMutable = true;
            advance();
        } else if (checkKeyword("oqim_mahalliy") || checkKeyword("thread_local")) {
            // thread_local — per-thread storage
            isThreadLocal = true;
            advance();
        } else if (checkKeyword("statik")) {
            // local static — Meyers singleton / lazy init
            isStaticLocal = true;
            advance();
        } else if (checkKeyword("qator_ichi") || checkKeyword("inline")) {
            // C++17 inline variables on namespace scope
            isInlineVar = true;
            advance();
        } else {
            break;
        }
    }
    if (isConstEval) throw ParseError("sobit_baholash faqat funksiyalarga qo'llaniladi");

    // Destructuring: ozgaruvchan [x, y] = ...
    if ((peek().value == "o'zgaruvchan" || peek().value == "o'zgarmas")
        && current_ + 1 < tokens_.size() && tokens_[current_ + 1].value == "[") {
        std::string typeName = advance().value; // ozgaruvchan / ozgarmas
        advance(); // '['
        std::string name = "[";
        while (!isAtEnd() && peek().value != "]") {
            name += advance().value;
            if (peek().value == ",") name += " ";
        }
        if (!isAtEnd()) name += advance().value; // ']'

        return parseVariableDeclaration(typeName, name);
    }

    // Ifoda sifatida boshlana oladigan kalit so'zlarni tur deb qabul qilmaslik
    static const std::unordered_set<std::string> exprOnlyKeywords = {
        "kutish", "irgitish", "yangi", "o'chirish", "chiqar_qadam"
    };
    if (peek().type == TokenType::Identifier && exprOnlyKeywords.contains(peek().value)) {
        auto expr = parseExpression();
        return std::make_unique<ExpressionStatement>(std::move(expr));
    }

    if (looksLikeDeclHelper(tokens_, current_)) {
        // o'zgarmas TYPE NAME -> const TYPE NAME
        bool isConst = false;
        if (peek().value == "o'zgarmas") {
            isConst = true;
            advance(); // consume o'zgarmas
        }

        std::string typeName = parseTypeString();
        if (isConst) typeName = "o'zgarmas " + typeName;
        std::string name = advance().value;
        auto varDecl = parseVariableDeclaration(typeName, name);
        if (isConstExpr) varDecl->setConstExpr(true);
        if (isConstInit) varDecl->setConstInit(true);
        if (isMutable) varDecl->setMutable(true);
        if (isThreadLocal) varDecl->setThreadLocal(true);
        if (isStaticLocal) varDecl->setStaticLocal(true);
        if (isInlineVar) varDecl->setInline(true);

        // Comma-separated declarations: butun a = 1, b = 2;
        if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ",") {
            std::vector<std::unique_ptr<Statement>> stmts;
            stmts.push_back(std::move(varDecl));
            while (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ",") {
                advance(); // consume ','
                std::string nextName = advance().value;
                auto more = parseVariableDeclaration(typeName, nextName);
                if (isConstExpr) more->setConstExpr(true);
                if (isConstInit) more->setConstInit(true);
                if (isMutable) more->setMutable(true);
                if (isThreadLocal) more->setThreadLocal(true);
                if (isStaticLocal) more->setStaticLocal(true);
                if (isInlineVar) more->setInline(true);
                stmts.push_back(std::move(more));
            }
            if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ";") advance();
            return std::make_unique<StatementList>(std::move(stmts));
        }

        return varDecl;
    }

    auto expr = parseExpression();
    
    // NOTE: Do NOT consume semicolon here - let each caller decide
    
    return std::make_unique<ExpressionStatement>(std::move(expr));
}

// ===== SEMANTIC DECLARATION PARSING =====

std::unique_ptr<ASTNode> Parser::parseGlobalDeclaration() {
    // Phase 2.5: Eskirgan sinonim ishlatilgan bo'lsa, yordamchi xato.
    checkDeprecatedSynonym();

    // extern "C" { ... } — FFI linkage block.
    // We emit it as raw C++ wrapping the parsed declarations.
    if (checkKeyword("tashqi") || checkKeyword("extern")) {
        const Token externToken = peek();
        // Look ahead: tashqi "C" { ... } or tashqi "C" decl;
        if (current_ + 1 < tokens_.size() && tokens_[current_ + 1].type == TokenType::StringLiteral) {
            advance(); // consume 'tashqi'
            std::string linkage = advance().value; // "C" or "C++" with quotes
            // Open brace?
            if (!isAtEnd() && peek().value == "{") {
                advance(); // '{'
                std::vector<std::unique_ptr<ASTNode>> children;
                // Marker child = opening 'extern "C" {'
                Token openMarker = externToken;
                openMarker.value = "extern " + linkage + " {\n";
                children.push_back(std::make_unique<TokenNode>(openMarker));
                while (!isAtEnd() && peek().value != "}") {
                    children.push_back(parseGlobalDeclaration());
                }
                if (!isAtEnd()) advance(); // '}'
                Token closeMarker = externToken;
                closeMarker.value = "}\n";
                children.push_back(std::make_unique<TokenNode>(closeMarker));
                Token open; open.value = ""; Token close; close.value = "";
                return std::make_unique<GroupNode>(open, close, std::move(children));
            }
        }
    }

    if (checkKeyword("eksport")) {
        const Token expToken = peek();
        advance(); // 'eksport' kalit so'zini yutish
        
        if (checkKeyword("modul")) {
            advance(); // 'modul'
            std::string modName = advance().value;
            // Module partition: eksport modul foo:bar;
            if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ":") {
                advance(); // ':'
                modName += ":" + advance().value;
            }
            if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ";") advance();
            return std::make_unique<ExportModuleStatement>(modName, expToken);
        }
    }

    if (checkKeyword("import")) {
        const Token token = advance();
        std::string moduleName;
        if (peek().type == TokenType::StringLiteral) {
            moduleName = advance().value;
            moduleName = moduleName.substr(1, moduleName.length() - 2);
        } else {
            moduleName = advance().value;
        }
        if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ";") advance();
        return std::make_unique<IncludeStatement>(moduleName, token);
    }

    if (checkKeyword("tushuncha")) {
        const Token token = advance();
        std::string name = advance().value;
        if (peek().value != "=") throw ParseError("Kutilgan '=' tushunchadan keyin");
        advance();
        
        std::string expr = "concept " + name + " = ";
        while (!isAtEnd() && peek().value != ";") {
            expr += advance().value + " ";
        }
        if (!isAtEnd()) advance(); // ';'
        expr += ";\n";
        
        Token t = token; t.value = expr;
        return std::make_unique<TokenNode>(t);
    }

    if (checkKeyword("statik_tasdiqlash")) {
        // Parse as a regular function call expression — Uzbek identifier names
        // inside (e.g. `butun`, `haqiqiy`) will be translated by codegen normally.
        // Codegen recognises "statik_tasdiqlash" and emits "static_assert".
        auto e = parseExpression();
        if (!isAtEnd() && peek().value == ";") advance();
        return std::make_unique<ExpressionStatement>(std::move(e));
    }

    if (checkKeyword("makro")) {
        const Token makroToken = advance(); // consume 'makro'
        if (isAtEnd() || peek().type != TokenType::Identifier) throw ParseError("Kutilgan makro nomi");
        std::string name = advance().value;
        std::string macroBody = "#define " + name;
        
        if (!isAtEnd() && peek().value == "(") {
            macroBody += advance().value; // '('
            while (!isAtEnd() && peek().value != ")") {
                macroBody += advance().value;
            }
            if (!isAtEnd()) macroBody += advance().value; // ')'
        }
        
        if (!isAtEnd() && (peek().value == "=>" || peek().value == "=")) advance();
        
        macroBody += " ";
        while (!isAtEnd() && peek().value != ";") {
            macroBody += advance().value + " ";
        }
        if (!isAtEnd()) advance(); // consume ';'
        
        macroBody += "\n";
        Token dummy = makroToken; dummy.value = macroBody;
        return std::make_unique<TokenNode>(dummy);
    }

    bool isAsync = false;
    bool isTest = false;
    bool isBench = false;
    bool isConstExpr = false;
    bool isConstEval = false;
    bool isConstInit = false;
    bool isNoDiscard = false;
    bool isDeprecated = false;
    bool isInlineVar = false;
    std::string alignment;

    while (!isAtEnd()) {
        if (peek().type == TokenType::Symbol && peek().value == "@") {
            advance(); // consume '@'
            if (peek().type == TokenType::Identifier && peek().value == "sinov") {
                isTest = true;
                advance(); // consume 'sinov'
            } else if (peek().type == TokenType::Identifier && peek().value == "bench") {
                isBench = true;
                advance(); // consume 'bench'
            } else if (peek().type == TokenType::Identifier && peek().value == "tashlab_yuborilmas") {
                isNoDiscard = true;
                advance();
            } else if (peek().type == TokenType::Identifier && peek().value == "eskirgan") {
                isDeprecated = true;
                advance();
            } else if (peek().type == TokenType::Identifier && peek().value == "tekislash") {
                // @tekislash(N) — C++ alignas(N) alignment specifier
                advance(); // consume 'tekislash'
                if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "(") {
                    advance(); // consume '('
                    if (!isAtEnd() && peek().type == TokenType::IntegerLiteral) {
                        alignment = advance().value; // get alignment value
                    }
                    if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ")") {
                        advance(); // consume ')'
                    }
                }
            } else {
                throw ParseError("Noma'lum annotatsiya " + formatLocation(peek()));
            }
        } else if (checkKeyword("asinxron")) {
            isAsync = true;
            advance();
        } else if (checkKeyword("o'zgarmas_ifoda") || checkKeyword("sobit_ifoda")) {
            isConstExpr = true;
            advance();
        } else if (checkKeyword("o'zgarmas_baholash") || checkKeyword("sobit_baholash")) {
            isConstEval = true;
            advance();
        } else if (checkKeyword("o'zgarmas_boshlangich") || checkKeyword("sobit_boshlangich")) {
            isConstInit = true;
            advance();
        } else if (checkKeyword("qator_ichi") || checkKeyword("inline")) {
            // C++17 inline variables on namespace scope
            isInlineVar = true;
            advance();
        } else {
            break;
        }
    }

    if (checkKeyword("shablon")) {
        const Token shablonToken = advance();
        if (isAtEnd() || peek().value != "<") throw ParseError("Kutilgan '<' shablon parametridan oldin");
        advance();
        
        std::string templateParams = "template <";
        bool first = true;
        while (!isAtEnd() && peek().value != ">") {
            if (!first) {
                if (peek().value == ",") {
                    templateParams += ", ";
                    advance();
                    continue;
                }
            }
            if (checkKeyword("tur")) {
                templateParams += "typename";
                advance();
                // Variadic template: shablon<tur... Args>  ->  template <typename... Args>
                if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "...") {
                    templateParams += "...";
                    advance();
                }
                templateParams += " ";
            } else if (peek().type == TokenType::Symbol && peek().value == "...") {
                // Standalone ellipsis after a previously consumed identifier
                templateParams += "... ";
                advance();
            } else if (peek().type == TokenType::Identifier) {
                templateParams += advance().value + " ";
            } else {
                templateParams += advance().value;
            }
            first = false;
        }
        if (isAtEnd() || peek().value != ">") throw ParseError("Kutilgan '>'");
        advance();
        templateParams += ">\n";
        
        if (checkKeyword("shart")) {
            advance(); // 'shart'
            templateParams += "requires ";
            while (!isAtEnd() && !checkKeyword("sinf") && !checkKeyword("funktsiya") && !checkKeyword("shartnoma") && !looksLikeDeclHelper(tokens_, current_)) {
                templateParams += advance().value + " ";
            }
            templateParams += "\n";
        }
        
        auto decl = parseGlobalDeclaration();
        Token openToken = shablonToken; openToken.value = templateParams;
        Token closeToken = shablonToken; closeToken.value = "";
        
        std::vector<std::unique_ptr<ASTNode>> children;
        children.push_back(std::move(decl));
        return std::make_unique<GroupNode>(openToken, closeToken, std::move(children));
    }

    if (checkKeyword("sanab_olish")) {
        const Token enumToken = advance();
        std::string name = advance().value;
        if (isAtEnd() || peek().value != "{") throw ParseError("Kutilgan '{' sanab_olishdan keyin");
        advance(); // '{'

        std::vector<EnumDeclaration::EnumValue> values;
        while (!isAtEnd() && peek().value != "}") {
            // Skip commas between values
            if (peek().value == ",") { advance(); continue; }
            EnumDeclaration::EnumValue ev;
            ev.name = advance().value;
            if (!isAtEnd() && peek().value == "=") {
                advance(); // '='
                // Collect the explicit value (integer literal or identifier)
                ev.explicitValue = advance().value;
            }
            values.push_back(std::move(ev));
        }
        if (!isAtEnd()) advance(); // '}'
        if (!isAtEnd() && peek().value == ";") advance();

        return std::make_unique<EnumDeclaration>(name, std::move(values), enumToken);
    }

    // Handle empty statements (just `;`)
    if (peek().type == TokenType::Symbol && peek().value == ";") {
        advance(); // consume ';'
        // Skip trailing semicolons - they might come after class/interface declarations
        // in namespace context. If the next token is '}', the namespace loop will catch it.
        if (isAtEnd() || (peek().type == TokenType::Symbol && peek().value == "}")) {
            // End of declarations or end of namespace - return a dummy node
            // We could return nullptr or an EmptyStatement, but let's just return a TokenNode
            Token emptyToken;
            emptyToken.type = TokenType::EndOfFile;
            emptyToken.value = "";
            emptyToken.line = peek().line;
            emptyToken.column = peek().column;
            return std::make_unique<TokenNode>(emptyToken);
        }
        // Otherwise, continue parsing the next declaration
        return parseGlobalDeclaration();
    }

    // C++ preprocessor direktivalarini (#include, #define, va boshqalar) qabul qilish
    if (peek().type == TokenType::Symbol && peek().value == "#") {
        const Token hashToken = advance(); // consume '#'
        std::string directive = advance().value; // 'include', 'define', va boshqalar
        if (directive == "include") {
            std::string headerName;
            if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "<") {
                advance(); // consume '<'
                while (!isAtEnd() && peek().value != ">") {
                    headerName += advance().value;
                }
                if (!isAtEnd()) advance(); // consume '>'
            } else if (!isAtEnd() && peek().type == TokenType::StringLiteral) {
                headerName = advance().value;
                headerName = headerName.substr(1, headerName.length() - 2);
            }
            return std::make_unique<IncludeStatement>(headerName, hashToken);
        }
        // Boshqa direktivalarni (define, pragma va h.k.) o'tkazib yuborish
        while (!isAtEnd() && peek().type != TokenType::EndOfFile &&
               peek().line == hashToken.line) {
            advance();
        }
        Token dummy = hashToken; dummy.value = "";
        return std::make_unique<TokenNode>(dummy);
    }

    if (checkKeyword("nomlar_fazosi")) {
        if (isAsync) throw ParseError("nomlar_fazosi asinxron bo'lmaydi");
        return parseNamespaceDeclaration();
    }
    if (checkKeyword("ulash") || (peek().type == TokenType::Symbol && peek().value == "#include")) {
        if (isAsync) throw ParseError("ulash asinxron bo'lmaydi");
        return parseIncludeStatement();
    }
    if (checkKeyword("ulash_kutubxona")) {
        if (isAsync) throw ParseError("ulash_kutubxona asinxron bo'lmaydi");
        return parseLinkStatement();
    }
    
    if (checkKeyword("shartnoma")) {
        if (isAsync) throw ParseError("shartnoma asinxron bo'lmaydi");
        return parseInterfaceDeclaration();
    }
    if (checkKeyword("sinf") || checkKeyword("tuzilma") || checkKeyword("birlashma")) {
        if (isAsync) throw ParseError("sinf asinxron bo'lmaydi");
        std::string kw = peek().value;
        auto cls = parseClassDeclaration();
        if (kw == "tuzilma") cls->setKind("struct");
        else if (kw == "birlashma") cls->setKind("union");
        if (!alignment.empty()) cls->setAlignment(alignment);
        return cls;
    }
    if (checkKeyword("tur")) {
        // Type alias: tur Nom = EskiTur;
        if (isAsync) throw ParseError("tur asinxron bo'lmaydi");
        const Token turToken = advance(); // 'tur'
        std::string alias = advance().value;  // alias name
        if (peek().value != "=") throw ParseError("Kutilgan '=' tur so'zidan keyin " + formatLocation(peek()));
        advance(); // '='
        std::string target = advance().value; // target type
        if (!isAtEnd() && peek().value == ";") advance();
        return std::make_unique<TypeAlias>(alias, target, turToken);
    }
    if (checkKeyword("funksiya")) {
        auto func = parseLegacyFunctionDeclaration();
        func->setAsync(isAsync);
        func->setTest(isTest);
        func->setBench(isBench);
        if (isConstExpr) func->setConstExpr(true);
        if (isConstEval) func->setConstEval(true);
        if (isConstInit) throw ParseError("sobit_boshlangich faqat o'zgaruvchilarga qo'llaniladi");
        if (isNoDiscard) func->setNoDiscard(true);
        if (isDeprecated) func->setDeprecated(true);
        return func;
    }
    
    // o'zgarmas TYPE NAME → const TYPE NAME (mirror of local parser)
    bool isGlobalConst = false;
    if (!isAtEnd() && peek().value == "o'zgarmas") {
        isGlobalConst = true;
        advance();
    }

    // C-Style disambiguation: <Type> <Name>
    if (looksLikeDeclHelper(tokens_, current_)) {
        std::string typeName = parseTypeString();
        if (isGlobalConst) typeName = "o'zgarmas " + typeName;
        std::string name = advance().value;

        if (name == "operator" && !isAtEnd() && peek().type == TokenType::Symbol) {
            name += advance().value;
            if (name == "operator(" && !isAtEnd() && peek().value == ")") name += advance().value;
            else if (name == "operator[" && !isAtEnd() && peek().value == "]") name += advance().value;
        }

        if (peek().type == TokenType::Symbol && peek().value == "(") {
                // Could be:
                // 1. Function declaration: Type Name(Type param1, Type param2) {...}
                // 2. Variable with constructor: Type Name(ctor_args) {...}
                // 
                // Disambiguate by looking ahead to see if we find '{' or ';'
                // If we find '{', it's likely a function. If ';', it's a variable.
                // Try function first; if it fails, parseVariableDeclaration handles ctor init
                
                std::size_t peekPos = current_ + 1; // skip '('
                int parenDepth = 1;
                bool foundBrace = false;
                
                while (peekPos < tokens_.size() && parenDepth > 0) {
                    if (tokens_[peekPos].value == "(") parenDepth++;
                    else if (tokens_[peekPos].value == ")") {
                        parenDepth--;
                        if (parenDepth == 0) {
                            // Look past optional post-paren modifiers:
                            // - trailing return type: -> Type...
                            // - function specifiers: xato_tashlamaydi/noexcept, o'zgarmas/const, ustidan_yozish
                            std::size_t afterParen = peekPos + 1;
                            while (afterParen < tokens_.size()) {
                                if (tokens_[afterParen].value == "->") {
                                    // Skip trailing return type: -> Type...
                                    // Handle complex types with <>, (), etc.
                                    afterParen++;
                                    int templateDepth = 0;
                                    int parenInType = 0;
                                    while (afterParen < tokens_.size()) {
                                        const std::string& tok = tokens_[afterParen].value;
                                        if (tok == "<") templateDepth++;
                                        else if (tok == ">") templateDepth--;
                                        else if (tok == "(") parenInType++;
                                        else if (tok == ")") {
                                            if (parenInType == 0) break; // End of return type
                                            parenInType--;
                                        } else if (tok == "{" || tok == ";") {
                                            break; // End of return type
                                        }
                                        
                                        // Stop if we see function modifiers at top level
                                        if (templateDepth == 0 && parenInType == 0 &&
                                            (tok == "xato_tashlamaydi" || tok == "o'zgarmas" ||
                                             tok == "ustidan_yozish")) {
                                            break;
                                        }
                                        afterParen++;
                                    }
                                    continue;
                                } else if (tokens_[afterParen].value == "xato_tashlamaydi" ||
                                           tokens_[afterParen].value == "o'zgarmas" ||
                                           tokens_[afterParen].value == "ustidan_yozish") {
                                    afterParen++;
                                    continue;
                                } else {
                                    break;
                                }
                            }
                            if (afterParen < tokens_.size() && tokens_[afterParen].value == "{") {
                                foundBrace = true;
                            }
                            break;
                        }
                    }
                    peekPos++;
                }
                
                if (foundBrace) {
                    // Looks like a function declaration
                    try {
                        auto func = parseFunctionDeclaration(typeName, name);
                        func->setAsync(isAsync);
                    func->setTest(isTest);
                    func->setBench(isBench);
                    if (isConstExpr) func->setConstExpr(true);
                    if (isConstEval) func->setConstEval(true);
                    if (isConstInit) throw ParseError("sobit_boshlangich faqat o'zgaruvchilarga qo'llaniladi");
                    if (isNoDiscard) func->setNoDiscard(true);
                    if (isDeprecated) func->setDeprecated(true);
                        return func;
                    } catch (const ParseError&) {
                        // Fall through to variable declaration with constructor
                    }
                }

                if (isAsync) throw ParseError("O'zgaruvchini asinxron qilib bo'lmaydi");
                if (isConstEval) throw ParseError("sobit_baholash faqat funksiyalarga qo'llaniladi");
                auto varDecl = parseVariableDeclaration(typeName, name);
                if (isConstExpr) varDecl->setConstExpr(true);
                if (isConstInit) varDecl->setConstInit(true);
                if (isInlineVar) varDecl->setInline(true);

                if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ";") {
                    advance();
                }

                return varDecl;
        } else {
            if (isAsync) throw ParseError("O'zgaruvchini asinxron qilib bo'lmaydi");
            auto varDecl = parseVariableDeclaration(typeName, name, isConstExpr, isConstEval, isConstInit);
            if (isInlineVar) varDecl->setInline(true);

            if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ";") {
                advance();
            }

            return varDecl;
        }
    }
    
    if (isAsync || isTest) throw ParseError("Faqat funktsiyalar asinxron yoki @sinov bo'lishi mumkin");
    return parseDeclarationOrExpressionStatement();
}

std::unique_ptr<NamespaceDeclaration> Parser::parseNamespaceDeclaration() {
    const Token token = advance(); // past 'nomlar_fazosi'

    // Anonymous namespace: `nomlar_fazosi { ... }` — empty name = file-scope privacy
    std::string name;
    if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "{") {
        // anonymous — leave name empty
    } else {
        name = advance().value;
        // Support C++ style nested namespaces (e.g. uzpp::Xavfsizlik)
        while (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "::") {
            advance();
            name += "::" + advance().value;
        }
    }

    if (peek().type == TokenType::Symbol && peek().value == ";") {
        advance(); // "nomlar_fazosi X;" -> "using namespace X;"
        Token usingTok = token;
        usingTok.value = "using namespace " + name + ";";
        return std::make_unique<NamespaceDeclaration>(name, std::vector<std::unique_ptr<ASTNode>>{}, token);
    }

    // Namespace alias: nomlar_fazosi Qisqa = Uzun::Ichki::Nom;  →  namespace Qisqa = Uzun::Ichki::Nom;
    if (!name.empty() && peek().type == TokenType::Symbol && peek().value == "=") {
        advance(); // consume '='
        std::string target;
        while (!isAtEnd() && peek().type == TokenType::Identifier) {
            target += advance().value;
            if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "::") {
                target += advance().value; // "::"
            } else {
                break;
            }
        }
        if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ";") advance();
        auto decl = std::make_unique<NamespaceDeclaration>(name, std::vector<std::unique_ptr<ASTNode>>{}, token);
        decl->setAliasTarget(target);
        return decl;
    }

    // Traditional bracketed namespace
    if (peek().value != "{") throw ParseError("Kutilgan '{' " + formatLocation(peek()));
    advance();
    
    std::vector<std::unique_ptr<ASTNode>> globals;
    while (!isAtEnd() && peek().value != "}") {
        globals.push_back(parseGlobalDeclaration());
    }
    advance(); // '}'
    
    return std::make_unique<NamespaceDeclaration>(name, std::move(globals), token);
}

std::unique_ptr<IncludeStatement> Parser::parseIncludeStatement() {
    const Token token = advance(); // past 'ulash' or '#include'
    std::string moduleName;
    // Angle-form `ulash <header>` — assemble from tokens since lexer splits '<', name, '>'.
    if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "<") {
        moduleName = advance().value; // '<'
        while (!isAtEnd() && peek().value != ">") {
            moduleName += peek().value;
            advance();
        }
        if (!isAtEnd()) moduleName += advance().value; // '>'
    } else {
        moduleName = advance().value;
    }

    // Path-traversal va xavfsizlik tekshiruvi.
    // moduleName lekserdan tirnoqlar bilan kelishi mumkin ("tarmoq.hpp"), shuning uchun
    // tekshiruvdan oldin ularni olib tashlaymiz.
    auto unquoted = moduleName;
    if (unquoted.size() >= 2 && unquoted.front() == '"' && unquoted.back() == '"') {
        unquoted = unquoted.substr(1, unquoted.size() - 2);
    }
    auto rejectInclude = [&](const std::string& reason) {
        throw ParseError("Xavfsiz bo'lmagan ulash yo'li (" + reason + "): \"" + unquoted + "\" " + formatLocation(token));
    };
    if (unquoted.empty()) {
        rejectInclude("bo'sh");
    }
    // <iostream> kabi tizimli sarlavhalar uchun istisno: ular `<...>` ichida bo'ladi
    // va kompilyatorga to'g'ridan-to'g'ri uzatiladi, fayl tizimiga aralashmaydi.
    const bool isAngleHeader = unquoted.front() == '<' && unquoted.back() == '>';
    if (!isAngleHeader) {
        if (unquoted.find("..") != std::string::npos) {
            rejectInclude("yuqoriga chiqish (..) ruxsat etilmaydi");
        }
        if (!unquoted.empty() && (unquoted.front() == '/' || unquoted.front() == '\\')) {
            rejectInclude("mutlaq yo'l ruxsat etilmaydi");
        }
        if (unquoted.size() >= 2 && unquoted[1] == ':') {
            rejectInclude("disk harfi bilan boshlanadigan yo'l ruxsat etilmaydi");
        }
        for (char ch : unquoted) {
            const auto uc = static_cast<unsigned char>(ch);
            if (uc < 0x20 || uc == 0x7f) {
                rejectInclude("boshqarish belgisi");
            }
        }
    }

    // If it's something like ulash "tarmoq";
    if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ";") {
        advance();
    }
    return std::make_unique<IncludeStatement>(moduleName, token);
}

std::unique_ptr<LinkStatement> Parser::parseLinkStatement() {
    const Token token = advance(); // past 'ulash_kutubxona'
    if (isAtEnd() || peek().type != TokenType::StringLiteral) {
        throw ParseError("Kutilgan kutubxona nomi (satr) " + formatLocation(peek()));
    }
    std::string libName = advance().value;
    libName = libName.substr(1, libName.length() - 2); // strip quotes
    
    if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ";") {
        advance();
    }
    return std::make_unique<LinkStatement>(libName, token);
}

std::unique_ptr<VariableDeclaration> Parser::parseVariableDeclaration(const std::string& typeName, const std::string& varName, bool isConstExpr, bool isConstEval, bool isConstInit) {
    // asosiy/main — ruxsat etilgan funksiya nomlari, o'zgaruvchi sifatida emas.
    // yangi/bosh — alias kalit so'zlar: parser lookahead (yangi → new) yoki
    // codegen localScopes_ orqali soyalash qo'llab-quvvatlanadi.
    static const std::vector<std::string> shadowableKeywords{
        "asosiy", "main", "yangi", "bosh"
    };
    bool isShadowable = false;
    for (const auto& kw : shadowableKeywords) {
        if (varName == kw) { isShadowable = true; break; }
    }
    if (!isShadowable && isUzbekKeyword(varName)) {
        throw ParseError("Kalit so'z o'zgaruvchi nomi sifatida ishlatilishi mumkin emas: `" + varName + "`");
    }

    Token token;
    token.type = TokenType::Identifier;
    token.value = varName;
    token.line = previous().line;
    token.column = previous().column;

    // C-style array dimension: butun arr[10];
    std::string arraySize;
    if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "[") {
        advance(); // consume '['
        if (!isAtEnd() && peek().type == TokenType::IntegerLiteral) {
            arraySize = advance().value;
        }
        if (isAtEnd() || peek().value != "]") {
            throw ParseError("Kutilgan ']' massiv hajmidan keyin " + formatLocation(peek()));
        }
        advance(); // consume ']'
    }

    std::unique_ptr<Expression> init = nullptr;
    if (peek().type == TokenType::Symbol && peek().value == "=") {
        advance();
        init = parseExpression();
    } else if (peek().type == TokenType::Symbol && peek().value == "(") {
        // Constructor initialization: Type Name(args...)
        advance(); // consume '('
        
        // Parse constructor arguments as a function call-like expression
        // For now, we'll create a function call expression with the type name as the function
        std::vector<std::unique_ptr<Expression>> args;
        
        while (!isAtEnd() && !(peek().type == TokenType::Symbol && peek().value == ")")) {
            args.push_back(parseExpression());
            
            if (peek().type == TokenType::Symbol && peek().value == ",") {
                advance(); // consume ','
            } else if (!(peek().type == TokenType::Symbol && peek().value == ")")) {
                throw ParseError("Kutilgan ',' yoki ')' konstruktor argumentlarida " + formatLocation(peek()));
            }
        }
        
        if (isAtEnd() || peek().value != ")") {
            throw ParseError("Kutilgan ')' konstruktor oxirida " + formatLocation(peek()));
        }
        advance(); // consume ')'
        
        // Create a function call expression for the constructor
        Token ctorToken;
        ctorToken.type = TokenType::Identifier;
        ctorToken.value = typeName;
        ctorToken.line = token.line;
        ctorToken.column = token.column;
        
        auto ctorExpr = std::make_unique<IdentifierExpression>(typeName, ctorToken);
        init = std::make_unique<FunctionCall>(std::move(ctorExpr), std::move(args), ctorToken);
    }
    
    // NOTE: Do NOT consume semicolon here - let callers decide based on context
    // Global declarations and statements have different semicolon requirements
    
    auto decl = std::make_unique<VariableDeclaration>(varName, typeName, std::move(init), token, false);
    decl->setArraySize(arraySize);
    if (isConstExpr) decl->setConstExpr(true);
    if (isConstInit) decl->setConstInit(true);
    (void)isConstEval; // consteval applies to functions, not variables
    return decl;
}

std::unique_ptr<FunctionDeclaration> Parser::parseLegacyFunctionDeclaration() {
    const Token funcToken = advance(); // consume 'funksiya'
    std::string funcName = advance().value;
    return parseFunctionDeclaration("void", funcName);
}

std::unique_ptr<FunctionDeclaration> Parser::parseFunctionDeclaration(const std::string& returnType, const std::string& funcName, bool isConstExpr, bool isConstEval) {
    Token funcToken;
    funcToken.value = funcName;
    funcToken.line = previous().line;
    funcToken.column = previous().column;
    funcToken.type = TokenType::Identifier;
    
    std::vector<FunctionDeclaration::Parameter> params = parseFunctionParameters();

    // C++11 trailing return type: funksiya foo(...) -> butun { ... }
    // Also works for auto-deduced abbreviated function templates.
    std::string finalReturnType = returnType;
    bool hasTrailingReturn = false;
    if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "->") {
        advance(); // consume '->'
        finalReturnType = parseTypeString();
        hasTrailingReturn = true;
    }

    bool isNoExcept = false;
    if (checkKeyword("xato_tashlamaydi")) {
        isNoExcept = true;
        advance();
    }

    // C++20 trailing requires-clause: shart (cond) before body
    std::string requiresClause;
    if (checkKeyword("shart")) {
        advance(); // 'shart'
        if (isAtEnd() || peek().value != "(") throw ParseError("Kutilgan '(' shart dan keyin");
        advance();
        int depth = 1;
        while (!isAtEnd() && depth > 0) {
            const std::string& v = peek().value;
            if (v == "(") depth++;
            else if (v == ")") { depth--; if (depth == 0) break; }
            requiresClause += peek().value + " ";
            advance();
        }
        if (!isAtEnd()) advance(); // ')'
    }

    if (isAtEnd() || peek().value != "{") {
        throw ParseError("Kutilgan funktsiya tanasi '{' " + formatLocation(peek()));
    }

    auto body = parseBlock();

    auto funcDecl = std::make_unique<FunctionDeclaration>(funcName, finalReturnType, std::move(params),
                                                 std::move(body), funcToken, false, false, false, isConstExpr, isConstEval);
    funcDecl->setNoExcept(isNoExcept);
    if (hasTrailingReturn) funcDecl->setTrailingReturn(true);
    if (!requiresClause.empty()) funcDecl->setRequiresClause(requiresClause);
    return funcDecl;
}

std::vector<FunctionDeclaration::Parameter> Parser::parseFunctionParameters() {
    std::vector<FunctionDeclaration::Parameter> params;
    
    if (isAtEnd() || peek().value != "(") {
        throw ParseError("Kutilgan '(' parametrlar oldida " + formatLocation(peek()));
    }
    
    advance(); // consume '('
    
    while (!isAtEnd() && !(peek().type == TokenType::Symbol && peek().value == ")")) {
        if (peek().type != TokenType::Identifier) {
            throw ParseError("Kutilgan parametr turi " + formatLocation(peek()));
        }
        
        FunctionDeclaration::Parameter param;
        
        // Check for explicit object parameter (C++23 deducing this): oz or bosh
        if (params.empty() && (peek().value == "oz" || peek().value == "bosh")) {
            param.isExplicitObject = true;
            param.name = advance().value; // consume "oz" or "bosh"
            param.token = previous();
            // After oz/bosh, expect reference type (e.g., oz Klass&& self)
            if (!isAtEnd() && peek().type == TokenType::Identifier) {
                param.type = parseTypeString();
                if (!isAtEnd() && peek().type == TokenType::Identifier &&
                    peek().value != "," && peek().value != ")") {
                    param.name = advance().value; // consume actual parameter name
                    param.token = previous();
                }
            }
        } else {
            if (peek().value == "o'zgarmas") {
                param.isConst = true;
                advance();
            }
            
            param.type = parseTypeString();
            
            // Second identifier = name (if present)
            if (!isAtEnd() && peek().type == TokenType::Identifier &&
                peek().value != "," && peek().value != ")") {
                param.name = advance().value;
                param.token = previous();
            } else {
                // Unnamed parameter — use auto-generated name
                param.name = "_p" + std::to_string(params.size());
                param.token = previous();
            }
        }

        // Massiv parametri: `belgi* argv[]`, `butun jadval[10]`.
        // C++ da massiv parametri ko'rsatkichga aylanadi — turga `*` qo'shamiz
        // (`belgi* argv[]` → `char**`), o'lcham e'tiborga olinmaydi.
        while (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "[") {
            advance(); // '['
            int bDepth = 1;
            while (!isAtEnd() && bDepth > 0) {
                if (peek().value == "[") bDepth++;
                else if (peek().value == "]") bDepth--;
                if (bDepth > 0) advance();
            }
            if (isAtEnd() || peek().value != "]") {
                throw ParseError("Kutilgan ']' massiv parametridan keyin " + formatLocation(peek()));
            }
            advance(); // ']'
            param.type += "*";
        }
        
        // Standart qiymat: butun son = 0
        if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "=") {
            advance(); // consume '='
            // Collect the default value expression as a token string
            // Handle nested parens/brackets/angles
            std::string defVal;
            int pDepth = 0;
            while (!isAtEnd()) {
                const std::string& v = peek().value;
                if (v == "(" || v == "[" || v == "{") { pDepth++; defVal += advance().value; }
                else if (v == ")" || v == "]" || v == "}") {
                    if (pDepth == 0) break;
                    pDepth--;
                    defVal += advance().value;
                }
                else if (v == "," && pDepth == 0) break;
                else defVal += advance().value;
            }
            param.defaultValue = defVal;
        }

        if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ",") {
            advance();
        }

        params.push_back(param);
    }

    if (isAtEnd() || peek().value != ")") {
        throw ParseError("Kutilgan ')' " + formatLocation(peek()));
    }

    advance(); // consume ')'

    return params;
}


// Asosiy sinf nomini o'qiydi. Quyidagilarni qabul qiladi:
//   Foo                      — oddiy nom
//   ochiq Foo                — kirish darajasi bilan (C++ `public Foo`)
//   std::runtime_error       — to'liq malakalangan nom
//   Baza<butun>              — shablon instansiyasi
std::string Parser::parseBaseClassName() {
    // Ixtiyoriy kirish darajasi — C++ da `public`/`protected`/`private`.
    // uz++ da meros doim ochiq, shuning uchun faqat o'tkazib yuboramiz.
    if (!isAtEnd() && peek().type == TokenType::Identifier &&
        (peek().value == "ochiq" || peek().value == "himoyalangan" || peek().value == "yopiq")) {
        advance();
    }

    if (isAtEnd() || peek().type != TokenType::Identifier) {
        throw ParseError("Kutilgan asosiy sinf yoki interfeys nomi " + formatLocation(peek()));
    }

    std::string name = advance().value;

    // To'liq malakalangan nom: A::B::C
    while (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "::") {
        advance();
        if (isAtEnd() || peek().type != TokenType::Identifier) {
            throw ParseError("Kutilgan identifikator '::' dan keyin " + formatLocation(peek()));
        }
        name += "::" + advance().value;
    }

    // Shablon argumentlari: Baza<butun, matn>
    if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "<") {
        std::string args;
        int depth = 0;
        do {
            const std::string& v = peek().value;
            if (v == "<") depth++;
            else if (v == ">") depth--;
            else if (v == ">>") depth -= 2;
            args += advance().value;
        } while (!isAtEnd() && depth > 0);
        name += args;
    }

    return name;
}

std::unique_ptr<ClassDeclaration> Parser::parseClassDeclaration() {
    const Token classToken = advance(); // consume 'sinf' / 'tuzilma' / 'birlashma'

    // Anonymous union/struct allowed: `birlashma { butun i; haqiqiy f; };`
    std::string className;
    if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "{") {
        // anonymous — no name; codegen emits `union { ... };` without a tag
    } else {
        if (isAtEnd() || peek().type != TokenType::Identifier) {
            throw ParseError("Kutilgan sinf nomi " + formatLocation(peek()));
        }
        className = peek().value;
        advance();
    }
    
    std::string baseClass;
    std::vector<std::string> interfaces;
    if (matchKeyword("meros") || (peek().type == TokenType::Symbol && peek().value == ":")) {
        if (peek().value == ":") advance(); // consume ':'
        baseClass = parseBaseClassName();
        // Multiple inheritance: meros A, B, C { ... }  → first is base, rest are interfaces.
        while (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ",") {
            advance(); // ','
            interfaces.push_back(parseBaseClassName());
        }
    }

    if (matchKeyword("amalga_oshirish")) {
        do {
            if (isAtEnd() || peek().type != TokenType::Identifier) {
                throw ParseError("Kutilgan interfeys nomi " + formatLocation(peek()));
            }
            interfaces.push_back(advance().value);
            if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ",") {
                advance(); // consume ','
            } else {
                break;
            }
        } while (!isAtEnd());
    }

    // NB: a "trailing" `shart (cond)` after the class header is NOT valid
    // C++20 — `class X requires C { };` is rejected by the compiler.
    // Class-template constraints belong in the template wrapper:
    //   shablon<tur T> shart (cond) sinf Foo { ... }
    // which the template parser at parser.cpp:~1994 already handles by
    // emitting `template <typename T> requires (cond)\n class Foo`.

    if (isAtEnd() || peek().value != "{") {
        throw ParseError("Kutilgan sinf tanasi '{' " + formatLocation(peek()));
    }
    
    advance(); // consume '{'
    
    std::vector<ClassDeclaration::Member> members;
    std::vector<std::unique_ptr<ClassDeclaration::Method>> methods;
    std::string currentAccess = "public"; // default

    while (!isAtEnd() && !(peek().type == TokenType::Symbol && peek().value == "}")) {
        // Access specifier section: ochiq:, yopiq:, himoyalangan:
        if (peek().type == TokenType::Identifier) {
            const std::string& val = peek().value;
            if (val == "ochiq" || val == "yopiq" || val == "maxfiy" || val == "himoyalangan" || val == "shaxsiy" ||
                val == "public" || val == "private" || val == "protected") {
                std::string spec = val;
                advance(); // consume specifier
                if (!isAtEnd() && peek().value == ":") advance(); // consume ':'
                // Map to canonical C++ name
                if (spec == "ochiq" || spec == "public") currentAccess = "public";
                else if (spec == "yopiq" || spec == "maxfiy" || spec == "shaxsiy" || spec == "private") currentAccess = "private";
                else if (spec == "himoyalangan" || spec == "protected") currentAccess = "protected";
                continue;
            }
        }

        // Friend declaration: `dust sinf Foo;` or `dust bosh foo(...);`
        // Collected as raw token text and emitted verbatim ("friend ...") in codegen.
        if (peek().type == TokenType::Identifier && (peek().value == "dust" || peek().value == "friend")) {
            advance();
            std::string friendDecl = "friend ";
            // Map sinf/tuzilma to class/struct, otherwise emit raw
            while (!isAtEnd() && peek().value != ";") {
                const std::string& v = peek().value;
                if (v == "sinf") friendDecl += "class ";
                else if (v == "tuzilma") friendDecl += "struct ";
                else if (v == "bosh") friendDecl += "void ";
                else if (v == "butun") friendDecl += "int ";
                else if (v == "haqiqiy") friendDecl += "double ";
                else if (v == "matn") friendDecl += "std::string ";
                else friendDecl += v + " ";
                advance();
            }
            if (!isAtEnd()) advance(); // consume ';'
            friendDecl += ";";
            // store on the parent class via a sentinel; we add it later
            // (here we keep it local; the loop reads it via classDecl below)
            // For simplicity, push directly to a vector captured by reference
            // — but we're not in the class decl yet. Use a marker member:
            members.emplace_back();
            members.back().type = "__uzpp_friend__";  // sentinel
            members.back().name = friendDecl;          // carries the raw line
            continue;
        }

        bool isStatic = false;
        bool isMavhum = false; // pure virtual (base class abstract method)
        // Pre-method modifiers: statik, mavhum
        while (!isAtEnd() && (peek().value == "statik" || peek().value == "mavhum")) {
            if (peek().value == "statik") isStatic = true;
            if (peek().value == "mavhum") isMavhum = true;
            advance();
        }

        // Check for Constructor: ClassName ( ... )
        if (peek().type == TokenType::Identifier && peek().value == className) {
            if (current_ + 1 < tokens_.size() && tokens_[current_ + 1].value == "(") {
                auto method = std::make_unique<ClassDeclaration::Method>();
                method->returnType = ""; // Empty return type for constructor
                method->name = className;
                method->token = advance(); // consume ClassName
                
                method->params = parseFunctionParameters(); // consumes (...)
                // Optional noexcept right after parameters: ClassName(params) xato_tashlamaydi ...
                if (!isAtEnd() && peek().value == "xato_tashlamaydi") {
                    method->isNoExcept = true;
                    advance();
                }
                // Constructor initializer list: ClassName(params) : field1(val), field2(val2) { }
                if (!isAtEnd() && peek().value == ":") {
                    method->initializerList = " :";
                    advance(); // consume ':'
                    int parenDepth = 0;
                    while (!isAtEnd() && (parenDepth > 0 || (peek().value != "{" && peek().value != ";"))) {
                        const std::string& v = peek().value;
                        if (v == "(") parenDepth++;
                        else if (v == ")") parenDepth--;
                        method->initializerList += " " + v;
                        advance();
                    }
                }
                if (!isAtEnd() && peek().value == "{") {
                    method->body = parseBlock();
                } else if (!isAtEnd() && peek().value == ";") {
                    advance();
                }
                method->accessSpecifier = currentAccess;
                methods.push_back(std::move(method));
                continue;
            }
        }

        // Check for Destructor: ~ ClassName ( ... )
        if (peek().type == TokenType::Symbol && peek().value == "~") {
            if (current_ + 1 < tokens_.size() && tokens_[current_ + 1].value == className) {
                auto method = std::make_unique<ClassDeclaration::Method>();
                method->returnType = "";
                advance(); // consume '~'
                method->name = "~" + advance().value; // consume ClassName
                method->token = previous();
                // mavhum ~Sinf() — virtual destructor
                if (isMavhum) method->isVirtual = true;

                method->params = parseFunctionParameters();
                if (!isAtEnd() && peek().value == "xato_tashlamaydi") {
                    method->isNoExcept = true;
                    advance();
                }
                // ustidan_yozish (override) after dtor params
                if (!isAtEnd() && peek().value == "ustidan_yozish") {
                    method->isVirtual = true;
                    advance();
                }
                if (!isAtEnd() && peek().value == "{") {
                    method->body = parseBlock();
                } else if (!isAtEnd() && peek().value == "=") {
                    advance(); // '='
                    if (!isAtEnd() && peek().value == "0") {
                        method->isPureVirtual = true;
                        method->isVirtual = true;
                        advance();
                    } else if (!isAtEnd() && peek().value == "default") {
                        method->isDefaulted = true;
                        advance();
                    } else if (!isAtEnd() && peek().value == "delete") {
                        method->isDeleted = true;
                        advance();
                    }
                    if (!isAtEnd() && peek().value == ";") advance();
                } else if (!isAtEnd() && peek().value == ";") {
                    advance();
                }
                method->accessSpecifier = currentAccess;
                methods.push_back(std::move(method));
                continue;
            }
        }

        // Optional `@noyob_manzil` attribute on a member field (C++20 [[no_unique_address]])
        bool hasNoUniqueAddr = false;
        if (peek().type == TokenType::Symbol && peek().value == "@" &&
            current_ + 1 < tokens_.size() && tokens_[current_ + 1].value == "noyob_manzil") {
            advance(); advance();
            hasNoUniqueAddr = true;
        }

        // `funksiya` keyword form: `funksiya name(params) [modifiers] [-> T] { ... }`.
        // This is the syntactic twin of free-function declarations and also
        // supports `o'zgarmas`/`ustidan_yozish`/etc plus an optional C++11
        // trailing return type — something the legacy `T name() o'zgarmas { ... }`
        // shape can't express together with `-> T`.
        if (checkKeyword("funksiya")) {
            advance(); // consume `funksiya`
            if (isAtEnd() || peek().type != TokenType::Identifier) {
                throw ParseError("Kutilgan metod nomi `funksiya` dan keyin " + formatLocation(peek()));
            }
            const std::string name = advance().value;

            auto method = std::make_unique<ClassDeclaration::Method>();
            method->returnType = "void"; // default unless trailing-return specifies otherwise
            method->name = name;
            method->token = previous();
            method->params = parseFunctionParameters();
            method->isStatic = isStatic;

            while (!isAtEnd() && (peek().value == "ustidan_yozish" || peek().value == "mavhum" ||
                                  peek().value == "o'zgarmas"     || peek().value == "xato_tashlamaydi")) {
                if (peek().value == "ustidan_yozish") method->isVirtual = true;
                if (peek().value == "mavhum")        method->isPureVirtual = true;
                if (peek().value == "o'zgarmas")     method->isConstMethod = true;
                if (peek().value == "xato_tashlamaydi") method->isNoExcept = true;
                advance();
            }
            if (!isAtEnd() && peek().type == TokenType::Symbol &&
                (peek().value == "&" || peek().value == "&&")) {
                method->refQualifier = advance().value;
            }
            if (!isAtEnd() && peek().value == "->") {
                advance();
                method->returnType = parseTypeString();
            }
            if (isMavhum) method->isPureVirtual = true;

            if (!isAtEnd() && peek().value == "{") {
                method->body = parseBlock();
            } else if (!isAtEnd() && peek().value == "=") {
                advance();
                if (!isAtEnd() && peek().value == "0") {
                    method->isPureVirtual = true;
                    advance();
                } else if (!isAtEnd() && peek().value == "default") {
                    method->isDefaulted = true;
                    advance();
                } else if (!isAtEnd() && peek().value == "delete") {
                    method->isDeleted = true;
                    advance();
                }
                if (!isAtEnd() && peek().value == ";") advance();
            } else if (!isAtEnd() && peek().value == ";") {
                advance();
            }
            method->accessSpecifier = currentAccess;
            methods.push_back(std::move(method));
            continue;
        }

        // Method or Field: Type Name ...
        if (looksLikeDeclHelper(tokens_, current_)) {
            std::string typeName = parseTypeString();
            std::string name = advance().value;
            
            if (name == "operator" && !isAtEnd() && peek().type == TokenType::Symbol) {
                name += advance().value;
                if (name == "operator(" && !isAtEnd() && peek().value == ")") name += advance().value;
                else if (name == "operator[" && !isAtEnd() && peek().value == "]") name += advance().value;
            }
            
            if (isStatic) {
                // Mark with "static " prefix in typeName only for fields
                // For methods, isStatic is set on the method struct itself
                if (isAtEnd() || peek().value != "(") typeName = "inline static " + typeName;
            }

            // Check for C-style array syntax: data[10]
            std::string arraySize;
            if (!isAtEnd() && peek().value == "[") {
                advance(); // consume '['
                if (!isAtEnd() && (peek().type == TokenType::IntegerLiteral)) {
                    arraySize = peek().value;
                    advance(); // consume size
                }
                if (!isAtEnd() && peek().value == "]") {
                    advance(); // consume ']'
                }
            }

            if (!isAtEnd() && peek().value == "(") {
                // Parse Method
                auto method = std::make_unique<ClassDeclaration::Method>();
                method->returnType = typeName;
                method->name = name;
                method->token = previous();
                
                method->params = parseFunctionParameters();
                method->isStatic = isStatic;

                // Post-params modifiers: o'zgarmas (const method), ustidan_yozish (override), mavhum (pure virtual), xato_tashlamaydi (noexcept)
                while (!isAtEnd() && (peek().value == "ustidan_yozish" || peek().value == "mavhum" || peek().value == "o'zgarmas" || peek().value == "xato_tashlamaydi")) {
                    if (peek().value == "ustidan_yozish") method->isVirtual = true;
                    if (peek().value == "mavhum") method->isPureVirtual = true;
                    if (peek().value == "o'zgarmas") method->isConstMethod = true;
                    if (peek().value == "xato_tashlamaydi") method->isNoExcept = true;
                    advance();
                }
                // C++11 ref-qualifier on method: void f() & / void f() &&
                if (!isAtEnd() && peek().type == TokenType::Symbol &&
                    (peek().value == "&" || peek().value == "&&")) {
                    method->refQualifier = advance().value;
                }
                if (isMavhum) method->isPureVirtual = true;

                if (!isAtEnd() && peek().value == "{") {
                    method->body = parseBlock();
                } else if (!isAtEnd() && peek().value == "=") {
                    advance(); // '='
                    if (!isAtEnd() && peek().value == "0") {
                        advance(); // pure virtual: = 0
                    } else if (!isAtEnd() && peek().value == "default") {
                        method->isDefaulted = true;
                        advance();
                    } else if (!isAtEnd() && peek().value == "delete") {
                        method->isDeleted = true;
                        advance();
                    }
                    if (!isAtEnd() && peek().value == ";") advance();
                } else if (!isAtEnd() && peek().value == ";") {
                    advance();
                }
                method->accessSpecifier = currentAccess;
                methods.push_back(std::move(method));
            } else {
                // Parse Field
                auto member = ClassDeclaration::Member();
                member.type = typeName;
                member.name = name;
                member.arraySize = arraySize; // For C-style arrays
                member.accessSpecifier = currentAccess;
                member.hasNoUniqueAddress = hasNoUniqueAddr;

                // Bitfield syntax: <type> <name> : <width>;
                if (member.arraySize.empty() && !isAtEnd() && peek().type == TokenType::Symbol && peek().value == ":") {
                    advance(); // consume ':'
                    if (isAtEnd() || peek().type != TokenType::IntegerLiteral) {
                        throw ParseError("Kutilgan bit kengligi (butun son) " + formatLocation(peek()));
                    }
                    member.bitWidth = advance().value;
                }

                // Standart qiymat: `butun soni_ = 42;` yoki `matn nomi_ = "x";`
                if (!isAtEnd() && peek().value == "=") {
                    advance(); // '='
                    member.defaultValue = std::shared_ptr<Expression>(parseExpression().release());
                } else if (!isAtEnd() && peek().value == "{") {
                    // Qavsli initsializatsiya: `vektor<butun> v {1, 2, 3};`
                    member.defaultValue = std::shared_ptr<Expression>(parseExpression().release());
                }

                members.push_back(std::move(member));
                if (!isAtEnd() && peek().value == ";") {
                    advance(); // consume ';'
                }
            }
            continue;
        }
        
        // Skip unhandled tokens to avoid infinite loops
        advance();
    }
    
    if (isAtEnd() || peek().value != "}") {
        throw ParseError("Kutilgan '}' " + formatLocation(peek()));
    }
    
    advance(); // consume '}'
    
    auto cls = std::make_unique<ClassDeclaration>(className, baseClass, std::move(interfaces), std::move(members),
                                              std::move(methods), classToken);
    return cls;
}

std::unique_ptr<InterfaceDeclaration> Parser::parseInterfaceDeclaration() {
    const Token interfaceToken = advance(); // consume 'shartnoma'
    
    if (isAtEnd() || peek().type != TokenType::Identifier) {
        throw ParseError("Kutilgan interfeys nomi " + formatLocation(peek()));
    }
    
    std::string name = advance().value;
    
    if (isAtEnd() || peek().value != "{") {
        throw ParseError("Kutilgan '{' interfeys tanasidan oldin " + formatLocation(peek()));
    }
    advance(); // consume '{'
    
    std::vector<std::unique_ptr<InterfaceDeclaration::MethodSignature>> methods;
    
    while (!isAtEnd() && peek().value != "}") {
        if (peek().type != TokenType::Identifier && peek().type != TokenType::Symbol) {
            throw ParseError("Kutilgan metod qaytarish turi " + formatLocation(peek()));
        }
        
        auto method = std::make_unique<InterfaceDeclaration::MethodSignature>();
        method->returnType = advance().value;
        
        if (isAtEnd() || peek().type != TokenType::Identifier) {
            throw ParseError("Kutilgan metod nomi " + formatLocation(peek()));
        }
        method->token = peek();
        method->name = advance().value;
        
        method->params = parseFunctionParameters(); 
        
        if (isAtEnd() || peek().value != ";") {
            throw ParseError("Kutilgan ';' metod signaturasi oxirida " + formatLocation(peek()));
        }
        advance(); // consume ';'
        
        methods.push_back(std::move(method));
    }
    
    if (isAtEnd() || peek().value != "}") {
        throw ParseError("Kutilgan '}' interfeys oxirida " + formatLocation(peek()));
    }
    advance(); // consume '}'
    
    return std::make_unique<InterfaceDeclaration>(name, std::move(methods), interfaceToken);
}

// Phase 12: Xatolikni yig'ish va sinxronizatsiya.
void Parser::recordError(const std::string& msg, const Token& token) {
    // Ko'p ParseError xabarlari o'z ichida allaqachon joylashuvni olib yuradi
    // (`... qator: 5 ustun: 1`). Ikkinchi marta qo'shsak, xabar takrorlanadi.
    if (msg.find("qator:") != std::string::npos) {
        errors_.push_back(msg);
        return;
    }
    errors_.push_back(msg + " " + formatLocation(token));
}

// Phase 2.5: Eskirgan sinonimlar — har bir C++ tushunchasi uchun bitta
// kanonik uz++ so'z bor. Bu xarita ikki turli kontekstga bo'lingan:
//
//   * `typeContextOnly` — faqat tur kontekstida ishlatiladigan sinonimlar.
//     Foydalanuvchi `umumiy`, `yagona`, `to'plam` ni o'zgaruvchi nomi sifatida
//     ishlatishi mumkin — shu sababli, biz faqat `parseTypeString()` da
//     tekshiramiz, ifoda/operator boshlanishida emas.
//
//   * `keywordContext` — kalit so'z sifatida ishlatiladigan, oddiy
//     o'zgaruvchi nomi bo'la olmaydigan sinonimlar. Bularni har joyda
//     tekshirsa bo'ladi.

static const std::map<std::string, std::string>& typeOnlyDeprecated() {
    static const std::map<std::string, std::string> m{
        {"mantiq",          "mantiqiy"},          // bool
        {"ikkilangan",      "haqiqiy"},           // double
        {"to'plam",         "vektor"},            // std::vector
        {"hesh_jadval",     "lug'at"},            // std::unordered_map
        {"hesh_xarita",     "lug'at"},            // std::unordered_map
        {"o'ziga_xos",      "tartib_to'plam"},    // std::set
        {"o'n",             "ikki_tomonlama_navbat"}, // std::deque
        {"yagona",          "yagona_korsatkich"}, // std::unique_ptr
        {"umumiy",          "umumiy_korsatkich"}, // std::shared_ptr
        {"aqlli_korsatkich","umumiy_korsatkich"}, // std::shared_ptr
        {"nozik_qulf",      "umumiy_qulf"},       // std::shared_mutex
        {"filter",          "filtr"},             // std::views::filter
        {"saralash",        "tartibla"},          // std::sort
        {"yigish",          "to'plash"},          // std::ranges::to
    };
    return m;
}

static const std::map<std::string, std::string>& keywordContextDeprecated() {
    static const std::map<std::string, std::string> m{
        // Apostrofsiz shakllar — yagona kanonik shakl apostrofli
        {"ozgaruvchan",     "o'zgaruvchan"},      // auto
        {"ozgarmas",        "o'zgarmas"},         // const
        {"ochirish",        "o'chirish"},         // delete
        // Boshqaruv kalit so'zlari
        {"aks",             "aks_holda"},         // else
        {"qaytish",         "qaytarish"},         // return
        // Oqim identifikatorlari (kalit so'z sifatida)
        {"chiqarish",       "yozish"},            // std::cout
        {"olish",           "kiritish"},          // std::cin
        // Tuzilma sinonimi (rus tilidan)
        {"straktura",       "tuzilma"},           // struct
    };
    return m;
}

void Parser::checkDeprecatedSynonym() const {
    if (isAtEnd() || peek().type != TokenType::Identifier) return;
    // Tur kontekstida — faqat parseTypeString chaqirsa, tur sinonimlarini
    // ham tekshiramiz. Boshqa kontekstda — faqat kalit so'z sinonimlari.
    const auto& kwMap = keywordContextDeprecated();
    auto it = kwMap.find(peek().value);
    if (it != kwMap.end()) {
        throw ParseError("'" + it->first + "' eskirgan sinonim — "
                         "uning o'rniga '" + it->second + "' ishlating "
                         "(Phase 2.5: bitta C++ tushunchasi = bitta uz++ so'z)");
    }
}

void Parser::checkDeprecatedTypeSynonym() const {
    if (isAtEnd() || peek().type != TokenType::Identifier) return;
    // Tur kontekstida ham kalit so'z, ham tur sinonimlarini tekshiramiz.
    checkDeprecatedSynonym();
    const auto& typeMap = typeOnlyDeprecated();
    auto it = typeMap.find(peek().value);
    if (it != typeMap.end()) {
        throw ParseError("'" + it->first + "' eskirgan sinonim — "
                         "uning o'rniga '" + it->second + "' ishlating "
                         "(Phase 2.5: bitta C++ tushunchasi = bitta uz++ so'z)");
    }
}

void Parser::synchronize() {
    errorMode_ = false;
    // Keyingi ifoda yoki deklaratsiya chegarasigacha tokenlarni o'tkazamiz.
    while (!isAtEnd()) {
        // Agar oldingi token ';' bo'lsa — biz allaqachon sinxronlashdik
        if (previous().type == TokenType::Symbol && previous().value == ";")
            return;
        // Agar joriy token ';', '}' yoki deklaratsiya kalit so'zi bo'lsa
        if (peek().type == TokenType::Symbol &&
            (peek().value == ";" || peek().value == "}"))
            return;
        // Deklaratsiya kalit so'zlari
        if (peek().type == TokenType::Identifier) {
            const std::string& kw = peek().value;
            if (kw == "sinf" || kw == "funksiya" || kw == "tuzilma" ||
                kw == "shablon" || kw == "nomlar_fazosi" || kw == "ulash" ||
                kw == "butun" || kw == "bosh" || kw == "haqiqiy" ||
                kw == "mantiqiy" || kw == "matn" || kw == "o'zgaruvchan" ||
                kw == "o'zgarmas" || kw == "agar" || kw == "uchun" ||
                kw == "toki" || kw == "moslash" || kw == "qaytarish" ||
                kw == "urinish" || kw == "statik_tasdiqlash")
                return;
        }
        advance();
    }
}

} // namespace uzpp







