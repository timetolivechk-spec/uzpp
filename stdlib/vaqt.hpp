#pragma once
#ifndef UZPP_GEN_VAQT_HPP_
#define UZPP_GEN_VAQT_HPP_
#line 1 "C:\\Users\\MSN\\uz++\\stdlib\\vaqt.uzpp"
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <thread>
namespace uzpp::Vaqt {
    struct SanaVaVaqt {
    public:
        int yil;
        int oy;
        int kun;
        int soat;
        int daqiqa;
        int soniya;
        std::string mintaqa;
        std::string formatlash(const std::string& format = "%Y-%m-%d %H:%M:%S") const
        {
            std::tm tm_vaqt;
            (tm_vaqt.tm_year =(yil - 1900));
            (tm_vaqt.tm_mon =(oy - 1));
            (tm_vaqt.tm_mday = kun);
            (tm_vaqt.tm_hour = soat);
            (tm_vaqt.tm_min = daqiqa);
            (tm_vaqt.tm_sec = soniya);
            std::ostringstream ss;
            (ss << std::put_time(& tm_vaqt, format.c_str()));
            if((! mintaqa.empty() &&(mintaqa != "Local")))
                {
                    (((ss << " (") << mintaqa) << ")");
                }
            return ss.str();
        }
    };
    inline auto hozir()->SanaVaVaqt
    {
        auto t = std::time(nullptr);
        auto local_tm = * std::localtime(& t);
        SanaVaVaqt natija;
        (natija.yil =(local_tm.tm_year + 1900));
        (natija.oy =(local_tm.tm_mon + 1));
        (natija.kun = local_tm.tm_mday);
        (natija.soat = local_tm.tm_hour);
        (natija.daqiqa = local_tm.tm_min);
        (natija.soniya = local_tm.tm_sec);
        (natija.mintaqa = "Local");
        return natija;
    }
    inline auto mintaqaVaqti(const std::string& mintaqa_nomi)->SanaVaVaqt
    {
        std::string envStr =("TZ=" + mintaqa_nomi);
        putenv(const_cast<char*>(envStr.c_str()));
        tzset();
        auto t = std::time(nullptr);
        auto local_tm = * std::localtime(& t);
        SanaVaVaqt natija;
        (natija.yil =(local_tm.tm_year + 1900));
        (natija.oy =(local_tm.tm_mon + 1));
        (natija.kun = local_tm.tm_mday);
        (natija.soat = local_tm.tm_hour);
        (natija.daqiqa = local_tm.tm_min);
        (natija.soniya = local_tm.tm_sec);
        (natija.mintaqa = mintaqa_nomi);
        putenv(const_cast<char*>(std::string("TZ=").c_str()));
        tzset();
        return natija;
    }
    inline auto kutish(int soniya)->void
    {
        std::this_thread::sleep_for(std::chrono::seconds(soniya));
    }
    inline auto kutish_ms(int millisoniya)->void
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(millisoniya));
    }
}
#endif
