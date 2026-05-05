#pragma once

#include "xavfsizlik.hpp"

#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <type_traits>

namespace uzpp::FaylTizimi {

struct JsonObyekt {
    std::unordered_map<std::string, std::string> maydonlar;

    [[nodiscard]] bool mavjud(const std::string& kalit) const {
        return maydonlar.contains(kalit);
    }

    [[nodiscard]] Xavfsizlik::Tanlov<std::string> olish(const std::string& kalit) const {
        const auto iterator = maydonlar.find(kalit);
        if (iterator == maydonlar.end()) {
            return std::nullopt;
        }
        return iterator->second;
    }
};

[[nodiscard]] inline Xavfsizlik::Natija<std::string> matnOqish(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input.is_open()) {
        return std::unexpected<std::string>("Faylni ochib bo'lmadi: " + path.string());
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

[[nodiscard]] inline std::expected<void, std::string> matnYozish(const std::filesystem::path& path,
                                                                 std::string_view content) {
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }

    std::ofstream output(path, std::ios::binary);
    if (!output.is_open()) {
        return std::unexpected<std::string>("Faylga yozib bo'lmadi: " + path.string());
    }

    output << content;
    return {};
}

[[nodiscard]] inline Xavfsizlik::Natija<std::vector<std::string>> qatorlarOqish(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input.is_open()) {
        return std::unexpected<std::string>("Qatorlarni o'qish uchun fayl ochilmadi: " + path.string());
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(input, line)) {
        lines.push_back(line);
    }

    return lines;
}

[[nodiscard]] inline Xavfsizlik::Natija<JsonObyekt> oddiyJsonOqish(const std::filesystem::path& path) {
    auto content = matnOqish(path);
    if (!content) {
        return std::unexpected<std::string>(content.error());
    }

    JsonObyekt object;
    const std::regex pairPattern(
        R"JSON("([^"]+)"\s*:\s*("([^"]*)"|[-+]?[0-9]+(?:\.[0-9]+)?|true|false|null))JSON");

    std::sregex_iterator iterator(content->begin(), content->end(), pairPattern);
    std::sregex_iterator end;

    for (; iterator != end; ++iterator) {
        std::string value = (*iterator)[2].str();
        if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
            value = value.substr(1, value.size() - 2);
        }
        object.maydonlar[(*iterator)[1].str()] = value;
    }

    return object;
}

// ===== PHASE 11-D: JSON SERIALIZATION (WRITING) EXPANSION =====

// Sodda JSON objektini stringga aylantirish
[[nodiscard]] inline std::string jsonGaAylantirish(const JsonObyekt& obj) {
    std::ostringstream result;
    result << "{";
    
    bool first = true;
    for (const auto& [kalit, qiymat] : obj.maydonlar) {
        if (!first) {
            result << ",";
        }
        result << "\"" << kalit << "\":\"" << qiymat << "\"";
        first = false;
    }
    
    result << "}";
    return result.str();
}

// JSON stringini faylga yozish
[[nodiscard]] inline std::expected<void, std::string> jsonYozish(const std::filesystem::path& path,
                                                                 const JsonObyekt& obj) {
    const std::string jsonString = jsonGaAylantirish(obj);
    return matnYozish(path, jsonString);
}

// Kengaytirilgan JSON turlari
struct JsonQiymat {
    enum class Turi {
        Matn,
        Raqam,
        Mantiq,
        Null,
        Massiv,
        Objekt
    };
    
    Turi turi = Turi::Null;
    std::string matnQiymati;
    double raqamQiymati = 0.0;
    bool mantiqQiymati = false;
    std::vector<JsonQiymat> massiv;
    std::unordered_map<std::string, JsonQiymat> objekt;
    
    // Konstruktorlar
    JsonQiymat() = default;
    explicit JsonQiymat(const std::string& matn) : turi(Turi::Matn), matnQiymati(matn) {}
    explicit JsonQiymat(double raqam) : turi(Turi::Raqam), raqamQiymati(raqam) {}
    explicit JsonQiymat(bool mantiq) : turi(Turi::Mantiq), mantiqQiymati(mantiq) {}
    
    // Massiv qo'shish
    void massivQosh(const JsonQiymat& qiymat) {
        if (turi != Turi::Massiv) {
            turi = Turi::Massiv;
            massiv.clear();
        }
        massiv.push_back(qiymat);
    }
    
    // Objekt maydonini qo'shish
    void objektQosh(const std::string& kalit, const JsonQiymat& qiymat) {
        if (turi != Turi::Objekt) {
            turi = Turi::Objekt;
            objekt.clear();
        }
        objekt[kalit] = qiymat;
    }
};

