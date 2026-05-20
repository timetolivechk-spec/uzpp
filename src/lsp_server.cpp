#include "lsp_server.h"
#include "lexer.h"
#include "parser.h"
#include "type_checker.hpp"
#include "formatter.h"

#include <functional>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <array>

namespace uzpp {

void LspServer::run() {
    while (std::cin) {
        std::string line;
        int contentLength = 0;
        
        // Read HTTP-like headers
        while (std::getline(std::cin, line)) {
            if (line.empty() || line == "\r") {
                break;
            }
            if (line.find("Content-Length: ") == 0) {
                contentLength = std::stoi(line.substr(16));
            }
        }
        
        if (contentLength > 0) {
            std::string content(contentLength, ' ');
            std::cin.read(&content[0], contentLength);
            handleMessage(content);
        }
    }
}

void LspServer::sendMessage(const std::string& jsonContent) {
    std::cout << "Content-Length: " << jsonContent.size() << "\r\n\r\n" << jsonContent;
    std::cout.flush();
}

std::string LspServer::extractJsonString(const std::string& json, const std::string& key) {
    std::string target = "\"" + key + "\":";
    size_t pos = json.find(target);
    if (pos == std::string::npos) return "";
    
    pos += target.length();
    while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n' || json[pos] == '\r')) {
        pos++;
    }
    
    if (pos < json.length() && json[pos] == '"') {
        pos++;
        size_t end = pos;
        while (end < json.length()) {
            if (json[end] == '"' && json[end-1] != '\\') break;
            end++;
        }
        return json.substr(pos, end - pos);
    }
    return "";
}

void LspServer::handleMessage(const std::string& content) {
    std::string method = extractJsonString(content, "method");
    
    if (method == "initialize") {
        std::string idStr = extractId(content);
        
        // Declare all features the server supports
        // SemanticTokensLegend describes the token types/modifiers we emit
        // in textDocument/semanticTokens/full responses below. Order MUST
        // match the indices we encode in buildSemanticTokens().
        std::string response = "{\"jsonrpc\":\"2.0\",\"id\":" + idStr +
                               ",\"result\":{\"capabilities\":{\"textDocumentSync\":2," // 2 = Incremental Sync
                               "\"completionProvider\":{\"triggerCharacters\":[\".\",\" \"]},"
                               "\"documentSymbolProvider\":true,"
                               "\"documentFormattingProvider\":true,"
                               "\"definitionProvider\":true,"
                               "\"renameProvider\":true,"
                               "\"referencesProvider\":true,"
                               "\"signatureHelpProvider\":{\"triggerCharacters\":[\"(\",\",\"]},"
                               "\"hoverProvider\":true,"
                               "\"semanticTokensProvider\":{"
                                   "\"legend\":{"
                                       "\"tokenTypes\":[\"keyword\",\"type\",\"function\",\"variable\",\"namespace\",\"string\",\"number\",\"comment\",\"operator\"],"
                                       "\"tokenModifiers\":[]"
                                   "},"
                                   "\"range\":false,"
                                   "\"full\":true"
                               "},"
                               "\"inlayHintProvider\":true,"
                               "\"codeActionProvider\":{\"codeActionKinds\":[\"quickfix\"]}"
                               "}}}";
        sendMessage(response);
    }
    else if (method == "textDocument/didChange" || method == "textDocument/didOpen") {
        std::string uri = extractJsonString(content, "uri");
        std::string text = extractJsonString(content, "text");
        
        if (method == "textDocument/didOpen") {
            documentCache_[uri] = "";
            applyContentChanges(documentCache_[uri], content); // Use our new robust text parser
        } else {
            applyContentChanges(documentCache_[uri], content);
        }
        
        try {
            Lexer lexer(documentCache_[uri]);
            auto tokens = lexer.tokenize();
            Parser parser(tokens);
            auto program = parser.parse();
            
            TypeChecker checker;
            checker.check(program.get());
            
            if (!checker.getErrors().empty() || !checker.getWarnings().empty()) {
                std::ostringstream diags;
                diags << "{\"jsonrpc\":\"2.0\",\"method\":\"textDocument/publishDiagnostics\",\"params\":{\"uri\":\"" << uri << "\",\"diagnostics\":[";
                bool first = true;
                for (const auto& err : checker.getErrors()) {
                    if (!first) diags << ",";
                    diags << "{\"range\":{\"start\":{\"line\":" << (err.line > 0 ? err.line - 1 : 0) << ",\"character\":" << (err.column > 0 ? err.column - 1 : 0) << "},"
                          << "\"end\":{\"line\":" << (err.line > 0 ? err.line - 1 : 0) << ",\"character\":" << (err.column > 0 ? err.column + 5 : 5) << "}},"
                          << "\"message\":\"" << err.message << "\",\"severity\":1}";
                    first = false;
                }
                for (const auto& warn : checker.getWarnings()) {
                    if (!first) diags << ",";
                    diags << "{\"range\":{\"start\":{\"line\":" << (warn.line > 0 ? warn.line - 1 : 0) << ",\"character\":" << (warn.column > 0 ? warn.column - 1 : 0) << "},"
                          << "\"end\":{\"line\":" << (warn.line > 0 ? warn.line - 1 : 0) << ",\"character\":" << (warn.column > 0 ? warn.column + 5 : 5) << "}},"
                          << "\"message\":\"" << warn.message << "\",\"severity\":2}";
                    first = false;
                }
                diags << "]}}";
                sendMessage(diags.str());
                return;
            }

            std::string response = "{\"jsonrpc\":\"2.0\",\"method\":\"textDocument/publishDiagnostics\",\"params\":{\"uri\":\"" + uri + "\",\"diagnostics\":[]}}";
            sendMessage(response);
        } catch (const std::exception& e) {
            // Found a parse error
            std::string msg = e.what();
            
            int line = 0, col = 0;
            size_t qatorPos = msg.find("qator: ");
            size_t ustunPos = msg.find("ustun: ", qatorPos != std::string::npos ? qatorPos : 0);
            if (qatorPos != std::string::npos && ustunPos != std::string::npos) {
                line = std::stoi(msg.substr(qatorPos + 7, ustunPos - qatorPos - 7));
                col = std::stoi(msg.substr(ustunPos + 7));
            }
            if (line > 0) line--;
            if (col > 0) col--;

            // Escape quotes in message
            std::string escapedMsg;
            for(char c : msg) {
                if(c == '"') escapedMsg += "\\\"";
                else escapedMsg += c;
            }
            
            std::ostringstream ss;
            ss << "{\"jsonrpc\":\"2.0\",\"method\":\"textDocument/publishDiagnostics\",\"params\":{\"uri\":\"" << uri << "\",\"diagnostics\":[{"
               << "\"range\":{\"start\":{\"line\":" << line << ",\"character\":" << col << "},"
               << "\"end\":{\"line\":" << line << ",\"character\":" << (col + 5) << "}},"
               << "\"message\":\"" << escapedMsg << "\",\"severity\":1}]}}";
               
            sendMessage(ss.str());
        } catch (...) {
            std::string response = "{\"jsonrpc\":\"2.0\",\"method\":\"textDocument/publishDiagnostics\",\"params\":{\"uri\":\"" + uri + "\",\"diagnostics\":[]}}";
            sendMessage(response);
        }
    }
    else if (method == "textDocument/formatting") {
        std::string idStr = extractId(content);
        std::string uri = extractJsonString(content, "uri");
        
        if (!uri.empty() && documentCache_.contains(uri)) {
            try {
                Lexer lexer(documentCache_[uri]);
                auto tokens = lexer.tokenize();
                Parser parser(tokens);
                auto program = parser.parse();
                
                Formatter formatter;
                std::string formatted = formatter.format(program.get());
                
                std::string escapedFormat;
                for (char c : formatted) {
                    if (c == '"') escapedFormat += "\\\"";
                    else if (c == '\n') escapedFormat += "\\n";
                    else if (c == '\r') escapedFormat += "\\r";
                    else if (c == '\t') escapedFormat += "\\t";
                    else escapedFormat += c;
                }
                
                std::string response = "{\"jsonrpc\":\"2.0\",\"id\":" + idStr + ",\"result\":[{\"range\":{\"start\":{\"line\":0,\"character\":0},\"end\":{\"line\":999999,\"character\":0}},\"newText\":\"" + escapedFormat + "\"}]}";
                sendMessage(response);
            } catch (...) {
                sendMessage("{\"jsonrpc\":\"2.0\",\"id\":" + idStr + ",\"result\":null}");
            }
        } else {
            sendMessage("{\"jsonrpc\":\"2.0\",\"id\":" + idStr + ",\"result\":null}");
        }
    }
    else if (method == "textDocument/completion") {
        std::string idStr = extractId(content);
        std::string response = "{\"jsonrpc\":\"2.0\",\"id\":" + idStr + ",\"result\":{\"isIncomplete\":false,\"items\":" + buildCompletions() + "}}";
        sendMessage(response);
    }
    else if (method == "textDocument/definition") {
        std::string idStr = extractId(content);
        std::string uri = extractJsonString(content, "uri");
        
        size_t linePos = content.find("\"line\":");
        size_t charPos = content.find("\"character\":");
        int hoverLine = 0, hoverChar = 0;
        if (linePos != std::string::npos) hoverLine = std::stoi(content.substr(linePos + 7, content.find(',', linePos) - linePos - 7));
        if (charPos != std::string::npos) hoverChar = std::stoi(content.substr(charPos + 12, content.find_first_of(",}", charPos) - charPos - 12));
        
        std::string word = getWordAtPosition(documentCache_[uri], hoverLine, hoverChar);
        std::string loc = findDefinitionAst(uri, word);
        sendMessage("{\"jsonrpc\":\"2.0\",\"id\":" + idStr + ",\"result\":" + loc + "}");
    }
    else if (method == "textDocument/signatureHelp") {
        std::string idStr = extractId(content);
        std::string uri = extractJsonString(content, "uri");
        
        size_t linePos = content.find("\"line\":");
        size_t charPos = content.find("\"character\":");
        int hoverLine = 0, hoverChar = 0;
        if (linePos != std::string::npos) hoverLine = std::stoi(content.substr(linePos + 7, content.find(',', linePos) - linePos - 7));
        if (charPos != std::string::npos) hoverChar = std::stoi(content.substr(charPos + 12, content.find_first_of(",}", charPos) - charPos - 12));
        
        std::string sig = buildSignatureHelp(uri, hoverLine, hoverChar);
        sendMessage("{\"jsonrpc\":\"2.0\",\"id\":" + idStr + ",\"result\":" + sig + "}");
    }
    else if (method == "textDocument/references") {
        std::string idStr = extractId(content);
        std::string uri = extractJsonString(content, "uri");
        
        size_t linePos = content.find("\"line\":");
        size_t charPos = content.find("\"character\":");
        int hoverLine = 0, hoverChar = 0;
        if (linePos != std::string::npos) hoverLine = std::stoi(content.substr(linePos + 7, content.find(',', linePos) - linePos - 7));
        if (charPos != std::string::npos) hoverChar = std::stoi(content.substr(charPos + 12, content.find_first_of(",}", charPos) - charPos - 12));
        
        std::string word = getWordAtPosition(documentCache_[uri], hoverLine, hoverChar);
        std::string refs = findReferencesAst(uri, word, hoverLine, hoverChar);
        sendMessage("{\"jsonrpc\":\"2.0\",\"id\":" + idStr + ",\"result\":" + refs + "}");
    }
    else if (method == "textDocument/rename") {
        std::string idStr = extractId(content);
        std::string uri = extractJsonString(content, "uri");
        std::string newName = extractJsonString(content, "newName");
        
        size_t linePos = content.find("\"line\":");
        size_t charPos = content.find("\"character\":");
        int hoverLine = 0, hoverChar = 0;
        if (linePos != std::string::npos) hoverLine = std::stoi(content.substr(linePos + 7, content.find(',', linePos) - linePos - 7));
        if (charPos != std::string::npos) hoverChar = std::stoi(content.substr(charPos + 12, content.find_first_of(",}", charPos) - charPos - 12));
        
        std::string word = getWordAtPosition(documentCache_[uri], hoverLine, hoverChar);
        std::string edits = buildRenameEditsAst(uri, word, newName, hoverLine, hoverChar);
        sendMessage("{\"jsonrpc\":\"2.0\",\"id\":" + idStr + ",\"result\":" + edits + "}");
    }
    else if (method == "textDocument/documentSymbol") {
        std::string idStr = extractId(content);
        std::string uri = extractJsonString(content, "uri");
        std::string response = "{\"jsonrpc\":\"2.0\",\"id\":" + idStr + ",\"result\":[]}";
        if (!uri.empty() && documentCache_.contains(uri)) {
            try {
                Lexer lexer(documentCache_[uri]);
                auto tokens = lexer.tokenize();
                Parser parser(tokens);
                auto program = parser.parse();
                response = "{\"jsonrpc\":\"2.0\",\"id\":" + idStr + ",\"result\":" + buildDocumentSymbols(program.get()) + "}";
            } catch (...) {
                // Ignore parse errors to keep outline working as much as possible
            }
        }
        sendMessage(response);
    }
    else if (method == "textDocument/hover") {
        std::string idStr = extractId(content);

        // Extract cursor position
        size_t linePos = content.find("\"line\":");
        size_t charPos = content.find("\"character\":");
        int hoverLine = 0, hoverChar = 0;
        if (linePos != std::string::npos) {
            hoverLine = std::stoi(content.substr(linePos + 7, content.find(',', linePos) - linePos - 7));
        }
        if (charPos != std::string::npos) {
            hoverChar = std::stoi(content.substr(charPos + 12, content.find_first_of(",}", charPos) - charPos - 12));
        }

        std::string uri = extractJsonString(content, "uri");
        std::string word;
        if (!uri.empty() && documentCache_.contains(uri)) {
            word = getWordAtPosition(documentCache_[uri], hoverLine, hoverChar);
        }

        // First try keyword documentation
        std::string hoverContent = buildHover(word);

        // If no keyword doc, try inferred type for o'zgaruvchan/o'zgarmas variables
        if (hoverContent.empty() && !word.empty() && !uri.empty() && documentCache_.contains(uri)) {
            std::string inferredType = getInferredTypeAtPosition(uri, hoverLine, hoverChar, word);
            if (!inferredType.empty()) {
                hoverContent = "**`" + word + "`** → `" + inferredType + "` *(tur avtomatik aniqlangan)*";
            }
        }

        std::string response;
        if (hoverContent.empty()) {
            response = "{\"jsonrpc\":\"2.0\",\"id\":" + idStr + ",\"result\":null}";
        } else {
            // Escape the content for JSON
            std::string escaped;
            for (char c : hoverContent) {
                if (c == '"') escaped += "\\\"";
                else if (c == '\\') escaped += "\\\\";
                else if (c == '\n') escaped += "\\n";
                else escaped += c;
            }
            response = "{\"jsonrpc\":\"2.0\",\"id\":" + idStr + ",\"result\":{\"contents\":{\"kind\":\"markdown\",\"value\":\"" + escaped + "\"}}}";
        }
        sendMessage(response);
    }
    else if (method == "textDocument/semanticTokens/full") {
        std::string idStr = extractId(content);
        std::string uri = extractJsonString(content, "uri");
        std::string text;
        if (!uri.empty() && documentCache_.contains(uri)) text = documentCache_[uri];
        std::string data = buildSemanticTokens(text);
        std::string response = "{\"jsonrpc\":\"2.0\",\"id\":" + idStr + ",\"result\":{\"data\":[" + data + "]}}";
        sendMessage(response);
    }
    else if (method == "textDocument/inlayHint") {
        std::string idStr = extractId(content);
        std::string uri = extractJsonString(content, "uri");
        std::string hints = buildInlayHints(uri);
        sendMessage("{\"jsonrpc\":\"2.0\",\"id\":" + idStr + ",\"result\":" + hints + "}");
    }
    else if (method == "textDocument/codeAction") {
        std::string idStr = extractId(content);
        std::string uri = extractJsonString(content, "uri");
        // Parse the request's range — two "line"/"character" pairs in order.
        auto findIntAfter = [&](const std::string& key, std::size_t from) -> std::pair<int, std::size_t> {
            std::size_t kpos = content.find("\"" + key + "\":", from);
            if (kpos == std::string::npos) return {0, from};
            kpos += key.size() + 3;
            while (kpos < content.size() && (content[kpos] == ' ' || content[kpos] == '\t')) ++kpos;
            std::size_t end = kpos;
            while (end < content.size() && (isdigit(static_cast<unsigned char>(content[end])) || content[end] == '-')) ++end;
            return {kpos == end ? 0 : std::stoi(content.substr(kpos, end - kpos)), end};
        };
        auto [sLine, p1] = findIntAfter("line", 0);
        auto [sChar, p2] = findIntAfter("character", p1);
        auto [eLine, p3] = findIntAfter("line", p2);
        auto [eChar, _] = findIntAfter("character", p3);
        std::string actions = buildCodeActions(uri, sLine, sChar, eLine, eChar);
        sendMessage("{\"jsonrpc\":\"2.0\",\"id\":" + idStr + ",\"result\":" + actions + "}");
    }
    else if (method == "shutdown") {
        std::string idStr = extractId(content);
        sendMessage("{\"jsonrpc\":\"2.0\",\"id\":" + idStr + ",\"result\":null}");
    }
    else if (method == "exit") {
        std::exit(0);
    }
}

