#include "codegen.h"
#include "lexer.h"
#include "package_manager.h"
#include "parser.h"
#include "formatter.h"
#include "lsp_server.h"
#include "type_checker.hpp"
#include "docgen.hpp"
#include "dap_server.h"

#include <cstdio>
#include <cstdlib>                       
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace uzpp {

namespace CompilerUtils {
    inline fs::path getExecutableDir() {
        fs::path exePath;
#ifdef _WIN32
        wchar_t path[MAX_PATH];
        GetModuleFileNameW(NULL, path, MAX_PATH);
        exePath = fs::path(path);
#else
        char path[1024];
        ssize_t count = readlink("/proc/self/exe", path, sizeof(path));
        if (count != -1) {
            exePath = fs::path(std::string(path, count));
        } else {
            exePath = fs::canonical("/proc/self/exe");
        }
#endif
        return exePath.parent_path();
    }
}


enum class CommandMode {
    Help,
    Version,
    Build,
    Run,
    Transpile,
    InstallPackage,
    InitProject,
    Format,
    LspServer,
    Test,
    DocGen,
    DapServer,
    Bench,
    UpdatePackage,
    Check
};

enum class BuildTarget {
    Host,
    Windows,
    Linux,
    Wasm
};

struct CliOptions {
    CommandMode mode = CommandMode::Help;
    BuildTarget target = BuildTarget::Host;
    std::string inputFile;
    std::string outputBinary;
    std::string outputCpp;
    std::string packageName;
    std::string projectName;
    bool debug = false;
    bool showCpp = false;
};

struct BuildLayout {
    fs::path inputFile;
    fs::path cppFile;
    fs::path binaryFile;
    std::optional<ProjectContext> project;
    std::optional<fs::path> stdlibRoot;
};

// Qobiq (shell) buyrug'iga uzatishdan oldin yo'lda xavfli belgilar borligini tekshiradi.
// `system`/`popen` faylga yo'lni qobiqqa beradi, shu sababli yo'lda `;`, `|`, `&`, `$`
// va shunga o'xshash metabelgilar bo'lsa, foydalanuvchi nomi orqali buyruq quyish
// (command injection) hujumi mumkin bo'ladi. Yo'l ajratuvchilari (`/`, `\`) va probel
// ruxsat etiladi — ular buyruq qatorida tirnoq bilan o'raladi.
inline bool isShellSafePath(const std::string& path) {
    for (char ch : path) {
        const auto uc = static_cast<unsigned char>(ch);
        if (uc < 0x20 || uc == 0x7f) return false; // boshqarish belgilari
        switch (ch) {
            case ';': case '|': case '&': case '<': case '>':
            case '^': case '$': case '`': case '"': case '*':
            case '?': case '\n': case '\r':
                return false;
            default: break;
        }
    }
    return true;
}

inline void requireShellSafePath(const fs::path& p, const char* whatFor) {
    if (!isShellSafePath(p.string())) {
        throw std::runtime_error(
            std::string("XATO: yo'l xavfli belgilarni o'z ichiga oladi (") + whatFor + "): " + p.string());
    }
}

// uzpm paketi nomini tekshiradi: `..` (yuqoriga chiqish) va qobiq metabelgilarini rad etadi.
// `github:user/repo/file.hpp` kabi nomlar uchun `/`, `:`, `.`, `_`, `-` ruxsat etiladi.
inline bool isValidPackageName(const std::string& name) {
    if (name.empty() || name.size() > 256) return false;
    if (name.find("..") != std::string::npos) return false;
    if (name.front() == '/' || name.front() == '\\') return false;
    for (char ch : name) {
        const auto uc = static_cast<unsigned char>(ch);
        if (uc < 0x20 || uc == 0x7f) return false;
        const bool ok = (uc >= 'a' && uc <= 'z') || (uc >= 'A' && uc <= 'Z')
                     || (uc >= '0' && uc <= '9')
                     || ch == '_' || ch == '-' || ch == '.'
                     || ch == '/' || ch == ':';
        if (!ok) return false;
    }
    return true;
}

// uzpm URL'ini tekshiradi: faqat `https://` sxemasini qabul qiladi va qobiq
// metabelgilarini rad etadi (URL `system("curl ... <URL>")` ga uzatiladi).
inline bool isValidPackageUrl(const std::string& url) {
    if (url.size() < 9 || url.size() > 2048) return false;
    if (url.rfind("https://", 0) != 0) return false;
    return isShellSafePath(url);
}

class UzppCompiler {
public:
    bool transpile(const fs::path& inputFile, const fs::path& outputFile, bool isTestMode = false, bool isBenchMode = false) const {
        std::ifstream input(inputFile, std::ios::binary);
        if (!input.is_open()) {
            std::cerr << "XATO: Fayl topilmadi -> " << inputFile.string() << '\n';
            return false;
        }

        std::ostringstream buffer;
        buffer << input.rdbuf();

        try {
            Lexer lexer(buffer.str());
            const auto tokens = lexer.tokenize();

            Parser parser(tokens);
            const auto program = parser.parse();

            TypeChecker checker;
            std::cout << "[0/2] Semantik analiz tekshirilmoqda...\n";
            
            auto printDiagnostic = [&](const std::string& colorCode, const std::string& label,
                                        const SemanticError& err) {
                // Header: file:line:col label: message
                std::cerr << "\033[1m" << inputFile.filename().string()
                          << ":" << err.line << ":" << err.column << ":\033[0m "
                          << colorCode << "\033[1m" << label << "\033[0m: "
                          << err.message << "\n";
                if (err.line > 0) {
                    std::istringstream srcStream(buffer.str());
                    std::string lineStr;
                    int curr = 1;
                    while (std::getline(srcStream, lineStr)) {
                        if (curr == err.line) {
                            std::string lineNum = std::to_string(err.line);
                            std::string pad(lineNum.size(), ' ');
                            // Context line
                            std::cerr << "  " << lineNum << " | " << lineStr << "\n";
                            // Caret with color
                            int col = err.column > 0 ? err.column - 1 : 0;
                            std::cerr << "  " << pad << " | "
                                      << std::string(col, ' ')
                                      << colorCode << "^\033[0m\n";
                            break;
                        }
                        curr++;
                    }
                }
            };
            
            if (!checker.check(program.get())) {
                for (const auto& err : checker.getErrors()) {
                    printDiagnostic("\033[31m", "xato", err);
                }
                int errCount = static_cast<int>(checker.getErrors().size());
                std::cerr << "\033[1;31m" << errCount << " ta xato topildi.\033[0m\n";
                return false;
            }
            for (const auto& warn : checker.getWarnings()) {
                printDiagnostic("\033[33m", "ogohlantirish", warn);
            }

            CodeGen codegen;
            const std::string cppCode = codegen.generate(program.get(), inputFile.string(), isTestMode, isBenchMode);

            if (outputFile.has_parent_path()) {
                fs::create_directories(outputFile.parent_path());
            }

            std::ofstream output(outputFile, std::ios::binary);
            if (!output.is_open()) {
                std::cerr << "XATO: Natija faylini yaratib bo'lmadi -> " << outputFile.string() << '\n';
                return false;
            }

            output << cppCode;
            return true;
        } catch (const std::exception& error) {
            std::cerr << "\033[1m" << inputFile.filename().string() << ":\033[0m "
                      << "\033[1;31mxato\033[0m: " << error.what() << '\n';
            return false;
        }
    }

    bool compileToBinary(const fs::path& cppFile,
                         const fs::path& binaryFile,
                         BuildTarget target,
                         const std::vector<fs::path>& includeDirs,
                         const std::optional<fs::path>& forcedIncludeHeader,
                         bool debugMode) const {
        std::cout << "[1/2] uz++ kodi C++23 ga transpilatsiya qilindi.\n";
        std::cout << "[2/2] Ikkilik fayl yig'ilmoqda...\n";

        try {
            requireShellSafePath(cppFile, "C++ fayl");
            requireShellSafePath(binaryFile, "ikkilik fayl");
            for (const auto& dir : includeDirs) requireShellSafePath(dir, "include katalogi");
            if (forcedIncludeHeader) requireShellSafePath(*forcedIncludeHeader, "forced include");
        } catch (const std::exception& e) {
            std::cerr << e.what() << '\n';
            return false;
        }

        const std::string command =
            buildCompileCommand(cppFile, binaryFile, resolveTarget(target), includeDirs, forcedIncludeHeader, debugMode);

        char buffer[256];
        std::string compilerOutput;

        FILE* pipe = popen(command.c_str(), "r");
        if (pipe == nullptr) {
            std::cerr << "XATO: Kompilyatorni ishga tushirib bo'lmadi.\n";
            return false;
        }

        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            compilerOutput += buffer;
        }

        const int returnCode = pclose(pipe);
        if (returnCode == 0) {
            std::cout << "MUVAFFAQIYAT: Dastur tayyor -> " << binaryFile.string() << '\n';
            return true;
        }

        std::cerr << "\n=============================================\n";
        std::cerr << "          DASTURDA XATOLIK TOPILDI!          \n";
        std::cerr << "=============================================\n\n";
        std::cerr << translateErrors(compilerOutput);
        std::cerr << "\nKompilyatorni yoki uz++ kodini tekshirib qayta urinib ko'ring.\n";
        return false;
    }

    bool installPackage(const std::string& packageName) const {
        if (packageName.empty()) {
            std::cerr << "XATO: `uzpp ornatish` uchun modul nomi kiritilmadi.\n";
            return false;
        }
        if (!isValidPackageName(packageName)) {
            std::cerr << "XATO: paket nomi xavfsiz emas yoki noto'g'ri belgilarni o'z ichiga oladi: "
                      << packageName << '\n';
            return false;
        }

        const auto project = ProjectManager::loadProject(fs::current_path());
        if (!project) {
            std::cerr << "XATO: `uzpp ornatish` uchun joriy katalogda `uzpp.toml` topilmadi.\n";
            return false;
        }

        std::string processedPackageName = packageName;
        std::string url;
        std::string source = "uzpm-registry";

        if (packageName.starts_with("http://")) {
            std::cerr << "XATO: faqat `https://` URL'lari qabul qilinadi (HTTP himoyalanmagan).\n";
            return false;
        }

        if (packageName.starts_with("https://")) {
            url = packageName;
            source = "url";
            size_t lastSlash = packageName.find_last_of('/');
            if (lastSlash != std::string::npos) {
                processedPackageName = packageName.substr(lastSlash + 1);
                if (processedPackageName.ends_with(".hpp")) processedPackageName = processedPackageName.substr(0, processedPackageName.length() - 4);
            } else processedPackageName = "noma_lum_paket";
        } else if (packageName.starts_with("github:")) {
            source = "github";
            std::string path = packageName.substr(7);
            url = "https://raw.githubusercontent.com/" + path;
            size_t lastSlash = path.find_last_of('/');
            if (lastSlash != std::string::npos) {
                processedPackageName = path.substr(lastSlash + 1);
                if (processedPackageName.ends_with(".hpp")) processedPackageName = processedPackageName.substr(0, processedPackageName.length() - 4);
            } else processedPackageName = path;
        } else {
            url = "https://raw.githubusercontent.com/uzlang/uzpm-registry/main/packages/" + packageName + ".hpp";
        }

        if (!isValidPackageUrl(url) || !isValidPackageName(processedPackageName)) {
            std::cerr << "XATO: paket URL yoki nomida xavfsiz bo'lmagan belgilar bor.\n";
            return false;
        }

        if (!ProjectManager::addDependency(project->root, processedPackageName, "1.0.0")) {
            std::cerr << "XATO: `uzpp.toml` ga kutubxona yozilmadi.\n";
            return false;
        }

        std::cout << "Kutubxona " << processedPackageName << " yuklab olinmoqda...\n";
        fs::path stdlibRoot = project->root / "stdlib";
        if (!fs::exists(stdlibRoot)) {
            fs::create_directories(stdlibRoot);
        }
        fs::path packageFile = stdlibRoot / (processedPackageName + ".hpp");
        
        // -L flagi redirectlar (misol uchun github qisqa ssilkasi) orqasidan borish uchun
        std::string cmd = "curl -s -f -L -o \"" + packageFile.string() + "\" \"" + url + "\"";
        
        if (std::system(cmd.c_str()) != 0 || !fs::exists(packageFile) || fs::file_size(packageFile) == 0) {
            std::cerr << "XATO: Paketni yuklab olishda xatolik. Internetni yoki paket nomini tekshiring.\n";
            if (fs::exists(packageFile)) fs::remove(packageFile);
            return false;
        }

        // UZPM Lock faylini yaratish (Reproducible builds)
        fs::path lockPath = project->root / "uzpp.lock";
        bool isNew = !fs::exists(lockPath);
        std::ofstream lockFile(lockPath, std::ios::app);
        if (lockFile.is_open()) {
            if (isNew) {
                lockFile << "# UZPM Lock Fayli - Qat'iy qaramliklar (Reproducible builds) uchun\n\n";
            }
            lockFile << "[[paket]]\nnom = \"" << processedPackageName << "\"\nversiya = \"1.0.0\"\nmanba = \"" << source << "\"\nurl = \"" << url << "\"\n\n";
        }

        std::cout << "MUVAFFAQIYAT: Kutubxona o'rnatildi -> " << packageFile.string() << "\n";
        
        std::ifstream inFile(packageFile);
        std::string line;
        while (std::getline(inFile, line)) {
            if (line.starts_with("#include \"")) {
                size_t start = line.find('"');
                size_t end = line.rfind(".hpp\"");
                if (start != std::string::npos && end != std::string::npos && end > start) {
                    std::string depName = line.substr(start + 1, end - start - 1);
                    if (depName != "uzpp_runtime" && depName != processedPackageName) {
                        fs::path depPath = stdlibRoot / (depName + ".hpp");
                        if (!fs::exists(depPath)) {
                            std::cout << "-> Qaramlik topildi: " << depName << " (yuklab olinmoqda...)\n";
                            installPackage(depName);
                        }
                    }
                }
            }
        }
        
        return true;
    }

    bool removePackage(const std::string& packageName) const {
        if (packageName.empty()) {
            std::cerr << "XATO: `uzpp uchirish` uchun modul nomi kiritilmadi.\n";
            return false;
        }

        const auto project = ProjectManager::loadProject(fs::current_path());
        if (!project) {
            std::cerr << "XATO: `uzpp uchirish` uchun joriy katalogda `uzpp.toml` topilmadi.\n";
            return false;
        }
        
        std::string processedPackageName = packageName;
        if (processedPackageName.ends_with(".hpp")) processedPackageName = processedPackageName.substr(0, processedPackageName.length() - 4);

        if (!ProjectManager::removeDependency(project->root, processedPackageName)) {
            std::cerr << "XATO: `uzpp.toml` dan kutubxona o'chirilmadi.\n";
            return false;
        }

        fs::path packageFile = project->root / "stdlib" / (processedPackageName + ".hpp");
        if (fs::exists(packageFile)) {
            fs::remove(packageFile);
            std::cout << "MUVAFFAQIYAT: Kutubxona o'chirildi -> " << packageFile.string() << "\n";
        } else {
            std::cout << "MUVAFFAQIYAT: Kutubxona `uzpp.toml` dan o'chirildi, lekin fayl topilmadi.\n";
        }

        return true;
    }

    bool initProject(const std::string& projectName) const {
        if (projectName.empty()) {
            std::cerr << "XATO: `uzpp init` uchun loyiha nomi kiritilmadi.\n";
            return false;
        }

        const fs::path projectPath(projectName);
        if (fs::exists(projectPath)) {
            std::cerr << "XATO: Katalog allaqachon mavjud -> " << projectPath.string() << '\n';
            return false;
        }

        if (!ProjectManager::createProjectStructure(projectPath, projectName)) {
            std::cerr << "XATO: Loyiha strukturasi yaratilmadi.\n";
            return false;
        }

        std::cout << "uz++ init: Loyiha skeleti yaratildi -> " << projectPath.string() << '\n';
        std::cout << "   src/asosiy.uzpp (asosiy kod fayli)\n";
        std::cout << "   uzpp.toml (loyiha manifesti)\n";
        std::cout << "\nKeyingi qadam: cd " << projectName << " && uzpp qurish\n";
        return true;
    }

    // uzpp tekshirish — lint/type-check without compiling
    bool lintOnly(const fs::path& inputFile) const {
        std::ifstream input(inputFile, std::ios::binary);
        if (!input.is_open()) {
            std::cerr << "XATO: Fayl topilmadi -> " << inputFile.string() << '\n';
            return false;
        }
        std::ostringstream buffer;
        buffer << input.rdbuf();
        const std::string src = buffer.str();

        try {
            Lexer lexer(src);
            const auto tokens = lexer.tokenize();
            Parser parser(tokens);
            const auto program = parser.parse();

            TypeChecker checker;

            auto printDiag = [&](const std::string& colorCode, const std::string& label,
                                  const SemanticError& err) {
                std::cerr << "\033[1m" << inputFile.filename().string()
                          << ":" << err.line << ":" << err.column << ":\033[0m "
                          << colorCode << "\033[1m" << label << "\033[0m: "
                          << err.message << "\n";
                if (err.line > 0) {
                    std::istringstream ss(src);
                    std::string lineStr;
                    int curr = 1;
                    while (std::getline(ss, lineStr)) {
                        if (curr == err.line) {
                            std::string ln = std::to_string(err.line);
                            std::cerr << "  " << ln << " | " << lineStr << "\n";
                            int col = err.column > 0 ? err.column - 1 : 0;
                            std::cerr << "  " << std::string(ln.size(), ' ') << " | "
                                      << std::string(col, ' ')
                                      << colorCode << "^\033[0m\n";
                            break;
                        }
                        curr++;
                    }
                }
            };

            bool ok = checker.check(program.get());

            int errCount  = 0;
            int warnCount = 0;

            for (const auto& err  : checker.getErrors())   { printDiag("\033[31m", "xato", err);          ++errCount;  }
            for (const auto& warn : checker.getWarnings()) { printDiag("\033[33m", "ogohlantirish", warn); ++warnCount; }

            if (errCount == 0 && warnCount == 0) {
                std::cout << "\033[1;32m" << inputFile.filename().string()
                          << ": hech qanday muammo topilmadi.\033[0m\n";
            } else {
                if (errCount > 0)
                    std::cerr << "\033[1;31m" << errCount << " ta xato";
                if (warnCount > 0) {
                    if (errCount > 0) std::cerr << ", ";
                    else std::cerr << "\033[1;33m";
                    std::cerr << warnCount << " ta ogohlantirish";
                }
                std::cerr << " topildi.\033[0m\n";
            }
            return ok;
        } catch (const std::exception& error) {
            std::cerr << "\033[1m" << inputFile.filename().string() << ":\033[0m "
                      << "\033[1;31mxato\033[0m: " << error.what() << '\n';
            return false;
        }
    }

    // uzpp transpile --show-cpp — side-by-side source and generated C++
    void showSideBySide(const fs::path& uzppFile, const fs::path& cppFile) const {
        auto readLines = [](const fs::path& p) -> std::vector<std::string> {
            std::vector<std::string> lines;
            std::ifstream f(p, std::ios::binary);
            std::string line;
            while (std::getline(f, line)) {
                // strip trailing \r
                if (!line.empty() && line.back() == '\r') line.pop_back();
                lines.push_back(line);
            }
            return lines;
        };

        const auto uzLines  = readLines(uzppFile);
        const auto cppLines = readLines(cppFile);

        constexpr int LEFT_WIDTH  = 48;
        constexpr int RIGHT_WIDTH = 52;

        auto pad = [](const std::string& s, int w) -> std::string {
            if (static_cast<int>(s.size()) >= w) return s.substr(0, w);
            return s + std::string(w - s.size(), ' ');
        };

        // Header
        std::cout << "\033[1;36m"
                  << pad("uz++ (" + uzppFile.filename().string() + ")", LEFT_WIDTH)
                  << "  |  "
                  << "C++ (generated)\033[0m\n";
        std::cout << std::string(LEFT_WIDTH + 5 + RIGHT_WIDTH, '-') << "\n";

        std::size_t maxLines = std::max(uzLines.size(), cppLines.size());
        for (std::size_t i = 0; i < maxLines; ++i) {
            const std::string& left  = i < uzLines.size()  ? uzLines[i]  : "";
            const std::string& right = i < cppLines.size() ? cppLines[i] : "";
            std::cout << pad(left, LEFT_WIDTH) << "  |  " << right << "\n";
        }
        std::cout << std::string(LEFT_WIDTH + 5 + RIGHT_WIDTH, '-') << "\n";
        std::cout << "\033[90m" << uzLines.size() << " qator uz++  |  "
                  << cppLines.size() << " qator C++\033[0m\n";
    }

    bool formatCode(const fs::path& filePath) const {
        std::ifstream input(filePath, std::ios::binary);
        if (!input.is_open()) {
            std::cerr << "XATO: Fayl topilmadi -> " << filePath.string() << '\n';
            return false;
        }
        std::ostringstream buffer;
        buffer << input.rdbuf();

        try {
            Lexer lexer(buffer.str());
            const auto tokens = lexer.tokenize();
            Parser parser(tokens);
            const auto program = parser.parse();

            Formatter formatter;
            const std::string formatted = formatter.format(program.get());

            std::ofstream output(filePath, std::ios::binary);
            output << formatted;
            std::cout << "Formatlandi -> " << filePath.string() << '\n';
            return true;
        } catch (const std::exception& error) {
            std::cerr << "XATO (Formatlah): " << error.what() << '\n';
            return false;
        }
    }

private:
    BuildTarget resolveTarget(BuildTarget target) const {
        if (target != BuildTarget::Host) {
            return target;
        }

#ifdef _WIN32
        return BuildTarget::Windows;
#else
        return BuildTarget::Linux;
#endif
    }

    std::string quote(const std::string& value) const {
        return "\"" + value + "\"";
    }

    // (`namespace CompilerUtils` used to live here, inside the class body,
    // alongside system-header `#include`s — illegal C++ that GCC accepted as
    // an extension but newer toolchains reject. The block has been moved out
    // to file scope above `class UzppCompiler` so the file builds cleanly.)

    std::string buildCompileCommand(const fs::path& cppFile,
                                    const fs::path& binaryFile,
                                    BuildTarget target,
                                    const std::vector<fs::path>& includeDirs,
                                    const std::optional<fs::path>& forcedIncludeHeader,
                                    bool debugMode) const {
        std::ostringstream command;
        
        fs::path exeDir = CompilerUtils::getExecutableDir();
        fs::path bundledWindowsCompiler = exeDir / "compiler" / "bin" / "g++.exe";
        fs::path bundledLinuxCompiler = exeDir / "compiler" / "bin" / "g++";

        switch (target) {
            case BuildTarget::Windows:
            case BuildTarget::Host:
#ifdef _WIN32
                if (fs::exists(bundledWindowsCompiler)) {
                    command << quote(bundledWindowsCompiler.string()) << " ";
                } else {
                    command << "g++ ";
                }
#else
                if (fs::exists(bundledLinuxCompiler)) {
                    command << quote(bundledLinuxCompiler.string()) << " ";
                } else {
                    command << "g++ ";
                }
#endif
                break;
            case BuildTarget::Linux:
                command << "x86_64-linux-gnu-g++ ";
                break;
            case BuildTarget::Wasm:
                command << "em++ ";
                break;
        }

        command << quote(cppFile.string()) << " -o " << quote(binaryFile.string()) << " -std=gnu++23 -fmodules-ts -DUZPP_NO_WINDOW ";

        if (debugMode) {
            command << "-g3 -O0 -D_GLIBCXX_DEBUG -Wall -Wextra ";
        } else {
            command << "-O3 -ffunction-sections -fdata-sections -Wl,--gc-sections ";
        }

        // Parse generated cpp files for library linking commands
        std::vector<std::string> linkedLibs;
        auto scanForLinks = [&](const fs::path& path) {
            std::ifstream in(path);
            std::string line;
            while (std::getline(in, line)) {
                size_t pos = line.find("// UZPP_LINK: ");
                if (pos != std::string::npos) {
                    std::string lib = line.substr(pos + 14);
                    lib.erase(lib.find_last_not_of(" \n\r\t") + 1);
                    bool exists = false;
                    for (const auto& l : linkedLibs) if (l == lib) exists = true;
                    if (!exists) linkedLibs.push_back(lib);
                }
            }
        };
        scanForLinks(cppFile);
        if (cppFile.has_parent_path() && fs::exists(cppFile.parent_path())) {
            std::error_code ec;
            for (const auto& entry : fs::recursive_directory_iterator(cppFile.parent_path(), ec)) {
                if (entry.is_regular_file() && (entry.path().extension() == ".cpp" || entry.path().extension() == ".hpp")) {
                    scanForLinks(entry.path());
                }
            }
        }

        for (const fs::path& includeDir : includeDirs) {
            command << "-I" << quote(includeDir.string()) << ' ';
        }

        if (forcedIncludeHeader) {
            command << "-include " << quote(forcedIncludeHeader->string()) << ' ';
        }

#ifndef _WIN32
        if (target != BuildTarget::Wasm) {
            command << "-pthread ";
        }
#endif

#ifdef _WIN32
        if (target == BuildTarget::Windows || target == BuildTarget::Host) {
            command << "-lws2_32 -lopengl32 -lgdi32 -Wl,-subsystem,console ";
        }
#endif

        if (target == BuildTarget::Wasm) {
            command << "-s WASM=1 -s USE_SDL=2 -s USE_SDL_TTF=2 -s ALLOW_MEMORY_GROWTH=1 ";
        }

        for (const auto& lib : linkedLibs) {
            command << "-l" << lib << " ";
        }

        command << "2>&1";
        return command.str();
    }

    std::string translateErrors(std::string compilerOutput) const {
        const std::vector<std::pair<std::string, std::string>> replacements = {
            {"error:", "XATO:"},
            {"warning:", "OGOHLANTIRISH:"},
            {"was not declared in this scope", "ushbu qamrovda e'lon qilinmagan"},
            {"no matching function for call to", "mos keladigan funksiya topilmadi"},
            {"is private within this context", "bu kontekstda yopiq"},
            {"cannot convert", "o'tkazib bo'lmaydi"},
            {"undefined reference", "noma'lum havola"},
            {"No such file or directory", "Bunday fayl yoki katalog topilmadi"},
            {"candidate expects", "nomzod kutadi"},
            {"arguments provided", "argumentlar berildi"},
            {"candidate:", "nomzod:"},
            {"note:", "eslatma:"},
            {"expected", "Kutilgan"},
            {"before", "shundan oldin"},
            {"from", "dan"},
            {"to", "ga"},
            {"std::unordered_map", "lug'at"},
            {"std::shared_ptr", "umumiy_korsatkich"},
            {"std::unique_ptr", "yagona_korsatkich"},
            {"std::string", "matn"},
            {"std::vector", "vektor"},
            {"std::optional", "ixtiyoriy"},
            {"std::pair", "juftlik"},
            {"int", "butun"},
            {"double", "haqiqiy"},
            {"float", "kasr"},
            {"char", "belgi"},
            {"bool", "mantiqiy"},
            {"void", "bosh"}
        };

        for (const auto& [from, to] : replacements) {
            std::size_t position = 0;
            while ((position = compilerOutput.find(from, position)) != std::string::npos) {
                compilerOutput.replace(position, from.size(), to);
                position += to.size();
            }
        }

        return compilerOutput;
    }
};

