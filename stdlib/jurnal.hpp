#pragma once
#ifndef UZPP_GEN_JURNAL_HPP_
#define UZPP_GEN_JURNAL_HPP_
#line 1 "C:\\Users\\MSN\\uz++\\stdlib\\jurnal.uzpp"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
namespace uzpp::Jurnal {
    enum class Daraja {
        JIM = 0,
        XATO = 1,
        OGOH = 2,
        MALUMOT = 3,
        TUZATISH = 4
    };

    inline std::string Daraja_nomi (Daraja _v) {
        switch (_v) {
            case Daraja::JIM: return "JIM";
            case Daraja::XATO: return "XATO";
            case Daraja::OGOH: return "OGOH";
            case Daraja::MALUMOT: return "MALUMOT";
            case Daraja::TUZATISH: return "TUZATISH";
            default: return "?";
        }
    }
    struct JurnalHolati {
    public:
        Daraja joriy_daraja;
        bool vaqt_damgasi;
        std::mutex jurnal_mutex;
    };
    inline auto holat_olish()->JurnalHolati&
    {
        static JurnalHolati jh;
        static bool birinchi_marta = true;
        if(birinchi_marta)
            {
                (birinchi_marta = false);
                (jh.joriy_daraja = Daraja::MALUMOT);
                (jh.vaqt_damgasi = true);
            }
        return jh;
    }
    inline auto vaqt_damgasini_olish()->std::string
    {
        auto hozirgi_vaqt = std::time(nullptr);
        auto mahalliy_vaqt = * std::localtime(& hozirgi_vaqt);
        std::ostringstream ss;
        (ss << std::put_time(& mahalliy_vaqt, "%Y-%m-%d %H:%M:%S"));
        return ss.str();
    }
    inline auto darajani_matnga(Daraja d)->std::string
    {
        {
            auto&& _match_val_1 = d;
            if(_match_val_1 == Daraja::JIM)
                {
                    return "JIM";
                }
            else if(_match_val_1 == Daraja::XATO)
                {
                    return "XATO";
                }
            else if(_match_val_1 == Daraja::OGOH)
                {
                    return "OGOHLANTIRISH";
                }
            else if(_match_val_1 == Daraja::MALUMOT)
                {
                    return "MALUMOT";
                }
            else if(_match_val_1 == Daraja::TUZATISH)
                {
                    return "TUZATISH";
                }
            else
                {
                    return "NOMA'LUM";
                }
        }
    }
    inline auto yozish_agar(Daraja d, const std::string& xabar)->void
    {
        auto& h = holat_olish();
        if((d > h.joriy_daraja))
            {
                return;
            }
        std::lock_guard<std::mutex> qulf = std::lock_guard<std::mutex>(h.jurnal_mutex);
        if(h.vaqt_damgasi)
            {
                (((std::cout << "[") << vaqt_damgasini_olish()) << "] ");
            }
        (((((std::cout << "[") << darajani_matnga(d)) << "] ") << xabar) << std::endl);
    }
    inline auto tuzatish(const std::string& xabar)->void
    {
        yozish_agar(Daraja::TUZATISH, xabar);
    }
    inline auto malumot(const std::string& xabar)->void
    {
        yozish_agar(Daraja::MALUMOT, xabar);
    }
    inline auto ogohlantirish(const std::string& xabar)->void
    {
        yozish_agar(Daraja::OGOH, xabar);
    }
    inline auto xato(const std::string& xabar)->void
    {
        yozish_agar(Daraja::XATO, xabar);
    }
    inline auto daraja_orqatish(Daraja d)->void
    {
        (holat_olish().joriy_daraja = d);
    }
    inline auto joriy_darajani_olish()->Daraja
    {
        return holat_olish().joriy_daraja;
    }
    inline auto vaqt_damgasini_yoqish(bool yoqish)->void
    {
        (holat_olish().vaqt_damgasi = yoqish);
    }
    class Logger {
    public:
        static void tuzatish(const std::string& m)
        {
            Jurnal::tuzatish(m);
        }
        static void malumot(const std::string& m)
        {
            Jurnal::malumot(m);
        }
        static void ogohlantirish(const std::string& m)
        {
            Jurnal::ogohlantirish(m);
        }
        static void xato(const std::string& m)
        {
            Jurnal::xato(m);
        }
    };
}
#endif