// --- Helper: Extract request ID ---
std::string LspServer::extractId(const std::string& content) {
    size_t idPos = content.find("\"id\":");
    if (idPos != std::string::npos) {
        idPos += 5;
        while (idPos < content.length() && (content[idPos] == ' ' || content[idPos] == '\t')) idPos++;
        size_t idEnd = idPos;
        while (idEnd < content.length() && isdigit(content[idEnd])) idEnd++;
        std::string s = content.substr(idPos, idEnd - idPos);
        return s.empty() ? "null" : s;
    }
    return "null";
}

// --- Helper: Build full completion list of uz++ keywords & stdlib ---
std::string LspServer::buildCompletions() {
    struct CompletionItem {
        std::string label;
        int kind; // 14=Keyword, 3=Function, 7=Class
        std::string detail;
    };

    static const CompletionItem items[] = {
        // Keywords
        {"butun",       14, "Butun son turi (int)"},
        {"haqiqiy",     14, "Haqiqiy son turi (double)"},
        {"mantiqiy",    14, "Mantiqiy tur (bool)"},
        {"matn",        14, "Matn turi (string)"},
        {"urinish",     14, "Xatoliklarni ushlash (try)"},
        {"ushlash",     14, "Xatolik holati (catch)"},
        {"irgitish",    14, "Istisno yaratish (throw)"},
        {"belgi",       14, "Belgi turi (char)"},
        {"bekor",       14, "Qaytish turi yo'q (void)"},
        {"agar",        14, "Shartli operator (if)"},
        {"aks",         14, "Aks holat (else)"},
        {"uchun",       14, "Takrorlash (for)"},
        {"gacha",       14, "Gacha takrorlash (while)"},
        {"qaytarish",   14, "Qaytarish (return)"},
        {"ko'chirish",  3,  "Xotira egaligini ko'chirish (std::move)"},
        {"moslash",     14, "Andozaga moslash (match)"},
        {"holat",       14, "Moslash holati (case)"},
        {"boshqa",      14, "Aks holat (default)"},
        {"makro",       14, "Kompilyatsiya vaqtida ishlovchi makro"},
        {"sinf",        14, "Sinf e'lon qilish (class)"},
        {"shablon",     14, "Shablon (Template/Generics)"},
        {"sanab_olish", 14, "Enum class e'lon qilish"},
        {"statik",      14, "Statik maydon yoki metod"},
        {"ulash_kutubxona", 14, "Tashqi C/C++ kutubxonasini ulash (masalan: curl)"},
        {"@sinov",      14, "Test funksiyasi annotatsiyasi"},
        {"@bench",      14, "Benchmark funksiyasi annotatsiyasi"},
        {"eksport",     14, "Modulni tashqariga uzatish (export)"},
        {"import",      14, "Modulni yuklash (import)"},
        {"modul",       14, "C++20 Modulini e'lon qilish (export module)"},
        {"tushuncha",   14, "C++20 Concept e'lon qilish"},
        {"shart",       14, "Shablon uchun cheklov (requires)"},
        {"yangi",       14, "Yangi misol yaratish (new)"},
        {"ulash",       14, "Fayl ulash (#include)"},
        {"nomlar_fazosi",14,"Nom fazosi (using namespace)"},
        {"asinxron",    14, "Asinxron funksiya"},
        {"kutish",      14, "Natijani kutish (co_await)"},
        {"o'zgaruvchan", 14, "Tur-chiqaruvchi o'zgaruvchi (auto)"},
        {"o'zgarmas",   14, "O'zgarmas qiymat (const)"},
        {"rost",        14, "Mantiqiy rost (true)"},
        {"yolg'on",     14, "Mantiqiy yolg'on (false)"},
        {"shartnoma",   14, "Interfeys/shartnoma (abstract class)"},
        {"uzaytirish",  14, "Merosxo'rlik (extends)"},
        {"amalga_oshirish", 14, "Joriy qilish (implements)"},
        {"oqim_boshla",  3, "Yangi oqimda bajarish (std::async)"},
        {"filter",       3, "Shart bo'yicha saralash (Ranges)"},
        {"map",          3, "Elementlarni o'zgartirish (Ranges)"},
        {"yigish",       3, "Oqimni to'plamga yig'ish (Ranges to<T>)"},
        {"kelajak",      7, "Asinxron natija (std::future)"},
        {"yozish",       3, "Konsolga chiqarish (std::cout)"},
        {"qator_oxiri",  3, "Yangi qator (std::endl)"},
        // Stdlib modules
        {"Tarmoq::Server",   7, "Asinxron HTTP-server"},
        {"Tarmoq::Mijoz",    7, "HTTP Mijoz (cURL orqali GET/POST)"},
        {"Tarmoq::Router",   7, "HTTP Marshrutizator (Router)"},
        {"Tarmoq::WebSocket", 7, "WebSocket interfeysi (Real-time)"},
        {"Tarmoq::Soket",    7, "Pastroq darajadagi TCP/UDP soket (Socket)"},
        {"Tarmoq::SoketTuri", 14, "Soket turi (TCP/UDP)"},
        {"Tarjima::Tarjimon", 7, "I18n (Ko'p tillilik) tarjimon obyekti"},
        {"Tarjima::I18N",    3, "Global tarjima funksiyasi / obyekti"},
        {"FaylTizimi::IkkilikYozuvchi", 7, "Ikkilik (binary) oqim yozuvchisi"},
        {"FaylTizimi::IkkilikOquvchi",  7, "Ikkilik (binary) oqim o'quvchisi"},
        {"FaylTizimi::Katalog", 7, "Katalog (papkalar) bilan ishlash moduli"},
        {"Grafika::Oyna",    7, "Grafik oyna (OpenGL)"},
        {"Kripto::sha256",   3, "Matnni SHA-256 algoritmi orqali xeshlash"},
        {"Kripto::xavfsizShifrlash", 3, "Matnni kalit orqali shifrlash"},
        {"Kripto::xavfsizOchish", 3, "Shifrlangan matnni kalit bilan ochish"},
        {"Kripto::jwtYaratish", 3, "JSON Web Token (JWT) yaratish"},
        {"Grafika::Holat",   7, "Reaktiv holat (State<T>)"},
        {"Grafika::Yangi",   3, "Vidjet yaratish (make_shared<T>)"},
        {"Baza::Ulanish",    7, "Ma'lumotlar bazasi ulanishi"},
        {"Baza::SorovQuruvchi",7, "Asinxron ORM interfeysi"},
        {"Baza::JadvalQuruvchi",7,"Ma'lumotlar bazasi migratsiyasi"},
        {"Baza::MySQL::Ulanish",    7, "MySQL ma'lumotlar bazasi ulanishi"},
        {"Baza::MySQL::SorovQuruvchi",7, "Asinxron MySQL ORM interfeysi"},
        {"Baza::MySQL::JadvalQuruvchi",7,"MySQL jadval migratsiyasi"},
        {"Baza::Qiymat",     14, "Dinamik qiymat turi (Variant)"},
        {"jadval",       3, "Bazadagi jadvalni tanlash (ORM)"},
        {"qayerda",      3, "ORM filter sharti (WHERE)"},
        {"Asinxron::Vazifa", 7, "Asinxron vazifa (coroutine)"},
        {"VAQT_OLCHOVI", 3, "Blok ishlash vaqtini o'lchash (Profiler)"},
        {"Xotira::Ulashilgan", 7, "Ulashilgan aqlli ko'rsatkich (Shared Pointer)"},
        {"Xotira::yangi",3, "Xotirada xavfsiz obyekt yaratish"},
        {"VebUI::div",   3, "Reaktiv UI: <div> elementi"},
        {"VebUI::tugma", 3, "Reaktiv UI: <button> elementi"},
        {"VebUI::Shtor", 7, "Reaktiv UI: Holat (State) boshqaruvchisi (Redux kabi)"},
        {"Oyna::teksturaYuklash", 3, "BMP rasmni xotiraga yuklash"},
        {"Grafika::KiritishVidjeti", 7, "Matn kiritish maydoni (TextField)"},
        {"Tizim::Jarayon", 7, "OS tizim jarayonlarini boshqarish"},
        {"buyruqNatijasi", 3, "OS terminal buyrug'i natijasini olish"},
        {"Tizim::Argumentlar", 7, "Konsol argumentlarini (CLI) o'qish"},
        {"hxGet", 3, "HTMX: GET so'rovi yuborish"},
        {"hxPost", 3, "HTMX: POST so'rovi yuborish"},
        {"hxTarget", 3, "HTMX: Natijani qaysi elementga joylash"},
        {"hxSwap", 3, "HTMX: Almashtirish uslubi (innerHTML, outerHTML)"},
        {"SuniyIntellekt::LLM", 7, "Mahalliy yoki bulutli Neyrotarmoq (AI)"},
        {"sorash", 3, "Neyrotarmoqqa (LLM) so'rov yuborish"},
        {"VebUI::sarlavha", 3, "Reaktiv UI: <h1>..<h6> elementi"},
        // Natija / Tanlov types
        {"Natija",          7, "uzpp::Natija<T> — Xatolikni qiymat sifatida ifodalash (Result<T,E>)"},
        {"Tanlov",          7, "uzpp::Tanlov<T> — Mavjud yoki yo'q qiymat (Option<T>)"},
        {"muvaffaqiyat",    3, "uzpp::Natija::muvaffaqiyat(val) — Muvaffaqiyatli natija"},
        {"xato",            3, "uzpp::Natija::xato(\"xabar\") — Xatolik natija"},
        {"yaroqliMi",       3, "natija.yaroqliMi() — Natija muvaffaqiyatlimi?"},
        {"xatoliMi",        3, "natija.xatoliMi() — Natija xatolikmi?"},
        {"qiymat",          3, "natija.qiymat() — Muvaffaqiyatli qiymatni olish"},
        {"xatoMazmun",      3, "natija.xatoMazmun() — Xato xabarini olish"},
        {"qiymat_yoki",     3, "natija.qiymat_yoki(standart) — Qiymat yoki standart"},
        {"va_keyin",        3, "natija.va_keyin(f) — Muvaffaqiyatli qiymatni o'zgartirish"},
        // Matematika module
        {"uzpp::Matematika::PI",          3, "π = 3.14159265358979"},
        {"uzpp::Matematika::E",           3, "Eyler soni e = 2.71828"},
        {"uzpp::Matematika::PHI",         3, "Oltin nisbat φ = 1.61803"},
        {"uzpp::Matematika::ildiz",       3, "uzpp::Matematika::ildiz(x) — Kvadrat ildiz (sqrt)"},
        {"uzpp::Matematika::kub_ildiz",   3, "uzpp::Matematika::kub_ildiz(x) — Kub ildiz (cbrt)"},
        {"uzpp::Matematika::daraja",      3, "uzpp::Matematika::daraja(asos, daraja) — Daraja (pow)"},
        {"uzpp::Matematika::sin",         3, "uzpp::Matematika::sin(x) — Sinus"},
        {"uzpp::Matematika::cos",         3, "uzpp::Matematika::cos(x) — Kosinus"},
        {"uzpp::Matematika::tan",         3, "uzpp::Matematika::tan(x) — Tangent"},
        {"uzpp::Matematika::log10",       3, "uzpp::Matematika::log10(x) — O'nlik logarifm"},
        {"uzpp::Matematika::ln",          3, "uzpp::Matematika::ln(x) — Tabiiy logarifm"},
        {"uzpp::Matematika::modul",       3, "uzpp::Matematika::modul(x) — Mutlaq qiymat (abs)"},
        {"uzpp::Matematika::yaxlitlash",  3, "uzpp::Matematika::yaxlitlash(x) — Yaxlitlash (round)"},
        {"uzpp::Matematika::tasodifiy_son",3,"uzpp::Matematika::tasodifiy_son(min, max) — Tasodifiy butun son"},
        {"uzpp::Matematika::ortacha",     3, "uzpp::Matematika::ortacha(vektor) — O'rtacha qiymat"},
        {"uzpp::Matematika::yigindisi",   3, "uzpp::Matematika::yigindisi(vektor) — Yig'indisi"},
        {"uzpp::Matematika::eng_katta_el",3, "uzpp::Matematika::eng_katta_el(vektor) — Eng katta element"},
        {"uzpp::Matematika::eng_kichik_el",3,"uzpp::Matematika::eng_kichik_el(vektor) — Eng kichik element"},
        {"uzpp::Matematika::tub_sonMi",   3, "uzpp::Matematika::tub_sonMi(n) — Tub sonmi? (isPrime)"},
        {"uzpp::Matematika::faktorial",   3, "uzpp::Matematika::faktorial(n) — n! hisoblash"},
        // Matn module
        {"uzpp::Matn::ajratish",          3, "uzpp::Matn::ajratish(matn, ajratgich) — Matnni bo'lish (split)"},
        {"uzpp::Matn::birlashtirish",     3, "uzpp::Matn::birlashtirish(vektor, ajratgich) — Birlashtirish (join)"},
        {"uzpp::Matn::qirqish",           3, "uzpp::Matn::qirqish(matn) — Bosh va oxirdagi bo'shliqlarni olib tashlash (trim)"},
        {"uzpp::Matn::kichik_harfga",     3, "uzpp::Matn::kichik_harfga(matn) — Kichik harflarga o'girish (toLower)"},
        {"uzpp::Matn::katta_harfga",      3, "uzpp::Matn::katta_harfga(matn) — Katta harflarga o'girish (toUpper)"},
        {"uzpp::Matn::almashtirish",      3, "uzpp::Matn::almashtirish(matn, eski, yangi) — Matn almashtirish (replace)"},
        {"uzpp::Matn::boshlanganda",      3, "uzpp::Matn::boshlanganda(matn, prefiks) — Prefiks bilan boshlanadimi?"},
        {"uzpp::Matn::tugaganda",         3, "uzpp::Matn::tugaganda(matn, suffiks) — Suffiks bilan tugaydimi?"},
        {"uzpp::Matn::ichida",            3, "uzpp::Matn::ichida(matn, qism) — Qism matn mavjudmi?"},
        {"uzpp::Matn::topish",            3, "uzpp::Matn::topish(matn, qism) — Pozitsiyani topish (find)"},
        {"uzpp::Matn::qism_matn",         3, "uzpp::Matn::qism_matn(matn, boshlanish, uzunlik) — Qism matn (substr)"},
        {"uzpp::Matn::teskari",           3, "uzpp::Matn::teskari(matn) — Matnni teskari aylantirish (reverse)"},
        {"uzpp::Matn::takrorlash",        3, "uzpp::Matn::takrorlash(matn, n) — Matnni n marta takrorlash (repeat)"},
        {"uzpp::Matn::butun_songa",       3, "uzpp::Matn::butun_songa(matn) — Matnni butun songa aylantirish (stoi)"},
        {"uzpp::Matn::kasr_songa",        3, "uzpp::Matn::kasr_songa(matn) — Matnni kasr songa aylantirish (stod)"},
        // JSON module
        {"uzpp::Json::yaratObyekt",       3, "uzpp::Json::yaratObyekt() — Bo'sh JSON obyekti yaratish"},
        {"uzpp::Json::yaratMassiv",       3, "uzpp::Json::yaratMassiv() — Bo'sh JSON massivi yaratish"},
        {"uzpp::Json::yaratMatn",         3, "uzpp::Json::yaratMatn(s) — JSON matn qiymati"},
        {"uzpp::Json::yaratSon",          3, "uzpp::Json::yaratSon(n) — JSON son qiymati"},
        {"uzpp::Json::yaratMantiq",       3, "uzpp::Json::yaratMantiq(b) — JSON mantiqiy qiymati"},
        {"uzpp::Json::yaratNull",         3, "uzpp::Json::yaratNull() — JSON null qiymati"},
        {"uzpp::Json::tahlil",            3, "uzpp::Json::tahlil(matn) — JSON matnni tahlil qilish (parse)"},
        {"uzpp::Json::chiqarish",         3, "uzpp::Json::chiqarish(qiymat, tirtish) — JSON ni matnga aylantirish (stringify)"},
        // Pipeline and lambda
        {"|>",              14, "Quvur operatori: qiymat |> funksiya == funksiya(qiymat)"},
        {"=>",              14, "Lambda strelka: |x| => x * 2 — Rust uslubidagi lambda"},
        // C++20 source_location
        {"manba_joyi",       7, "uzpp::manba_joyi (std::source_location) — Joriy chaqiriq joyi (fayl, qator, funksiya)"},
    };

    std::ostringstream ss;
    ss << "[";
    bool first = true;
    for (const auto& item : items) {
        if (!first) ss << ",";
        first = false;
        ss << "{\"label\":\"" << item.label
           << "\",\"kind\":" << item.kind
           << ",\"detail\":\"" << item.detail << "\"";
        ss << "}";
    }
    ss << "]";
    return ss.str();
}

