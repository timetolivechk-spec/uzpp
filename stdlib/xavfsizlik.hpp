#pragma once

#include <expected>
#include <optional>
#include <string>
#include <utility>

namespace uzpp::Xavfsizlik {

template <typename T>
using Tanlov = std::optional<T>;

template <typename T, typename E = std::string>
using Natija = std::expected<T, E>;

template <typename T, typename E = std::string>
[[nodiscard]] inline Natija<T, E> muvaffaqiyat(T value) {
    return Natija<T, E>(std::move(value));
}

template <typename E = std::string>
[[nodiscard]] inline std::expected<void, E> muvaffaqiyatVoid() {
    return std::expected<void, E>();
}

// xato() returns std::unexpected<E> which implicitly converts to any std::expected<T, E>
template <typename E = std::string>
[[nodiscard]] inline std::unexpected<E> xato(E error) {
    return std::unexpected<E>(std::move(error));
}

template <typename E = std::string>
[[nodiscard]] inline std::expected<void, E> xatoVoid(E error) {
    return std::unexpected<E>(std::move(error));
}

namespace detail {
    template <typename T>
    struct is_expected : std::false_type {};
    template <typename T, typename E>
    struct is_expected<std::expected<T, E>> : std::true_type {};
    
    template <typename T>
    auto get_error(const T& val) {
        if constexpr (is_expected<std::decay_t<T>>::value) {
            return std::unexpected(val.error());
        } else {
            return std::nullopt;
        }
    }
} // namespace detail

} // namespace uzpp::Xavfsizlik

#define UZPP_TRY(expr) \
    ({ \
        auto _uzpp_val = (expr); \
        if (!_uzpp_val.has_value()) return ::uzpp::Xavfsizlik::detail::get_error(_uzpp_val); \
        std::move(_uzpp_val.value()); \
    })

