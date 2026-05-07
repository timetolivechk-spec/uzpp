#pragma once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace uzpp {

struct SemVer {
    int major = 0;
    int minor = 0;
    int patch = 0;
    
    static std::optional<SemVer> parse(const std::string& versionStr) {
        SemVer v;
        std::string cleanVer = versionStr;
        if (!cleanVer.empty() && (cleanVer[0] == '^' || cleanVer[0] == '~' || cleanVer[0] == '=' || cleanVer[0] == '>' || cleanVer[0] == '<')) {
            size_t start = 1;
            if (cleanVer.size() > 1 && cleanVer[1] == '=') start = 2;
            cleanVer = cleanVer.substr(start);
        }
        
        size_t dot1 = cleanVer.find('.');
        if (dot1 == std::string::npos) return std::nullopt;
        size_t dot2 = cleanVer.find('.', dot1 + 1);
        if (dot2 == std::string::npos) return std::nullopt;
        
        try {
            v.major = std::stoi(cleanVer.substr(0, dot1));
            v.minor = std::stoi(cleanVer.substr(dot1 + 1, dot2 - dot1 - 1));
            v.patch = std::stoi(cleanVer.substr(dot2 + 1));
            return v;
        } catch (...) {
            return std::nullopt;
        }
    }
};

struct DependencyInfo {
    std::string name;
    std::string version;
};

struct PlatformSupport {
    bool windows = true;
    bool linux = true;
    bool macos = true;
    bool android = true;
    bool ios = true;
};

struct ProjectManifest {
    std::string name = "unnamed_project";
    std::string version = "0.1.0";
    std::string author;
    std::string description;
    std::string entryFile = "src/asosiy.uzpp";
    std::vector<DependencyInfo> dependencies;
    PlatformSupport platforms;
};

struct ProjectContext {
    fs::path root;
    fs::path manifestPath;
    fs::path entryFile;
    ProjectManifest manifest;
};

class TomlParser {
public:
    explicit TomlParser(std::string content)
        : content_(std::move(content)) {}

    ProjectManifest parse() {
        ProjectManifest manifest;

        while (position_ < content_.size()) {
            skipWhitespaceAndComments();
            if (position_ >= content_.size()) {
                break;
            }

            if (peek() == '[') {
                parseSection(manifest);
            } else {
                skipLine();
            }
        }

        return manifest;
    }

private:
    std::string content_;
    std::size_t position_ = 0;

    char peek() const {
        if (position_ >= content_.size()) {
            return '\0';
        }
        return content_[position_];
    }

    char consume() {
        if (position_ >= content_.size()) {
            return '\0';
        }
        return content_[position_++];
    }

    void skipLine() {
        while (position_ < content_.size() && content_[position_] != '\n') {
            ++position_;
        }
        if (position_ < content_.size() && content_[position_] == '\n') {
            ++position_;
        }
    }

    void skipWhitespaceAndComments() {
        while (position_ < content_.size()) {
            const char current = peek();
            if (current == ' ' || current == '\t' || current == '\n' || current == '\r') {
                consume();
                continue;
            }
            if (current == '#') {
                skipLine();
                continue;
            }
            break;
        }
    }

    void skipInlineWhitespace() {
        while (position_ < content_.size() && (peek() == ' ' || peek() == '\t')) {
            consume();
        }
    }

    void parseSection(ProjectManifest& manifest) {
        if (consume() != '[') {
            throw std::runtime_error("TOML ParsingError: [ beklandi");
        }

        const std::string sectionName = parseSectionName();
        if (consume() != ']') {
            throw std::runtime_error("TOML ParsingError: ] beklandi");
        }

        if (sectionName == "loyiha") {
            parseProjectSection(manifest);
            return;
        }

        if (sectionName == "kutubxonalar") {
            parseDependenciesSection(manifest);
            return;
        }

        if (sectionName == "platforma") {
            parsePlatformSection(manifest);
            return;
        }

        skipToNextSection();
    }

    std::string parseSectionName() {
        std::string sectionName;
        while (position_ < content_.size()) {
            const char current = peek();
            if (current == ']') {
                break;
            }
            if (current == ' ' || current == '\t') {
                skipInlineWhitespace();
                continue;
            }
            sectionName += consume();
        }

        if (sectionName.empty()) {
            throw std::runtime_error("TOML ParsingError: bo'sh section nomi");
        }

        return sectionName;
    }

    void parseProjectSection(ProjectManifest& manifest) {
        while (position_ < content_.size()) {
            skipWhitespaceAndComments();
            if (position_ >= content_.size() || peek() == '[') {
                break;
            }

            const auto [key, value] = parseKeyValue();
            if (key == "nomi") {
                manifest.name = value;
            } else if (key == "versiya") {
                manifest.version = value;
            } else if (key == "muallif") {
                manifest.author = value;
            } else if (key == "tavsif") {
                manifest.description = value;
            } else if (key == "asosiy_fayl" || key == "kirish_fayli") {
                manifest.entryFile = value;
            }
        }
    }

