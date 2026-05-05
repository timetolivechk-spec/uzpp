#pragma once

#include "platforma.hpp"
#include "tarmoq.hpp"
#include "veb_ui.hpp"
#include "jurnal.hpp"

#include <string>
#include <memory>

namespace uzpp::Koinot {

// Koinot - Express.js va FastAPI darajasidagi yuqori veb-freymvork
class VebIlova {
    Tarmoq::Server server_;
    Tarmoq::Router router_;
    int port_;

public:
    explicit VebIlova(int port = 8080) : server_(port), port_(port) {}

    void get(const std::string& yol, std::function<void(Tarmoq::Sorov, Tarmoq::Javob)> handler) {
        router_.get(yol, std::move(handler));
    }

    void post(const std::string& yol, std::function<void(Tarmoq::Sorov, Tarmoq::Javob)> handler) {
        router_.post(yol, std::move(handler));
    }

    // Ilovani asinxron ishga tushirish
    Asinxron::Vazifa<void> ishgaTushirish() {
        router_.ulash(&server_);
        Jurnal::Logger::malumot("Koinot Veb-freymvorki ishga tushdi: http://localhost:" + std::to_string(port_));
        co_return co_await server_.ishlash();
    }
};

} // namespace uzpp::Koinot