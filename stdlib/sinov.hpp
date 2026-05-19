#pragma once
#ifndef UZPP_GEN_SINOV_HPP_
#define UZPP_GEN_SINOV_HPP_
#line 1 "C:\\Users\\MSN\\uz++\\stdlib\\sinov.uzpp"
#include <exception>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
namespace uzpp::Sinov {
    inline auto tasdiqlash(bool shart, const std::string& xabar = "")->void
    {
        if(! shart)
            {
                std::string xato = "Tasdiqlash barbod";
                if(! xabar.empty())
                    {
                        (xato =((xato + ": ") + xabar));
                    }
                throw std::runtime_error(xato);
            }
    }
    template <typename T >
 inline auto tasdiqlash_teng(const T& kutilgan, const T& qiymat, const std::string& xabar = "")->void
    {
        if(!(kutilgan == qiymat))
            {
                throw std::runtime_error(("Tenglik barbod: kutilgan != qiymat" +(xabar.empty() ? std::string(""):(std::string(" — ") + xabar))));
            }
    }
    inline auto tasdiqlash_yaqin(double kutilgan, double qiymat, double epsilon = 0.0001, const std::string& xabar = "")->void
    {
        double farq =(kutilgan - qiymat);
        if((farq < 0))
            {
                (farq = - farq);
            }
        if((farq > epsilon))
            {
                throw std::runtime_error(("Yaqinlik barbod: farq > epsilon" +(xabar.empty() ? std::string(""):(std::string(" — ") + xabar))));
            }
    }
    class TestToʼplami {
    public:
        std::string nomi_;
        std::vector<std::function<void()>> testlar_;
        std::vector<std::string> nomlar_;
        TestToʼplami(const std::string& nom) : nomi_ ( nom )
        {
        }
        void test_qoshish(const std::string& nom, std::function<void()> test)
        {
            nomlar_.push_back(nom);
            testlar_.push_back(test);
        }
        int ishga_tushirish()
        {
            (std::cout << std::endl);
            ((std::cout << "=============================================") << std::endl);
            (((std::cout << "  ") << nomi_) << std::endl);
            ((std::cout << "=============================================") << std::endl);
            int otdi = 0;
            int quladi = 0;
            std::vector<std::string> qulagan_nomlar;
            for(std::size_t i = 0;(i < testlar_.size());(i =(i + 1)))
                {
                    (((((((std::cout << "  [") <<(i + 1)) << "/") << testlar_.size()) << "] ") << nomlar_[i]) << " ... ");
                    try
                        {
                            testlar_[i]();
                            ((std::cout << "OTDI") << std::endl);
                            (otdi =(otdi + 1));
                        }
                    catch(std :: exception & e)
                        {
                            ((std::cout << "QULADI") << std::endl);
                            (((std::cout << "        Sabab: ") << e.what()) << std::endl);
                            (quladi =(quladi + 1));
                            qulagan_nomlar.push_back(nomlar_[i]);
                        }
                    catch(...)
                        {
                            ((std::cout << "QULADI") << std::endl);
                            ((std::cout << "        Sabab: noma'lum xato") << std::endl);
                            (quladi =(quladi + 1));
                            qulagan_nomlar.push_back(nomlar_[i]);
                        }
                }
            ((std::cout << "---------------------------------------------") << std::endl);
            (((std::cout << "  Jami:  ") <<(otdi + quladi)) << std::endl);
            (((std::cout << "  O'tdi: ") << otdi) << std::endl);
            (((std::cout << "  Quladi:") << quladi) << std::endl);
            if((quladi > 0))
                {
                    (std::cout << std::endl);
                    ((std::cout << "  Qulagan testlar:") << std::endl);
                    for(std::size_t i = 0;(i < qulagan_nomlar.size());(i =(i + 1)))
                        {
                            (((std::cout << "    - ") << qulagan_nomlar[i]) << std::endl);
                        }
                }
            ((std::cout << "=============================================") << std::endl);
            return quladi;
        }
    };
    inline auto test_bajar(const std::string& nom, std::function<void()> test)->bool
    {
        (((std::cout << "  ") << nom) << " ... ");
        try
            {
                test();
                ((std::cout << "OTDI") << std::endl);
                return true;
            }
        catch(std :: exception & e)
            {
                (((std::cout << "QULADI: ") << e.what()) << std::endl);
                return false;
            }
        catch(...)
            {
                ((std::cout << "QULADI: noma'lum xato") << std::endl);
                return false;
            }
    }
}
#endif