// --- Helper: Hover documentation for known words ---
std::string LspServer::buildHover(const std::string& word) {
    static const std::unordered_map<std::string, std::string> docs = {
        {"butun",       "**butun** — Butun son turi. C++ `int` ga transpilyatsiya bo'ladi."},
        {"haqiqiy",    "**haqiqiy** — Kasr son turi. C++ `double` ga transpilyatsiya bo'ladi."},
        {"mantiqiy",   "**mantiqiy** — Mantiqiy tur. `haqiqat` yoki `yolg'on` qiymat oladi."},
        {"matn",       "**matn** — Matn qatori turi. C++ `std::string` ga transpilyatsiya bo'ladi."},
        {"urinish",    "**urinish** { ... } **ushlash** (Xato x) { ... } — Istisnolarni ushlash bloki (Try-Catch)."},
        {"irgitish",   "**irgitish** ifoda; — Istisno (exception) firlatish (Throw)."},
        {"asinxron",   "**asinxron** — Funksiyani C++20 coroutine `Vazifa<T>` ga aylantiradi."},
        {"kutish",     "**kutish** — `co_await` operatori. Asinxron natijani kutadi."},
        {"shartnoma",  "**shartnoma** — Abstrakt interfeys e'lon qilish. C++ pure virtual classga transpilyatsiya bo'ladi."},
        {"shablon",    "**shablon** \\<tur T\\> — Generik sinf yoki funktsiya yaratish (Templates)."},
        {"sanab_olish","**sanab_olish** Nom { A, B } — Raqamlangan ro'yxat (Enum class) yaratish."},
        {"statik",     "**statik** — Sinf (class) darajasidagi maydon yoki metod yaratadi."},
        {"sinov",      "**@sinov** — Funksiyani unit-test sifatida belgilaydi. `uzpp sinov` orqali ishga tushiriladi."},
        {"bench",      "**@bench** — Funksiyani benchmark (samaradorlik o'lchovi) sifatida belgilaydi. `uzpp bench` orqali ishga tushiriladi."},
        {"ulash_kutubxona", "**ulash_kutubxona** \"curl\"; — C/C++ dinamik kutubxonasini (linker uchun) dasturga to'g'ridan-to'g'ri ulaydi."},
        {"eksport",    "**eksport** — Modul, sinf yoki funksiyani boshqa fayllar ishlata olishi uchun e'lon qiladi."},
        {"modul",      "**modul** Nom; — C++20 Module interfeysini yaratadi. `import` orqali tezlashtirilgan ulanish uchun."},
        {"import",     "**import** \"modul\"; — Boshqa fayldagi (uzpp) kodlarni joriy faylga ulaydi."},
        {"tushuncha",  "**tushuncha** Nom = ifoda; — C++20 Concept yordamida shablon uchun qoida e'lon qiladi."},
        {"shart",      "**shart**(qoida) — Shablon generik parametrlari uchun cheklov (requires) qo'shadi."},
        {"ozgaruvchan","**ozgaruvchan** — Type inference. C++ `auto` ga transpilyatsiya bo'ladi."},
        {"agar",       "**agar** (shart) { ... } **aks** { ... } — Shartli tarmoqlanish."},
        {"uchun",      "**uchun** (element : kolleksiya) { ... } — Range-based for tsikli."},
        {"gacha",      "**gacha** (shart) { ... } — While tsikli."},
        {"to'xtatish", "**to'xtatish**; — Siklni to'xtatish (Break)."},
        {"davom_etish","**davom_etish**; — Siklning keyingi iteratsiyasiga o'tish (Continue)."},
        {"moslash",    "**moslash** (qiymat) { **holat** andoza: ... } — Pattern matching (Switch o'rniga)."},
        {"holat",      "**holat** andoza: — `moslash` blokidagi aniq bir shart (case)."},
        {"ko'chirish", "**ko'chirish**(obyekt) — Obyektga bo'lgan egalik huquqini (ownership) boshqa o'zgaruvchiga o'tkazadi. Zero-cost abstraction yordamida ishlashni ta'minlaydi (std::move)."},
        {"makro",      "**makro** Nom(x) => ifoda; — Kompilyatsiya vaqtida generatsiya bo'luvchi makros (C++ #define)."},
        {"qaytarish",  "**qaytarish** qiymat; — Funksiyadan qiymat qaytarish (return)."},
        {"sinf",       "**sinf** Nom { ... }; — C++ klassini e'lon qilish."},
        {"yozish",     "**yozish** << qiymat << qator_oxiri; — Konsolga chiqarish (std::cout)."},
        {"Holat",      "**Holat\\<T\\>** — Reaktiv holat klassi. `ornatish()` metodida barcha kuzatuvchilar xabardor bo'ladi."},
        {"Vazifa",     "**Asinxron::Vazifa\\<T\\>** — C++20 coroutine wrapper. `kutish` bilan ishlatiladi."},
        {"oqim_boshla","**oqim_boshla**(funksiya, args...) — Funksiyani yangi parallel oqimda (Thread) ishga tushiradi."},
        {"filter",     "**filter**(shart) — Elementlarni shart bo'yicha filtrlaydi (C++20 Ranges). `|>` operatori bilan ishlatiladi."},
        {"map",        "**map**(funksiya) — Har bir elementni o'zgartiradi (C++20 Ranges). `|>` operatori bilan ishlatiladi."},
        {"yigish",     "**yigish**\\<tur\\>() — Oqimni (Range) ko'rsatilgan to'plam turiga yig'adi. Masalan: `yigish<vektor<butun>>()`. "},
        {"SorovQuruvchi","**Baza::SorovQuruvchi** — Ma'lumotlar bazasiga Asinxron ORM orqali oson ulanish va kod yozish interfeysi."},
        {"JadvalQuruvchi","**Baza::JadvalQuruvchi** — Yangi jadvallar va ularning ustunlarini generatsiya qilish (Migration) uchun yordamchi klas."},
        {"MySQL::Ulanish", "**Baza::MySQL::Ulanish** — MySQL yoki MariaDB bazasiga ulanish. `ochish(host, user, pass, db)` orqali ulanadi."},
        {"jadval",     "**baza.jadval(\"nomi\")** — ORM interfeysini ma'lum bir jadval bilan bog'laydi. Undan so'ng `olish()`, `kiritish()`, `qayerda()` larni ulab ketish mumkin."},
        {"qayerda",    "**qayerda**(shart, args...) — So'rovni filtrlash uchun. SQL inyeksiyadan himoyalangan (masalan: `qayerda(\"yosh > ?\", 18)`)."},
        {"VAQT_OLCHOVI","**VAQT_OLCHOVI**(\"Nom\"); — Dasturning ushbu bloki ishlashiga ketgan vaqtni (ms) o'lchab, avtomatik ravishda jurnalga chiqaradi (Benchmarking)."},
        {"Tarmoq::Mijoz", "**Tarmoq::Mijoz** — cURL asosidagi HTTP klient. GET va POST so'rovlarini tashqi API larga asinxron tarzda yuborish uchun ishlatiladi."},
        {"IkkilikYozuvchi", "**IkkilikYozuvchi(\"fayl.bin\")** — Obyektlar va matnlarni diskga seralizatsiya qilib yozish mexanizmi (Stream write)."},
        {"IkkilikOquvchi", "**IkkilikOquvchi(\"fayl.bin\")** — Diskdan ikkilik (binary) ma'lumotlarni o'qish (Stream read)."},
        {"Katalog", "**FaylTizimi::Katalog** — Operatsion tizimdagi papkalarni yaratish, tekshirish va ichidagi fayllar ro'yxatini olish uchun modul."},
        {"Tarmoq::Soket", "**Tarmoq::Soket** — Past darajadagi tarmoq protokollari (TCP, UDP) orqali ulanish va ma'lumot almashish uchun klass."},
        {"Tarjima::I18N", "**Tarjima::I18N**(\"kalit\") — Global tarjimon. So'zlarni joriy tilga moslab qaytaradi (i18n). Tilni o'zgartirish uchun: `I18N.tilniOrnatish(\"uz\")`"},
        {"Tarmoq::Router", "**Tarmoq::Router** — GET/POST/PUT/DELETE marshrutlarini boshqarish uchun klass. `router.ulash(&server)` orqali serverga ulanadi."},
        {"sha256", "**Kripto::sha256**(\"matn\") — Matnni xavfsiz SHA-256 xeshiga aylantiradi (64 ta belgi)."},
        {"xavfsizShifrlash", "**Kripto::xavfsizShifrlash**(matn, kalit) — Ma'lumotni maxfiy kalit yordamida shifrlaydi (Base64 qaytaradi)."},
        {"xavfsizOchish", "**Kripto::xavfsizOchish**(shifr, kalit) — Shifrlangan Base64 matnni maxfiy kalit yordamida asliga qaytaradi."},
        {"jwtYaratish", "**Kripto::jwtYaratish**(payloadJson, maxfiyKalit) — Xavfsiz JWT token generatsiya qiladi."},
        {"Tarmoq::WebSocket", "**Tarmoq::WebSocket** — Real vaqt rejimida xabarlar yuborish va o'qish (WebSockets API). Router orqali `javob.webSocketQabulQilish(sorov)` qilib ulanadi."},
        {"Xotira::Ulashilgan", "**Xotira::Ulashilgan\\<T\\>** — RAII xotira boshqaruvidagi ulashilgan ko'rsatkich (`std::shared_ptr`). C++ an'analariga mos xavfsiz dasturlash uchun."},
        {"Xotira::yangi", "**Xotira::yangi\\<T\\>(...)** — GC xotirasida yangi ob'ekt yaratadi."},
        {"Xotira::Arena", "**Xotira::Arena(hajm_baytlarda)** — O'yinlar va yuqori tezlik talab qilinadigan jarayonlar uchun super-tez (O(1)) xotira ajratuvchi. `.tozalash()` orqali barcha xotirani bir zumda bo'shatadi."},
        {"VebUI::div", "**VebUI::div()** — HTML `<div>` blokini hosil qiladi. Unga zanjir orqali `.klass()`, `.bola()` ulash mumkin."},
        {"VebUI::Shtor", "**VebUI::Shtor\\<Holat, Amal\\>** — Redux arxitekturasidagi State Manager. UI reaktiv ishlashi uchun `.yuborish(amal)` va `.obuna()` funksiyalaridan foydalaniladi."},
        {"Fazo::Matritsa", "**Fazo::Matritsa** — Data Science va AI uchun 2D Matritsa (Tensor). Matritsalarni ko'paytirish (`*`), qo'shish (`+`) va transponirlash mumkin."},
        {"ortacha", "**Fazo::Statistika::ortacha(vektor)** — Berilgan sonlar massivining (vektor) o'rta arifmetik qiymatini hisoblaydi."},
        {"Koinot::VebIlova", "**Koinot::VebIlova** — Express.js va FastAPI darajasidagi zamonaviy MVC Web-freymvork. `.get()` va `.post()` orqali marshrutlar ulanadi."},
        {"Model3D", "**Grafika::Model3D** — OBJ formatidagi 3D model va uning teksturasini o'zida saqlovchi obyekt."},
        {"uchOchlamliRejim", "**oyna.uchOchlamliRejim(fov, zYaqin, zUzoq)** — Oynani 3D (Perspective) rejimiga o'tkazadi va chuqurlik testini (Depth Test) yoqadi."},
        {"ikkiOchlamliRejim", "**oyna.ikkiOchlamliRejim()** — Oynani standart 2D (Orthographic) rejimiga qaytaradi."},
        {"modelYuklash", "**oyna.modelYuklash(\"fayl.obj\", tekstura)** — OBJ fayldan 3D modelni (vertex, normal, uv) o'qiydi va `Model3D` obyektini qaytaradi."},
        {"modelChizish", "**oyna.modelChizish(model)** — Xotiraga yuklangan 3D modelni ekranga chizadi."},
        {"surish3D", "**oyna.surish3D(x, y, z)** — 3D fazoda obyekt yoki kamerani berilgan koordinatalar bo'yicha ko'chiradi."},
        {"burish3D", "**oyna.burish3D(burchak, x, y, z)** — 3D fazoda ma'lum bir o'q (x, y yoki z) bo'ylab obyektni aylantiradi."},
        {"teksturaYuklash", "**oyna.teksturaYuklash(\"rasm.bmp\")** — Grafik oynaga BMP rasm yuklaydi va tekstura ID sini qaytaradi."},
        {"hodisa", "**teg.hodisa(\"onclick\", \"amal()\")** — HTML DOM elementiga JavaScript hodisasini bog'laydi."},
        {"KiritishVidjeti", "**Grafika::KiritishVidjeti(\"Placeholder\")** — Foydalanuvchidan matn kiritishni (klaviatura) qabul qiluvchi oyna vidjeti."},
        {"Tizim::Jarayon", "**Tizim::Jarayon** — Operatsion tizim jarayonlari (process) va terminal buyruqlari bilan ishlash moduli."},
        {"buyruqNatijasi", "**Tizim::Jarayon::buyruqNatijasi**(\"ls -la\") — Terminal/CMD buyrug'ini ishga tushiradi va uning konsolga chiqargan natijasini (stdout) matn sifatida qaytaradi."},
        {"Argumentlar", "**Tizim::Argumentlar** — CLI terminalidan kelgan bayroqlarni (`--port=8080`) va argumentlarni qulay tahlil qiluvchi klass."},
        {"hxGet", "**hxGet(url)** — HTMX yordamida elementga AJAX GET so'rovini biriktiradi (VebUI). Server-Side Rendering (SSR) uchun juda qulay."},
        {"hxPost", "**hxPost(url)** — HTMX yordamida AJAX POST so'rovini biriktiradi."},
        {"SuniyIntellekt::LLM", "**SuniyIntellekt::LLM**(\"llama3\", \"http://...\") — Neyrotarmoq (Sun'iy intellekt) modeliga ulanish uchun klass. Standart ravishda Ollama ga ulanadi."},
        {"sorash", "**llm.sorash**(\"Salom, sen kimsan?\") — Neyrotarmoqdan asinxron/sinxron javob oladi."},
        {"VebUI::tugma", "**VebUI::tugma(\"Matn\")** — HTML `<button>` elementini yaratadi."},
        // Natija / Tanlov
        {"Natija",       "**`uzpp::Natija<T, E>`** — Xatolikni qiymat sifatida ifodalovchi tur (Result type).\\n\\n```\\nuzpp::Natija<butun> bo'lish(butun a, butun b) {\\n    agar (b == 0) qaytarish uzpp::Natija<butun>::xato(\"Nol!\");\\n    qaytarish uzpp::Natija<butun>::muvaffaqiyat(a / b);\\n}\\n```"},
        {"Tanlov",       "**`uzpp::Tanlov<T>`** — Mavjud (`bor`) yoki yo'q (`yoq`) qiymat (Option type).\\n\\n```\\nuzpp::Tanlov<matn> topish(vektor<matn>& v, butun i) {\\n    agar (i < v.size()) qaytarish uzpp::Tanlov<matn>::bor(v[i]);\\n    qaytarish uzpp::Tanlov<matn>::yoq();\\n}\\n```"},
        {"yaroqliMi",    "**`natija.yaroqliMi()`** → `mantiqiy` — Natija muvaffaqiyatlimi?"},
        {"xatoliMi",     "**`natija.xatoliMi()`** → `mantiqiy` — Natija xatolikmi?"},
        {"qiymat",       "**`natija.qiymat()`** → `T` — Muvaffaqiyatli qiymatni olish. Xatolik bo'lsa istisno yuz beradi."},
        {"xatoMazmun",   "**`natija.xatoMazmun()`** → `E` — Xato xabarini olish. Muvaffaqiyat bo'lsa istisno yuz beradi."},
        {"qiymat_yoki",  "**`natija.qiymat_yoki(standart)`** → `T` — Qiymat bo'lsa uni, bo'lmasa standartni qaytaradi."},
        {"va_keyin",     "**`natija.va_keyin(f)`** — Muvaffaqiyat bo'lsa `f(qiymat)` chaqiradi, xatolikni o'tkazib yuboradi."},
        // Matematika
        {"uzpp::Matematika::PI",  "**`uzpp::Matematika::PI`** — π ≈ 3.14159265358979"},
        {"uzpp::Matematika::E",   "**`uzpp::Matematika::E`** — Eyler soni e ≈ 2.71828182845905"},
        {"uzpp::Matematika::ildiz","**`uzpp::Matematika::ildiz(x)`** — Kvadrat ildiz. `sqrt(x)` ga ekvivalent."},
        {"uzpp::Matematika::daraja","**`uzpp::Matematika::daraja(asos, ko'rsatkich)`** — Darajaga ko'tarish. `pow(x,n)` ga ekvivalent."},
        {"uzpp::Matematika::sin", "**`uzpp::Matematika::sin(x)`** — Sinus funksiyasi (radianlarda)."},
        {"uzpp::Matematika::cos", "**`uzpp::Matematika::cos(x)`** — Kosinus funksiyasi (radianlarda)."},
        {"uzpp::Matematika::tasodifiy_son","**`uzpp::Matematika::tasodifiy_son(min, max)`** — `[min, max]` oralig'ida tasodifiy butun son."},
        {"uzpp::Matematika::ortacha","**`uzpp::Matematika::ortacha(v)`** → `haqiqiy` — Sonlar vektorining o'rtacha qiymati."},
        {"uzpp::Matematika::tub_sonMi","**`uzpp::Matematika::tub_sonMi(n)`** → `mantiqiy` — n tub sonmi?"},
        {"uzpp::Matematika::faktorial","**`uzpp::Matematika::faktorial(n)`** → `uzun_butun` — n! (faktorial)."},
        // Matn
        {"uzpp::Matn::ajratish","**`uzpp::Matn::ajratish(matn, ajratgich)`** → `vektor<matn>` — Matnni bo'laklarga ajratish.\\n\\n```\\nvektor<matn> qismlar = uzpp::Matn::ajratish(\"a,b,c\", \",\");\\n```"},
        {"uzpp::Matn::birlashtirish","**`uzpp::Matn::birlashtirish(vektor, ajratgich)`** → `matn` — Matnlarni birlashtirish."},
        {"uzpp::Matn::qirqish","**`uzpp::Matn::qirqish(matn)`** → `matn` — Bosh va oxirdagi bo'shliqlarni olib tashlash (trim)."},
        {"uzpp::Matn::kichik_harfga","**`uzpp::Matn::kichik_harfga(matn)`** → `matn` — Kichik harflarga o'girish (toLower)."},
        {"uzpp::Matn::katta_harfga","**`uzpp::Matn::katta_harfga(matn)`** → `matn` — Katta harflarga o'girish (toUpper)."},
        {"uzpp::Matn::almashtirish","**`uzpp::Matn::almashtirish(matn, eski, yangi)`** → `matn` — Barcha uchrashmalarni almashtirish."},
        {"uzpp::Matn::teskari","**`uzpp::Matn::teskari(matn)`** → `matn` — Matnni teskari aylantirish (reverse)."},
        // JSON
        {"uzpp::Json::yaratObyekt","**`uzpp::Json::yaratObyekt()`** → `shared_ptr<Qiymat>` — Bo'sh JSON obyekti.\\n\\n```\\nozgaruvchan jObj = uzpp::Json::yaratObyekt();\\njObj->set(\"ism\", uzpp::Json::yaratMatn(\"Ali\"));\\n```"},
        {"uzpp::Json::tahlil","**`uzpp::Json::tahlil(matn)`** → `shared_ptr<Qiymat>` — JSON satrini tahlil qilish.\\n\\n```\\nozgaruvchan j = uzpp::Json::tahlil(\"{\\\\\"ism\\\\\": \\\\\"Ali\\\\\"}\");\\n```"},
        {"uzpp::Json::chiqarish","**`uzpp::Json::chiqarish(qiymat, tirtish)`** → `matn` — JSON ni formatli satrga aylantirish. `tirtish=2` inson uchun o'qilishi."}
    };

    auto it = docs.find(word);
    if (it != docs.end()) return it->second;
    return "";
}

