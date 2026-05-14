#include "lsp_server.h"
#include "lexer.h"
#include "parser.h"
#include "type_checker.hpp"
#include "formatter.h"

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
                               "}}}}";
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
        std::string loc = findDefinition(uri, word);
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
        std::ostringstream res;
        res << "[";
        if (!word.empty() && documentCache_.contains(uri)) {
            try {
                Lexer lexer(documentCache_[uri]);
                auto tokens = lexer.tokenize();
                bool first = true;
                for (const auto& t : tokens) {
                    if (t.value == word) {
                        if (!first) res << ",";
                        first = false;
                        int l = t.line > 0 ? t.line - 1 : 0;
                        int c = t.column > 0 ? t.column - 1 : 0;
                        res << "{\"uri\":\"" << uri << "\",\"range\":{\"start\":{\"line\":" << l << ",\"character\":" << c << "},\"end\":{\"line\":" << l << ",\"character\":" << (c + word.length()) << "}}}";
                    }
                }
            } catch(...) {}
        }
        res << "]";
        sendMessage("{\"jsonrpc\":\"2.0\",\"id\":" + idStr + ",\"result\":" + res.str() + "}");
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
        
        std::ostringstream res;
        res << "{\"changes\":{\"" << uri << "\":[";
        if (!word.empty() && !newName.empty() && documentCache_.contains(uri)) {
            try {
                Lexer lexer(documentCache_[uri]);
                auto tokens = lexer.tokenize();
                bool first = true;
                for (const auto& t : tokens) {
                    if (t.value == word) {
                        if (!first) res << ",";
                        first = false;
                        int l = t.line > 0 ? t.line - 1 : 0;
                        int c = t.column > 0 ? t.column - 1 : 0;
                        res << "{\"range\":{\"start\":{\"line\":" << l << ",\"character\":" << c << "},\"end\":{\"line\":" << l << ",\"character\":" << (c + word.length()) << "}},\"newText\":\"" << newName << "\"}";
                    }
                }
            } catch(...) {}
        }
        res << "]}}";
        sendMessage("{\"jsonrpc\":\"2.0\",\"id\":" + idStr + ",\"result\":" + res.str() + "}");
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
        // LSP spec doesn't send a "word" — extract it from the cached document
        // at the given position. (Bug fix: previous code used extractJsonString(content,"word")
        // which never matched, so hover always saw empty word and returned null.)
        std::string word;
        if (!uri.empty() && documentCache_.contains(uri)) {
            word = getWordAtPosition(documentCache_[uri], hoverLine, hoverChar);
        }

        std::string hoverContent = buildHover(word);
        std::string response;
        if (hoverContent.empty()) {
            response = "{\"jsonrpc\":\"2.0\",\"id\":" + idStr + ",\"result\":null}";
        } else {
            response = "{\"jsonrpc\":\"2.0\",\"id\":" + idStr + ",\"result\":{\"contents\":{\"kind\":\"markdown\",\"value\":\"" + hoverContent + "\"}}}";
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
        {"Natija",       "**`uzpp::Natija<T, E>`** — Xatolikni qiymat sifatida ifodalovchi tur (Result type).\\n\\n```\\nuzpp::Natija<butun> bolish(butun a, butun b) {\\n    agar (b == 0) qaytarish uzpp::Natija<butun>::xato(\"Nol!\");\\n    qaytarish uzpp::Natija<butun>::muvaffaqiyat(a / b);\\n}\\n```"},
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

// Implementation of missing LSP methods
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

} // namespace uzpp
