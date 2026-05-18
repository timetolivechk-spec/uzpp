#pragma once
#ifndef UZPP_GEN_XATOLIKLAR_HPP_
#define UZPP_GEN_XATOLIKLAR_HPP_
#line 1 "C:\\Users\\MSN\\uz++\\stdlib\\xatoliklar.uzpp"
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
namespace uzpp::Xatoliklar {
    template <typename T >
 class MajburiyNatija {
    public:
        bool ok_;
        T qiymat_;
        std::string xato_matni_;
        MajburiyNatija(const std::string& xato) : ok_ ( false ) , xato_matni_ ( xato )
        {
        }
        MajburiyNatija(T q) : ok_ ( true ) , qiymat_ ( q )
        {
        }
        bool xatomi() const
        {
            return ! ok_;
        }
        bool rostmi() const
        {
            return ok_;
        }
        T echish() const
        {
            if(! ok_)
                {
                    throw std::runtime_error(("MajburiyNatija echishda xato: " + xato_matni_));
                }
            return qiymat_;
        }
        T yokida(T standart_qiymat) const
        {
            return(ok_ ? qiymat_: standart_qiymat);
        }
        std::string xatoMatni() const
        {
            return xato_matni_;
        }
    };
    class IshNatijasi {
    public:
        bool ok_;
        std::string xato_matni_;
        IshNatijasi() : ok_ ( true )
        {
        }
        IshNatijasi(const std::string& xato) : ok_ ( false ) , xato_matni_ ( xato )
        {
        }
        bool muvaffaqiyatlimi() const
        {
            return ok_;
        }
        std::string xatoniOlish() const
        {
            return xato_matni_;
        }
    };
    inline auto dastur_qulashi_xabari(int signal)->void
    {
        (std::cout << "\n======================================================\n");
        (std::cout << " [UZ++ HALOKAT]: Dastur tizim xatosi tufayli quladi!\n");
        ((std::cout << " Signal kodi: ") << signal);
        if((signal == SIGSEGV))
            {
                (std::cout << " (Xotiraga ruxsatsiz murojaat / Segfault)");
            }
        else
            if((signal == SIGFPE))
                {
                    (std::cout << " (Matematik xato / Nolga bo'lish)");
                }
            else
                if((signal == SIGABRT))
                    {
                        (std::cout << " (Dastur majburiy to'xtatildi / Abort)");
                    }
        (std::cout << std::endl);
        (std::cout << "\n --- BACKTRACE (Chaqiruvlar ro'yxati) ---\n");
        (std::cout << "\n Iltimos, dasturni '--debug' bayrog'i bilan qurib tekshiring.\n");
        (std::cout << "======================================================\n");
        std::exit(signal);
    }
    inline auto xavfsizlikni_yoqish()->void
    {
        std::signal(SIGSEGV, dastur_qulashi_xabari);
        std::signal(SIGABRT, dastur_qulashi_xabari);
        std::signal(SIGILL, dastur_qulashi_xabari);
        std::signal(SIGFPE, dastur_qulashi_xabari);
    }
}
#endif
