#include "parser.h"

#include <sstream>
#include <unordered_set>

namespace uzpp {

namespace {
    bool looksLikeDeclHelper(const std::vector<Token>& tokens, std::size_t current) {
        std::size_t i = current;
        if (i >= tokens.size() || tokens[i].type != TokenType::Identifier) return false;
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
    while (!isAtEnd()) {
        globals.push_back(parseGlobalDeclaration());
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
        // Asosiy boshqarish oqimi
        "agar", "aks", "aks_holda", "uchun", "toki", "qaytarish", "qaytish",
        "to'xtatish", "davom_etish",
        // O'zgaruvchilar (kanonlari va qisqartmalar)
        "o'zgaruvchan", "o'zgarmas", "ozgaruvchan", "ozgarmas",
        // Sinf va funksiyalar
        "sinf", "tuzilma", "funksiya", "mavhum", "meros", "amalga_oshirish",
        "shartnoma", "shablon", "statik",
        // Modullar
        "ulash", "eksport", "import", "modul", "nomlar_fazosi",
        // Istisno boshqaruvi
        "urinish", "ushlash", "irgitish",
        // Asinxron
        "asinxron", "kutish",
        // Turlar
        "bosh", "yangi", "bekor",
        // Mantiq
        "va", "yoki", "rost", "yolg'on", "noto'g'ri",
        // Moslash
        "moslash", "holat", "boshqa",
        // Boshqalar
        "sanab_olish", "tushuncha", "shart", "makro", "ulash_kutubxona",
        "o'n", "asosiy", "yayin", "kutilish", "hammasi", "yoxud",
        "nomlari", "vazifi", "ustidan_yozish", "sikldan", "satr", "son",
        "xotira", "fayl", "yagona", "umumiy", "null"
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
           text == "/=" || text == "%=" || text == "&=" || text == "|=" || text == "^=" || text == "??" "=";
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
    return text == "<" || text == ">" || text == "<=" || text == ">=";
}

bool Parser::isAdditiveOperator(const std::string& text) const {
    return text == "+" || text == "-" || text == "<<" || text == ">>";
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
    
    while (!isAtEnd() && peek().type == TokenType::Symbol && isLogicalOrOperator(peek().value)) {
        const Token opToken = advance();
        auto right = parseLogicalAndExpression();
        left = std::make_unique<BinaryExpression>(std::move(left), opToken.value, std::move(right), opToken);
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parseLogicalAndExpression() {
    auto left = parseEqualityExpression();
    
    while (!isAtEnd() && peek().type == TokenType::Symbol && isLogicalAndOperator(peek().value)) {
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
    auto left = parseAdditiveExpression();
    
    while (!isAtEnd() && peek().type == TokenType::Symbol && isRelationalOperator(peek().value)) {
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
            // Subscript
            const Token bracketToken = advance();
            auto index = parseExpression();
            
            if (isAtEnd() || peek().value != "]") {
                throw ParseError("Kutilgan ']' " + formatLocation(peek()));
            }
            
            advance(); // consume ']'
            expr = std::make_unique<SubscriptAccess>(std::move(expr), std::move(index), bracketToken);
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
    
    const Token& current = peek();
    
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
    
    // Boolean literal (true/false)
    if (current.type == TokenType::Identifier && (current.value == "rost" || current.value == "to'g'ri" ||
            current.value == "yolg'on" || current.value == "yolgon" || current.value == "noto'g'ri")) {
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
                    if (lookahead + 1 < tokens_.size() && tokens_[lookahead + 1].value == "(") {
                        mightBeLambda = true;
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
                if (peek().value == "&") {
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
                    cap.name = advance().value;
                    cap.byRef = false;
                }
                captures.push_back(std::move(cap));
                if (!isAtEnd() && peek().value == ",") advance();
            }
            if (!isAtEnd()) advance(); // consume ']'

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

            // Body: block or expression
            std::unique_ptr<Statement> body;
            if (!isAtEnd() && peek().value == "{") {
                body = parseBlock();
            } else if (!isAtEnd() && (peek().value == "=>" || peek().value == "->")) {
                advance(); // consume '=>' or '->'
                auto retExpr = parseExpression();
                Token retTok = lambdaToken;
                body = std::make_unique<ReturnStatement>(std::move(retExpr), retTok);
            }

            return std::make_unique<LambdaExpression>(std::move(captures), std::move(params),
                                                      std::move(body), lambdaToken);
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

    // irgitish (Throw Expression) -> translates to __uzpp_throw(X)
    if (current.type == TokenType::Identifier && current.value == "irgitish") {
        Token throwToken = advance(); // consume 'irgitish'
        auto expr = parseExpression();
        
        Token funcToken = throwToken;
        funcToken.value = "__uzpp_throw";
        auto callee = std::make_unique<IdentifierExpression>("__uzpp_throw", funcToken);
        std::vector<std::unique_ptr<Expression>> args;
        args.push_back(std::move(expr));
        return std::make_unique<FunctionCall>(std::move(callee), std::move(args), throwToken);
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
                                                  std::move(body), lambdaToken);
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
            else if (tokens_[i].value == ";" || tokens_[i].value == "{" || tokens_[i].value == "}") {
                break;
            }
            // Logical/comparison operators can't appear inside template args — stop scanning
            else if (depth == 1 && (tokens_[i].value == "||" || tokens_[i].value == "&&" ||
                                    tokens_[i].value == "!=" || tokens_[i].value == "yoki" ||
                                    tokens_[i].value == "va")) {
                break;
            }
            i++;
        }
        
        if (isTemplate) {
            token.value += advance().value; // '<'
            while (peek().value != ">") {
                token.value += advance().value;
            }
            token.value += advance().value; // '>'

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
    if (checkKeyword("agar")) {
        return parseIfStatement();
    }
    
    if (checkKeyword("moslash")) {
        return parseMatchStatement();
    }

    if (checkKeyword("urinish")) {
        const Token tryToken = advance(); // consume 'urinish'
        auto tryBlock = parseBlock();
        
        struct CatchBlock {
            Token catchToken;
            std::string catchDecl;
            std::unique_ptr<Block> block;
        };
        std::vector<CatchBlock> catches;
        
        while (checkKeyword("ushlash")) {
            CatchBlock cb;
            cb.catchToken = advance(); // 'ushlash'
            if (isAtEnd() || peek().value != "(") throw ParseError("Kutilgan '(' ushlashdan keyin " + formatLocation(peek()));
            advance(); // '('
            while (!isAtEnd() && peek().value != ")") {
                cb.catchDecl += advance().value;
                if (peek().value != ")") cb.catchDecl += " ";
            }
            advance(); // ')'
            cb.block = parseBlock();
            catches.push_back(std::move(cb));
        }
        
        std::unique_ptr<Statement> elseBranch = nullptr;
        for (auto it = catches.rbegin(); it != catches.rend(); ++it) {
            Token catchCondToken = it->catchToken;
            catchCondToken.value = "__uzpp_catch " + it->catchDecl;
            auto catchCond = std::make_unique<IdentifierExpression>(catchCondToken.value, catchCondToken);
            elseBranch = std::make_unique<IfStatement>(std::move(catchCond), std::move(it->block), std::move(elseBranch), it->catchToken);
        }
        
        Token tryCondToken = tryToken;
        tryCondToken.value = "__uzpp_try";
        auto tryCond = std::make_unique<IdentifierExpression>("__uzpp_try", tryCondToken);
        return std::make_unique<IfStatement>(std::move(tryCond), std::move(tryBlock), std::move(elseBranch), tryToken);
    }
    
    if (checkKeyword("toki")) {
        return parseWhileStatement();
    }
    
    if (checkKeyword("uchun")) {
        return parseForStatement();
    }
    
    if (checkKeyword("qaytish") || checkKeyword("qaytarish")) {
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
        
        matchCase->body = parseStatement();
        cases.push_back(std::move(matchCase));
    }
    
    if (isAtEnd() || peek().value != "}") {
        throw ParseError("Kutilgan '}' moslash oxirida " + formatLocation(peek()));
    }
    advance(); // consume '}'
    
    return std::make_unique<MatchStatement>(std::move(condition), std::move(cases), matchToken);
}

std::unique_ptr<IfStatement> Parser::parseIfStatement() {
    const Token ifToken = advance(); // consume 'agar'
    
    if (isAtEnd() || peek().value != "(") {
        throw ParseError("Kutilgan '(' agar ifodasidan keyin " + formatLocation(peek()));
    }
    
    advance(); // consume '('
    auto condition = parseExpression();
    
    if (isAtEnd() || peek().value != ")") {
        throw ParseError("Kutilgan ')' " + formatLocation(peek()));
    }
    
    advance(); // consume ')'
    auto thenBranch = parseStatement();
    
    std::unique_ptr<Statement> elseBranch;
    if (matchKeyword("aks") || matchKeyword("aks_holda")) {
        // else-if: aks agar (...) or aks_holda agar (...)
        if (checkKeyword("agar")) {
            elseBranch = parseIfStatement();
        } else {
            elseBranch = parseStatement();
        }
    }

    return std::make_unique<IfStatement>(std::move(condition), std::move(thenBranch),
                                        std::move(elseBranch), ifToken);
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

std::unique_ptr<ForStatement> Parser::parseForStatement() {
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
    // Destructuring: ozgaruvchan [x, y] = ...
    if ((peek().value == "ozgaruvchan" || peek().value == "o'zgaruvchan" || peek().value == "ozgarmas" || peek().value == "o'zgarmas")
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
        "kutish", "irgitish", "yangi"
    };
    if (peek().type == TokenType::Identifier && exprOnlyKeywords.contains(peek().value)) {
        auto expr = parseExpression();
        return std::make_unique<ExpressionStatement>(std::move(expr));
    }

    if (looksLikeDeclHelper(tokens_, current_)) {
        // o'zgarmas/ozgarmas TYPE NAME -> const TYPE NAME
        bool isConst = false;
        if (peek().value == "o'zgarmas" || peek().value == "ozgarmas") {
            isConst = true;
            advance(); // consume o'zgarmas
        }

        auto parseTypeString = [this]() -> std::string {
            std::string typeStr = advance().value;
            while (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "::") {
                typeStr += advance().value;
                if (!isAtEnd()) typeStr += advance().value;
            }
            if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "<") {
                typeStr += advance().value;
                int depth = 1;
                while (!isAtEnd() && depth > 0) {
                    if (peek().value == "<") depth++;
                    else if (peek().value == ">") depth--;
                    typeStr += advance().value;
                }
            }
            while (!isAtEnd() && peek().type == TokenType::Symbol &&
                   (peek().value == "&" || peek().value == "*" || peek().value == "&&")) {
                typeStr += advance().value;
            }
            return typeStr;
        };

        std::string typeName = parseTypeString();
        if (isConst) typeName = "o'zgarmas " + typeName;
        std::string name = advance().value;
        auto varDecl = parseVariableDeclaration(typeName, name);
        return varDecl;
    }

    auto expr = parseExpression();
    
    // NOTE: Do NOT consume semicolon here - let each caller decide
    
    return std::make_unique<ExpressionStatement>(std::move(expr));
}

// ===== SEMANTIC DECLARATION PARSING =====

std::unique_ptr<ASTNode> Parser::parseGlobalDeclaration() {
    bool isAsync = false;
    bool isTest = false;
    bool isBench = false;

    if (checkKeyword("eksport")) {
        const Token expToken = peek();
        advance(); // 'eksport' kalit so'zini yutish
        
        if (checkKeyword("modul")) {
            advance(); // 'modul'
            std::string modName = advance().value;
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

    if (peek().type == TokenType::Symbol && peek().value == "@") {
        advance(); // consume '@'
        if (peek().type == TokenType::Identifier && peek().value == "sinov") {
            isTest = true;
            advance(); // consume 'sinov'
        } else if (peek().type == TokenType::Identifier && peek().value == "bench") {
            isBench = true;
            advance(); // consume 'bench'
        } else {
            throw ParseError("Noma'lum annotatsiya " + formatLocation(peek()));
        }
    }

    if (checkKeyword("asinxron")) {
        isAsync = true;
        advance(); // consume 'asinxron'
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
                templateParams += "typename ";
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
        advance();
        
        std::string enumBody = "enum class " + name + " {\n";
        while (!isAtEnd() && peek().value != "}") {
            enumBody += advance().value;
            if (peek().value == ",") enumBody += ",\n";
            else if (peek().value == "=") enumBody += " = ";
        }
        advance(); // '}'
        if (!isAtEnd() && peek().value == ";") advance();
        enumBody += "\n};";
        
        Token dummy = enumToken; dummy.value = enumBody;
        return std::make_unique<TokenNode>(dummy);
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
    if (checkKeyword("sinf") || checkKeyword("tuzilma")) {
        if (isAsync) throw ParseError("sinf asinxron bo'lmaydi");
        return parseClassDeclaration();
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
        return func;
    }
    
    // C-Style disambiguation: <Type> <Name>
    if (looksLikeDeclHelper(tokens_, current_)) {
        auto parseTypeString = [this]() -> std::string {
            std::string typeStr = advance().value;
            while (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "::") {
                typeStr += advance().value;
                if (!isAtEnd()) typeStr += advance().value;
            }
            if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "<") {
                typeStr += advance().value;
                int depth = 1;
                while (!isAtEnd() && depth > 0) {
                    if (peek().value == "<") depth++;
                    else if (peek().value == ">") depth--;
                    typeStr += advance().value;
                }
            }
            while (!isAtEnd() && peek().type == TokenType::Symbol && 
                   (peek().value == "&" || peek().value == "*" || peek().value == "&&")) {
                typeStr += advance().value;
            }
            return typeStr;
        };

        std::string typeName = parseTypeString();
        std::string name = advance().value;
        
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
                            // Check if next token after closing paren is '{'
                            if (peekPos + 1 < tokens_.size() && tokens_[peekPos + 1].value == "{") {
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
                        return func;
                    } catch (const ParseError&) {
                        // Fall through to variable declaration with constructor
                    }
                }
                
                if (isAsync) throw ParseError("O'zgaruvchini asinxron qilib bo'lmaydi");
                auto varDecl = parseVariableDeclaration(typeName, name);
                
                if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == ";") {
                    advance();
                }
                
                return varDecl;
        } else {
            if (isAsync) throw ParseError("O'zgaruvchini asinxron qilib bo'lmaydi");
            auto varDecl = parseVariableDeclaration(typeName, name);
                
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
    std::string name = advance().value;
    
    // Support C++ style nested namespaces (e.g. uzpp::Xavfsizlik)
    while (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "::") {
        advance();
        name += "::" + advance().value;
    }
    
    if (peek().type == TokenType::Symbol && peek().value == ";") {
        advance(); // C++17 single-line namespace
        std::vector<std::unique_ptr<ASTNode>> globals;
        while (!isAtEnd()) {
            globals.push_back(parseGlobalDeclaration());
        }
        return std::make_unique<NamespaceDeclaration>(name, std::move(globals), token);
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
    std::string moduleName = advance().value;
    
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

std::unique_ptr<VariableDeclaration> Parser::parseVariableDeclaration(const std::string& typeName, const std::string& varName) {
    Token token;
    token.type = TokenType::Identifier;
    token.value = varName;
    token.line = previous().line;
    token.column = previous().column;
    
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
    
    return std::make_unique<VariableDeclaration>(varName, typeName, std::move(init), token, false);
}

std::unique_ptr<FunctionDeclaration> Parser::parseLegacyFunctionDeclaration() {
    const Token funcToken = advance(); // consume 'funksiya'
    std::string funcName = advance().value;
    return parseFunctionDeclaration("void", funcName);
}

std::unique_ptr<FunctionDeclaration> Parser::parseFunctionDeclaration(const std::string& returnType, const std::string& funcName) {
    Token funcToken;
    funcToken.value = funcName;
    funcToken.line = previous().line;
    funcToken.column = previous().column;
    funcToken.type = TokenType::Identifier;
    
    std::vector<FunctionDeclaration::Parameter> params = parseFunctionParameters();
    
    if (isAtEnd() || peek().value != "{") {
        throw ParseError("Kutilgan funktsiya tanasi '{' " + formatLocation(peek()));
    }
    
    auto body = parseBlock();
    
    return std::make_unique<FunctionDeclaration>(funcName, returnType, std::move(params), 
                                                 std::move(body), funcToken);
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
        
        if (peek().value == "ozgarmas" || peek().value == "o'zgarmas") {
            param.isConst = true;
            advance();
        }
        
        auto parseTypeString = [this]() -> std::string {
            std::string typeStr = advance().value;
            while (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "::") {
                typeStr += advance().value;
                if (!isAtEnd()) typeStr += advance().value;
            }
            if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "<") {
                typeStr += advance().value;
                int depth = 1;
                while (!isAtEnd() && depth > 0) {
                    if (peek().value == "<") depth++;
                    else if (peek().value == ">") depth--;
                    typeStr += advance().value;
                }
            }
            while (!isAtEnd() && peek().type == TokenType::Symbol && 
                   (peek().value == "&" || peek().value == "*" || peek().value == "&&")) {
                typeStr += advance().value;
            }
            return typeStr;
        };
        
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


std::unique_ptr<ClassDeclaration> Parser::parseClassDeclaration() {
    const Token classToken = advance(); // consume 'sinf'
    
    if (isAtEnd() || peek().type != TokenType::Identifier) {
        throw ParseError("Kutilgan sinf nomi " + formatLocation(peek()));
    }
    
    const std::string className = peek().value;
    advance();
    
    std::string baseClass;
    if (matchKeyword("meros") || (peek().type == TokenType::Symbol && peek().value == ":")) {
        if (peek().value == ":") advance(); // consume ':'
        if (isAtEnd() || peek().type != TokenType::Identifier) {
            throw ParseError("Kutilgan asosiy sinf yoki interfeys nomi " + formatLocation(peek()));
        }
        baseClass = peek().value;
        advance();
    }
    
    std::vector<std::string> interfaces;
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

    if (isAtEnd() || peek().value != "{") {
        throw ParseError("Kutilgan sinf tanasi '{' " + formatLocation(peek()));
    }
    
    advance(); // consume '{'
    
    std::vector<ClassDeclaration::Member> members;
    std::vector<std::unique_ptr<ClassDeclaration::Method>> methods;
    
    while (!isAtEnd() && !(peek().type == TokenType::Symbol && peek().value == "}")) {
        if (peek().type == TokenType::Identifier) {
            const std::string& val = peek().value;
            if (val == "ochiq" || val == "maxfiy" || val == "himoyalangan" || val == "shaxsiy" ||
                val == "public" || val == "private" || val == "protected") {
                advance(); // consume modifier
                if (!isAtEnd() && peek().value == ":") {
                    advance(); // consume ':'
                }
                continue;
            }
        }
        
        bool isStatic = false;
        if (peek().value == "statik") {
            isStatic = true;
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
                
                method->params = parseFunctionParameters();
                if (!isAtEnd() && peek().value == "{") {
                    method->body = parseBlock();
                } else if (!isAtEnd() && peek().value == ";") {
                    advance();
                }
                methods.push_back(std::move(method));
                continue;
            }
        }

        // Method or Field: Type Name ...
        if (looksLikeDeclHelper(tokens_, current_)) {
            auto parseTypeString = [this]() -> std::string {
                std::string typeStr = advance().value;
                while (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "::") {
                    typeStr += advance().value;
                    if (!isAtEnd()) typeStr += advance().value;
                }
                if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "<") {
                    typeStr += advance().value;
                    int depth = 1;
                    while (!isAtEnd() && depth > 0) {
                        if (peek().value == "<") depth++;
                        else if (peek().value == ">") depth--;
                        typeStr += advance().value;
                    }
                }
                while (!isAtEnd() && peek().type == TokenType::Symbol && 
                       (peek().value == "&" || peek().value == "*" || peek().value == "&&")) {
                    typeStr += advance().value;
                }
                return typeStr;
            };
            std::string typeName = parseTypeString();
            std::string name = advance().value;
            
            if (isStatic) {
                if (!isAtEnd() && peek().value == "(") typeName = "static " + typeName;
                else typeName = "inline static " + typeName;
            }

            if (!isAtEnd() && peek().value == "(") {
                // Parse Method
                auto method = std::make_unique<ClassDeclaration::Method>();
                method->returnType = typeName;
                method->name = name;
                method->token = previous();
                
                method->params = parseFunctionParameters();
                
                while (!isAtEnd() && (peek().value == "ustidan_yozish" || peek().value == "mavhum")) {
                    advance();
                }
                
                if (!isAtEnd() && peek().value == "{") {
                    method->body = parseBlock();
                } else if (!isAtEnd() && peek().value == "=") {
                    advance(); // '='
                    if (!isAtEnd() && peek().value == "0") advance();
                    if (!isAtEnd() && peek().value == ";") advance();
                } else if (!isAtEnd() && peek().value == ";") {
                    advance();
                }
                methods.push_back(std::move(method));
            } else {
                // Parse Field
                auto member = ClassDeclaration::Member();
                member.type = typeName;
                member.name = name;
                members.push_back(member);
                
                if (!isAtEnd() && peek().value == "=") {
                    advance(); // skip '='
                    while (!isAtEnd() && peek().value != ";") {
                        advance();
                    }
                }
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
    
    return std::make_unique<ClassDeclaration>(className, baseClass, std::move(interfaces), std::move(members), 
                                              std::move(methods), classToken);
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

} // namespace uzpp
