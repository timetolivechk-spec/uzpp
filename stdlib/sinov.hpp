#pragma once

#include "platforma.hpp"

#if !defined(UZPP_EMBEDDED)
#include <string>
#include <vector>
#include <functional>
#include <stdexcept>
#include <chrono>
#include "jurnal.hpp"
#endif

namespace uzpp::Sinov {

// Krossplatfoman Unit-Test muhiti (faqat kompyuterlar tizimida)
#if !defined(UZPP_EMBEDDED)

inline void assertKutish(bool holat, const std::string& xabar = "Kutilgan qiymat tasdiqlanmadi") {
    if (!holat) {
        throw std::runtime_error("Sinov XATOSI: " + xabar);
    }
}

template<typename T1, typename T2>
inline void assertTenglik(const T1& a, const T2& b, const std::string& xabar = "Tenglik kutildi") {
    if (a != b) {
        throw std::runtime_error("Sinov XATOSI: Qiymatlar teng emas (" + xabar + ")");
    }
}

class SinovlarToplami {
    struct TestYozuv {
        std::string nom;
        std::function<void()> funksiya;
    };
    std::vector<TestYozuv> testlar_;
    
public:
    void testQoshish(const std::string& nom, std::function<void()> f) {
        testlar_.push_back({nom, std::move(f)});
    }
    
    void ishgaTushirish() {
        int muvaffaqiyat = 0;
        int xatolar = 0;
        
        Jurnal::Logger::malumot("=== SINOVLAR BOSHLANDI ===");
        
        for (const auto& t : testlar_) {
            try {
                t.funksiya();
                Jurnal::Logger::malumot("[O'TDI] " + t.nom);
                muvaffaqiyat++;
            } catch (const std::exception& e) {
                Jurnal::Logger::xato("[YIQILDI] " + t.nom + " -> " + e.what());
                xatolar++;
            } catch (...) {
                Jurnal::Logger::xato("[YIQILDI] " + t.nom + " -> Noma'lum xato");
                xatolar++;
            }
        }
        
        Jurnal::Logger::malumot("Natija: O'tdi = " + std::to_string(muvaffaqiyat) + ", Yiqildi = " + std::to_string(xatolar));
        if (xatolar > 0) {
            throw std::runtime_error("Dastur testlardan o'tmadi!");
        }
    }
};

class BenchToplami {
    struct BenchYozuv {
        std::string nom;
        std::function<void()> funksiya;
    };
    std::vector<BenchYozuv> benchlar_;
    
public:
    void benchQoshish(const std::string& nom, std::function<void()> f) {
        benchlar_.push_back({nom, std::move(f)});
    }
    
    void ishgaTushirish() {
        Jurnal::Logger::malumot("=== BENCHMARKLAR BOSHLANDI ===");
        for (const auto& b : benchlar_) {
            // Warmup (Keshlarni qizitish)
            b.funksiya();
            auto start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < 1000; i++) {
                b.funksiya();
            }
            auto end = std::chrono::high_resolution_clock::now();
            double davomiyligiMs = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;
            double bittaMs = davomiyligiMs / 1000.0;
            Jurnal::Logger::malumot("[BENCH] " + b.nom + " -> Jami (1000 marta): " + std::to_string(davomiyligiMs) + " ms | O'rtacha 1 ta: " + std::to_string(bittaMs) + " ms");
        }
    }
};

#endif

} // namespace uzpp::Sinov