// Walks the AST to find a VariableDeclaration at (targetLine, targetChar)
// and returns its inferred type from the TypeChecker. Returns ""
// if not found or type couldn't be inferred.
std::string LspServer::getInferredTypeAtPosition(const std::string& uri, int targetLine, int targetChar, const std::string& word) {
    auto it = documentCache_.find(uri);
    if (it == documentCache_.end()) return "";

    try {
        Lexer lexer(it->second);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto program = parser.parse();

        TypeChecker checker;
        checker.check(program.get());

        // Iterate all inferred auto types, find the one at cursor position
        for (const auto& [var, type] : checker.getInferredAutoTypes()) {
            const auto& tok = var->getDeclToken();
            // LSP uses 0-based lines, uzpp tokens use 1-based
            if (tok.line - 1 == targetLine && var->getName() == word) {
                return type;
            }
        }
        return "";
    } catch (...) { return ""; }
}

// Implementation of missing LSP methods
// Walk the AST and collect all class/struct member names (fields and methods)
// for semantic-token highlighting.
void LspServer::collectClassMembers(const ASTNode* node, std::unordered_set<std::string>& members) {
    if (!node) return;

    if (node->getType() == ASTNodeType::ClassDeclaration) {
        auto cls = static_cast<const ClassDeclaration*>(node);
        // Collect field names
        for (const auto& member : cls->getMembers()) {
            members.insert(member.name);
        }
        // Collect method names and recurse into method bodies
        for (const auto& method : cls->getMethods()) {
            members.insert(method->name);
            if (method->body) {
                collectClassMembers(method->body.get(), members);
            }
        }
        return;
    }

    // Recurse: Program, Block
    if (node->getType() == ASTNodeType::Program) {
        const auto& children = static_cast<const Program*>(node)->getChildren();
        for (const auto& child : children) {
            collectClassMembers(child.get(), members);
        }
        return;
    }
    if (node->getType() == ASTNodeType::Block) {
        const auto& stmts = static_cast<const Block*>(node)->getStatements();
        for (const auto& stmt : stmts) {
            collectClassMembers(stmt.get(), members);
        }
        return;
    }

    // Function body
    if (auto* fn = dynamic_cast<const FunctionDeclaration*>(node)) {
        if (fn->getBody()) collectClassMembers(fn->getBody(), members);
        return;
    }

    // If statement
    if (auto* ifs = dynamic_cast<const IfStatement*>(node)) {
        if (ifs->getThenBranch()) collectClassMembers(ifs->getThenBranch(), members);
        if (ifs->getElseBranch()) collectClassMembers(ifs->getElseBranch(), members);
        return;
    }

    // While loop
    if (auto* ws = dynamic_cast<const WhileStatement*>(node)) {
        if (ws->getBody()) collectClassMembers(ws->getBody(), members);
        return;
    }

    // For loop
    if (auto* fs = dynamic_cast<const ForStatement*>(node)) {
        if (fs->getBody()) collectClassMembers(fs->getBody(), members);
        return;
    }

    // Try-catch
    if (auto* ts = dynamic_cast<const TryStatement*>(node)) {
        if (ts->getTryBlock()) collectClassMembers(ts->getTryBlock(), members);
        for (const auto& c : ts->getCatchClauses()) {
            if (c->block) collectClassMembers(c->block.get(), members);
        }
        return;
    }
}

