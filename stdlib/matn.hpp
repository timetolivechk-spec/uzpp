#pragma once

#include "xavfsizlik.hpp"

#include <string>
#include <string_view>
#include <vector>
#include <regex>
#include <algorithm>
#include <cctype>
#include <charconv>
#include <optional>
#include <sstream>

#if !defined(UZPP_EMBEDDED)
#include <format>
#endif

namespace uzpp::Matn {

// ===== BO'LISH / BIRLASHTIRISH =====

[[nodiscard]] inline std::vector<std::string> ajratish(const std::string& matn, const std::string& ajratuvchi) {
    std::vector<std::string> natija;
    if (ajratuvchi.empty()) {
        for (char c : matn) natija.push_back(std::string(1, c));
        return natija;
    }
    std::size_t bosh = 0, oxir = matn.find(ajratuvchi);
    while (oxir != std::string::npos) {
        natija.push_back(matn.substr(bosh, oxir - bosh));
        bosh = oxir + ajratuvchi.size();
        oxir = matn.find(ajratuvchi, bosh);
    }
    natija.push_back(matn.substr(bosh));
    return natija;
}

[[nodiscard]] inline std::vector<std::string> qatorlarga_ajratish(const std::string& matn) {
    std::vector<std::string> natija;
    std::istringstream ss(matn);
    std::string qator;
    while (std::getline(ss, qator)) {
        if (!qator.empty() && qator.back() == '\r') qator.pop_back();
        natija.push_back(std::move(qator));
    }
    return natija;
}

[[nodiscard]] inline std::string birlashtirish(const std::vector<std::string>& massiv, const std::string& biriktiruvchi) {
    if (massiv.empty()) return "";
    std::string natija = massiv[0];
    for (std::size_t i = 1; i < massiv.size(); ++i) natija += biriktiruvchi + massiv[i];
    return natija;
}

// ===== QIDIRISH =====

[[nodiscard]] inline bool boshlanganda(const std::string& matn, const std::string& prefiks) noexcept {
    return matn.starts_with(prefiks);
}

[[nodiscard]] inline bool tugaganda(const std::string& matn, const std::string& sufiks) noexcept {
    return matn.ends_with(sufiks);
}

[[nodiscard]] inline bool ichida(const std::string& matn, const std::string& qidiriluvchi) noexcept {
    return matn.find(qidiriluvchi) != std::string::npos;
}

[[nodiscard]] inline std::optional<std::size_t> topish(const std::string& matn, const std::string& qidiriluvchi, std::size_t boshlanish = 0) noexcept {
    auto pos = matn.find(qidiriluvchi, boshlanish);
    if (pos == std::string::npos) return std::nullopt;
    return pos;
}

[[nodiscard]] inline int hisoblash(const std::string& matn, const std::string& naqsh) {
    int son = 0;
    std::size_t pos = 0;
    while ((pos = matn.find(naqsh, pos)) != std::string::npos) { ++son; pos += naqsh.size(); }
    return son;
}

// ===== O'ZGARTIRISH =====

[[nodiscard]] inline std::string almashtirish(std::string matn, const std::string& eski, const std::string& yangi) {
    if (eski.empty()) return matn;
    std::size_t pos = 0;
    while ((pos = matn.find(eski, pos)) != std::string::npos) {
        matn.replace(pos, eski.size(), yangi);
        pos += yangi.size();
    }
    return matn;
}

[[nodiscard]] inline std::string birinchi_almashtirish(const std::string& matn, const std::string& eski, const std::string& yangi) {
    std::string natija = matn;
    auto pos = natija.find(eski);
    if (pos != std::string::npos) natija.replace(pos, eski.size(), yangi);
    return natija;
}

[[nodiscard]] inline std::string kichik_harfga(std::string matn) {
    std::transform(matn.begin(), matn.end(), matn.begin(), [](unsigned char c) { return std::tolower(c); });
    return matn;
}

[[nodiscard]] inline std::string katta_harfga(std::string matn) {
    std::transform(matn.begin(), matn.end(), matn.begin(), [](unsigned char c) { return std::toupper(c); });
    return matn;
}

[[nodiscard]] inline std::string birinchi_harfni_katta(std::string matn) {
    if (!matn.empty()) matn[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(matn[0])));
    return matn;
}

[[nodiscard]] inline std::string teskari(std::string matn) {
    std::reverse(matn.begin(), matn.end());
    return matn;
}

[[nodiscard]] inline std::string takrorlash(const std::string& matn, int bor) {
    if (bor <= 0) return "";
    std::string natija;
    natija.reserve(matn.size() * static_cast<std::size_t>(bor));
    for (int i = 0; i < bor; ++i) natija += matn;
    return natija;
}

// ===== TOZALASH / QIRQISH =====

[[nodiscard]] inline std::string chapdan_qirqish(std::string matn) {
    auto it = std::find_if(matn.begin(), matn.end(), [](unsigned char c) { return !std::isspace(c); });
    return std::string(it, matn.end());
}

[[nodiscard]] inline std::string ongdan_qirqish(std::string matn) {
    auto it = std::find_if(matn.rbegin(), matn.rend(), [](unsigned char c) { return !std::isspace(c); });
    return std::string(matn.begin(), it.base());
}

[[nodiscard]] inline std::string qirqish(std::string matn) {
    return chapdan_qirqish(ongdan_qirqish(std::move(matn)));
}

// ===== TEKSHIRISH =====

