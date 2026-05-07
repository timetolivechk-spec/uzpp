#pragma once
// status: EXPERIMENTAL — bu modul to'liq ishlamaydi va o'zgarishi mumkin.
// This module is incomplete and subject to change. See docs/stdlib-status.md.

#include "platforma.hpp"

#if defined(ARDUINO) || defined(UZPP_EMBEDDED)
#include <Arduino.h>
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
};

// UART port (Serial) abstraksiyasi
class SerialUlanish {
public:
    static void boshlash(int baud_rate = 9600) {
#if defined(UZPP_EMBEDDED)
        Serial.begin(baud_rate);
#endif
    }
    
    static void yozish(const char* matn) {
#if defined(UZPP_EMBEDDED)
        Serial.print(matn);
#endif
    }
};

} // namespace uzpp::Apparat
