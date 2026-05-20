#pragma once

#include "xavfsizlik.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace uzpp::FaylTizimi {

class JsonQiymat {
public:
    std::string tur = "obyekt";
    std::string matn_qiymati;
    bool mantiqiy_qiymat = false;
    std::vector<JsonQiymat> bolalar;
    std::vector<std::string> bolalar_kalitlari;
    JsonQiymat() : tur("obyekt") {}
    JsonQiymat(const std::string& s) : tur("matn"), matn_qiymati(s) {}
    JsonQiymat(bool b) : tur("mantiqiy"), mantiqiy_qiymat(b) {}
    void objektQosh(const std::string& kalit, const JsonQiymat& qiymat) {
        bolalar_kalitlari.push_back(kalit);
        bolalar.push_back(qiymat);
    }
};

using JsonObyekt = JsonQiymat;

inline std::string jsonQiymatStringKilish(const JsonQiymat& jq) {
    if (jq.tur == "matn") return "\"" + jq.matn_qiymati + "\"";
    if (jq.tur == "mantiqiy") return jq.mantiqiy_qiymat ? "true" : "false";
    std::string natija = "{";
    for (std::size_t i = 0; i < jq.bolalar.size(); ++i) {
        if (i > 0) natija += ", ";
        natija += "\"" + jq.bolalar_kalitlari[i] + "\": " + jsonQiymatStringKilish(jq.bolalar[i]);
    }
    natija += "}";
    return natija;
}

inline std::string jsonQiymatStringKilish(const std::string& s) { return s; }
inline std::string jsonGaAylantirish(const JsonObyekt& json) { return jsonQiymatStringKilish(json); }

inline Xavfsizlik::Natija<std::string> matnOqish(const std::string& yol) {
    try {
        std::ifstream f(yol);
        if (!f.is_open()) return Xavfsizlik::xato("Fayl ochilmadi: " + yol);
        std::stringstream buf;
        buf << f.rdbuf();
        return Xavfsizlik::muvaffaqiyat(buf.str());
    } catch (const std::exception& e) {
        return Xavfsizlik::xato(std::string("Xato: ") + e.what());
    }
}

// ===== FAYL TIZIMI AMALLARI =====

inline bool faylMavjudmi(const std::string& yol) {
    return std::filesystem::exists(yol) && std::filesystem::is_regular_file(yol);
}

inline bool katalogMavjudmi(const std::string& yol) {
    return std::filesystem::exists(yol) && std::filesystem::is_directory(yol);
}

inline bool mavjudmi(const std::string& yol) {
    return std::filesystem::exists(yol);
}

inline uintmax_t faylHajmi(const std::string& yol) {
    return std::filesystem::file_size(yol);
}

inline bool faylgaYozish(const std::string& yol, const std::string& mazmun) {
    std::ofstream f(yol, std::ios::trunc);
    if (!f.is_open()) return false;
    f << mazmun;
    return true;
}

inline bool faylgaQoshish(const std::string& yol, const std::string& mazmun) {
    std::ofstream f(yol, std::ios::app);
    if (!f.is_open()) return false;
    f << mazmun;
    return true;
}

inline std::string faylniOqish(const std::string& yol) {
    std::ifstream f(yol);
    if (!f.is_open()) return "";
    std::stringstream buf;
    buf << f.rdbuf();
    return buf.str();
}

inline std::vector<std::string> qatorlarniOqish(const std::string& yol) {
    std::vector<std::string> natija;
    std::ifstream f(yol);
    if (!f.is_open()) return natija;
    std::string qator;
    while (std::getline(f, qator)) natija.push_back(qator);
    return natija;
}

inline bool faylniOchirish(const std::string& yol) {
    return std::filesystem::remove(yol);
}

inline bool katalogYaratish(const std::string& yol) {
    return std::filesystem::create_directories(yol);
}

inline std::string joriyKatalog() {
    return std::filesystem::current_path().string();
}

inline std::string faylNomi(const std::string& yol) {
    return std::filesystem::path(yol).filename().string();
}

inline std::string faylKengaytmasi(const std::string& yol) {
    return std::filesystem::path(yol).extension().string();
}

inline std::string otaKatalog(const std::string& yol) {
    return std::filesystem::path(yol).parent_path().string();
}

} // namespace uzpp::FaylTizimi
