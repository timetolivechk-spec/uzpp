#pragma once
#ifndef UZPP_GEN_ERROR_REMAP_HPP_
#define UZPP_GEN_ERROR_REMAP_HPP_
#line 1 "stdlib\\error_remap.uzpp"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>
namespace uzpp::ErrorRemap {
    struct QatorXarita {
    public:
        int cppQatori;
        std::string uzppYol;
        int uzppQatori;
    };
    inline auto remapPozitsiyalar(std::string chiqish, const std::string& cppMazmuni, const std::string& generatsiyaQilinganCppNomi)->std::string
    {
        std::vector<QatorXarita> xaritalar;
        {
            std::stringstream ss = std::stringstream(cppMazmuni);
            std::string qator;
            int cppQatorNo = 0;
            while(std::getline(ss, qator))
                {
                    ++ cppQatorNo;
                    auto firstNonWs = qator.find_first_not_of(" \t");
                    if((firstNonWs == std::string::npos))
                        continue;
                    if((qator.compare(firstNonWs, 5, "#line") != 0))
                        continue;
                    std::size_t i =(firstNonWs + 5);
                    while(((i < qator.size()) && std::isspace(static_cast<int>(qator[i]))))
                        ++ i;
                    int raqam = 0;
                    bool raqamBor = false;
                    while(((i < qator.size()) && std::isdigit(static_cast<int>(qator[i]))))
                        {
                            (raqam =((raqam * 10) +(qator[i] - '0')));
                            ++ i;
                            (raqamBor = true);
                        }
                    if(! raqamBor)
                        continue;
                    while(((i < qator.size()) && std::isspace(static_cast<int>(qator[i]))))
                        ++ i;
                    if(((i >= qator.size()) ||(qator[i] != '"')))
                        continue;
                    ++ i;
                    auto end = qator.find('"', i);
                    if((end == std::string::npos))
                        continue;
                    std::string raw = qator.substr(i,(end - i));
                    std::string yol;
                    yol.reserve(raw.size());
                    for(std::size_t k = 0;(k < raw.size()); ++ k)
                        {
                            if((((raw[k] == '\\') &&((k + 1) < raw.size())) &&(raw[(k + 1)] == '\\')))
                                {
                                    (yol += '\\');
                                    ++ k;
                                }
                            else
                                {
                                    (yol += raw[k]);
                                }
                        }
                    xaritalar.push_back({(cppQatorNo + 1), yol, raqam});
                }
        }
        if(xaritalar.empty())
            return chiqish;
        std::string natija;
        natija.reserve(chiqish.size());
        std::size_t qatorBoshlanishi = 0;
        while((qatorBoshlanishi <= chiqish.size()))
            {
                auto nl = chiqish.find('\n', qatorBoshlanishi);
                std::string chiqishQatori = chiqish.substr(qatorBoshlanishi,((nl == std::string::npos) ? std::string::npos:(nl - qatorBoshlanishi)));
                std::string ozgartirilgan;
                std::size_t skan = 0;
                while((skan < chiqishQatori.size()))
                    {
                        auto cppPoz = chiqishQatori.find(".cpp", skan);
                        if((cppPoz == std::string::npos))
                            {
                                ozgartirilgan.append(chiqishQatori, skan, std::string::npos);
                                break;
                            }
                        std::size_t nomBoshlanishi = cppPoz;
                        while((nomBoshlanishi > 0))
                            {
                                char c = chiqishQatori[(nomBoshlanishi - 1)];
                                if(((((((((c == ' ') ||(c == '\t')) ||(c == ':')) ||(c == '(')) ||(c == '\'')) ||(c == '"')) ||(c == ',')) ||(c == '<')))
                                    break;
                                -- nomBoshlanishi;
                            }
                        std::string faylNomi = chiqishQatori.substr(nomBoshlanishi,((cppPoz + 4) - nomBoshlanishi));
                        std::filesystem::path faylNomiYol = std::filesystem::path(faylNomi);
                        if((faylNomiYol.filename().string() != generatsiyaQilinganCppNomi))
                            {
                                ozgartirilgan.append(chiqishQatori, skan,((cppPoz + 4) - skan));
                                (skan =(cppPoz + 4));
                                continue;
                            }
                        std::size_t keyin =(cppPoz + 4);
                        if(((keyin >= chiqishQatori.size()) ||(chiqishQatori[keyin] != ':')))
                            {
                                ozgartirilgan.append(chiqishQatori, skan,(keyin - skan));
                                (skan = keyin);
                                continue;
                            }
                        ++ keyin;
                        int cppXatoQatori = 0;
                        bool qatorTopildi = false;
                        while(((keyin < chiqishQatori.size()) && std::isdigit(static_cast<int>(chiqishQatori[keyin]))))
                            {
                                (cppXatoQatori =((cppXatoQatori * 10) +(chiqishQatori[keyin] - '0')));
                                ++ keyin;
                                (qatorTopildi = true);
                            }
                        if(! qatorTopildi)
                            {
                                ozgartirilgan.append(chiqishQatori, skan,(keyin - skan));
                                (skan = keyin);
                                continue;
                            }
                        int bestIdx = - 1;
                        for(auto j = 0;(j < static_cast<int>(xaritalar.size())); ++ j)
                            {
                                if((xaritalar[j].cppQatori <= cppXatoQatori))
                                    {
                                        if(((bestIdx == - 1) ||(xaritalar[j].cppQatori > xaritalar[bestIdx].cppQatori)))
                                            (bestIdx = j);
                                    }
                                else
                                    {
                                        break;
                                    }
                            }
                        if((bestIdx == - 1))
                            {
                                ozgartirilgan.append(chiqishQatori, skan,(keyin - skan));
                                (skan = keyin);
                                continue;
                            }
                        int uzppXatoQatori =(xaritalar[bestIdx].uzppQatori +(cppXatoQatori - xaritalar[bestIdx].cppQatori));
                        ozgartirilgan.append(chiqishQatori, skan,(nomBoshlanishi - skan));
                        (ozgartirilgan += xaritalar[bestIdx].uzppYol);
                        (ozgartirilgan += ':');
                        (ozgartirilgan += std::to_string(uzppXatoQatori));
                        (skan = keyin);
                    }
                (natija += ozgartirilgan);
                if((nl == std::string::npos))
                    break;
                (natija += '\n');
                (qatorBoshlanishi =(nl + 1));
            }
        return natija;
    }
}
#endif
