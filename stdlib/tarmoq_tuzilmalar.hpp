#pragma once
// status: REAL — Tarmoq so'rov/javob tuzilmalari.

#include <string>
#include <unordered_map>
#include <memory>
#include "fayl_tizimi.hpp"
#include "veb_ui.hpp"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

namespace uzpp::Tarmoq {

    struct Sorov {
        std::string metod;
        std::string yol;
        std::string tana;
        std::unordered_map<std::string, std::string> sarlavhalar;
    };

    struct Javob {
#ifdef _WIN32
        SOCKET klientSoketi = INVALID_SOCKET;
#else
        int klientSoketi = -1;
#endif

        void jsonQaytarish(int statusKodi, const uzpp::FaylTizimi::JsonObyekt& json);
        void matnQaytarish(int statusKodi, const std::string& matn);
        void htmlQaytarish(int statusKodi, const std::string& html);
        void uiQaytarish(int statusKodi, const uzpp::Xotira::Ulashilgan<uzpp::VebUI::Element>& ui);
        void webSocketQabulQilish(const Sorov& sorov);
    };

} // namespace uzpp::Tarmoq
