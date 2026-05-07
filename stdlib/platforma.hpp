#pragma once
// status: REAL — Platformani aniqlash makroslari va funksiyalari.

// UZPP Kross-platforma Makroslari
// Platformani aniqlash

#if defined(_WIN32) || defined(_WIN64)
    #define UZPP_OS_WINDOWS 1
#elif defined(__APPLE__) && defined(__MACH__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
        #define UZPP_OS_IOS 1
        #define UZPP_MOBILE 1
    #else
        #define UZPP_OS_MACOS 1
    #endif
#elif defined(__ANDROID__)
    #define UZPP_OS_ANDROID 1
    #define UZPP_MOBILE 1
#elif defined(__linux__)
    #define UZPP_OS_LINUX 1
#elif defined(__unix__)
    #define UZPP_OS_UNIX 1
#endif

// Mikrokontrollerlarni (Embedded) tekshirish
// Agar foydalanuvchi C++ kompilatorida ataylab UZPP_EMBEDDED define qilsa
// Barcha og'ir kutubxonalar (fayl tizimi, oqimlar) o'chiriladi.
#if defined(ARDUINO) || defined(ESP32) || defined(ESP8266) || defined(STM32)
    #ifndef UZPP_EMBEDDED
        #define UZPP_EMBEDDED 1
    #endif
#endif

namespace uzpp::Platforma {

#if defined(UZPP_OS_WINDOWS)
    constexpr bool Windows = true;
    constexpr bool Linux = false;
    constexpr bool MacOS = false;
    constexpr bool Android = false;
    constexpr bool iOS = false;
#elif defined(UZPP_OS_LINUX)
    constexpr bool Windows = false;
    constexpr bool Linux = true;
    constexpr bool MacOS = false;
    constexpr bool Android = false;
    constexpr bool iOS = false;
#elif defined(UZPP_OS_ANDROID)
    constexpr bool Windows = false;
    constexpr bool Linux = false;
    constexpr bool MacOS = false;
    constexpr bool Android = true;
    constexpr bool iOS = false;
#elif defined(UZPP_OS_IOS)
    constexpr bool Windows = false;
    constexpr bool Linux = false;
    constexpr bool MacOS = false;
    constexpr bool Android = false;
    constexpr bool iOS = true;
#elif defined(UZPP_OS_MACOS)
    constexpr bool Windows = false;
    constexpr bool Linux = false;
    constexpr bool MacOS = true;
    constexpr bool Android = false;
    constexpr bool iOS = false;
#else
    constexpr bool Windows = false;
    constexpr bool Linux = false;
    constexpr bool MacOS = false;
    constexpr bool Android = false;
    constexpr bool iOS = false;
#endif

#if defined(UZPP_MOBILE)
    constexpr bool MobilOS = true;
#else
    constexpr bool MobilOS = false;
#endif

#if defined(UZPP_EMBEDDED)
    constexpr bool Mikrokontroller = true;
    constexpr bool OgirKutubxonalarMavjud = false;
#else
    constexpr bool Mikrokontroller = false;
    constexpr bool OgirKutubxonalarMavjud = true;
#endif

    constexpr const char* nom() {
        if (Windows) return "Windows";
        if (Linux) return "Linux";
        if (MacOS) return "MacOS";
        if (Android) return "Android";
        if (iOS) return "iOS";
        if (Mikrokontroller) return "Embedded";
        return "Noma'lum";
    }

} // namespace uzpp::Platforma