// Encode the document as LSP semantic tokens: array of 5-tuples
// [deltaLine, deltaStartChar, length, tokenType, tokenModifier].
// We classify every identifier-like word against three sets:
//   - keyword  (control flow, declarations, modifiers)
//   - type     (primitive + stdlib types)
//   - function (well-known stdlib calls)
// Everything else is left unclassified so the IDE's syntax highlighter
// can handle user identifiers normally.
std::string LspServer::buildSemanticTokens(const std::string& text) {
    // Type indices MUST match the legend declared in `initialize`:
    //   0=keyword 1=type 2=function 3=variable 4=namespace
    //   5=string  6=number 7=comment 8=operator
    static const std::unordered_map<std::string, int> kind = {
        // keywords (0)
        {"agar",0},{"aks",0},{"aks_holda",0},{"uchun",0},{"toki",0},{"qaytarish",0},{"qaytish",0},
        {"to'xtatish",0},{"davom_etish",0},{"sinf",0},{"tuzilma",0},{"birlashma",0},{"funksiya",0},
        {"shablon",0},{"shartnoma",0},{"shart",0},{"mavhum",0},{"meros",0},{"amalga_oshirish",0},
        {"statik",0},{"ulash",0},{"eksport",0},{"import",0},{"modul",0},{"nomlar_fazosi",0},
        {"urinish",0},{"ushlash",0},{"irgitish",0},{"asinxron",0},{"kutish",0},{"chiqar_qadam",0},
        {"yangi",0},{"o'chirish",0},{"ochirish",0},{"ko'chirish",0},{"va",0},{"yoki",0},
        {"rost",0},{"yolg'on",0},{"noto'g'ri",0},{"to'g'ri",0},{"null",0},
        {"moslash",0},{"holat",0},{"boshqa",0},{"sanab_olish",0},{"tushuncha",0},{"makro",0},
        {"o'zgaruvchan",0},{"ozgaruvchan",0},{"o'zgarmas",0},{"ozgarmas",0},
        {"sobit_ifoda",0},{"sobit_baholash",0},{"sobit_boshlangich",0},
        {"o'zgarmas_ifoda",0},{"o'zgarmas_baholash",0},{"o'zgarmas_boshlangich",0},
        {"statik_otkazish",0},{"dinamik_otkazish",0},{"o'zgarmas_otkazish",0},{"qayta_otkazish",0},
        {"sabit_otkazish",0},{"statik_tasdiqlash",0},{"xato_tashlamaydi",0},
        {"ustidan_yozish",0},{"dust",0},{"tashqi",0},{"tur",0},{"bu",0},{"oz",0},
        {"qator_ichi",0},{"oqim_mahalliy",0},{"ozgaruvchi_o'zgartirish",0},
        {"tur_baholash",0},{"decltype",0},
        // types (1)
        {"butun",1},{"haqiqiy",1},{"kasr",1},{"belgi",1},{"mantiqiy",1},{"matn",1},{"bosh",1},
        {"vektor",1},{"lug'at",1},{"to'plam",1},{"ixtiyoriy",1},{"juftlik",1},{"uchlik",1},
        {"yagona_korsatkich",1},{"umumiy_korsatkich",1},{"kuchsiz_korsatkich",1},
        {"musbat_butun",1},{"musbat_qisqa",1},{"musbat_uzun",1},{"musbat_belgi",1},
        {"qisqa",1},{"uzun",1},{"butun8",1},{"butun16",1},{"butun32",1},{"butun64",1},
        {"musbat_butun8",1},{"musbat_butun16",1},{"musbat_butun32",1},{"musbat_butun64",1},
        {"hajm_turi",1},{"atomik",1},{"atomik_bayroq",1},{"boshlovchi_royxat",1},
        {"Natija",1},{"Tanlov",1},{"funksiya_tur",1},
        // functions (2)
        {"yozish",2},{"chiqarish",2},{"olish",2},{"kiritish",2},{"qator_oxiri",2},
        {"tartibla",2},{"qidirish",2},{"saralash",2},{"teskari",2},
        {"map",2},{"filter",2},{"filtr",2},{"xaritalash",2},{"yigish",2},
        {"yangi_yagona",2},{"yangi_umumiy",2},
    };

    // Pre-scan: collect class/struct member names for semantic highlighting
    std::unordered_set<std::string> classMembers;
    try {
        Lexer lexer(text);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto program = parser.parse();
        collectClassMembers(program.get(), classMembers);
    } catch (...) {}

    std::ostringstream out;
    int prevLine = 0, prevStart = 0;
    bool first = true;
    int lineNum = 0;
    std::size_t pos = 0;
    while (pos < text.size()) {
        // line start: scan tokens until newline
        std::size_t lineStart = pos;
        std::size_t i = pos;
        bool inBlockComment = false;
        while (i < text.size() && text[i] != '\n') {
            // skip line comments
            if (!inBlockComment && i + 1 < text.size() && text[i] == '/' && text[i + 1] == '/') {
                int col = static_cast<int>(i - lineStart);
                int len = 0;
                while (i + len < text.size() && text[i + len] != '\n') len++;
                int dLine = lineNum - prevLine;
                int dStart = (dLine == 0) ? col - prevStart : col;
                if (!first) out << ",";
                out << dLine << "," << dStart << "," << len << ",7,0";
                first = false;
                prevLine = lineNum; prevStart = col;
                i += len;
                continue;
            }
            // skip strings — don't try to tokenize inside
            if (!inBlockComment && text[i] == '"') {
                std::size_t s = i;
                i++;
                while (i < text.size() && text[i] != '"' && text[i] != '\n') {
                    if (text[i] == '\\' && i + 1 < text.size()) i++;
                    i++;
                }
                if (i < text.size() && text[i] == '"') i++;
                int col = static_cast<int>(s - lineStart);
                int len = static_cast<int>(i - s);
                int dLine = lineNum - prevLine;
                int dStart = (dLine == 0) ? col - prevStart : col;
                if (!first) out << ",";
                out << dLine << "," << dStart << "," << len << ",5,0";
                first = false;
                prevLine = lineNum; prevStart = col;
                continue;
            }
            // identifier — alphanumeric/_/apostrophe/backtick start
            unsigned char c = static_cast<unsigned char>(text[i]);
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
                std::size_t s = i;
                while (i < text.size()) {
                    unsigned char cc = static_cast<unsigned char>(text[i]);
                    if ((cc >= 'a' && cc <= 'z') || (cc >= 'A' && cc <= 'Z') ||
                        (cc >= '0' && cc <= '9') || cc == '_' || cc == '\'' || cc == '`') {
                        i++;
                    } else break;
                }
                std::string word = text.substr(s, i - s);
                auto it = kind.find(word);
                if (it != kind.end()) {
                    int col = static_cast<int>(s - lineStart);
                    int len = static_cast<int>(i - s);
                    int dLine = lineNum - prevLine;
                    int dStart = (dLine == 0) ? col - prevStart : col;
                    if (!first) out << ",";
                    out << dLine << "," << dStart << "," << len << "," << it->second << ",0";
                    first = false;
                    prevLine = lineNum; prevStart = col;
                } else if (classMembers.contains(word)) {
                    int col = static_cast<int>(s - lineStart);
                    int len = static_cast<int>(i - s);
                    int dLine = lineNum - prevLine;
                    int dStart = (dLine == 0) ? col - prevStart : col;
                    if (!first) out << ",";
                    out << dLine << "," << dStart << "," << len << ",3,0";
                    first = false;
                    prevLine = lineNum; prevStart = col;
                }
                continue;
            }
            i++;
        }
        // advance past newline
        if (i < text.size() && text[i] == '\n') i++;
        pos = i;
        lineNum++;
    }
    return out.str();
}

std::string LspServer::getWordAtPosition(const std::string& text, int line, int character) {
    std::istringstream stream(text);
    std::string currentLine;
    int currentLineNum = 0;
    
    while (std::getline(stream, currentLine) && currentLineNum < line) {
        currentLineNum++;
    }
    
    if (currentLineNum != line || character > static_cast<int>(currentLine.size())) {
        return "";
    }
    
    int start = character;
    int end = character;
    
    while (start > 0 && (std::isalnum(currentLine[start - 1]) || currentLine[start - 1] == '_')) {
        start--;
    }
    while (end < static_cast<int>(currentLine.size()) && (std::isalnum(currentLine[end]) || currentLine[end] == '_')) {
        end++;
    }
    
    return currentLine.substr(start, end - start);
}

std::string LspServer::findDefinition(const std::string& uri, const std::string& word) {
    auto it = documentCache_.find(uri);
    if (it == documentCache_.end()) return "[]";
    try {
        Lexer lexer(it->second);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto program = parser.parse();
        
        for (const auto& node : program->getChildren()) {
            if (node->getType() == ASTNodeType::FunctionDeclaration) {
                auto func = static_cast<const FunctionDeclaration*>(node.get());
                if (func->getName() == word) {
                    int line = std::max(0, func->getFunctionToken().line - 1);
                    int col = std::max(0, func->getFunctionToken().column - 1);
                    return "[{\"uri\":\"" + uri + "\",\"range\":{\"start\":{\"line\":" + std::to_string(line) + ",\"character\":" + std::to_string(col) + "},\"end\":{\"line\":" + std::to_string(line) + ",\"character\":" + std::to_string(col) + "}}}]";
                }
            } else if (node->getType() == ASTNodeType::ClassDeclaration) {
                auto cls = static_cast<const ClassDeclaration*>(node.get());
                if (cls->getName() == word) {
                    int line = std::max(0, cls->getClassToken().line - 1);
                    int col = std::max(0, cls->getClassToken().column - 1);
                    return "[{\"uri\":\"" + uri + "\",\"range\":{\"start\":{\"line\":" + std::to_string(line) + ",\"character\":" + std::to_string(col) + "},\"end\":{\"line\":" + std::to_string(line) + ",\"character\":" + std::to_string(col) + "}}}]";
                }
            } else if (node->getType() == ASTNodeType::VariableDeclaration) {
                auto varDecl = static_cast<const VariableDeclaration*>(node.get());
                if (varDecl->getName() == word) {
                    int line = std::max(0, varDecl->getDeclToken().line - 1);
                    int col = std::max(0, varDecl->getDeclToken().column - 1);
                    return "[{\"uri\":\"" + uri + "\",\"range\":{\"start\":{\"line\":" + std::to_string(line) + ",\"character\":" + std::to_string(col) + "},\"end\":{\"line\":" + std::to_string(line) + ",\"character\":" + std::to_string(col) + "}}}]";
                }
            }
        }
    } catch (...) {}
    return "[]";
}