namespace {

bool startsWith(const std::string& value, const std::string& prefix) {
    return value.rfind(prefix, 0) == 0;
}

bool isCompileMode(CommandMode mode) {
    return mode == CommandMode::Build || mode == CommandMode::Run ||
           mode == CommandMode::Transpile || mode == CommandMode::Test ||
           mode == CommandMode::Check;
}

void printHelp() {
    std::cout << "uz++ OMEGA CLI\n\n";
    std::cout << "Foydalanish:\n";
    std::cout << "  uzpp init <loyiha-nomi>                         Yangi loyiha yaratish\n";
    std::cout << "  uzpp qurish  [<fayl.uzpp>] [--debug]            Kompilyatsiya (build)\n";
    std::cout << "  uzpp ishga-tushirish [<fayl.uzpp>]              Kompilyatsiya va ishga tushirish\n";
    std::cout << "  uzpp tekshirish <fayl.uzpp>                     Faqat tip tekshirish (lint)\n";
    std::cout << "  uzpp transpile [<fayl.uzpp>] [--show-cpp]       C++23 ga tarjima\n";
    std::cout << "  uzpp transpile <fayl.uzpp> --show-cpp           Yon-yon ko'rinish (uz++ | C++)\n";
    std::cout << "  uzpp sinov (yoki test) [<fayl.uzpp>]            Testlarni yugurtirish\n";
    std::cout << "  uzpp bench [<fayl.uzpp>]                        Benchmark\n";
    std::cout << "  uzpp ornatish <modul>                           Paket o'rnatish\n";
    std::cout << "  uzpp yangilash <modul>                          Paketni yangilash\n";
    std::cout << "  uzpp formatlah [<fayl.uzpp>]                    Kodni formatlash\n";
    std::cout << "  uzpp lsp                                        LSP serverni ishga tushirish\n";
    std::cout << "  uzpp dap                                        DAP serverni ishga tushirish\n";
    std::cout << "  uzpp hujjat (yoki doc) [<chiqish_papkasi>]      Hujjat yaratish\n";
    std::cout << "  uzpp --version (yoki -v)                        Versiyani ko'rsatish\n";
    std::cout << "\nMisollar:\n";
    std::cout << "  uzpp ishga-tushirish salom.uzpp\n";
    std::cout << "  uzpp tekshirish kod.uzpp\n";
    std::cout << "  uzpp transpile kod.uzpp --show-cpp\n";
    std::cout << "\nEslatma:\n";
    std::cout << "  - `qurish`, `ishga-tushirish` va `transpile` faylsiz chaqirilsa, `uzpp.toml` dan foydalaniladi.\n";
}

BuildTarget hostTarget() {
#ifdef _WIN32
    return BuildTarget::Windows;
#else
    return BuildTarget::Linux;
#endif
}

BuildTarget parseTarget(const std::string& value) {
    if (value == "windows") {
        return BuildTarget::Windows;
    }
    if (value == "linux") {
        return BuildTarget::Linux;
    }
    if (value == "wasm") {
        return BuildTarget::Wasm;
    }
    if (value == "host") {
        return BuildTarget::Host;
    }
    throw std::runtime_error("Noma'lum target: " + value);
}

CliOptions parseArguments(int argc, char* argv[]) {
    CliOptions options;
    if (argc < 2) {
        return options;
    }

    const std::string first = argv[1];

    if (first == "init") {
        if (argc < 3) {
            std::cerr << "XATO: `uzpp init` uchun loyiha nomi kiritilmadi.\n";
            options.mode = CommandMode::Help;
            return options;
        }
        options.mode = CommandMode::InitProject;
        options.projectName = argv[2];
        return options;
    }

    if (first == "ornatish") {
        if (argc < 3) {
            std::cerr << "XATO: `uzpp ornatish` uchun modul nomi kiritilmadi.\n";
            options.mode = CommandMode::Help;
            return options;
        }
        options.mode = CommandMode::InstallPackage;
        options.packageName = argv[2];
        return options;
    }

    if (first == "yangilash" || first == "update") {
        if (argc < 3) {
            std::cerr << "XATO: `uzpp yangilash` uchun modul nomi kiritilmadi.\n";
            options.mode = CommandMode::Help;
            return options;
        }
        options.mode = CommandMode::UpdatePackage;
        options.packageName = argv[2];
        return options;
    }

    if (first == "uzpm") {
        if (argc >= 4 && std::string(argv[2]) == "ornatish") {
            options.mode = CommandMode::InstallPackage;
            options.packageName = argv[3];
            return options;
        }
        throw std::runtime_error("`uzpm` uchun faqat `ornatish <modul>` qo'llab-quvvatlanadi.");
    }

    if (first == "tekshirish" || first == "check" || first == "lint") {
        options.mode = CommandMode::Check;
        if (argc >= 3) {
            options.inputFile = argv[2];
        }
        return options;
    }

    if (first == "formatlah") {
        options.mode = CommandMode::Format;
        if (argc >= 3) {
            options.inputFile = argv[2];
        }
        return options;
    }

    if (first == "lsp") {
        options.mode = CommandMode::LspServer;
        return options;
    }

    if (first == "dap") {
        options.mode = CommandMode::DapServer;
        return options;
    }

    if (first == "hujjat" || first == "doc") {
        options.mode = CommandMode::DocGen;
        if (argc >= 3) {
            options.outputBinary = argv[2]; // Repurposing outputBinary for output path
        }
        return options;
    }

    if (first == "test" || first == "sinov") {
        options.mode = CommandMode::Test;
        for (int index = 2; index < argc; ++index) {
            std::string argument = argv[index];
            if (!argument.empty() && argument[0] != '-') {
                options.inputFile = argument;
            }
        }
        return options;
    }

    if (first == "bench") {
        options.mode = CommandMode::Bench;
        for (int index = 2; index < argc; ++index) {
            std::string argument = argv[index];
            if (!argument.empty() && argument[0] != '-') {
                options.inputFile = argument;
            }
        }
        return options;
    }

    if (first == "build" || first == "qurish" || first == "run" || first == "ishga-tushirish" ||
        first == "transpile") {
        options.mode = (first == "build" || first == "qurish")
                           ? CommandMode::Build
                           : ((first == "run" || first == "ishga-tushirish") ? CommandMode::Run
                                                                              : CommandMode::Transpile);

        for (int index = 2; index < argc; ++index) {
            std::string argument = argv[index];
            if (argument == "--debug") {
                options.debug = true;
            } else if (startsWith(argument, "--target=")) {
                options.target = parseTarget(argument.substr(9));
            } else if (startsWith(argument, "--output=")) {
                options.outputBinary = argument.substr(9);
            } else if (startsWith(argument, "--emit-cpp=")) {
                options.outputCpp = argument.substr(11);
            } else if (argument == "--show-cpp") {
                options.showCpp = true;
            } else if (!argument.empty() && argument[0] != '-') {
                options.inputFile = argument;
            }
        }
        return options;
    }

    if (first == "--qurish" || first == "--ishga-tushirish") {
        options.mode = first == "--qurish" ? CommandMode::Build : CommandMode::Run;
        for (int index = 2; index < argc; ++index) {
            std::string argument = argv[index];
            if (!argument.empty() && argument[0] != '-') {
                options.inputFile = argument;
            }
        }
        return options;
    }

    if (first == "--version" || first == "-v") {
        options.mode = CommandMode::Version;
        return options;
    }

    if (first == "--help" || first == "help") {
        options.mode = CommandMode::Help;
        return options;
    }

    throw std::runtime_error("Noma'lum buyruq: " + first);
}

fs::path defaultBuildDirectory(const std::optional<ProjectContext>& project) {
    if (project) {
        return project->root / "build";
    }
    return fs::path("build");
}

fs::path defaultCppPath(const CliOptions& options,
                        const fs::path& inputFile,
                        const std::optional<ProjectContext>& project) {
    if (!options.outputCpp.empty()) {
        return fs::path(options.outputCpp);
    }

    return defaultBuildDirectory(project) / (inputFile.stem().string() + ".generated.cpp");
}

fs::path defaultBinaryPath(const CliOptions& options,
                           const fs::path& inputFile,
                           const std::optional<ProjectContext>& project) {
    if (!options.outputBinary.empty()) {
        return fs::path(options.outputBinary);
    }

    const BuildTarget effectiveTarget = options.target == BuildTarget::Host ? hostTarget() : options.target;
    const std::string stem = inputFile.stem().string();
    const fs::path buildDir = defaultBuildDirectory(project);

    switch (effectiveTarget) {
        case BuildTarget::Windows:
            return buildDir / (stem + ".exe");
        case BuildTarget::Linux:
            return buildDir / stem;
        case BuildTarget::Wasm:
            return buildDir / (stem + ".html");
        case BuildTarget::Host:
            return buildDir / stem;
    }

    return buildDir / stem;
}

BuildLayout resolveBuildLayout(const CliOptions& options) {
    BuildLayout layout;
    layout.project = ProjectManager::loadProject(fs::current_path());
    layout.stdlibRoot = ProjectManager::findStdlibRoot(fs::current_path());

    // Fallback for installer / standalone / dev-build users: when nothing on
    // the path from CWD upward looks like a uz++ checkout, look near the
    // running uzpp.exe instead. Three layouts are accepted, in this order:
    //   <exe-dir>/stdlib              (Inno Setup installer layout)
    //   <exe-dir>/../stdlib           (dev `build/uzpp.exe` next to repo `stdlib/`)
    //   walking up from <exe-dir>     (any other layout that puts stdlib above)
    // Without this fallback, `uzpp ishga-tushirish C:\anywhere\foo.uzpp` from
    // a random CWD failed with "Bunday fayl yoki katalog topilmadi" on the
    // very first `ulash "uzpp_runtime.hpp"`.
    if (!layout.stdlibRoot) {
        std::error_code ec;
        fs::path probe = CompilerUtils::getExecutableDir();
        for (int depth = 0; depth < 8 && !probe.empty(); ++depth) {
            const fs::path candidate = probe / "stdlib";
            if (fs::exists(candidate / "uzpp_runtime.hpp", ec)) {
                layout.stdlibRoot = candidate;
                break;
            }
            const fs::path parent = probe.parent_path();
            if (parent == probe) break;
            probe = parent;
        }
    }

    if (!options.inputFile.empty()) {
        layout.inputFile = fs::path(options.inputFile);
    } else if (layout.project && isCompileMode(options.mode)) {
        layout.inputFile = layout.project->entryFile;
    }

    if (layout.inputFile.empty()) {
        throw std::runtime_error("uz++ fayli ko'rsatilmadi va `uzpp.toml` topilmadi.");
    }

    if (layout.inputFile.is_relative() && layout.project) {
        const fs::path candidate = layout.project->root / layout.inputFile;
        if (fs::exists(candidate)) {
            layout.inputFile = candidate;
        }
    }

    layout.inputFile = layout.inputFile.lexically_normal();
    layout.cppFile = defaultCppPath(options, layout.inputFile, layout.project);
    layout.binaryFile = defaultBinaryPath(options, layout.inputFile, layout.project);
    return layout;
}

bool validateProjectDependencies(const ProjectContext& project, const std::optional<fs::path>& stdlibRoot) {
    if (project.manifest.dependencies.empty()) {
        return true;
    }

    if (!stdlibRoot) {
        std::cerr << "XATO: `stdlib/` katalogi topilmadi, paketlar ulanmaydi.\n";
        return false;
    }

    for (const auto& dependency : project.manifest.dependencies) {
        const fs::path headerPath = *stdlibRoot / (dependency.name + ".hpp");
        if (!fs::exists(headerPath)) {
            std::cerr << "XATO: Kutubxona `" << dependency.name << "` uchun header topilmadi -> "
                      << headerPath.string() << '\n';
            return false;
        }
    }

    return true;
}

std::vector<fs::path> collectIncludeDirs(const BuildLayout& layout) {
    std::vector<fs::path> includeDirs;

    auto pushUnique = [&](const fs::path& path) {
        if (path.empty()) {
            return;
        }
        const fs::path normalized = path.lexically_normal();
        for (const auto& existing : includeDirs) {
            if (existing == normalized) {
                return;
            }
        }
        includeDirs.push_back(normalized);
    };

    pushUnique(fs::current_path());
    pushUnique(layout.inputFile.parent_path());
    if (layout.project) {
        pushUnique(layout.project->root);
    }
    if (layout.stdlibRoot) {
        pushUnique(*layout.stdlibRoot);
        pushUnique(layout.stdlibRoot->parent_path());
    }

    return includeDirs;
}

std::optional<fs::path> writeDependencyBridge(const BuildLayout& layout) {
    if (!layout.project || !layout.stdlibRoot || layout.project->manifest.dependencies.empty()) {
        return std::nullopt;
    }

    const fs::path bridgeHeader = defaultBuildDirectory(layout.project) / "uzpp_dependencies.hpp";
    fs::create_directories(bridgeHeader.parent_path());

    std::ofstream output(bridgeHeader, std::ios::binary);
    if (!output.is_open()) {
        std::cerr << "XATO: Paket header ko'prigini yaratib bo'lmadi -> " << bridgeHeader.string() << '\n';
        return std::nullopt;
    }

    output << "#pragma once\n";
    for (const auto& dependency : layout.project->manifest.dependencies) {
        output << "#include \"" << dependency.name << ".hpp\"\n";
    }

    return bridgeHeader;
}

int runBinary(const fs::path& binaryPath) {
    fs::path launchPath = binaryPath;
    std::string command = launchPath.string();

    if (!isShellSafePath(command)) {
        std::cerr << "XATO: ikkilik fayl yo'lida xavfsiz bo'lmagan belgilar bor: "
                  << command << '\n';
        return 1;
    }

#ifdef _WIN32
    for (char& value : command) {
        if (value == '/') {
            value = '\\';
        }
    }
    if (!launchPath.is_absolute() && !startsWith(command, ".\\")) {
        command = ".\\" + command;
    }
#else
    if (!launchPath.is_absolute() && !startsWith(command, "./")) {
        command = "./" + command;
    }
#endif

    command = "\"" + command + "\"";
    return std::system(command.c_str());
}

} // namespace

} // namespace uzpp

