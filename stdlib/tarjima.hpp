#pragma once

#include "platforma.hpp"
#include "xavfsizlik.hpp"
#include "fayl_tizimi.hpp"
#include "json.hpp"

#include <string>
#include <unordered_map>

namespace uzpp::Tarjima {

class Tarjimon {
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> lugatlar_;
    std::string joriyTil_ = "uz";
public:
    void tilniOrnatish(const std::string& til) { joriyTil_ = til; }
    
    [[nodiscard]] std::string joriyTil() const { return joriyTil_; }
    
    Xavfsizlik::Natija<bool> lugatYuklash(const std::string& til, const std::string& faylYoli) {
        auto faylNatija = FaylTizimi::matnOqish(faylYoli);
        if (!faylNatija) return Xavfsizlik::xato(faylNatija.error());
        
        try {
            auto jsonObyekt = Json::tahlil(faylNatija.value());
            if (!jsonObyekt || !jsonObyekt->obyektmi()) {
                return Xavfsizlik::xato(std::string("JSON noto'g'ri yoki ob'yekt emas"));
            }
            for (const auto& [kalit, qiymat] : jsonObyekt->obyekt()) {
                if (qiymat->matnmi()) {
                    lugatlar_[til][kalit] = qiymat->matn();
                }
            }
            return Xavfsizlik::muvaffaqiyat(true);
        } catch(const std::exception& e) {
            return Xavfsizlik::xato(std::string("JSON tahlilida xato: ") + e.what());
        }
    }
    
    std::string tarjima(const std::string& kalit) {
        if (lugatlar_.contains(joriyTil_) && lugatlar_[joriyTil_].contains(kalit)) {
            return lugatlar_[joriyTil_][kalit];
        }
        return kalit; // Agar tarjima topilmasa kalitning o'zini qaytaramiz (Fallback)
    }
    
    std::string operator()(const std::string& kalit) { return tarjima(kalit); }
};

inline Tarjimon I18N; // Global instansiya

} // namespace uzpp::Tarjima