std::string LspServer::buildSignatureHelp(const std::string& uri, int line, int character) {
    auto it = documentCache_.find(uri);
    if (it == documentCache_.end()) return "{\"signatures\":[]}";
    
    std::string text = it->second;
    std::istringstream stream(text);
    std::string currentLine;
    int currentLineNum = 0;
    while (std::getline(stream, currentLine) && currentLineNum < line) {
        currentLineNum++;
    }
    
    if (currentLineNum != line || character > static_cast<int>(currentLine.size())) {
        return "{\"signatures\":[]}";
    }
    
    int pos = character - 1;
    while (pos >= 0 && currentLine[pos] != '(') {
        pos--;
    }
    
    if (pos > 0 && currentLine[pos] == '(') {
        std::string funcName = getWordAtPosition(text, line, pos);
        if (!funcName.empty()) {
            std::string doc = buildHover(funcName);
            if (!doc.empty()) {
                std::string escapedDoc;
                for (char c : doc) {
                    if (c == '"') escapedDoc += "\\\"";
                    else if (c == '\n') escapedDoc += "\\n";
                    else if (c == '\\') escapedDoc += "\\\\";
                    else escapedDoc += c;
                }
                return "{\"signatures\":[{\"label\":\"" + funcName + "(...)\",\"documentation\":{\"kind\":\"markdown\",\"value\":\"" + escapedDoc + "\"}}],\"activeSignature\":0,\"activeParameter\":0}";
            }
        }
    }
    
    return "{\"signatures\":[]}";
}

std::string LspServer::buildDocumentSymbols(const Program* program) {
    if (!program) return "[]";
    std::ostringstream ss;
    ss << "[";
    bool first = true;
    
    auto addSymbol = [&](const std::string& name, int kind, int line, int col) {
        if (!first) ss << ",";
        line = std::max(0, line - 1);
        col = std::max(0, col - 1);
        ss << "{\"name\":\"" << name << "\",\"kind\":" << kind 
           << ",\"range\":{\"start\":{\"line\":" << line << ",\"character\":" << col 
           << "},\"end\":{\"line\":" << line << ",\"character\":" << col 
           << "}},\"selectionRange\":{\"start\":{\"line\":" << line << ",\"character\":" << col 
           << "},\"end\":{\"line\":" << line << ",\"character\":" << col << "}}}";
        first = false;
    };

    for (const auto& node : program->getChildren()) {
        if (node->getType() == ASTNodeType::FunctionDeclaration) {
            auto func = static_cast<const FunctionDeclaration*>(node.get());
            addSymbol(func->getName(), 12, func->getFunctionToken().line, func->getFunctionToken().column);
        } else if (node->getType() == ASTNodeType::ClassDeclaration) {
            auto cls = static_cast<const ClassDeclaration*>(node.get());
            addSymbol(cls->getName(), 5, cls->getClassToken().line, cls->getClassToken().column);
        } else if (node->getType() == ASTNodeType::VariableDeclaration) {
            auto varDecl = static_cast<const VariableDeclaration*>(node.get());
            addSymbol(varDecl->getName(), 13, varDecl->getDeclToken().line, varDecl->getDeclToken().column);
        } else if (node->getType() == ASTNodeType::EnumDeclaration) {
            auto enumDecl = static_cast<const EnumDeclaration*>(node.get());
            addSymbol(enumDecl->getName(), 10, enumDecl->getToken().line, enumDecl->getToken().column);
        }
    }
    ss << "]";
    return ss.str();
}

std::string LspServer::buildInlayHints(const std::string& uri) {
    if (!documentCache_.contains(uri)) return "[]";
    return computeInlayHints(documentCache_[uri]);
}

// Walks the AST and emits LSP `InlayHint` items showing the inferred type for
// each `o'zgaruvchan x = ...` declaration. The position is placed right after
// the variable name; the IDE renders it as a faded `: <type>` annotation.
std::string LspServer::computeInlayHints(const std::string& text) {
    std::unique_ptr<Program> program;
    TypeChecker checker;
    try {
        Lexer lexer(text);
        const auto tokens = lexer.tokenize();
        Parser parser(tokens);
        program = parser.parse();
        checker.check(program.get());
    } catch (...) {
        return "[]";
    }
    if (!program) return "[]";

    std::ostringstream ss;
    ss << "[";
    bool first = true;

    auto emit = [&](const VariableDeclaration* var) {
        const std::string* inferred = checker.getInferredAutoType(var);
        if (inferred == nullptr || inferred->empty() || *inferred == "noma'lum") return;
        const Token& tok = var->getDeclToken();
        if (tok.line <= 0) return;
        const int line = tok.line - 1;
        const int character = std::max(0, tok.column - 1)
                            + static_cast<int>(var->getName().size());
        if (!first) ss << ",";
        ss << "{\"position\":{\"line\":" << line
           << ",\"character\":" << character
           << "},\"label\":\": " << *inferred
           << "\",\"kind\":1,\"paddingLeft\":false}";
        first = false;
    };

    std::function<void(const ASTNode*)> walk = [&](const ASTNode* node) {
        if (node == nullptr) return;
        switch (node->getType()) {
            case ASTNodeType::VariableDeclaration:
                emit(static_cast<const VariableDeclaration*>(node));
                return;
            case ASTNodeType::Block:
                for (const auto& s : static_cast<const Block*>(node)->getStatements()) walk(s.get());
                return;
            case ASTNodeType::StatementList:
                for (const auto& s : static_cast<const StatementList*>(node)->getStatements()) walk(s.get());
                return;
            case ASTNodeType::IfStatement: {
                const auto* ifs = static_cast<const IfStatement*>(node);
                walk(ifs->getThenBranch());
                walk(ifs->getElseBranch());
                return;
            }
            case ASTNodeType::WhileStatement:
                walk(static_cast<const WhileStatement*>(node)->getBody());
                return;
            case ASTNodeType::ForStatement: {
                const auto* fs = static_cast<const ForStatement*>(node);
                walk(fs->getInit());
                walk(fs->getBody());
                return;
            }
            case ASTNodeType::TryStatement: {
                const auto* ts = static_cast<const TryStatement*>(node);
                walk(ts->getTryBlock());
                for (const auto& cc : ts->getCatchClauses()) walk(cc->block.get());
                return;
            }
            case ASTNodeType::FunctionDeclaration:
                walk(static_cast<const FunctionDeclaration*>(node)->getBody());
                return;
            case ASTNodeType::ClassDeclaration:
                for (const auto& m : static_cast<const ClassDeclaration*>(node)->getMethods()) {
                    walk(m->body.get());
                }
                return;
            default:
                return;
        }
    };

    for (const auto& child : program->getChildren()) walk(child.get());

    ss << "]";
    return ss.str();
}

std::string LspServer::buildCodeActions(const std::string& uri,
                                        int rangeStartLine, int rangeStartChar,
                                        int rangeEndLine, int rangeEndChar) {
    if (!documentCache_.contains(uri)) return "[]";
    return computeCodeActions(documentCache_[uri], uri,
                              rangeStartLine, rangeStartChar,
                              rangeEndLine, rangeEndChar);
}

// Emits quick-fix CodeActions for TypeChecker warnings whose line is inside
// the request range. Currently handles the "unused variable" warning with two
// fixes: prefix with `_` (mark intentional) or remove the declaration line.
std::string LspServer::computeCodeActions(const std::string& text,
                                          const std::string& uri,
                                          int rangeStartLine, int /*rangeStartChar*/,
                                          int rangeEndLine, int /*rangeEndChar*/) {
    std::unique_ptr<Program> program;
    TypeChecker checker;
    try {
        Lexer lexer(text);
        const auto tokens = lexer.tokenize();
        Parser parser(tokens);
        program = parser.parse();
        checker.check(program.get());
    } catch (...) {
        return "[]";
    }
    if (!program) return "[]";

    // Split text into lines for "remove declaration" edits.
    std::vector<std::string> lines;
    {
        std::string cur;
        for (char c : text) {
            if (c == '\n') { lines.push_back(std::move(cur)); cur.clear(); }
            else if (c != '\r') cur += c;
        }
        lines.push_back(std::move(cur));
    }

    static const std::string kUnusedPrefix = "O'zgaruvchi '";
    static const std::string kUnusedSuffix = "' e'lon qilingan, lekin ishlatilmagan.";

    std::ostringstream out;
    out << "[";
    bool first = true;

    for (const auto& w : checker.getWarnings()) {
        if (w.message.size() < kUnusedPrefix.size() + kUnusedSuffix.size()) continue;
        if (w.message.compare(0, kUnusedPrefix.size(), kUnusedPrefix) != 0) continue;
        if (w.message.compare(w.message.size() - kUnusedSuffix.size(),
                              kUnusedSuffix.size(), kUnusedSuffix) != 0) continue;
        const std::string name = w.message.substr(
            kUnusedPrefix.size(),
            w.message.size() - kUnusedPrefix.size() - kUnusedSuffix.size());
        if (w.line <= 0) continue;
        const int lineIdx = w.line - 1;
        if (lineIdx < rangeStartLine || lineIdx > rangeEndLine) continue;
        if (lineIdx >= static_cast<int>(lines.size())) continue;
        const int col = std::max(0, w.column - 1);

        // Quick-fix 1: prefix with `_`
        if (!first) out << ",";
        out << "{\"title\":\"O'zgaruvchini '_" << name
            << "' deb belgilash (ishlatilmaganini ko'rsatish)\",\"kind\":\"quickfix\","
            << "\"edit\":{\"changes\":{\"" << uri << "\":["
            << "{\"range\":{\"start\":{\"line\":" << lineIdx
            << ",\"character\":" << col
            << "},\"end\":{\"line\":" << lineIdx
            << ",\"character\":" << (col + static_cast<int>(name.size()))
            << "}},\"newText\":\"_" << name << "\"}]}}}";
        first = false;

        // Quick-fix 2: remove the entire declaration line
        out << ",{\"title\":\"E'lonni o'chirish (qator " << w.line
            << ")\",\"kind\":\"quickfix\","
            << "\"edit\":{\"changes\":{\"" << uri << "\":["
            << "{\"range\":{\"start\":{\"line\":" << lineIdx
            << ",\"character\":0},\"end\":{\"line\":" << (lineIdx + 1)
            << ",\"character\":0}},\"newText\":\"\"}]}}}";
    }

    // Code action: unreachable code → remove the line
    static const std::string kUnreachable = "Ushbu kodga hech qachon etib kelinmaydi";
    for (const auto& w : checker.getWarnings()) {
        if (w.message.compare(0, kUnreachable.size(), kUnreachable) != 0) continue;
        if (w.line <= 0) continue;
        const int lineIdx = w.line - 1;
        if (lineIdx < rangeStartLine || lineIdx > rangeEndLine) continue;
        if (!first) out << ",";
        first = false;
        out << "{\"title\":\"Yetib bo'lmaydigan kodni o'chirish (qator " << w.line
            << ")\",\"kind\":\"quickfix\","
            << "\"edit\":{\"changes\":{\"" << uri << "\":["
            << "{\"range\":{\"start\":{\"line\":" << lineIdx
            << ",\"character\":0},\"end\":{\"line\":" << (lineIdx + 1)
            << ",\"character\":0}},\"newText\":\"\"}]}}}";
    }

    // Code action: type mismatch → wrap initializer with static_cast
    // Warning format: "Tur nomutanosibligi: '<from>' kutilgan, lekin '<to>' berildi."
    static const std::string kTypeMismatchPrefix = "Tur nomutanosibligi: '";
    static const std::string kTypeMismatchMid = "' kutilgan, lekin '";
    for (const auto& w : checker.getWarnings()) {
        if (w.message.compare(0, kTypeMismatchPrefix.size(), kTypeMismatchPrefix) != 0) continue;
        const auto midPos = w.message.find(kTypeMismatchMid, kTypeMismatchPrefix.size());
        if (midPos == std::string::npos) continue;
        const std::string fromType = w.message.substr(kTypeMismatchPrefix.size(),
            midPos - kTypeMismatchPrefix.size());
        if (w.line <= 0) continue;
        const int lineIdx = w.line - 1;
        if (lineIdx < rangeStartLine || lineIdx > rangeEndLine) continue;
        if (lineIdx >= static_cast<int>(lines.size())) continue;

        // Find the '=' on this line to wrap the initializer
        const std::string& srcLine = lines[lineIdx];
        const auto eqPos = srcLine.find('=');
        if (eqPos == std::string::npos) continue;
        // The initializer starts after '=' plus optional whitespace
        size_t initStart = eqPos + 1;
        while (initStart < srcLine.size() && (srcLine[initStart] == ' ' || srcLine[initStart] == '\t'))
            initStart++;
        if (initStart >= srcLine.size()) continue;
        // End of initializer: before ';' or end of line
        size_t initEnd = srcLine.find(';', initStart);
        if (initEnd == std::string::npos) initEnd = srcLine.size();
        while (initEnd > initStart && (srcLine[initEnd-1] == ' ' || srcLine[initEnd-1] == '\t'))
            initEnd--;

        if (!first) out << ",";
        first = false;
        out << "{\"title\":\"statik_otkazish<" << fromType
            << ">(...) bilan o'rash\",\"kind\":\"quickfix\","
            << "\"edit\":{\"changes\":{\"" << uri << "\":["
            << "{\"range\":{\"start\":{\"line\":" << lineIdx
            << ",\"character\":" << initStart
            << "},\"end\":{\"line\":" << lineIdx
            << ",\"character\":" << initEnd << "}},"
            << "\"newText\":\"statik_otkazish<" << fromType << ">("
            << srcLine.substr(initStart, initEnd - initStart) << ")\"}]}}}";
    }

    // Code action: return type mismatch → wrap with static_cast
    // Warning format: "Funksiya '<type>' qaytarishi kerak, lekin '<type>' qaytarilmoqda."
    static const std::string kReturnMismatchPrefix = "Funksiya '";
    static const std::string kReturnMismatchMid = "' qaytarishi kerak, lekin '";
    for (const auto& w : checker.getWarnings()) {
        if (w.message.compare(0, kReturnMismatchPrefix.size(), kReturnMismatchPrefix) != 0) continue;
        const auto midPos = w.message.find(kReturnMismatchMid, kReturnMismatchPrefix.size());
        if (midPos == std::string::npos) continue;
        const std::string retType = w.message.substr(kReturnMismatchPrefix.size(),
            midPos - kReturnMismatchPrefix.size());
        if (w.line <= 0) continue;
        const int lineIdx = w.line - 1;
        if (lineIdx < rangeStartLine || lineIdx > rangeEndLine) continue;
        if (lineIdx >= static_cast<int>(lines.size())) continue;

        // Find 'qaytarish' or 'qaytish' on this line
        const std::string& srcLine = lines[lineIdx];
        size_t retPos = srcLine.find("qaytarish");
        if (retPos == std::string::npos) retPos = srcLine.find("qaytish");
        if (retPos == std::string::npos) continue;
        // Start after the keyword
        size_t exprStart = retPos + (srcLine[retPos] == 'q' ? std::string("qaytarish").size() : std::string("qaytish").size());
        while (exprStart < srcLine.size() && (srcLine[exprStart] == ' ' || srcLine[exprStart] == '\t'))
            exprStart++;
        if (exprStart >= srcLine.size()) continue;
        size_t exprEnd = srcLine.find(';', exprStart);
        if (exprEnd == std::string::npos) exprEnd = srcLine.size();
        while (exprEnd > exprStart && (srcLine[exprEnd-1] == ' ' || srcLine[exprEnd-1] == '\t'))
            exprEnd--;

        if (!first) out << ",";
        first = false;
        out << "{\"title\":\"statik_otkazish<" << retType
            << ">(...) bilan o'rash\",\"kind\":\"quickfix\","
            << "\"edit\":{\"changes\":{\"" << uri << "\":["
            << "{\"range\":{\"start\":{\"line\":" << lineIdx
            << ",\"character\":" << exprStart
            << "},\"end\":{\"line\":" << lineIdx
            << ",\"character\":" << exprEnd << "}},"
            << "\"newText\":\"statik_otkazish<" << retType << ">("
            << srcLine.substr(exprStart, exprEnd - exprStart) << ")\"}]}}}";
    }
    out << "]";
    return out.str();
}