[[nodiscard]] inline bool raqammi(const std::string& matn) noexcept {
    if (matn.empty()) return false;
    std::size_t i = 0;
    if (matn[i] == '-' || matn[i] == '+') ++i;
    bool hasDigit = false;
    while (i < matn.size() && std::isdigit(static_cast<unsigned char>(matn[i]))) { ++i; hasDigit = true; }
    if (i < matn.size() && matn[i] == '.') {
        ++i;
        while (i < matn.size() && std::isdigit(static_cast<unsigned char>(matn[i]))) { ++i; hasDigit = true; }
    }
    return hasDigit && i == matn.size();
}

[[nodiscard]] inline bool harflarmi(const std::string& matn) noexcept {
    return !matn.empty() && std::all_of(matn.begin(), matn.end(), [](unsigned char c) { return std::isalpha(c); });
}

[[nodiscard]] inline bool harf_raqamlami(const std::string& matn) noexcept {
    return !matn.empty() && std::all_of(matn.begin(), matn.end(), [](unsigned char c) { return std::isalnum(c); });
}

[[nodiscard]] inline bool boshmi(const std::string& matn) noexcept { return matn.empty(); }
[[nodiscard]] inline bool bosh_emasmi(const std::string& matn) noexcept { return !matn.empty(); }

// ===== KESISH =====

[[nodiscard]] inline std::string qism_matn(const std::string& matn, std::size_t bosh, std::size_t uzunlik = std::string::npos) {
    if (bosh >= matn.size()) return "";
    return matn.substr(bosh, uzunlik);
}

[[nodiscard]] inline std::string chapdan(const std::string& matn, std::size_t n) {
    return matn.substr(0, std::min(n, matn.size()));
}

[[nodiscard]] inline std::string ongdan(const std::string& matn, std::size_t n) {
    if (n >= matn.size()) return matn;
    return matn.substr(matn.size() - n);
}

// ===== KONVERSIYA =====

[[nodiscard]] inline int butun_songa(const std::string& matn) {
    int val;
    auto [ptr, ec] = std::from_chars(matn.data(), matn.data() + matn.size(), val);
    if (ec != std::errc{}) throw std::invalid_argument("Matn butun songa aylantirib bo'lmadi: " + matn);
    return val;
}

[[nodiscard]] inline std::optional<double> kasr_songa(const std::string& matn) noexcept {
    try { return std::stod(matn); } catch (...) { return std::nullopt; }
}

[[nodiscard]] inline std::string songa_aylantirish(int n)       { return std::to_string(n); }
[[nodiscard]] inline std::string songa_aylantirish(double d)    { return std::to_string(d); }
[[nodiscard]] inline std::string songa_aylantirish(long long n) { return std::to_string(n); }

// ===== TO'LDIRISH =====

[[nodiscard]] inline std::string chapdan_toldirish(const std::string& matn, std::size_t kenglik, char toldiruvchi = ' ') {
    if (matn.size() >= kenglik) return matn;
    return std::string(kenglik - matn.size(), toldiruvchi) + matn;
}

[[nodiscard]] inline std::string ongdan_toldirish(const std::string& matn, std::size_t kenglik, char toldiruvchi = ' ') {
    if (matn.size() >= kenglik) return matn;
    return matn + std::string(kenglik - matn.size(), toldiruvchi);
}

[[nodiscard]] inline std::string markazga_toldirish(const std::string& matn, std::size_t kenglik, char toldiruvchi = ' ') {
    if (matn.size() >= kenglik) return matn;
    std::size_t chap = (kenglik - matn.size()) / 2;
    std::size_t ong  = kenglik - matn.size() - chap;
    return std::string(chap, toldiruvchi) + matn + std::string(ong, toldiruvchi);
}

// ===== FORMAT (C++20) =====

#if !defined(UZPP_EMBEDDED)
template<typename... Args>
[[nodiscard]] inline std::string formatlash(std::format_string<Args...> fmt, Args&&... args) {
    return std::format(fmt, std::forward<Args>(args)...);
}
#endif

// ===== REGEX =====

class RegEx {
public:
    explicit RegEx(const std::string& naqsh, bool katta_kichik_farqsiz = false)
        : regex_(naqsh, katta_kichik_farqsiz ? std::regex::icase : std::regex::ECMAScript) {}

    [[nodiscard]] bool mos(const std::string& matn) const {
        return std::regex_match(matn, regex_);
    }
    [[nodiscard]] bool qidirish(const std::string& matn) const {
        return std::regex_search(matn, regex_);
    }
    [[nodiscard]] std::string almashtirish(const std::string& matn, const std::string& yangi) const {
        return std::regex_replace(matn, regex_, yangi);
    }
    [[nodiscard]] std::vector<std::string> topish_hammasi(const std::string& matn) const {
        std::vector<std::string> natija;
        auto it  = std::sregex_iterator(matn.begin(), matn.end(), regex_);
        auto end = std::sregex_iterator();
        for (; it != end; ++it) natija.push_back((*it)[0].str());
        return natija;
    }
    [[nodiscard]] std::vector<std::string> guruhlar(const std::string& matn) const {
        std::smatch m;
        if (!std::regex_search(matn, m, regex_)) return {};
        std::vector<std::string> natija;
        for (std::size_t i = 1; i < m.size(); ++i) natija.push_back(m[i].str());
        return natija;
    }

private:
    std::regex regex_;
};

// ===== UNICODE-YORDAMCHI =====

[[nodiscard]] inline std::size_t uzunlik(const std::string& matn) noexcept {
    return matn.size();
}

} // namespace uzpp::Matn