int main(int argc, char* argv[]) {
    using namespace uzpp;

    try {
        const CliOptions options = parseArguments(argc, argv);
        if (options.mode == CommandMode::Help) {
            printHelp();
            return argc < 2 ? 1 : 0;
        }

        if (options.mode == CommandMode::Version) {
            std::cout << "uz++ OMEGA CLI v4.0.0 (C++23 transpile engine)\n";
            return 0;
        }

        const UzppCompiler compiler;

        if (options.mode == CommandMode::InitProject) {
            return compiler.initProject(options.projectName) ? 0 : 1;
        }

        if (options.mode == CommandMode::InstallPackage || options.mode == CommandMode::UpdatePackage) {
            return compiler.installPackage(options.packageName) ? 0 : 1;
        }

        if (options.mode == CommandMode::Check) {
            if (options.inputFile.empty()) {
                std::cerr << "XATO: `uzpp tekshirish` uchun fayl nomi kiritilmadi.\n";
                return 1;
            }
            return compiler.lintOnly(fs::path(options.inputFile)) ? 0 : 1;
        }

        if (options.mode == CommandMode::Format) {
            if (!options.inputFile.empty()) {
                return compiler.formatCode(options.inputFile) ? 0 : 1;
            }
            
            auto project = ProjectManager::loadProject(fs::current_path());
            if (project) {
                bool success = true;
                for (const auto& entry : fs::recursive_directory_iterator(project->root / "src")) {
                    if (entry.path().extension() == ".uzpp") {
                        if (!compiler.formatCode(entry.path())) success = false;
                    }
                }
                return success ? 0 : 1;
            }
            std::cerr << "XATO: uzpp.toml topilmadi va fayl ko'rsatilmadi.\n";
            return 1;
        }

        if (options.mode == CommandMode::LspServer) {
            LspServer().run();
            return 0;
        }

        if (options.mode == CommandMode::DapServer) {
            DapServer().run();
            return 0;
        }

        if (options.mode == CommandMode::DocGen) {
            DocGen docGen;
            fs::path srcDir = fs::current_path();
            fs::path outDir = fs::current_path() / "docs";
            
            auto project = ProjectManager::loadProject(fs::current_path());
            if (project) {
                srcDir = project->root / "src";
                outDir = project->root / "docs";
            }
            
            if (!options.outputBinary.empty()) {
                outDir = options.outputBinary;
            }
            return docGen.generateDocs(srcDir, outDir) ? 0 : 1;
        }

        BuildLayout layout = resolveBuildLayout(options);
        if (!fs::exists(layout.inputFile)) {
            std::cerr << "XATO: Asosiy uz++ fayli topilmadi -> " << layout.inputFile.string() << '\n';
            return 1;
        }

        if (layout.project) {
#if defined(_WIN32)
            if (!layout.project->manifest.platforms.windows) throw std::runtime_error("Bu loyiha sizning operatsion tizimingizni qo'llab-quvvatlamaydi.");
#elif defined(__APPLE__)
            if (!layout.project->manifest.platforms.macos) throw std::runtime_error("Bu loyiha sizning operatsion tizimingizni qo'llab-quvvatlamaydi.");
#elif defined(__linux__) && !defined(__ANDROID__)
            if (!layout.project->manifest.platforms.linux) throw std::runtime_error("Bu loyiha sizning operatsion tizimingizni qo'llab-quvvatlamaydi.");
#endif

            if (!validateProjectDependencies(*layout.project, layout.stdlibRoot)) {
                return 1;
            }

            std::cout << ">>> Manifest yuklandi: " << layout.project->manifest.name << '\n';
            std::cout << ">>> Asosiy fayl: " << layout.inputFile.string() << "\n\n";
            
            std::cout << ">>> Loyiha fayllari qidirilmoqda...\n";
            fs::path srcDir = layout.project->root / "src";
            if (!fs::exists(srcDir)) srcDir = layout.project->root;
            
            bool buildSuccess = true;
            for (const auto& entry : fs::recursive_directory_iterator(srcDir)) {
                if (entry.is_regular_file() && entry.path().extension() == ".uzpp") {
                    fs::path rel = fs::relative(entry.path(), srcDir);
                    fs::path outPath = defaultBuildDirectory(layout.project) / rel;
                    
                    if (entry.path().lexically_normal() == layout.inputFile.lexically_normal()) {
                        outPath.replace_extension(".cpp");
                        layout.cppFile = outPath;
                    } else {
                        outPath.replace_extension(".hpp");
                    }
                    
                    fs::create_directories(outPath.parent_path());
                    std::cout << " -> " << rel.string() << "\n";
                    if (!compiler.transpile(entry.path(), outPath, options.mode == CommandMode::Test, options.mode == CommandMode::Bench)) {
                        buildSuccess = false;
                    }
                }
            }
            if (!buildSuccess) return 1;
            
        } else {
            fs::create_directories(layout.cppFile.parent_path());
            fs::create_directories(layout.binaryFile.parent_path());

            std::cout << ">>> Transpilatsiya boshlandi: " << layout.inputFile.string() << '\n';
            if (!compiler.transpile(layout.inputFile, layout.cppFile, options.mode == CommandMode::Test, options.mode == CommandMode::Bench)) {
                return 1;
            }
        }

        if (options.mode == CommandMode::Transpile) {
            std::cout << ">>> C++ natija fayli tayyor: " << layout.cppFile.string() << '\n';
            if (options.showCpp) {
                std::cout << "\n";
                compiler.showSideBySide(layout.inputFile, layout.cppFile);
            }
            return 0;
        }

        const std::vector<fs::path> includeDirs = collectIncludeDirs(layout);
        const std::optional<fs::path> dependencyBridge = writeDependencyBridge(layout);

        std::cout << ">>> C++ kompilyatsiyasi boshlandi...\n" << std::endl;
        if (!compiler.compileToBinary(
                layout.cppFile, layout.binaryFile, options.target, includeDirs, dependencyBridge, options.debug)) {
            return 1;
        }

        if (options.mode == CommandMode::Run || options.mode == CommandMode::Test || options.mode == CommandMode::Bench) {
            std::cout << "\n>>> Dastur ishga tushirilmoqda...\n" << std::endl;
            std::cout.flush();
            const int exitCode = runBinary(layout.binaryFile);
            std::cout << "\n[Dastur yakunlandi, exit code: " << exitCode << "]\n";
            return exitCode;
        } else {
            std::cout << "\n>>> Dastur muvaffaqiyatli qurildi: " << layout.binaryFile.string() << std::endl;
        }

        return 0;
    } catch (const std::exception& error) {
        std::cerr << "XATO: " << error.what() << '\n';
        return 1;
    }
}
