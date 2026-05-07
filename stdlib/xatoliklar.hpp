#pragma once
// status: REAL — Rust-uslubidagi xatoliklar va signal/backtrace ishlovchisi.

#include "platforma.hpp"

#if !defined(UZPP_EMBEDDED)
#include <string>
#include <stdexcept>
#include <csignal>
#include <iostream>
#include <cstdlib>

#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
#include <execinfo.h>
#include <unistd.h>
#endif
#endif

namespace uzpp::Xatoliklar {

// Embedded uskunalarda try/catch qo'llab-quvvatlanmasligi mumkin (xotira tejamkori)
// Shuning uchun Rust uslubidagi "MajburiyNatija" mexanizmi orqali ishlanadi.

template<typename T>
class MajburiyNatija {
    bool ok_;
    T qiymat_;
    
#if !defined(UZPP_EMBEDDED)
    std::string xato_matni_;
#else
    const char* xato_matni_; // Embedded larda std::string xotira olib qoyadi
#endif

public:
#if !defined(UZPP_EMBEDDED)
    MajburiyNatija(const std::string& xato) : ok_(false), xato_matni_(xato) {}
#endif
    MajburiyNatija(const char* xato) : ok_(false), xato_matni_(xato) {}
    MajburiyNatija(T q) : ok_(true), qiymat_(q) {}
    
    [[nodiscard]] bool xatomi() const { return !ok_; }
    [[nodiscard]] bool rostmi() const { return ok_; }
    
    // Rust uslubida "unwrap" qilish. Agar xato bo'lsa darxol xatolikka tushadi.
    T echish() const {
        if (!ok_) {
#if defined(UZPP_EMBEDDED)
            // Microkontrollerda tizim qotish (Hard-Fault / Reset Loop)
            while(true) {}
#else
            throw std::runtime_error("MajburiyNatija echishda xato: " + xato_matni_);
#endif
        }
        return qiymat_;
    }
    
    // Xato bo'lsa default (standart) ko'rsatilgan qiymat qaytadi.
    T yokida(T standart_qiymat) const {
        return ok_ ? qiymat_ : standart_qiymat;
    }
    
#if !defined(UZPP_EMBEDDED)
    std::string xatoMatni() const { return xato_matni_; }
#else
    const char* xatoMatni() const { return xato_matni_; }
#endif
};

// Hech narsa qaytarmaydigan lekin faqat xatolik statini saqlaydigan
class IshNatijasi {
    bool ok_;
#if !defined(UZPP_EMBEDDED)
    std::string xato_matni_;
#else
    const char* xato_matni_;
#endif
    
public:
    IshNatijasi() : ok_(true), xato_matni_("") {}
#if !defined(UZPP_EMBEDDED)
    IshNatijasi(const std::string& xato) : ok_(false), xato_matni_(xato) {}
#endif
    IshNatijasi(const char* xato) : ok_(false), xato_matni_(xato) {}
    
    [[nodiscard]] bool muvaffaqiyatlimi() const { return ok_; }
    
#if !defined(UZPP_EMBEDDED)
    std::string xatoniOlish() const { return xato_matni_; }
#else
    const char* xatoniOlish() const { return xato_matni_; }
#endif
};

#if !defined(UZPP_EMBEDDED)
inline void dastur_qulashi_xabari(int signal) {
    std::cerr << "\n======================================================\n";
    std::cerr << " [UZ++ HALOKAT]: Dastur tizim xatosi tufayli quladi!\n";
    std::cerr << " Signal kodi: " << signal;
    if (signal == SIGSEGV) std::cerr << " (Xotiraga ruxsatsiz murojaat / Segfault)";
    else if (signal == SIGFPE) std::cerr << " (Matematik xato / Nolga bo'lish)";
    else if (signal == SIGABRT) std::cerr << " (Dastur majburiy to'xtatildi / Abort)";
    std::cerr << "\n";

#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
    std::cerr << "\n --- BACKTRACE (Chaqiruvlar ro'yxati) ---\n";
    void* callstack[128];
    int frames = backtrace(callstack, 128);
    char** strs = backtrace_symbols(callstack, frames);
    if (strs) {
        for (int i = 0; i < frames; ++i) {
            std::cerr << "  [" << i << "] " << strs[i] << "\n";
        }
        free(strs);
    }
#endif

    std::cerr << "\n Iltimos, dasturni '--debug' bayrog'i bilan qurib tekshiring.\n";
    std::cerr << "======================================================\n";
    std::exit(signal);
}

inline void xavfsizlikni_yoqish() {
    std::signal(SIGSEGV, dastur_qulashi_xabari);
    std::signal(SIGABRT, dastur_qulashi_xabari);
    std::signal(SIGILL, dastur_qulashi_xabari);
    std::signal(SIGFPE, dastur_qulashi_xabari);
}
#else
inline void xavfsizlikni_yoqish() {}
#endif

} // namespace uzpp::Xatoliklar
