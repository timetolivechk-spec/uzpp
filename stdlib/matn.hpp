#pragma once
#ifndef UZPP_GEN_MATN_HPP_
#define UZPP_GEN_MATN_HPP_
#line 1 "C:\\Users\\MSN\\uz++\\.claude\\worktrees\\musing-satoshi-e40317\\stdlib\\matn.uzpp"
#include <regex>
#include <cctype>
#include <algorithm>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
namespace uzpp::Matn {
    inline auto uzunlik(const std::string& s)->std::size_t
    {
        return s.size();
    }
    inline auto boshmi(const std::string& s)->bool
    {
        return s.empty();
    }
    inline auto bosh_emasmi(const std::string& s)->bool
    {
        return ! s.empty();
    }
    inline auto boshlanganda(const std::string& s, const std::string& prefiks)->bool
    {
        return s.starts_with(prefiks);
    }
    inline auto tugaganda(const std::string& s, const std::string& sufiks)->bool
    {
        return s.ends_with(sufiks);
    }
    inline auto ichida(const std::string& s, const std::string& qidiriluvchi)->bool
    {
        return(s.find(qidiriluvchi) != std::string::npos);
    }
    inline auto topish(const std::string& s, const std::string& qidiriluvchi, std::size_t boshlanish = 0)->std::optional<std::size_t>
    {
        auto p = s.find(qidiriluvchi, boshlanish);
        if((p == std::string::npos))
            {
                return std::nullopt;
            }
        return p;
    }
    inline auto hisoblash(const std::string& s, const std::string& naqsh)->int
    {
        if(naqsh.empty())
            {
                return 0;
            }
        int adad = 0;
        std::size_t pos = 0;
        while(((pos = s.find(naqsh, pos)) != std::string::npos))
            {
                (adad =(adad + 1));
                (pos =(pos + naqsh.size()));
            }
        return adad;
    }
    inline auto qism_matn(const std::string& s, std::size_t boshlanish, std::size_t uzunlik = std::string::npos)->std::string
    {
        if((boshlanish >= s.size()))
            {
                return "";
            }
        return s.substr(boshlanish, uzunlik);
    }
    inline auto chapdan(const std::string& s, std::size_t n)->std::string
    {
        if((n >= s.size()))
            {
                return s;
            }
        return s.substr(0, n);
    }
    inline auto ongdan(const std::string& s, std::size_t n)->std::string
    {
        if((n >= s.size()))
            {
                return s;
            }
        return s.substr((s.size() - n));
    }
    inline auto kichik_harfga(std::string s)->std::string
    {
        for(std::size_t i = 0;(i < s.size());(i =(i + 1)))
            {
                (s[i] = static_cast < char >(std::tolower(static_cast < unsigned char >(s[i]))));
            }
        return s;
    }
    inline auto katta_harfga(std::string s)->std::string
    {
        for(std::size_t i = 0;(i < s.size());(i =(i + 1)))
            {
                (s[i] = static_cast < char >(std::toupper(static_cast < unsigned char >(s[i]))));
            }
        return s;
    }
    inline auto birinchi_harfni_katta(std::string s)->std::string
    {
        if(! s.empty())
            {
                (s[0] = static_cast < char >(std::toupper(static_cast < unsigned char >(s[0]))));
            }
        return s;
    }
    inline auto teskari(std::string s)->std::string
    {
        std::reverse(s.begin(), s.end());
        return s;
    }
    inline auto takrorlash(const std::string& s, int marotaba)->std::string
    {
        if((marotaba <= 0))
            {
                return "";
            }
        std::string natija;
        natija.reserve((s.size() * static_cast < std::size_t >(marotaba)));
        for(int i = 0;(i < marotaba);(i =(i + 1)))
            {
                (natija =(natija + s));
            }
        return natija;
    }
    inline auto almashtirish(std::string s, const std::string& eski, const std::string& yangisi)->std::string
    {
        if(eski.empty())
            {
                return s;
            }
        std::size_t pos = 0;
        while(((pos = s.find(eski, pos)) != std::string::npos))
            {
                s.replace(pos, eski.size(), yangisi);
                (pos =(pos + yangisi.size()));
            }
        return s;
    }
    inline auto birinchi_almashtirish(const std::string& s, const std::string& eski, const std::string& yangisi)->std::string
    {
        std::string natija = s;
        auto p = natija.find(eski);
        if((p != std::string::npos))
            {
                natija.replace(p, eski.size(), yangisi);
            }
        return natija;
    }
    inline auto chapdan_qirqish(const std::string& s)->std::string
    {
        std::size_t i = 0;
        while(((i < s.size()) && std::isspace(static_cast < unsigned char >(s[i]))))
            {
                (i =(i + 1));
            }
        return s.substr(i);
    }
    inline auto ongdan_qirqish(const std::string& s)->std::string
    {
        if(s.empty())
            {
                return s;
            }
        std::size_t i = s.size();
        while(((i > 0) && std::isspace(static_cast < unsigned char >(s[(i - 1)]))))
            {
                (i =(i - 1));
            }
        return s.substr(0, i);
    }
    inline auto qirqish(const std::string& s)->std::string
    {
        return chapdan_qirqish(ongdan_qirqish(s));
    }
    inline auto raqammi(const std::string& s)->bool
    {
        if(s.empty())
            {
                return false;
            }
        std::size_t i = 0;
        if(((s[i] == '-') ||(s[i] == '+')))
            {
                (i =(i + 1));
            }
        bool raqam_bor = false;
        while(((i < s.size()) && std::isdigit(static_cast < unsigned char >(s[i]))))
            {
                (i =(i + 1));
                (raqam_bor = true);
            }
        if(((i < s.size()) &&(s[i] == '.')))
            {
                (i =(i + 1));
                while(((i < s.size()) && std::isdigit(static_cast < unsigned char >(s[i]))))
                    {
                        (i =(i + 1));
                        (raqam_bor = true);
                    }
            }
        return(raqam_bor &&(i == s.size()));
    }
    inline auto harflarmi(const std::string& s)->bool
    {
        if(s.empty())
            {
                return false;
            }
        for(std::size_t i = 0;(i < s.size());(i =(i + 1)))
            {
                if(! std::isalpha(static_cast < unsigned char >(s[i])))
                    {
                        return false;
                    }
            }
        return true;
    }
    inline auto harf_raqamlami(const std::string& s)->bool
    {
        if(s.empty())
            {
                return false;
            }
        for(std::size_t i = 0;(i < s.size());(i =(i + 1)))
            {
                if(! std::isalnum(static_cast < unsigned char >(s[i])))
                    {
                        return false;
                    }
            }
        return true;
    }
    inline auto ajratish(const std::string& s, const std::string& ajratuvchi)->std::vector<std::string>
    {
        std::vector<std::string> natija;
        if(ajratuvchi.empty())
            {
                for(char c: s)
                    {
                        natija.push_back(std::string(1, c));
                    }
                return natija;
            }
        std::size_t boshi = 0;
        std::size_t oxir = s.find(ajratuvchi);
        while((oxir != std::string::npos))
            {
                natija.push_back(s.substr(boshi,(oxir - boshi)));
                (boshi =(oxir + ajratuvchi.size()));
                (oxir = s.find(ajratuvchi, boshi));
            }
        natija.push_back(s.substr(boshi));
        return natija;
    }
    inline auto qatorlarga_ajratish(const std::string& s)->std::vector<std::string>
    {
        std::vector<std::string> natija;
        std::string joriy;
        for(char c: s)
            {
                if((c == '\n'))
                    {
                        if((! joriy.empty() &&(joriy.back() == '\r')))
                            {
                                joriy.pop_back();
                            }
                        natija.push_back(joriy);
                        joriy.clear();
                    }
                else
                    {
                        (joriy =(joriy + std::string(1, c)));
                    }
            }
        if((! joriy.empty() &&(joriy.back() == '\r')))
            {
                joriy.pop_back();
            }
        if(! joriy.empty())
            {
                natija.push_back(joriy);
            }
        return natija;
    }
    inline auto birlashtirish(const std::vector<std::string>& massiv, const std::string& biriktiruvchi)->std::string
    {
        if(massiv.empty())
            {
                return "";
            }
        std::string natija = massiv[0];
        for(std::size_t i = 1;(i < massiv.size());(i =(i + 1)))
            {
                (natija =((natija + biriktiruvchi) + massiv[i]));
            }
        return natija;
    }
    inline auto chapdan_toldirish(const std::string& s, std::size_t kenglik, char toldiruvchi = ' ')->std::string
    {
        if((s.size() >= kenglik))
            {
                return s;
            }
        return(std::string((kenglik - s.size()), toldiruvchi) + s);
    }
    inline auto ongdan_toldirish(const std::string& s, std::size_t kenglik, char toldiruvchi = ' ')->std::string
    {
        if((s.size() >= kenglik))
            {
                return s;
            }
        return(s + std::string((kenglik - s.size()), toldiruvchi));
    }
    inline auto markazga_toldirish(const std::string& s, std::size_t kenglik, char toldiruvchi = ' ')->std::string
    {
        if((s.size() >= kenglik))
            {
                return s;
            }
        std::size_t chap =((kenglik - s.size()) / 2);
        std::size_t ong =((kenglik - s.size()) - chap);
        return((std::string(chap, toldiruvchi) + s) + std::string(ong, toldiruvchi));
    }
    inline auto butun_songa(const std::string& s)->int
    {
        try
            {
                return std::stoi(s);
            }
        catch(...)
            {
                throw std::invalid_argument(("Matn butun songa aylantirib bo'lmadi: " + s));
            }
    }
    inline auto kasr_songa(const std::string& s)->std::optional<double>
    {
        try
            {
                return std::stod(s);
            }
        catch(...)
            {
                return std::nullopt;
            }
    }
    inline auto songa_aylantirish(int n)->std::string
    {
        return std::to_string(n);
    }
    inline auto songa_aylantirish(double d)->std::string
    {
        return std::to_string(d);
    }
    inline auto songa_aylantirish(long long n)->std::string
    {
        return std::to_string(n);
    }
    inline auto qadar_qirqish(const std::string& s, std::size_t maks_uzunlik, const std::string& ko_pnuqta = "...")->std::string
    {
        if((s.size() <= maks_uzunlik))
            {
                return s;
            }
        if((ko_pnuqta.size() >= maks_uzunlik))
            {
                return ko_pnuqta.substr(0, maks_uzunlik);
            }
        return(s.substr(0,(maks_uzunlik - ko_pnuqta.size())) + ko_pnuqta);
    }
    inline auto qator_son(const std::string& s)->int
    {
        if(s.empty())
            {
                return 0;
            }
        int adad = 1;
        for(char c: s)
            {
                if((c == '\n'))
                    {
                        (adad =(adad + 1));
                    }
            }
        if((s.back() == '\n'))
            {
                (adad =(adad - 1));
            }
        return adad;
    }
    inline auto belgi_almashtirish(std::string s, char eski_belgi, char yangi_belgi)->std::string
    {
        for(std::size_t i = 0;(i < s.size());(i =(i + 1)))
            {
                if((s[i] == eski_belgi))
                    {
                        (s[i] = yangi_belgi);
                    }
            }
        return s;
    }
    inline auto bosh_belgi(const std::string& s)->char
    {
        if(s.empty())
            {
                throw std::out_of_range("bosh_belgi: matn bo'sh");
            }
        return s.front();
    }
    inline auto oxir_belgi(const std::string& s)->char
    {
        if(s.empty())
            {
                throw std::out_of_range("oxir_belgi: matn bo'sh");
            }
        return s.back();
    }
    inline auto katta_yokib(const std::string& s)->bool
    {
        bool harf_bor = false;
        for(std::size_t i = 0;(i < s.size());(i =(i + 1)))
            {
                auto c = static_cast < unsigned char >(s[i]);
                if(std::isalpha(c))
                    {
                        (harf_bor = true);
                        if(! std::isupper(c))
                            {
                                return false;
                            }
                    }
            }
        return harf_bor;
    }
    inline auto kichik_yokib(const std::string& s)->bool
    {
        bool harf_bor = false;
        for(std::size_t i = 0;(i < s.size());(i =(i + 1)))
            {
                auto c = static_cast < unsigned char >(s[i]);
                if(std::isalpha(c))
                    {
                        (harf_bor = true);
                        if(! std::islower(c))
                            {
                                return false;
                            }
                    }
            }
        return harf_bor;
    }
    inline auto formatlash_indeksli(const std::string& naqsh, const std::vector<std::string>& argumentlar)->std::string
    {
        std::string natija;
        natija.reserve(naqsh.size());
        std::size_t i = 0;
        while((i < naqsh.size()))
            {
                char c = naqsh[i];
                if((c == '{'))
                    {
                        if((((i + 1) < naqsh.size()) &&(naqsh[(i + 1)] == '{')))
                            {
                                (natija =(natija + std::string(1, '{')));
                                (i =(i + 2));
                                continue;
                            }
                        std::size_t j =(i + 1);
                        int indeks = 0;
                        bool indeks_bor = false;
                        while(((j < naqsh.size()) && std::isdigit(static_cast < unsigned char >(naqsh[j]))))
                            {
                                (indeks =((indeks * 10) +(naqsh[j] - '0')));
                                (j =(j + 1));
                                (indeks_bor = true);
                            }
                        if(((! indeks_bor ||(j >= naqsh.size())) ||(naqsh[j] != '}')))
                            {
                                throw std::invalid_argument(("formatlash_indeksli: noto'g'ri joy egasi pozitsiya " + std::to_string(i)));
                            }
                        if((static_cast < std::size_t >(indeks) >= argumentlar.size()))
                            {
                                throw std::out_of_range((("formatlash_indeksli: argument indeksi {" + std::to_string(indeks)) + "} chegaradan tashqari"));
                            }
                        (natija =(natija + argumentlar[static_cast < std::size_t >(indeks)]));
                        (i =(j + 1));
                        continue;
                    }
                if((c == '}'))
                    {
                        if((((i + 1) < naqsh.size()) &&(naqsh[(i + 1)] == '}')))
                            {
                                (natija =(natija + std::string(1, '}')));
                                (i =(i + 2));
                                continue;
                            }
                        throw std::invalid_argument(("formatlash_indeksli: yopish qavs '}' bo'sh joy egasi pozitsiya " + std::to_string(i)));
                    }
                (natija =(natija + std::string(1, c)));
                (i =(i + 1));
            }
        return natija;
    }
    inline auto oxir_topish(const std::string& s, const std::string& qidiriluvchi)->std::optional<std::size_t>
    {
        auto p = s.rfind(qidiriluvchi);
        if((p == std::string::npos))
            {
                return std::nullopt;
            }
        return p;
    }
    inline auto bir_marta_ajratish(const std::string& s, const std::string& bo_luvchi)->std::pair<std::string, std::string>
    {
        if(bo_luvchi.empty())
            {
                return std::make_pair(s, std::string(""));
            }
        auto p = s.find(bo_luvchi);
        if((p == std::string::npos))
            {
                return std::make_pair(s, std::string(""));
            }
        return std::make_pair(s.substr(0, p), s.substr((p + bo_luvchi.size())));
    }
    inline auto belgilar_soni(const std::string& s)->std::size_t
    {
        std::size_t adad = 0;
        for(char c: s)
            {
                auto b = static_cast < unsigned char >(c);
                if(((b & 0xC0) != 0x80))
                    {
                        (adad =(adad + 1));
                    }
            }
        return adad;
    }
    inline auto belgi_pozitsiyasi(const std::string& s, std::size_t belgi_indeks)->std::size_t
    {
        std::size_t joriy_belgi = 0;
        for(std::size_t i = 0;(i < s.size());(i =(i + 1)))
            {
                auto b = static_cast < unsigned char >(s[i]);
                if(((b & 0xC0) != 0x80))
                    {
                        if((joriy_belgi == belgi_indeks))
                            {
                                return i;
                            }
                        (joriy_belgi =(joriy_belgi + 1));
                    }
            }
        return s.size();
    }
    inline auto chapdan_belgi_bo_yicha(const std::string& s, std::size_t n)->std::string
    {
        std::size_t oxir = belgi_pozitsiyasi(s, n);
        return s.substr(0, oxir);
    }
    inline auto ongdan_belgi_bo_yicha(const std::string& s, std::size_t n)->std::string
    {
        std::size_t jami = belgilar_soni(s);
        if((n >= jami))
            {
                return s;
            }
        std::size_t boshi = belgi_pozitsiyasi(s,(jami - n));
        return s.substr(boshi);
    }
    inline auto qism_matn_belgi_bo_yicha(const std::string& s, std::size_t boshlanish, std::size_t uzunlik = std::string::npos)->std::string
    {
        std::size_t bayt_boshi = belgi_pozitsiyasi(s, boshlanish);
        if((bayt_boshi >= s.size()))
            {
                return "";
            }
        if((uzunlik == std::string::npos))
            {
                return s.substr(bayt_boshi);
            }
        std::size_t bayt_oxiri = belgi_pozitsiyasi(s,(boshlanish + uzunlik));
        return s.substr(bayt_boshi,(bayt_oxiri - bayt_boshi));
    }
    inline auto belgi_uzunligi(char boshlovchi_bayt)->std::size_t
    {
        auto b = static_cast < unsigned char >(boshlovchi_bayt);
        if(((b & 0x80) == 0x00))
            return 1;
        if(((b & 0xE0) == 0xC0))
            return 2;
        if(((b & 0xF0) == 0xE0))
            return 3;
        if(((b & 0xF8) == 0xF0))
            return 4;
        return 1;
    }
    inline auto belgi_olish(const std::string& s, std::size_t belgi_indeks)->std::string
    {
        std::size_t p = belgi_pozitsiyasi(s, belgi_indeks);
        if((p >= s.size()))
            {
                return "";
            }
        std::size_t bayt_uzunligi = belgi_uzunligi(s[p]);
        if(((p + bayt_uzunligi) > s.size()))
            {
                (bayt_uzunligi =(s.size() - p));
            }
        return s.substr(p, bayt_uzunligi);
    }
    class RegEx {
    private:
        std::regex regex_;
    public:
        RegEx(const std::string& naqsh, bool katta_kichik_farqsiz = false) : regex_ ( naqsh , katta_kichik_farqsiz ? std :: regex :: icase : std :: regex :: ECMAScript )
        {
        }
        bool mos(const std::string& s) const
        {
            return std::regex_match(s, regex_);
        }
        bool qidirish(const std::string& s) const
        {
            return std::regex_search(s, regex_);
        }
        std::string almashtirish(const std::string& s, const std::string& yangisi) const
        {
            return std::regex_replace(s, regex_, yangisi);
        }
        std::vector<std::string> topish_hammasi(const std::string& s) const
        {
            std::vector<std::string> natija;
            auto it = std::sregex_iterator(s.begin(), s.end(), regex_);
            auto oxir = std::sregex_iterator();
            while((it != oxir))
                {
                    natija.push_back((* it)[0].str());
                    ++ it;
                }
            return natija;
        }
    };
}
#endif
