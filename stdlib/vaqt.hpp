#pragma once

#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>
#include <ctime>
#include <string>
#include <cstdlib>

#if defined(ARDUINO) || defined(UZPP_EMBEDDED)
#include <Arduino.h>
#endif

namespace uzpp::Vaqt {

struct SanaVaVaqt {
    int yil;
    int oy;
    int kun;
    int soat;
    int daqiqa;
    int soniya;
    std::string mintaqa;

    [[nodiscard]] std::string formatlash(const std::string& format = "%Y-%m-%d %H:%M:%S") const {
        std::tm tm_time{};
        tm_time.tm_year = yil - 1900;
        tm_time.tm_mon = oy - 1;
        tm_time.tm_mday = kun;
        tm_time.tm_hour = soat;
        tm_time.tm_min = daqiqa;
        tm_time.tm_sec = soniya;

        std::ostringstream ss;
        ss << std::put_time(&tm_time, format.c_str());
        if (!mintaqa.empty() && mintaqa != "Local") {
            ss << " (" << mintaqa << ")";
        }
        return ss.str();
    }
};

[[nodiscard]] inline SanaVaVaqt hozir() {
    auto t = std::time(nullptr);
    auto local_tm = *std::localtime(&t);

    return SanaVaVaqt{
        local_tm.tm_year + 1900,
        local_tm.tm_mon + 1,
        local_tm.tm_mday,
        local_tm.tm_hour,
        local_tm.tm_min,
        local_tm.tm_sec,
        "Local"
    };
}

[[nodiscard]] inline SanaVaVaqt mintaqaVaqti(const std::string& mintaqa_nomi) {
#if __cplusplus >= 202002L && defined(__cpp_lib_chrono) && __cpp_lib_chrono >= 201907L
    try {
        auto t = std::chrono::system_clock::now();
        auto zt = std::chrono::zoned_time(mintaqa_nomi, t);
        auto local_time = zt.get_local_time();
        auto dp = std::chrono::floor<std::chrono::days>(local_time);
        std::chrono::year_month_day ymd{dp};
        std::chrono::hh_mm_ss time{std::chrono::floor<std::chrono::seconds>(local_time - dp)};
        
        return SanaVaVaqt{
            static_cast<int>(ymd.year()),
            static_cast<int>(static_cast<unsigned>(ymd.month())),
            static_cast<int>(static_cast<unsigned>(ymd.day())),
            static_cast<int>(time.hours().count()),
            static_cast<int>(time.minutes().count()),
            static_cast<int>(time.seconds().count()),
            mintaqa_nomi
        };
    } catch (...) {
        return hozir();
    }
#else
#if defined(_WIN32)
    std::string envStr = "TZ=" + mintaqa_nomi;
    _putenv(envStr.c_str());
    _tzset();
#else
    setenv("TZ", mintaqa_nomi.c_str(), 1);
    tzset();
#endif
    auto t = std::time(nullptr);
    auto local_tm = *std::localtime(&t);
    SanaVaVaqt natija = {
        local_tm.tm_year + 1900, local_tm.tm_mon + 1, local_tm.tm_mday,
        local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec, mintaqa_nomi
    };
#if defined(_WIN32)
    _putenv("TZ="); _tzset();
#else
    unsetenv("TZ"); tzset();
#endif
    return natija;
#endif
}

inline void kutish(int soniya) {
    std::this_thread::sleep_for(std::chrono::seconds(soniya));
}

inline void kutish_ms(int millisoniya) {
#if defined(ARDUINO) || defined(UZPP_EMBEDDED)
    delay(millisoniya);
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(millisoniya));
#endif
}

} // namespace uzpp::Vaqt