void LspServer::applyContentChanges(std::string& document, const std::string& contentChangesJson) {
    // Simple text extraction from contentChangesJson
    std::string textKey = "\"text\":\"";
    size_t pos = contentChangesJson.find(textKey);
    if (pos != std::string::npos) {
        pos += textKey.length();
        size_t endPos = contentChangesJson.find("\"", pos);
        if (endPos != std::string::npos) {
            std::string newText = contentChangesJson.substr(pos, endPos - pos);
            // Handle escape sequences
            size_t escapePos = 0;
            while ((escapePos = newText.find("\\n", escapePos)) != std::string::npos) {
                newText.replace(escapePos, 2, "\n");
            }
            document = newText;
        }
    }
}

// ===== AST-AWARE DEFINITION / REFERENCES / RENAME =====

// Helper: format a single LSP Location as a JSON fragment.
static std::string formatLocation(const std::string& uri, int line, int col, int endCol) {
    int l = line > 0 ? line - 1 : 0;
    int c = col > 0 ? col - 1 : 0;
    return "{\"uri\":\"" + uri + "\",\"range\":{\"start\":{\"line\":" +
           std::to_string(l) + ",\"character\":" + std::to_string(c) +
           "},\"end\":{\"line\":" + std::to_string(l) +
           ",\"character\":" + std::to_string(c + endCol) + "}}}";
}

void LspServer::collectDefinitions(const ASTNode* node, const std::string& word,
                                    const std::string& uri, std::ostringstream& out, bool& first) {
    if (!node) return;
    switch (node->getType()) {
        case ASTNodeType::FunctionDeclaration: {
            auto* f = static_cast<const FunctionDeclaration*>(node);
            if (f->getName() == word) {
                if (!first) out << ","; first = false;
                out << formatLocation(uri, f->getFunctionToken().line,
                                      f->getFunctionToken().column, static_cast<int>(word.size()));
            }
            // Recurse into body
            if (f->getBody()) collectDefinitions(f->getBody(), word, uri, out, first);
            break;
        }
        case ASTNodeType::ClassDeclaration: {
            auto* c = static_cast<const ClassDeclaration*>(node);
            if (c->getName() == word) {
                if (!first) out << ","; first = false;
                out << formatLocation(uri, c->getClassToken().line,
                                      c->getClassToken().column, static_cast<int>(word.size()));
            }
            for (const auto& m : c->getMethods()) {
                if (m->name == word) {
                    if (!first) out << ","; first = false;
                    out << formatLocation(uri, m->token.line, m->token.column, static_cast<int>(word.size()));
                }
                if (m->body) collectDefinitions(m->body.get(), word, uri, out, first);
            }
            for (const auto& m : c->getMembers()) {
                if (m.name == word) {
                    if (!first) out << ","; first = false;
                    out << formatLocation(uri, m.token.line, m.token.column, static_cast<int>(word.size()));
                }
            }
            break;
        }
        case ASTNodeType::VariableDeclaration: {
            auto* v = static_cast<const VariableDeclaration*>(node);
            if (v->getName() == word) {
                if (!first) out << ","; first = false;
                out << formatLocation(uri, v->getDeclToken().line,
                                      v->getDeclToken().column, static_cast<int>(word.size()));
            }
            break;
        }
        case ASTNodeType::NamespaceDeclaration: {
            auto* ns = static_cast<const NamespaceDeclaration*>(node);
            for (const auto& child : ns->getChildren())
                collectDefinitions(child.get(), word, uri, out, first);
            break;
        }
        case ASTNodeType::Block: {
            auto* b = static_cast<const Block*>(node);
            for (const auto& s : b->getStatements())
                collectDefinitions(s.get(), word, uri, out, first);
            break;
        }
        case ASTNodeType::ForStatement: {
            auto* fs = static_cast<const ForStatement*>(node);
            if (fs->getInit()) collectDefinitions(fs->getInit(), word, uri, out, first);
            if (fs->getBody()) collectDefinitions(fs->getBody(), word, uri, out, first);
            break;
        }
        case ASTNodeType::IfStatement: {
            auto* ifs = static_cast<const IfStatement*>(node);
            if (ifs->getThenBranch()) collectDefinitions(ifs->getThenBranch(), word, uri, out, first);
            if (ifs->getElseBranch()) collectDefinitions(ifs->getElseBranch(), word, uri, out, first);
            break;
        }
        case ASTNodeType::WhileStatement: {
            auto* ws = static_cast<const WhileStatement*>(node);
            if (ws->getBody()) collectDefinitions(ws->getBody(), word, uri, out, first);
            break;
        }
        case ASTNodeType::MatchStatement: {
            auto* ms = static_cast<const MatchStatement*>(node);
            for (const auto& c : ms->getCases())
                if (c->body) collectDefinitions(c->body.get(), word, uri, out, first);
            break;
        }
        case ASTNodeType::TryStatement: {
            auto* ts = static_cast<const TryStatement*>(node);
            if (ts->getTryBlock()) collectDefinitions(ts->getTryBlock(), word, uri, out, first);
            for (const auto& cc : ts->getCatchClauses())
                if (cc->block) collectDefinitions(cc->block.get(), word, uri, out, first);
            break;
        }
        case ASTNodeType::Program: {
            auto* p = static_cast<const Program*>(node);
            for (const auto& child : p->getChildren())
                collectDefinitions(child.get(), word, uri, out, first);
            break;
        }
        case ASTNodeType::EnumDeclaration: {
            auto* e = static_cast<const EnumDeclaration*>(node);
            if (e->getName() == word) {
                if (!first) out << ","; first = false;
                out << formatLocation(uri, e->getToken().line,
                                      e->getToken().column, static_cast<int>(word.size()));
            }
            break;
        }
        case ASTNodeType::Group: {
            auto* g = static_cast<const GroupNode*>(node);
            for (const auto& child : g->getChildren())
                collectDefinitions(child.get(), word, uri, out, first);
            break;
        }
        case ASTNodeType::ExpressionStatement: {
            auto* es = static_cast<const ExpressionStatement*>(node);
            if (es->getExpression() && es->getExpression()->getType() == ASTNodeType::LambdaExpression) {
                auto* lam = static_cast<const LambdaExpression*>(es->getExpression());
                if (lam->getBody()) collectDefinitions(lam->getBody(), word, uri, out, first);
            }
            break;
        }
        default:
            break;
    }
}

