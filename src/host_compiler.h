#pragma once

// ============================================================================
//  Host C++ kompilyatorini aniqlash va platformaga mos bayroqlarni tanlash.
// ----------------------------------------------------------------------------
//  uz++ transpilyator .uzpp ni C++23 ga aylantiradi, keyin uni tizimdagi C++
//  kompilyatori bilan yig'adi. Ilgari bu buyruq har doim `g++` deb qattiq
//  yozilgan va GCC bayroqlari (`-fmodules-ts`, `-Wl,--gc-sections`,
//  `-D_GLIBCXX_DEBUG`) qo'shilardi. macOS da `g++` — bu Apple Clang uchun
//  simvolik havola, va u bu bayroqlarni qabul qilmaydi:
//
//      ld: unknown option: --gc-sections
//      clang++: error: unknown argument: '-fmodules-ts'
//
//  Natijada macOS da HAR QANDAY `uzpp qurish` ishlamasdi. Bu modul
//  kompilyatorni bir marta aniqlaydi, uning turini (`GCC` / `Clang` /
//  `AppleClang`) va versiyasini o'qiydi, so'ng har bir bayroqni faqat uni
//  qo'llab-quvvatlaydigan kompilyatorga beradi.
// ============================================================================

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

namespace uzpp {

namespace fs = std::filesystem;

enum class CxxFlavor { Gcc, Clang, AppleClang, Unknown };

struct HostCompiler {
    std::string command;          // ishga tushiriladigan buyruq (kerak bo'lsa qo'shtirnoqli)
    CxxFlavor   flavor = CxxFlavor::Unknown;
    int         majorVersion = 0; // `--version` dan olingan bosh raqam
    bool        found = false;
    std::string versionLine;      // diagnostika uchun birinchi qator
};

namespace hostcxx {

inline std::string quoteIfNeeded(const std::string& s) {
    if (s.find(' ') == std::string::npos) return s;
    return "\"" + s + "\"";
}

// Buyruqni ishga tushirib, birinchi qatorini qaytaradi. Bajarilmasa — bo'sh satr.
inline std::string firstLineOf(const std::string& command) {
#ifdef _WIN32
    FILE* pipe = _popen((command + " 2>&1").c_str(), "r");
#else
    FILE* pipe = popen((command + " 2>&1").c_str(), "r");
#endif
    if (!pipe) return "";
    char buffer[512];
    std::string first;
    if (std::fgets(buffer, sizeof(buffer), pipe)) {
        first = buffer;
    }
    // Qolganini oxirigacha o'qiymiz — aks holda `pclose` SIGPIPE berishi mumkin.
    while (std::fgets(buffer, sizeof(buffer), pipe)) { }
#ifdef _WIN32
    const int rc = _pclose(pipe);
#else
    const int rc = pclose(pipe);
#endif
    if (rc != 0) return "";
    while (!first.empty() && (first.back() == '\n' || first.back() == '\r')) first.pop_back();
    return first;
}

inline CxxFlavor flavorFromVersionLine(const std::string& line) {
    if (line.find("Apple clang") != std::string::npos ||
        line.find("Apple LLVM") != std::string::npos) {
        return CxxFlavor::AppleClang;
    }
    if (line.find("clang") != std::string::npos || line.find("Clang") != std::string::npos) {
        return CxxFlavor::Clang;
    }
    if (line.find("g++") != std::string::npos || line.find("gcc") != std::string::npos ||
        line.find("GCC") != std::string::npos ||
        line.find("Free Software Foundation") != std::string::npos) {
        return CxxFlavor::Gcc;
    }
    return CxxFlavor::Unknown;
}

// "g++ (GCC) 15.2.0" / "Apple clang version 16.0.0 (clang-1600.0.26.3)"
inline int majorFromVersionLine(const std::string& line) {
    for (std::size_t i = 0; i < line.size(); ++i) {
        if (line[i] < '0' || line[i] > '9') continue;
        // Raqam versiya ko'rinishida bo'lishi kerak: <son>.<son>
        std::size_t j = i;
        int value = 0;
        while (j < line.size() && line[j] >= '0' && line[j] <= '9') {
            value = value * 10 + (line[j] - '0');
            ++j;
        }
        if (j < line.size() && line[j] == '.') return value;
        i = j;
    }
    return 0;
}

inline bool probe(const std::string& command, HostCompiler& out) {
    const std::string line = firstLineOf(quoteIfNeeded(command) + " --version");
    if (line.empty()) return false;
    out.command      = quoteIfNeeded(command);
    out.versionLine  = line;
    out.flavor       = flavorFromVersionLine(line);
    out.majorVersion = majorFromVersionLine(line);
    out.found        = true;
    return true;
}

// Kompilyatorni bir marta aniqlaydi va keshlaydi.
//
// Tanlash tartibi:
//   1. `UZPP_CXX` muhit o'zgaruvchisi — foydalanuvchi to'g'ridan-to'g'ri belgilaydi
//   2. uzpp yonidagi `compiler/bin/g++[.exe]` (Windows installer ichidagi MinGW)
//   3. `CXX` muhit o'zgaruvchisi
//   4. platformaga mos nomzodlar ro'yxati
inline const HostCompiler& detect(const fs::path& exeDir) {
    static HostCompiler cached;
    static bool initialised = false;
    if (initialised) return cached;
    initialised = true;

    if (const char* forced = std::getenv("UZPP_CXX")) {
        if (*forced && probe(forced, cached)) return cached;
    }

    std::error_code ec;
#ifdef _WIN32
    const fs::path bundled = exeDir / "compiler" / "bin" / "g++.exe";
#else
    const fs::path bundled = exeDir / "compiler" / "bin" / "g++";
#endif
    if (fs::exists(bundled, ec) && probe(bundled.string(), cached)) return cached;

    if (const char* cxx = std::getenv("CXX")) {
        if (*cxx && probe(cxx, cached)) return cached;
    }

    static const std::vector<std::string> candidates = {
#if defined(_WIN32)
        "g++", "clang++", "c++"
#elif defined(__APPLE__)
        // Homebrew GCC birinchi: u `-fmodules-ts` va `<generator>` ni
        // qo'llab-quvvatlaydi. Topilmasa — tizim clang++ ham yetarli.
        "g++-15", "g++-14", "g++-13", "clang++", "c++", "g++"
#else
        "g++-15", "g++-14", "g++", "clang++", "c++"
#endif
    };
    for (const std::string& candidate : candidates) {
        if (probe(candidate, cached)) return cached;
    }

    cached.found = false;
    return cached;
}

// Foydalanuvchiga nima o'rnatish kerakligini aytadigan xabar.
inline std::string installHint() {
#if defined(_WIN32)
    return "  Windows: uzpp-setup.exe o'rnatuvchisini ishlating — u MinGW GCC ni ichiga oladi.\n"
           "           Yoki MSYS2 orqali: pacman -S mingw-w64-ucrt-x86_64-gcc\n";
#elif defined(__APPLE__)
    return "  macOS:   xcode-select --install        (Apple Command Line Tools)\n"
           "           brew install gcc              (to'liq C++23 uchun tavsiya etiladi)\n";
#else
    return "  Debian/Ubuntu: sudo apt-get install -y g++ libstdc++-15-dev\n"
           "  Fedora/RHEL:   sudo dnf install -y gcc-c++\n"
           "  Arch:          sudo pacman -S gcc\n";
#endif
}

// ---- Bayroqlar ------------------------------------------------------------

inline bool isClangLike(CxxFlavor f) {
    return f == CxxFlavor::Clang || f == CxxFlavor::AppleClang;
}

// C++23 standart bayrog'i. Clang 17 dan oldin `-std=c++23` nomi yo'q edi —
// o'sha versiyalar uchun `-std=c++2b` ishlatiladi.
inline std::string standardFlag(const HostCompiler& cxx) {
    if (isClangLike(cxx.flavor)) {
        // Apple Clang versiyalari LLVM versiyasidan farq qiladi: Apple clang 15
        // taxminan LLVM 16 ga to'g'ri keladi, ya'ni `-std=c++2b` xavfsizroq.
        const int threshold = (cxx.flavor == CxxFlavor::AppleClang) ? 16 : 17;
        return cxx.majorVersion >= threshold ? "-std=c++23" : "-std=c++2b";
    }
    return "-std=gnu++23";
}

// GCC ning modul-TS bayrog'i. Clang uni bilmaydi va xato beradi.
inline bool supportsModulesTs(const HostCompiler& cxx) {
    return cxx.flavor == CxxFlavor::Gcc;
}

// Ishlatilmagan kodni tashlash. Mach-O linkeri `--gc-sections` ni bilmaydi —
// unda `-dead_strip` bor.
inline std::string deadCodeStripFlags(const HostCompiler& cxx) {
#if defined(__APPLE__)
    (void)cxx;
    return "-Wl,-dead_strip ";
#else
    (void)cxx;
    return "-ffunction-sections -fdata-sections -Wl,--gc-sections ";
#endif
}

// Debug rejimidagi standart kutubxona tekshiruvlari — faqat libstdc++ (GCC) da.
inline std::string debugStdlibFlags(const HostCompiler& cxx) {
    return cxx.flavor == CxxFlavor::Gcc ? "-D_GLIBCXX_DEBUG " : "";
}

} // namespace hostcxx
} // namespace uzpp
