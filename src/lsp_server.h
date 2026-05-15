#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace uzpp {

class Program;
class LspServer {
public:
    void run();

    // Public for direct unit testing — computes the JSON array of inlay hints
    // for the given document text. Returns "[]" on parse/type errors.
    static std::string computeInlayHints(const std::string& text);

    // Public for direct unit testing — computes the JSON array of code actions
    // (quick-fixes) for warnings that intersect the given LSP range. `uri` is
    // echoed verbatim into the workspace edit map.
    static std::string computeCodeActions(const std::string& text,
                                          const std::string& uri,
                                          int rangeStartLine,
                                          int rangeStartChar,
                                          int rangeEndLine,
                                          int rangeEndChar);

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
    std::string buildInlayHints(const std::string& uri);
    std::string buildCodeActions(const std::string& uri,
                                 int rangeStartLine, int rangeStartChar,
                                 int rangeEndLine, int rangeEndChar);
    void applyContentChanges(std::string& document, const std::string& contentChangesJson);

    std::unordered_map<std::string, std::string> documentCache_;
};

} // namespace uzpp
