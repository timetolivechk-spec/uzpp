#pragma once

#include "platforma.hpp"

#if !defined(UZPP_EMBEDDED)
#include <functional>
#include <vector>
#include <memory>
#endif
#if !defined(UZPP_EMBEDDED)
#if __cplusplus >= 202002L
#include <coroutine>
#include <exception>

namespace uzpp::Asinxron {

    template<typename T>
    struct Vazifa {
        struct promise_type {
            T value;
            Vazifa get_return_object() { return Vazifa{std::coroutine_handle<promise_type>::from_promise(*this)}; }
            std::suspend_never initial_suspend() { return {}; }
            std::suspend_always final_suspend() noexcept { return {}; }
            void return_value(T v) { value = v; }
            void unhandled_exception() { std::terminate(); }
        };
        std::coroutine_handle<promise_type> h_;
        explicit Vazifa(std::coroutine_handle<promise_type> h) : h_(h) {}
        ~Vazifa() { if (h_) h_.destroy(); }
        bool await_ready() { return h_.done(); }
        void await_suspend(std::coroutine_handle<>) {}
        T await_resume() { return h_.promise().value; }
    };
    
    template<> struct Vazifa<void> {
        struct promise_type {
            Vazifa get_return_object() { return Vazifa{std::coroutine_handle<promise_type>::from_promise(*this)}; }
            std::suspend_never initial_suspend() { return {}; }
            std::suspend_always final_suspend() noexcept { return {}; }
            void return_void() {}
            void unhandled_exception() { std::terminate(); }
        };
        std::coroutine_handle<promise_type> h_;
        explicit Vazifa(std::coroutine_handle<promise_type> h) : h_(h) {}
        ~Vazifa() { if (h_) h_.destroy(); }
        bool await_ready() { return h_.done(); }
        void await_suspend(std::coroutine_handle<>) {}
        void await_resume() {}
    };

    // Hodisalar Halqasi uchun eski legacy typedef
    using LegacyVazifa = std::function<void()>;
#else
namespace uzpp::Asinxron {
    using LegacyVazifa = std::function<void()>;
#endif

class HodisalarHalqasi {
    std::vector<LegacyVazifa> kutilayotgan_vazifalar_;
public:
    void vazifaQoshish(LegacyVazifa v) {
        kutilayotgan_vazifalar_.push_back(std::move(v));
    }
    
    void birMartaAylantirish() {
        if (kutilayotgan_vazifalar_.empty()) return;
        
        // Ularni olib, oynani tozalaymiz (yangi vazifalar qo'shilishi mumkin)
        std::vector<LegacyVazifa> amaldagi = std::move(kutilayotgan_vazifalar_);
        kutilayotgan_vazifalar_.clear();
        
        for(auto& vazifa : amaldagi) {
            vazifa();
        }
    }
    
    [[nodiscard]] bool kutilmoqdami() const {
        return !kutilayotgan_vazifalar_.empty();
    }
};

#endif

} // namespace uzpp::Asinxron
