#pragma once

#include "platforma.hpp"
#include "vaqt.hpp"
#include <string>

#if !defined(UZPP_EMBEDDED)
    #include <iostream>
    #include <fstream>
    #include <mutex>
#else
    // Mikrokontrollerlar uchun UART orqali output (Serial.print kabi)
    extern void uzpp_embedded_print(const char* matn);
#endif

namespace uzpp::Jurnal {

enum class Daraja {
    DEBUG,
    MALUMOT,    // INFO
    OGOHLANTIRISH, // WARN
    XATO        // ERROR
};

class Logger {
public:
    static void yozish(Daraja daraja, const std::string& matn) {
#if !defined(UZPP_EMBEDDED)
        std::lock_guard<std::mutex> lock(mutex_);
#endif
        std::string teg = "[M'ALUMOT]";
        std::string rang = "\033[0m"; // Reset

        switch (daraja) {
            case Daraja::DEBUG: teg = "[DEBUG]"; rang = "\033[36m"; break; // Cyan
            case Daraja::MALUMOT: teg = "[INFO]"; rang = "\033[32m"; break; // Yashil
            case Daraja::OGOHLANTIRISH: teg = "[OGOHLANTIRISH]"; rang = "\033[33m"; break; // Sariq
            case Daraja::XATO: teg = "[XATO]"; rang = "\033[31m"; break; // Qizil
        }

        std::string xabar = Vaqt::hozir().formatlash("%H:%M:%S") + " " + teg + " " + matn + "\n";

#if !defined(UZPP_EMBEDDED)
#if defined(UZPP_OS_ANDROID)
        // Android uchun Logcat ishlatilishi kerak. Bu mock implementatsiya (kengaytirish qilinadi)
        std::cout << xabar;
#else
        // Windows/Linux PC Output (Rangli)
        std::cout << rang << xabar << "\033[0m";
#endif
#else
        // Embedded (Arduino/STM32) chiqarish uzpp_embedded_print orqali
        uzpp_embedded_print(xabar.c_str());
#endif
    }

    static void debug(const std::string& m) { yozish(Daraja::DEBUG, m); }
    static void malumot(const std::string& m) { yozish(Daraja::MALUMOT, m); }
    static void ogohlantirish(const std::string& m) { yozish(Daraja::OGOHLANTIRISH, m); }
    static void xato(const std::string& m) { yozish(Daraja::XATO, m); }

private:
#if !defined(UZPP_EMBEDDED)
    static inline std::mutex mutex_;
#endif
};

#if !defined(UZPP_EMBEDDED)
class Proufayler {
    std::string nomi_;
    std::chrono::time_point<std::chrono::high_resolution_clock> boshlanish_;
public:
    explicit Proufayler(std::string nomi) 
        : nomi_(std::move(nomi)), boshlanish_(std::chrono::high_resolution_clock::now()) {}
    
    ~Proufayler() {
        auto tugash = std::chrono::high_resolution_clock::now();
        auto davomiylik = std::chrono::duration_cast<std::chrono::microseconds>(tugash - boshlanish_).count();
        double ms = davomiylik / 1000.0;
        Logger::malumot("[PROFILER] '" + nomi_ + "' bajarildi: " + std::to_string(ms) + " ms");
    }
};
#endif

} // namespace uzpp::Jurnal

#if !defined(UZPP_EMBEDDED)
#define UZPP_CONCAT_IMPL(a, b) a##b
#define UZPP_CONCAT(a, b) UZPP_CONCAT_IMPL(a, b)
#define VAQT_OLCHOVI(nom) uzpp::Jurnal::Proufayler UZPP_CONCAT(_proufayler_, __LINE__)(nom)
#else
#define VAQT_OLCHOVI(nom)
#endif