    void parseDependenciesSection(ProjectManifest& manifest) {
        while (position_ < content_.size()) {
            skipWhitespaceAndComments();
            if (position_ >= content_.size() || peek() == '[') {
                break;
            }

            const auto [key, value] = parseKeyValue();
            manifest.dependencies.push_back({key, value});
        }
    }

    void parsePlatformSection(ProjectManifest& manifest) {
        while (position_ < content_.size()) {
            skipWhitespaceAndComments();
            if (position_ >= content_.size() || peek() == '[') {
                break;
            }

            const auto [key, value] = parseKeyValue();
            bool isTrue = (value == "true" || value == "rost");
            if (key == "windows") manifest.platforms.windows = isTrue;
            else if (key == "linux") manifest.platforms.linux = isTrue;
            else if (key == "macos") manifest.platforms.macos = isTrue;
            else if (key == "android") manifest.platforms.android = isTrue;
            else if (key == "ios") manifest.platforms.ios = isTrue;
        }
    }

    std::pair<std::string, std::string> parseKeyValue() {
        const std::string key = parseKey();
        skipInlineWhitespace();

        if (consume() != '=') {
            throw std::runtime_error("TOML ParsingError: = beklandi");
        }

        skipInlineWhitespace();
        const std::string value = parseValue();
        skipWhitespaceAndComments();
        return {key, value};
    }

    std::string parseKey() {
        std::string key;
        while (position_ < content_.size()) {
            const char current = peek();
            if (current == '=' || current == ' ' || current == '\t' || current == '\n' || current == '\r') {
                break;
            }
            key += consume();
        }

        if (key.empty()) {
            throw std::runtime_error("TOML ParsingError: bo'sh kalit");
        }

        return key;
    }

    std::string parseValue() {
        if (peek() == '"') {
            return parseQuotedString();
        }
        if (peek() == '\'') {
            return parseSingleQuotedString();
        }

        std::string value;
        while (position_ < content_.size()) {
            const char current = peek();
            if (current == '\n' || current == '\r' || current == '#') {
                break;
            }
            value += consume();
        }

        while (!value.empty() && (value.back() == ' ' || value.back() == '\t')) {
            value.pop_back();
        }

        return value;
    }

    std::string parseQuotedString() {
        if (consume() != '"') {
            throw std::runtime_error("TOML ParsingError: \" beklandi");
        }

        std::string value;
        while (position_ < content_.size()) {
            const char current = consume();
            if (current == '"') {
                return value;
            }

            if (current == '\\' && position_ < content_.size()) {
                const char escaped = consume();
                switch (escaped) {
                    case 'n':
                        value += '\n';
                        break;
                    case 't':
                        value += '\t';
                        break;
                    case 'r':
                        value += '\r';
                        break;
                    case '\\':
                        value += '\\';
                        break;
                    case '"':
                        value += '"';
                        break;
                    default:
                        value += escaped;
                        break;
                }
                continue;
            }

            value += current;
        }

        throw std::runtime_error("TOML ParsingError: yopilmagan satr");
    }

    std::string parseSingleQuotedString() {
        if (consume() != '\'') {
            throw std::runtime_error("TOML ParsingError: ' beklandi");
        }

        std::string value;
        while (position_ < content_.size()) {
            const char current = consume();
            if (current == '\'') {
                return value;
            }
            value += current;
        }

        throw std::runtime_error("TOML ParsingError: yopilmagan satr");
    }

    void skipToNextSection() {
        while (position_ < content_.size() && peek() != '[') {
            skipLine();
        }
    }
};

class ProjectManager {
public:
    static std::optional<ProjectManifest> loadManifest(const fs::path& projectRoot) {
        const fs::path manifestPath = projectRoot / "uzpp.toml";
        if (!fs::exists(manifestPath)) {
            return std::nullopt;
        }

        std::ifstream input(manifestPath, std::ios::binary);
        if (!input.is_open()) {
            return std::nullopt;
        }

        std::ostringstream buffer;
        buffer << input.rdbuf();

        try {
            TomlParser parser(buffer.str());
            return parser.parse();
        } catch (const std::exception&) {
            return std::nullopt;
        }
    }

    static bool saveManifest(const fs::path& projectRoot, const ProjectManifest& manifest) {
        fs::create_directories(projectRoot);

        std::ofstream output(projectRoot / "uzpp.toml", std::ios::binary);
        if (!output.is_open()) {
            return false;
        }

        output << "[loyiha]\n";
        output << "nomi = \"" << escapeTomlString(manifest.name) << "\"\n";
        output << "versiya = \"" << escapeTomlString(manifest.version) << "\"\n";
        if (!manifest.author.empty()) {
            output << "muallif = \"" << escapeTomlString(manifest.author) << "\"\n";
        }
        if (!manifest.description.empty()) {
            output << "tavsif = \"" << escapeTomlString(manifest.description) << "\"\n";
        }
        output << "asosiy_fayl = \"" << escapeTomlString(manifest.entryFile) << "\"\n";

        if (!manifest.dependencies.empty()) {
            output << "\n[kutubxonalar]\n";
            for (const auto& dependency : manifest.dependencies) {
                output << escapeTomlString(dependency.name) << " = \""
                       << escapeTomlString(dependency.version) << "\"\n";
            }
        }

        return true;
    }

