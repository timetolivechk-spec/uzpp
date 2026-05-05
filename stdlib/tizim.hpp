#pragma once

#include "platforma.hpp"

#if !defined(UZPP_EMBEDDED)
#include <cstdlib>
#include <string>
#include <array>
#include <memory>
#include <stdexcept>
#include <vector>
#include <unordered_map>
#endif

namespace uzpp::Tizim {

[[nodiscard]] inline bool buWindows() { return Platforma::Windows; }
[[nodiscard]] inline bool buLinux() { return Platforma::Linux; }
[[nodiscard]] inline bool buMacOS() { return Platforma::MacOS; }
[[nodiscard]] inline bool buMobil() { return Platforma::MobilOS; }

#if !defined(UZPP_EMBEDDED)

// Muhit o'zgaruvchisini olish (Environment Variable)
[[nodiscard]] inline std::string muhitOzgaruvchisi(const std::string& ism) {
    const char* qiymat = std::getenv(ism.c_str());
    return qiymat ? std::string(qiymat) : "";
}

// Tizimli buyruq yuborish (Terminal chaqiruvi)
inline int buyruqBajarish(const std::string& buyruq) {
    return std::system(buyruq.c_str());
}

class Jarayon {
public:
    // Tizim buyrugini ishga tushirish va natijasini (stdout) matn sifatida qaytarish
    static std::string buyruqNatijasi(const std::string& buyruq) {
        std::array<char, 128> buffer;
        std::string result;
        
#ifdef _WIN32
        std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(buyruq.c_str(), "r"), _pclose);
#else
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(buyruq.c_str(), "r"), pclose);
#endif
        if (!pipe) {
            throw std::runtime_error("Buyruqni ishga tushirib bo'lmadi!");
        }
        while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        return result;
    }
};

// Buyruqlar qatori argumentlarini (CLI arguments) qulay o'qish uchun klass
class Argumentlar {
    std::vector<std::string> args_;
    std::unordered_map<std::string, std::string> flags_;
public:
    explicit Argumentlar(const std::vector<std::string>& a) {
        for (size_t i = 0; i < a.size(); ++i) {
            if (a[i].starts_with("--")) {
                size_t eq = a[i].find('=');
                if (eq != std::string::npos) {
                    flags_[a[i].substr(2, eq - 2)] = a[i].substr(eq + 1);
                } else {
                    flags_[a[i].substr(2)] = "rost";
                }
            } else if (a[i].starts_with("-")) {
                flags_[a[i].substr(1)] = "rost";
            } else {
                args_.push_back(a[i]);
            }
        }
    }
    
    [[nodiscard]] bool mavjudmi(const std::string& nom) const { return flags_.contains(nom); }
    [[nodiscard]] std::string olish(const std::string& nom, const std::string& stdVal = "") const {
        return flags_.contains(nom) ? flags_.at(nom) : stdVal;
    }
    [[nodiscard]] const std::vector<std::string>& qolganlari() const { return args_; }
};

#endif

} // namespace uzpp::Tizim
