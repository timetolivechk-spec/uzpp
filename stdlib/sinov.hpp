#pragma once
#ifndef UZPP_GEN_SINOV_HPP_
#define UZPP_GEN_SINOV_HPP_
#line 1 "stdlib\\sinov.uzpp"
#include <exception>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <chrono>
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
    class BenchToʼplami {
    public:
        std::vector<std::function<void()>> olchovlar_;
        std::vector<std::string> nomlar_;
        int takror_ = 1000;
        BenchToʼplami()
        {
        }
        void takror_belgilash(int n)
        {
            if((n > 0))
                {
                    (takror_ = n);
                }
        }
        void bench_qoshish(const std::string& nom, std::function<void()> olchov)
        {
            nomlar_.push_back(nom);
            olchovlar_.push_back(olchov);
        }
        int ishga_tushirish()
        {
            (std::cout << std::endl);
            ((std::cout << "=============================================") << std::endl);
            ((((std::cout << "  Benchmark (") << takror_) << " marta takrorlash)") << std::endl);
            ((std::cout << "=============================================") << std::endl);
            int xatolar = 0;
            for(std::size_t i = 0;(i < olchovlar_.size());(i =(i + 1)))
                {
                    (((((((std::cout << "  [") <<(i + 1)) << "/") << olchovlar_.size()) << "] ") << nomlar_[i]) << " ... ");
                    try
                        {
                            olchovlar_[i]();
                            auto boshlanish = std::chrono::steady_clock::now();
                            for(int k = 0;(k < takror_);(k =(k + 1)))
                                {
                                    olchovlar_[i]();
                                }
                            auto tugash = std::chrono::steady_clock::now();
                            auto jami_ns = std::chrono::duration_cast<std::chrono::nanoseconds>((tugash - boshlanish)).count();
                            double ortacha_ns =(static_cast < double >(jami_ns) / static_cast < double >(takror_));
                            ((((((std::cout << ortacha_ns) << " ns/amal") << "  (jami ") <<(static_cast < double >(jami_ns) / 1000000.0)) << " ms)") << std::endl);
                        }
                    catch(std :: exception & e)
                        {
                            (((std::cout << "XATO: ") << e.what()) << std::endl);
                            (xatolar =(xatolar + 1));
                        }
                    catch(...)
                        {
                            ((std::cout << "XATO: noma'lum") << std::endl);
                            (xatolar =(xatolar + 1));
                        }
                }
            ((std::cout << "=============================================") << std::endl);
            return xatolar;
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
