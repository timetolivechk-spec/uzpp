#pragma once

#include "platforma.hpp"

#if !defined(UZPP_EMBEDDED)
#include <unordered_map>
#include <string>
#include <mutex>
#endif

namespace uzpp::Kesh {

#if !defined(UZPP_EMBEDDED)

class XotiraKesh {
    std::unordered_map<std::string, std::string> malumotlar_;
    std::mutex mutex_;
    
public:
    void saqlash(const std::string& kalit, const std::string& qiymat) {
        std::lock_guard<std::mutex> lock(mutex_);
        malumotlar_[kalit] = qiymat;
    }
    
    std::string olish(const std::string& kalit, const std::string& defaultVal = "") {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = malumotlar_.find(kalit);
        return (it != malumotlar_.end()) ? it->second : defaultVal;
    }
    
    bool mavjudmi(const std::string& kalit) {
        std::lock_guard<std::mutex> lock(mutex_);
        return malumotlar_.contains(kalit);
    }
    
    void ochirish(const std::string& kalit) {
        std::lock_guard<std::mutex> lock(mutex_);
        malumotlar_.erase(kalit);
    }
};

#endif

} // namespace uzpp::Kesh