void LspServer::collectReferences(const ASTNode* node, const std::string& word,
                                   const std::string& uri, std::ostringstream& out, bool& first) {
    if (!node) return;
    switch (node->getType()) {
        case ASTNodeType::IdentifierExpression: {
            auto* id = static_cast<const IdentifierExpression*>(node);
            if (id->getName() == word) {
                if (!first) out << ","; first = false;
                out << formatLocation(uri, id->getSourceToken().line,
                                      id->getSourceToken().column, static_cast<int>(word.size()));
            }
            break;
        }
        case ASTNodeType::BinaryExpression: {
            auto* b = static_cast<const BinaryExpression*>(node);
            collectReferences(b->getLeft(), word, uri, out, first);
            collectReferences(b->getRight(), word, uri, out, first);
            break;
        }
        case ASTNodeType::UnaryExpression: {
            auto* u = static_cast<const UnaryExpression*>(node);
            collectReferences(u->getExpression(), word, uri, out, first);
            break;
        }
        case ASTNodeType::FunctionCall: {
            auto* fc = static_cast<const FunctionCall*>(node);
            collectReferences(fc->getCallee(), word, uri, out, first);
            for (const auto& a : fc->getArguments())
                collectReferences(a.get(), word, uri, out, first);
            break;
        }
        case ASTNodeType::MemberAccess: {
            auto* ma = static_cast<const MemberAccess*>(node);
            collectReferences(ma->getObject(), word, uri, out, first);
            break;
        }
        case ASTNodeType::SubscriptAccess: {
            auto* sa = static_cast<const SubscriptAccess*>(node);
            collectReferences(sa->getArray(), word, uri, out, first);
            collectReferences(sa->getIndex(), word, uri, out, first);
            break;
        }
        case ASTNodeType::AssignmentExpression: {
            auto* ae = static_cast<const AssignmentExpression*>(node);
            collectReferences(ae->getTarget(), word, uri, out, first);
            collectReferences(ae->getValue(), word, uri, out, first);
            break;
        }
        case ASTNodeType::VariableDeclaration: {
            auto* v = static_cast<const VariableDeclaration*>(node);
            if (v->getInitializer())
                collectReferences(v->getInitializer(), word, uri, out, first);
            break;
        }
        case ASTNodeType::ReturnStatement: {
            auto* r = static_cast<const ReturnStatement*>(node);
            if (r->getValue()) collectReferences(r->getValue(), word, uri, out, first);
            break;
        }
        case ASTNodeType::ExpressionStatement: {
            auto* es = static_cast<const ExpressionStatement*>(node);
            collectReferences(es->getExpression(), word, uri, out, first);
            break;
        }
        case ASTNodeType::IfStatement: {
            auto* ifs = static_cast<const IfStatement*>(node);
            collectReferences(ifs->getCondition(), word, uri, out, first);
            if (ifs->getThenBranch()) collectReferences(ifs->getThenBranch(), word, uri, out, first);
            if (ifs->getElseBranch()) collectReferences(ifs->getElseBranch(), word, uri, out, first);
            break;
        }
        case ASTNodeType::WhileStatement: {
            auto* ws = static_cast<const WhileStatement*>(node);
            collectReferences(ws->getCondition(), word, uri, out, first);
            if (ws->getBody()) collectReferences(ws->getBody(), word, uri, out, first);
            break;
        }
        case ASTNodeType::ForStatement: {
            auto* fs = static_cast<const ForStatement*>(node);
            if (fs->getInit()) collectReferences(fs->getInit(), word, uri, out, first);
            if (fs->getCondition()) collectReferences(fs->getCondition(), word, uri, out, first);
            if (fs->getIncrement()) collectReferences(fs->getIncrement(), word, uri, out, first);
            if (fs->getBody()) collectReferences(fs->getBody(), word, uri, out, first);
            break;
        }
        case ASTNodeType::Block: {
            auto* b = static_cast<const Block*>(node);
            for (const auto& s : b->getStatements())
                collectReferences(s.get(), word, uri, out, first);
            break;
        }
        case ASTNodeType::StatementList: {
            auto* sl = static_cast<const StatementList*>(node);
            for (const auto& s : sl->getStatements())
                collectReferences(s.get(), word, uri, out, first);
            break;
        }
        case ASTNodeType::MatchStatement: {
            auto* ms = static_cast<const MatchStatement*>(node);
            collectReferences(ms->getCondition(), word, uri, out, first);
            for (const auto& c : ms->getCases()) {
                if (c->pattern) collectReferences(c->pattern.get(), word, uri, out, first);
                if (c->body) collectReferences(c->body.get(), word, uri, out, first);
            }
            break;
        }
        case ASTNodeType::TryStatement: {
            auto* ts = static_cast<const TryStatement*>(node);
            if (ts->getTryBlock()) collectReferences(ts->getTryBlock(), word, uri, out, first);
            for (const auto& cc : ts->getCatchClauses())
                if (cc->block) collectReferences(cc->block.get(), word, uri, out, first);
            break;
        }
        case ASTNodeType::FunctionDeclaration: {
            auto* f = static_cast<const FunctionDeclaration*>(node);
            if (f->getBody()) collectReferences(f->getBody(), word, uri, out, first);
            break;
        }
        case ASTNodeType::ClassDeclaration: {
            auto* c = static_cast<const ClassDeclaration*>(node);
            for (const auto& m : c->getMethods())
                if (m->body) collectReferences(m->body.get(), word, uri, out, first);
            break;
        }
        case ASTNodeType::NamespaceDeclaration: {
            auto* ns = static_cast<const NamespaceDeclaration*>(node);
            for (const auto& child : ns->getChildren())
                collectReferences(child.get(), word, uri, out, first);
            break;
        }
        case ASTNodeType::Program: {
            auto* p = static_cast<const Program*>(node);
            for (const auto& child : p->getChildren())
                collectReferences(child.get(), word, uri, out, first);
            break;
        }
        case ASTNodeType::Group: {
            auto* g = static_cast<const GroupNode*>(node);
            for (const auto& child : g->getChildren())
                collectReferences(child.get(), word, uri, out, first);
            break;
        }
        case ASTNodeType::ThrowExpression: {
            auto* te = static_cast<const ThrowExpression*>(node);
            collectReferences(te->getExpression(), word, uri, out, first);
            break;
        }
        case ASTNodeType::AwaitExpression: {
            auto* ae = static_cast<const AwaitExpression*>(node);
            collectReferences(ae->getExpression(), word, uri, out, first);
            break;
        }
        case ASTNodeType::PipelineExpression: {
            auto* pe = static_cast<const PipelineExpression*>(node);
            collectReferences(pe->getLeft(), word, uri, out, first);
            collectReferences(pe->getRight(), word, uri, out, first);
            break;
        }
        case ASTNodeType::TernaryExpression: {
            auto* te = static_cast<const TernaryExpression*>(node);
            collectReferences(te->getCondition(), word, uri, out, first);
            collectReferences(te->getThenExpr(), word, uri, out, first);
            collectReferences(te->getElseExpr(), word, uri, out, first);
            break;
        }
        default:
            break;
    }
}

void LspServer::collectRenameEdits(const ASTNode* node, const std::string& word,
                                    const std::string& newName, const std::string& uri,
                                    std::ostringstream& out, bool& first) {
    if (!node) return;
    if (node->getType() == ASTNodeType::IdentifierExpression) {
        auto* id = static_cast<const IdentifierExpression*>(node);
        if (id->getName() == word) {
            if (!first) out << ","; first = false;
            int l = id->getSourceToken().line > 0 ? id->getSourceToken().line - 1 : 0;
            int c = id->getSourceToken().column > 0 ? id->getSourceToken().column - 1 : 0;
            out << "{\"range\":{\"start\":{\"line\":" << l
                << ",\"character\":" << c << "},\"end\":{\"line\":" << l
                << ",\"character\":" << (c + static_cast<int>(word.size()))
                << "}},\"newText\":\"" << newName << "\"}";
        }
        return;
    }
    // Recurse into child nodes (same dispatch as collectReferences)
    switch (node->getType()) {
        case ASTNodeType::BinaryExpression: {
            auto* b = static_cast<const BinaryExpression*>(node);
            collectRenameEdits(b->getLeft(), word, newName, uri, out, first);
            collectRenameEdits(b->getRight(), word, newName, uri, out, first);
            break;
        }
        case ASTNodeType::UnaryExpression: {
            auto* u = static_cast<const UnaryExpression*>(node);
            collectRenameEdits(u->getExpression(), word, newName, uri, out, first);
            break;
        }
        case ASTNodeType::FunctionCall: {
            auto* fc = static_cast<const FunctionCall*>(node);
            collectRenameEdits(fc->getCallee(), word, newName, uri, out, first);
            for (const auto& a : fc->getArguments())
                collectRenameEdits(a.get(), word, newName, uri, out, first);
            break;
        }
        case ASTNodeType::MemberAccess: {
            auto* ma = static_cast<const MemberAccess*>(node);
            collectRenameEdits(ma->getObject(), word, newName, uri, out, first);
            break;
        }
        case ASTNodeType::SubscriptAccess: {
            auto* sa = static_cast<const SubscriptAccess*>(node);
            collectRenameEdits(sa->getArray(), word, newName, uri, out, first);
            collectRenameEdits(sa->getIndex(), word, newName, uri, out, first);
            break;
        }
        case ASTNodeType::AssignmentExpression: {
            auto* ae = static_cast<const AssignmentExpression*>(node);
            collectRenameEdits(ae->getTarget(), word, newName, uri, out, first);
            collectRenameEdits(ae->getValue(), word, newName, uri, out, first);
            break;
        }
        case ASTNodeType::VariableDeclaration: {
            auto* v = static_cast<const VariableDeclaration*>(node);
            if (v->getInitializer())
                collectRenameEdits(v->getInitializer(), word, newName, uri, out, first);
            break;
        }
        case ASTNodeType::ReturnStatement: {
            auto* r = static_cast<const ReturnStatement*>(node);
            if (r->getValue()) collectRenameEdits(r->getValue(), word, newName, uri, out, first);
            break;
        }
        case ASTNodeType::ExpressionStatement: {
            auto* es = static_cast<const ExpressionStatement*>(node);
            collectRenameEdits(es->getExpression(), word, newName, uri, out, first);
            break;
        }
        case ASTNodeType::IfStatement: {
            auto* ifs = static_cast<const IfStatement*>(node);
            collectRenameEdits(ifs->getCondition(), word, newName, uri, out, first);
            if (ifs->getThenBranch()) collectRenameEdits(ifs->getThenBranch(), word, newName, uri, out, first);
            if (ifs->getElseBranch()) collectRenameEdits(ifs->getElseBranch(), word, newName, uri, out, first);
            break;
        }
        case ASTNodeType::WhileStatement: {
            auto* ws = static_cast<const WhileStatement*>(node);
            collectRenameEdits(ws->getCondition(), word, newName, uri, out, first);
            if (ws->getBody()) collectRenameEdits(ws->getBody(), word, newName, uri, out, first);
            break;
        }
        case ASTNodeType::ForStatement: {
            auto* fs = static_cast<const ForStatement*>(node);
            if (fs->getInit()) collectRenameEdits(fs->getInit(), word, newName, uri, out, first);
            if (fs->getCondition()) collectRenameEdits(fs->getCondition(), word, newName, uri, out, first);
            if (fs->getIncrement()) collectRenameEdits(fs->getIncrement(), word, newName, uri, out, first);
            if (fs->getBody()) collectRenameEdits(fs->getBody(), word, newName, uri, out, first);
            break;
        }
        case ASTNodeType::Block: {
            auto* b = static_cast<const Block*>(node);
            for (const auto& s : b->getStatements())
                collectRenameEdits(s.get(), word, newName, uri, out, first);
            break;
        }
        case ASTNodeType::StatementList: {
            auto* sl = static_cast<const StatementList*>(node);
            for (const auto& s : sl->getStatements())
                collectRenameEdits(s.get(), word, newName, uri, out, first);
            break;
        }
        case ASTNodeType::MatchStatement: {
            auto* ms = static_cast<const MatchStatement*>(node);
            collectRenameEdits(ms->getCondition(), word, newName, uri, out, first);
            for (const auto& c : ms->getCases()) {
                if (c->pattern) collectRenameEdits(c->pattern.get(), word, newName, uri, out, first);
                if (c->body) collectRenameEdits(c->body.get(), word, newName, uri, out, first);
            }
            break;
        }
        case ASTNodeType::TryStatement: {
            auto* ts = static_cast<const TryStatement*>(node);
            if (ts->getTryBlock()) collectRenameEdits(ts->getTryBlock(), word, newName, uri, out, first);
            for (const auto& cc : ts->getCatchClauses())
                if (cc->block) collectRenameEdits(cc->block.get(), word, newName, uri, out, first);
            break;
        }
        case ASTNodeType::FunctionDeclaration: {
            auto* f = static_cast<const FunctionDeclaration*>(node);
            if (f->getBody()) collectRenameEdits(f->getBody(), word, newName, uri, out, first);
            break;
        }
        case ASTNodeType::ClassDeclaration: {
            auto* c = static_cast<const ClassDeclaration*>(node);
            for (const auto& m : c->getMethods())
                if (m->body) collectRenameEdits(m->body.get(), word, newName, uri, out, first);
            break;
        }
        case ASTNodeType::NamespaceDeclaration: {
            auto* ns = static_cast<const NamespaceDeclaration*>(node);
            for (const auto& child : ns->getChildren())
                collectRenameEdits(child.get(), word, newName, uri, out, first);
            break;
        }
        case ASTNodeType::Program: {
            auto* p = static_cast<const Program*>(node);
            for (const auto& child : p->getChildren())
                collectRenameEdits(child.get(), word, newName, uri, out, first);
            break;
        }
        case ASTNodeType::Group: {
            auto* g = static_cast<const GroupNode*>(node);
            for (const auto& child : g->getChildren())
                collectRenameEdits(child.get(), word, newName, uri, out, first);
            break;
        }
        case ASTNodeType::ThrowExpression: {
            auto* te = static_cast<const ThrowExpression*>(node);
            collectRenameEdits(te->getExpression(), word, newName, uri, out, first);
            break;
        }
        case ASTNodeType::AwaitExpression: {
            auto* ae = static_cast<const AwaitExpression*>(node);
            collectRenameEdits(ae->getExpression(), word, newName, uri, out, first);
            break;
        }
        case ASTNodeType::PipelineExpression: {
            auto* pe = static_cast<const PipelineExpression*>(node);
            collectRenameEdits(pe->getLeft(), word, newName, uri, out, first);
            collectRenameEdits(pe->getRight(), word, newName, uri, out, first);
            break;
        }
        case ASTNodeType::TernaryExpression: {
            auto* te = static_cast<const TernaryExpression*>(node);
            collectRenameEdits(te->getCondition(), word, newName, uri, out, first);
            collectRenameEdits(te->getThenExpr(), word, newName, uri, out, first);
            collectRenameEdits(te->getElseExpr(), word, newName, uri, out, first);
            break;
        }
        default:
            break;
    }
}

std::string LspServer::findDefinitionAst(const std::string& uri, const std::string& word) {
    auto it = documentCache_.find(uri);
    if (it == documentCache_.end() || word.empty()) return "[]";
    try {
        Lexer lexer(it->second);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto program = parser.parse();
        std::ostringstream out;
        out << "[";
        bool first = true;
        collectDefinitions(program.get(), word, uri, out, first);
        out << "]";
        return out.str();
    } catch (...) { return "[]"; }
}

std::string LspServer::findReferencesAst(const std::string& uri, const std::string& word,
                                          int cursorLine, int cursorChar) {
    auto it = documentCache_.find(uri);
    if (it == documentCache_.end() || word.empty()) return "[]";
    try {
        Lexer lexer(it->second);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto program = parser.parse();
        std::ostringstream out;
        out << "[";
        bool first = true;
        collectReferences(program.get(), word, uri, out, first);
        out << "]";
        return out.str();
    } catch (...) { return "[]"; }
}

std::string LspServer::buildRenameEditsAst(const std::string& uri, const std::string& word,
                                            const std::string& newName, int cursorLine, int cursorChar) {
    auto it = documentCache_.find(uri);
    if (it == documentCache_.end() || word.empty() || newName.empty()) return "null";
    try {
        Lexer lexer(it->second);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto program = parser.parse();
        std::ostringstream out;
        out << "{\"changes\":{\"" << uri << "\":[";
        bool first = true;
        collectRenameEdits(program.get(), word, newName, uri, out, first);
        out << "]}}";
        return out.str();
    } catch (...) { return "null"; }
}

} // namespace uzpp
