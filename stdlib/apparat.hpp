#pragma once
// status: REAL — Mikrokontroller abstraksiyasi. PC uchun mock loglar bilan to'ldirilgan.

#include "platforma.hpp"

#if defined(ARDUINO) || defined(UZPP_EMBEDDED)
#include <Arduino.h>
#else
#include <iostream>
#include <thread>
#include <chrono>
#endif

namespace uzpp::Apparat {

// Mikrokontroller Pinlari bilan ishlash uchun hardware abstraksiyasi
// PC va Mobil operatsion tizimlarda faqat Mock shaklida qoladi.

enum class PinHolati {
    Past = 0, // Arduino LOW o'rnida
    Baland = 1 // Arduino HIGH o'rnida
};

enum class PinTartibi {
    Oqish, // INPUT
    Yozish // OUTPUT
};

class Pin {
    int raqam_;
    PinTartibi tartib_;
public:
    Pin(int r, PinTartibi t) : raqam_(r), tartib_(t) {}
    
    void yozish(PinHolati h) {
#if defined(UZPP_EMBEDDED)
        pinMode(raqam_, OUTPUT);
        digitalWrite(raqam_, static_cast<int>(h));
#else
        std::cout << "[MOCK] Pin " << raqam_ << " holati o'zgartirildi: " << (h == PinHolati::Baland ? "Baland" : "Past") << "\n";
#endif
    }
    
    void analogYozish(int qiymat) {
#if defined(UZPP_EMBEDDED)
        pinMode(raqam_, OUTPUT);
        analogWrite(raqam_, qiymat);
#else
        std::cout << "[MOCK] Pin " << raqam_ << " analog yozildi: " << qiymat << "\n";
#endif
    }
    
    [[nodiscard]] PinHolati oqish() const {
#if defined(UZPP_EMBEDDED)
        pinMode(raqam_, INPUT);
        return digitalRead(raqam_) == HIGH ? PinHolati::Baland : PinHolati::Past;
#else
        return PinHolati::Past; // Mock for PC
#endif
    }
    
    [[nodiscard]] int analogOqish() const {
#if defined(UZPP_EMBEDDED)
        pinMode(raqam_, INPUT);
        return analogRead(raqam_);
#else
        return 0; // Mock for PC
#endif
    }
};

inline void kutish(int millisekund) {
#if defined(UZPP_EMBEDDED)
    delay(millisekund);
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(millisekund));
#endif
}

// UART port (Serial) abstraksiyasi
class SerialUlanish {
public:
    static void boshlash(int baud_rate = 9600) {
#if defined(UZPP_EMBEDDED)
        Serial.begin(baud_rate);
#else
        std::cout << "[MOCK] Serial ulanish boshlandi: " << baud_rate << " baud.\n";
#endif
    }
    
    static void yozish(const char* matn) {
#if defined(UZPP_EMBEDDED)
        Serial.print(matn);
#else
        std::cout << matn;
#endif
    }
};

} // namespace uzpp::Apparat
