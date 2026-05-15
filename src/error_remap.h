#pragma once

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

namespace uzpp::ErrorRemap {

// Remaps `<basename>.cpp:<line>:<col>:` mentions in g++ output back to the
// original `<file>.uzpp:<line>:<col>` using #line directives in the generated
// .cpp content. g++ honors #line for source paths and line numbers within the
// active region, but errors emitted from synthesised wrappers (lines outside
// any #line block — e.g. the auto-generated _uzpp_main wrapper) still cite
// the .cpp file. This helper rewrites those positions.
inline std::string remapPositions(std::string output,
                                  const std::string& cppContent,
                                  const std::string& generatedCppName) {
    struct LineMap {
        int cppLineAfter;     // First .cpp line covered by this directive.
        std::string uzppPath; // .uzpp file the directive points at.
        int uzppLine;         // .uzpp line number for cppLineAfter.
    };
    std::vector<LineMap> maps;

    {
        std::stringstream ss(cppContent);
        std::string line;
        int cppLineNo = 0;
        while (std::getline(ss, line)) {
            ++cppLineNo;
            const auto firstNonWs = line.find_first_not_of(" \t");
            if (firstNonWs == std::string::npos) continue;
            if (line.compare(firstNonWs, 5, "#line") != 0) continue;
            std::size_t i = firstNonWs + 5;
            while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i]))) ++i;
            int num = 0;
            bool hasDigit = false;
            while (i < line.size() && std::isdigit(static_cast<unsigned char>(line[i]))) {
                num = num * 10 + (line[i] - '0');
                ++i;
                hasDigit = true;
            }
            if (!hasDigit) continue;
            while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i]))) ++i;
            if (i >= line.size() || line[i] != '"') continue;
            ++i;
            const std::size_t end = line.find('"', i);
            if (end == std::string::npos) continue;
            const std::string raw = line.substr(i, end - i);
            std::string path;
            path.reserve(raw.size());
            for (std::size_t k = 0; k < raw.size(); ++k) {
                if (raw[k] == '\\' && k + 1 < raw.size() && raw[k + 1] == '\\') {
                    path += '\\';
                    ++k;
                } else {
                    path += raw[k];
                }
            }
            maps.push_back({cppLineNo + 1, path, num});
        }
    }
    if (maps.empty()) return output;

    std::string result;
    result.reserve(output.size());
    std::size_t lineStart = 0;
    while (lineStart <= output.size()) {
        const std::size_t nl = output.find('\n', lineStart);
        const std::string outLine = output.substr(
            lineStart,
            nl == std::string::npos ? std::string::npos : nl - lineStart);

        std::string transformed;
        std::size_t scan = 0;
        while (scan < outLine.size()) {
            const std::size_t cppPos = outLine.find(".cpp", scan);
            if (cppPos == std::string::npos) {
                transformed.append(outLine, scan, std::string::npos);
                break;
            }
            std::size_t nameStart = cppPos;
            while (nameStart > 0) {
                const char c = outLine[nameStart - 1];
                if (c == ' ' || c == '\t' || c == ':' || c == '(' || c == '\'' ||
                    c == '"' || c == ',' || c == '<') break;
                --nameStart;
            }
            const std::string fname = outLine.substr(nameStart, cppPos + 4 - nameStart);
            const std::filesystem::path fnamePath(fname);
            if (fnamePath.filename().string() != generatedCppName) {
                transformed.append(outLine, scan, cppPos + 4 - scan);
                scan = cppPos + 4;
                continue;
            }
            std::size_t after = cppPos + 4;
            if (after >= outLine.size() || outLine[after] != ':') {
                transformed.append(outLine, scan, after - scan);
                scan = after;
                continue;
            }
            ++after;
            int cppErrLine = 0;
            bool gotLine = false;
            while (after < outLine.size() && std::isdigit(static_cast<unsigned char>(outLine[after]))) {
                cppErrLine = cppErrLine * 10 + (outLine[after] - '0');
                ++after;
                gotLine = true;
            }
            if (!gotLine) {
                transformed.append(outLine, scan, after - scan);
                scan = after;
                continue;
            }
            const LineMap* best = nullptr;
            for (const auto& m : maps) {
                if (m.cppLineAfter <= cppErrLine) {
                    if (best == nullptr || m.cppLineAfter > best->cppLineAfter) best = &m;
                } else {
                    break;
                }
            }
            if (best == nullptr) {
                transformed.append(outLine, scan, after - scan);
                scan = after;
                continue;
            }
            const int uzppErrLine = best->uzppLine + (cppErrLine - best->cppLineAfter);
            transformed.append(outLine, scan, nameStart - scan);
            transformed += best->uzppPath;
            transformed += ':';
            transformed += std::to_string(uzppErrLine);
            scan = after;
        }
        result += transformed;
        if (nl == std::string::npos) break;
        result += '\n';
        lineStart = nl + 1;
    }
    return result;
}

} // namespace uzpp::ErrorRemap