    static bool createProjectStructure(const fs::path& projectPath, const std::string& projectName) {
        ProjectManifest manifest;
        manifest.name = projectName;
        manifest.version = "0.1.0";
        manifest.entryFile = "src/asosiy.uzpp";

        const fs::path entryFile = projectPath / manifest.entryFile;
        fs::create_directories(entryFile.parent_path());

        if (!saveManifest(projectPath, manifest)) {
            return false;
        }

        std::ofstream output(entryFile, std::ios::binary);
        if (!output.is_open()) {
            return false;
        }

        output << "butun asosiy() {\n";
        output << "    yozish << \"Salom dunyo\" << qator_oxiri;\n";
        output << "    qaytarish 0;\n";
        output << "}\n";
        return true;
    }

    static std::optional<fs::path> findProjectRoot(fs::path startDir) {
        if (startDir.empty()) {
            startDir = fs::current_path();
        }

        std::error_code ec;
        fs::path current = fs::absolute(startDir, ec);
        if (ec) {
            current = startDir;
        }

        if (fs::is_regular_file(current, ec)) {
            current = current.parent_path();
        }

        while (!current.empty()) {
            if (fs::exists(current / "uzpp.toml")) {
                return current;
            }

            const fs::path parent = current.parent_path();
            if (parent == current) {
                break;
            }
            current = parent;
        }

        return std::nullopt;
    }

    static std::optional<ProjectContext> loadProject(const fs::path& startDir) {
        const auto projectRoot = findProjectRoot(startDir);
        if (!projectRoot) {
            return std::nullopt;
        }

        const auto manifest = loadManifest(*projectRoot);
        if (!manifest) {
            return std::nullopt;
        }

        ProjectContext context;
        context.root = *projectRoot;
        context.manifestPath = context.root / "uzpp.toml";
        context.manifest = *manifest;
        context.entryFile = resolveEntryFile(context.root, context.manifest);
        return context;
    }

    static fs::path resolveEntryFile(const fs::path& projectRoot, const ProjectManifest& manifest) {
        fs::path entry = manifest.entryFile.empty() ? fs::path("src/asosiy.uzpp") : fs::path(manifest.entryFile);
        if (entry.is_relative()) {
            entry = projectRoot / entry;
        }
        return entry.lexically_normal();
    }

    static bool addDependency(const fs::path& projectRoot, const std::string& dependencyName, const std::string& version) {
        if (!SemVer::parse(version) && version != "latest") {
            std::cerr << "OGOHLANTIRISH: '" << version << "' versiyasi SemVer (X.Y.Z) formatiga mos kelmaydi.\n";
        }

        auto manifest = loadManifest(projectRoot);
        if (!manifest) {
            return false;
        }

        auto existing = std::find_if(manifest->dependencies.begin(),
                                     manifest->dependencies.end(),
                                     [&](const DependencyInfo& dependency) {
                                         return dependency.name == dependencyName;
                                     });

        if (existing == manifest->dependencies.end()) {
            manifest->dependencies.push_back({dependencyName, version});
        } else {
            existing->version = version;
        }

        return saveManifest(projectRoot, *manifest);
    }

    static bool removeDependency(const fs::path& projectRoot, const std::string& dependencyName) {
        auto manifest = loadManifest(projectRoot);
        if (!manifest) return false;

        auto it = std::remove_if(manifest->dependencies.begin(),
                                 manifest->dependencies.end(),
                                 [&](const DependencyInfo& dependency) {
                                     return dependency.name == dependencyName;
                                 });

        if (it != manifest->dependencies.end()) {
            manifest->dependencies.erase(it, manifest->dependencies.end());
            return saveManifest(projectRoot, *manifest);
        }
        
        return true; // already removed
    }

    static std::optional<fs::path> findStdlibRoot(fs::path startDir) {
        if (startDir.empty()) {
            startDir = fs::current_path();
        }

        std::error_code ec;
        fs::path current = fs::absolute(startDir, ec);
        if (ec) {
            current = startDir;
        }

        if (fs::is_regular_file(current, ec)) {
            current = current.parent_path();
        }

        while (!current.empty()) {
            const fs::path candidate = current / "stdlib";
            if (fs::exists(candidate / "uzpp_runtime.hpp")) {
                return candidate;
            }

            const fs::path parent = current.parent_path();
            if (parent == current) {
                break;
            }
            current = parent;
        }

        return std::nullopt;
    }

private:
    static std::string escapeTomlString(const std::string& value) {
        std::string escaped;
        for (const char c : value) {
            switch (c) {
                case '"':
                    escaped += "\\\"";
                    break;
                case '\\':
                    escaped += "\\\\";
                    break;
                case '\n':
                    escaped += "\\n";
                    break;
                case '\t':
                    escaped += "\\t";
                    break;
                case '\r':
                    escaped += "\\r";
                    break;
                default:
                    escaped += c;
                    break;
            }
        }
        return escaped;
    }
};

} // namespace uzpp
