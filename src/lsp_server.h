#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace uzpp {

class Program;
class LspServer {
public:
    void run();

private:
    void handleMessage(const std::string& content);
    void sendMessage(const std::string& jsonContent);
    std::string extractJsonString(const std::string& json, const std::string& key);
    std::string extractId(const std::string& content);
    std::string buildCompletions();
    std::string buildHover(const std::string& word);
    std::string getWordAtPosition(const std::string& text, int line, int character);
    std::string buildDocumentSymbols(const Program* program);
    std::string findDefinition(const std::string& uri, const std::string& word);
    std::string buildSignatureHelp(const std::string& uri, int line, int character);
    std::string buildSemanticTokens(const std::string& text);
    void applyContentChanges(std::string& document, const std::string& contentChangesJson);

    std::unordered_map<std::string, std::string> documentCache_;
};

} // namespace uzpp