// Kengaytirilgan JSON qiymatini stringga aylantirish
[[nodiscard]] inline std::string jsonQiymatStringKilish(const JsonQiymat& qiymat, int chuqurligi = 0) {
    std::ostringstream result;
    
    switch (qiymat.turi) {
        case JsonQiymat::Turi::Matn:
            result << "\"" << qiymat.matnQiymati << "\"";
            break;
        case JsonQiymat::Turi::Raqam:
            result << qiymat.raqamQiymati;
            break;
        case JsonQiymat::Turi::Mantiq:
            result << (qiymat.mantiqQiymati ? "true" : "false");
            break;
        case JsonQiymat::Turi::Null:
            result << "null";
            break;
        case JsonQiymat::Turi::Massiv:
            result << "[";
            for (std::size_t i = 0; i < qiymat.massiv.size(); ++i) {
                if (i > 0) result << ",";
                result << jsonQiymatStringKilish(qiymat.massiv[i], chuqurligi + 1);
            }
            result << "]";
            break;
        case JsonQiymat::Turi::Objekt: {
            result << "{";
            bool first = true;
            for (const auto& [kalit, qiymat] : qiymat.objekt) {
                if (!first) result << ",";
                result << "\"" << kalit << "\":" << jsonQiymatStringKilish(qiymat, chuqurligi + 1);
                first = false;
            }
            result << "}";
            break;
        }
    }
    
    return result.str();
}

// Kengaytirilgan JSON objektini faylga yozish
[[nodiscard]] inline std::expected<void, std::string> kengaytirilganJsonYozish(
    const std::filesystem::path& path,
    const JsonQiymat& qiymat) {
    const std::string jsonString = jsonQiymatStringKilish(qiymat);
    return matnYozish(path, jsonString);
}

// ===== IKKILIK OQIMLAR (BINARY IO STREAMS) =====

class IkkilikYozuvchi {
    std::ofstream oqim_;
public:
    explicit IkkilikYozuvchi(const std::filesystem::path& yol) 
        : oqim_(yol, std::ios::binary) {}

    [[nodiscard]] bool ochiqmi() const { return oqim_.is_open(); }
    
    template<typename T>
    bool yozish(const T& qiymat) {
        if constexpr (std::is_trivially_copyable_v<T>) {
            oqim_.write(reinterpret_cast<const char*>(&qiymat), sizeof(T));
            return oqim_.good();
        } else if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
            std::size_t len = qiymat.size();
            oqim_.write(reinterpret_cast<const char*>(&len), sizeof(std::size_t));
            oqim_.write(qiymat.c_str(), len);
            return oqim_.good();
        } else {
            static_assert(std::is_trivially_copyable_v<T> || std::is_same_v<std::decay_t<T>, std::string>, "Ushbu turdagi ob'ektni serializatsiya qilib bo'lmaydi.");
            return false;
        }
    }
    
    void yopish() { oqim_.close(); }
};

class IkkilikOquvchi {
    std::ifstream oqim_;
public:
    explicit IkkilikOquvchi(const std::filesystem::path& yol) 
        : oqim_(yol, std::ios::binary) {}

    [[nodiscard]] bool ochiqmi() const { return oqim_.is_open(); }
    [[nodiscard]] bool oxirimi() { return oqim_.eof() || oqim_.peek() == EOF; }
    
    template<typename T>
    Xavfsizlik::Tanlov<T> oqish() {
        T qiymat;
        if constexpr (std::is_trivially_copyable_v<T>) {
            if (oqim_.read(reinterpret_cast<char*>(&qiymat), sizeof(T))) {
                return qiymat;
            }
        } else if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
            std::size_t len;
            if (oqim_.read(reinterpret_cast<char*>(&len), sizeof(std::size_t))) {
                std::string matn(len, '\0');
                if (oqim_.read(&matn[0], len)) {
                    return matn;
                }
            }
        } else {
            static_assert(std::is_trivially_copyable_v<T> || std::is_same_v<std::decay_t<T>, std::string>, "Ushbu turdagi ob'ektni deserializatsiya qilib bo'lmaydi.");
        }
        return std::nullopt;
    }
    
    void yopish() { oqim_.close(); }
};

// ===== KATALOGLAR BILAN ISHLASH =====

class Katalog {
public:
    static bool mavjudmi(const std::string& yol) {
        return std::filesystem::exists(yol) && std::filesystem::is_directory(yol);
    }
    
    static bool yaratish(const std::string& yol) {
        return std::filesystem::create_directories(yol);
    }
    
    static std::vector<std::string> fayllarRoyxati(const std::string& yol) {
        std::vector<std::string> natija;
        if (mavjudmi(yol)) {
            for (const auto& entry : std::filesystem::directory_iterator(yol)) {
                natija.push_back(entry.path().string());
            }
        }
        return natija;
    }
};

} // namespace uzpp::FaylTizimi
