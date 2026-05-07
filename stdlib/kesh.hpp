#pragma once

// status: REAL — shablonli xotira keshi, production uchun tayyor.

#include "platforma.hpp"

#if !defined(UZPP_EMBEDDED)
#include <unordered_map>
#include <mutex>
#include <optional>
#endif

namespace uzpp::Kesh {

#if !defined(UZPP_EMBEDDED)

template<typename Kalit = std::string, typename Qiymat = std::string>
class XotiraKesh {
    std::unordered_map<Kalit, Qiymat> malumotlar_;
    mutable std::mutex mutex_;
    
public:
    void saqlash(const Kalit& kalit, const Qiymat& qiymat) {
        std::lock_guard<std::mutex> lock(mutex_);
        malumotlar_[kalit] = qiymat;
    }
    
    Qiymat olish(const Kalit& kalit, const Qiymat& defaultVal = Qiymat{}) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = malumotlar_.find(kalit);
        return (it != malumotlar_.end()) ? it->second : defaultVal;
    }
    
    std::optional<Qiymat> izlash(const Kalit& kalit) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = malumotlar_.find(kalit);
        if (it != malumotlar_.end()) {
            return it->second;
        }
        return std::nullopt;
    }
    
    bool mavjudmi(const Kalit& kalit) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return malumotlar_.contains(kalit);
    }
    
    void ochirish(const Kalit& kalit) {
        std::lock_guard<std::mutex> lock(mutex_);
        malumotlar_.erase(kalit);
    }
    
    void tozalash() {
        std::lock_guard<std::mutex> lock(mutex_);
        malumotlar_.clear();
    }
    
    size_t hajm() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return malumotlar_.size();
    }
};

#endif

} // namespace uzpp::Kesh
