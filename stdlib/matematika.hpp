#pragma once

#include <cmath>
#include <random>
#include <algorithm>
#include <numeric>
#include <vector>
#include <stdexcept>
#include <limits>
#include <array>

namespace uzpp::Matematika {

// ===== KONSTANTALAR =====
constexpr double PI       = 3.14159265358979323846;
constexpr double E        = 2.71828182845904523536;
constexpr double PHI      = 1.61803398874989484820; // Oltin nisbat
constexpr double SQRT2    = 1.41421356237309504880;
constexpr double LN2      = 0.69314718055994530942;
constexpr double LN10     = 2.30258509299404568402;
constexpr double SONSIZLIK = std::numeric_limits<double>::infinity();
constexpr double NAN_SON   = std::numeric_limits<double>::quiet_NaN();

// ===== ASOSIY FUNKSIYALAR =====

template<typename T>
[[nodiscard]] inline T modul(T x) noexcept { return std::abs(x); }

template<typename T, typename U>
[[nodiscard]] inline auto daraja(T x, U y) noexcept { return std::pow(x, y); }

template<typename T>
[[nodiscard]] inline auto ildiz(T x) noexcept { return std::sqrt(x); }

template<typename T>
[[nodiscard]] inline auto kub_ildiz(T x) noexcept { return std::cbrt(x); }

template<typename T>
[[nodiscard]] inline T yuqori_yaxlitlash(T x) noexcept { return static_cast<T>(std::ceil(x)); }

template<typename T>
[[nodiscard]] inline T quyi_yaxlitlash(T x) noexcept { return static_cast<T>(std::floor(x)); }

template<typename T>
[[nodiscard]] inline T yaxlitlash(T x) noexcept { return static_cast<T>(std::round(x)); }

template<typename T>
[[nodiscard]] inline T kesish(T x) noexcept { return static_cast<T>(std::trunc(x)); }

template<typename T>
[[nodiscard]] inline int ishorat(T x) noexcept {
    if (x > T{0}) return 1;
    if (x < T{0}) return -1;
    return 0;
}

template<typename T>
[[nodiscard]] inline bool sonMi(T x) noexcept { return !std::isnan(x); }

template<typename T>
[[nodiscard]] inline bool cheksizMi(T x) noexcept { return std::isinf(x); }

// ===== LOGARIFM VA EKSPONENTA =====

template<typename T>
[[nodiscard]] inline double ln(T x) noexcept { return std::log(static_cast<double>(x)); }

template<typename T>
[[nodiscard]] inline double log10(T x) noexcept { return std::log10(static_cast<double>(x)); }

template<typename T>
[[nodiscard]] inline double log2(T x) noexcept { return std::log2(static_cast<double>(x)); }

template<typename T, typename B>
[[nodiscard]] inline double log_asosi(T x, B base) noexcept {
    return std::log(static_cast<double>(x)) / std::log(static_cast<double>(base));
}

template<typename T>
[[nodiscard]] inline double exp(T x) noexcept { return std::exp(static_cast<double>(x)); }

// ===== TRIGONOMETRIYA =====

template<typename T>
[[nodiscard]] inline double sin(T x) noexcept { return std::sin(static_cast<double>(x)); }

template<typename T>
[[nodiscard]] inline double cos(T x) noexcept { return std::cos(static_cast<double>(x)); }

template<typename T>
[[nodiscard]] inline double tan(T x) noexcept { return std::tan(static_cast<double>(x)); }

template<typename T>
[[nodiscard]] inline double asin(T x) noexcept { return std::asin(static_cast<double>(x)); }

template<typename T>
[[nodiscard]] inline double acos(T x) noexcept { return std::acos(static_cast<double>(x)); }

template<typename T>
[[nodiscard]] inline double atan(T x) noexcept { return std::atan(static_cast<double>(x)); }

template<typename T, typename U>
[[nodiscard]] inline double atan2(T y, U x) noexcept {
    return std::atan2(static_cast<double>(y), static_cast<double>(x));
}

[[nodiscard]] inline double darajadan_radianga(double daraja) noexcept { return daraja * PI / 180.0; }
[[nodiscard]] inline double radiandan_darajaga(double radian) noexcept { return radian * 180.0 / PI; }

// ===== GIPERBOLIK FUNKSIYALAR =====

template<typename T>
[[nodiscard]] inline double sinh(T x) noexcept { return std::sinh(static_cast<double>(x)); }

template<typename T>
[[nodiscard]] inline double cosh(T x) noexcept { return std::cosh(static_cast<double>(x)); }

template<typename T>
[[nodiscard]] inline double tanh(T x) noexcept { return std::tanh(static_cast<double>(x)); }

// ===== MIN / MAX / ORALIQ =====

template<typename T, typename U>
[[nodiscard]] inline auto eng_katta(T x, U y) noexcept { return x > y ? x : y; }

template<typename T, typename U>
[[nodiscard]] inline auto eng_kichik(T x, U y) noexcept { return x < y ? x : y; }

template<typename T>
[[nodiscard]] inline T oraliqda_cheklash(T x, T quyi, T yuqori) noexcept {
    return std::clamp(x, quyi, yuqori);
}

template<typename T>
[[nodiscard]] inline T chiziqli_interpolatsiya(T a, T b, double t) noexcept {
    return static_cast<T>(a + (b - a) * t);
}

// ===== QOLDIQ VA BO'LINUVCHANLIK =====

[[nodiscard]] inline long long egcd(long long a, long long b) noexcept {
    while (b != 0) { long long t = b; b = a % b; a = t; }
    return a;
}

[[nodiscard]] inline long long kub(long long a, long long b) noexcept {
    return a / egcd(a, b) * b;
}

[[nodiscard]] inline bool toqMi(long long n) noexcept { return n % 2 != 0; }
[[nodiscard]] inline bool juftMi(long long n) noexcept { return n % 2 == 0; }

[[nodiscard]] inline bool tub_sonMi(long long n) noexcept {
    if (n < 2) return false;
    if (n == 2 || n == 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (long long i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

[[nodiscard]] inline long long faktorial(int n) {
    if (n < 0) throw std::invalid_argument("Faktorial manfiy son uchun aniqlanmagan");
    if (n > 20) throw std::overflow_error("Faktorial juda katta");
    long long result = 1;
    for (int i = 2; i <= n; ++i) result *= i;
    return result;
}

[[nodiscard]] inline long long kombinatsiya(int n, int k) {
    if (k < 0 || k > n) return 0;
    if (k == 0 || k == n) return 1;
    k = std::min(k, n - k);
    long long result = 1;
    for (int i = 0; i < k; ++i) {
        result = result * (n - i) / (i + 1);
    }
    return result;
}

// ===== TASODIFIY SONLAR =====

inline std::mt19937& tasodifiy_dvigatel() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return gen;
}

[[nodiscard]] inline int tasodifiy_son(int min, int max) {
    std::uniform_int_distribution<> d(min, max);
    return d(tasodifiy_dvigatel());
}

[[nodiscard]] inline double tasodifiy_kasr(double min = 0.0, double max = 1.0) {
    std::uniform_real_distribution<> d(min, max);
    return d(tasodifiy_dvigatel());
}

[[nodiscard]] inline double normal_taqsimot(double ort = 0.0, double sigma = 1.0) {
    std::normal_distribution<> d(ort, sigma);
    return d(tasodifiy_dvigatel());
}

template<typename T>
inline void aralash(std::vector<T>& v) {
    std::shuffle(v.begin(), v.end(), tasodifiy_dvigatel());
}

// ===== STATISTIKA =====

template<typename Container>
[[nodiscard]] inline double ortacha(const Container& c) {
    if (c.empty()) throw std::invalid_argument("Bo'sh to'plamning o'rtachasi yo'q");
    double sum = 0.0;
    for (const auto& x : c) sum += static_cast<double>(x);
    return sum / static_cast<double>(c.size());
}

template<typename Container>
[[nodiscard]] inline double dispersiya(const Container& c) {
    if (c.size() < 2) throw std::invalid_argument("Dispersiya kamida 2 element talab qiladi");
    double m = ortacha(c);
    double sum = 0.0;
    for (const auto& x : c) {
        double d = static_cast<double>(x) - m;
        sum += d * d;
    }
    return sum / static_cast<double>(c.size());
}

template<typename Container>
[[nodiscard]] inline double standart_chetlanish(const Container& c) {
    return std::sqrt(dispersiya(c));
}

template<typename Container>
[[nodiscard]] inline auto eng_katta_el(const Container& c) {
    if (c.empty()) throw std::invalid_argument("Bo'sh to'plam");
    return *std::max_element(c.begin(), c.end());
}

template<typename Container>
[[nodiscard]] inline auto eng_kichik_el(const Container& c) {
    if (c.empty()) throw std::invalid_argument("Bo'sh to'plam");
    return *std::min_element(c.begin(), c.end());
}

template<typename Container>
[[nodiscard]] inline double yigindisi(const Container& c) {
    double s = 0.0;
    for (const auto& x : c) s += static_cast<double>(x);
    return s;
}

// ===== 2D va 3D VEKTORLAR =====

struct Vektor2 {
    double x = 0.0, y = 0.0;

    Vektor2 operator+(const Vektor2& b) const noexcept { return {x + b.x, y + b.y}; }
    Vektor2 operator-(const Vektor2& b) const noexcept { return {x - b.x, y - b.y}; }
    Vektor2 operator*(double s) const noexcept { return {x * s, y * s}; }
    Vektor2 operator/(double s) const noexcept { return {x / s, y / s}; }
    [[nodiscard]] double uzunlik() const noexcept { return std::sqrt(x*x + y*y); }
    [[nodiscard]] double uzunlik_sq() const noexcept { return x*x + y*y; }
    [[nodiscard]] Vektor2 normallashtirish() const { double l = uzunlik(); return l > 0 ? *this / l : *this; }
    [[nodiscard]] double dot(const Vektor2& b) const noexcept { return x*b.x + y*b.y; }
    [[nodiscard]] double cross(const Vektor2& b) const noexcept { return x*b.y - y*b.x; }
    [[nodiscard]] double masofaGacha(const Vektor2& b) const noexcept { return (*this - b).uzunlik(); }
};

struct Vektor3 {
    double x = 0.0, y = 0.0, z = 0.0;

    Vektor3 operator+(const Vektor3& b) const noexcept { return {x+b.x, y+b.y, z+b.z}; }
    Vektor3 operator-(const Vektor3& b) const noexcept { return {x-b.x, y-b.y, z-b.z}; }
    Vektor3 operator*(double s) const noexcept { return {x*s, y*s, z*s}; }
    Vektor3 operator/(double s) const noexcept { return {x/s, y/s, z/s}; }
    [[nodiscard]] double uzunlik() const noexcept { return std::sqrt(x*x + y*y + z*z); }
    [[nodiscard]] Vektor3 normallashtirish() const { double l = uzunlik(); return l > 0 ? *this / l : *this; }
    [[nodiscard]] double dot(const Vektor3& b) const noexcept { return x*b.x + y*b.y + z*b.z; }
    [[nodiscard]] Vektor3 cross(const Vektor3& b) const noexcept {
        return {y*b.z - z*b.y, z*b.x - x*b.z, x*b.y - y*b.x};
    }
    [[nodiscard]] double masofaGacha(const Vektor3& b) const noexcept { return (*this - b).uzunlik(); }
};

// ===== MATRITSA (2x2 va 3x3) =====

struct Matritsa2x2 {
    std::array<std::array<double, 2>, 2> m{};

    [[nodiscard]] double det() const noexcept { return m[0][0]*m[1][1] - m[0][1]*m[1][0]; }

    Matritsa2x2 operator*(const Matritsa2x2& b) const noexcept {
        Matritsa2x2 r;
        for (int i = 0; i < 2; ++i)
            for (int j = 0; j < 2; ++j)
                for (int k = 0; k < 2; ++k)
                    r.m[i][j] += m[i][k] * b.m[k][j];
        return r;
    }

    [[nodiscard]] Matritsa2x2 teskari() const {
        double d = det();
        if (d == 0.0) throw std::runtime_error("Teskari matritsa yo'q (det = 0)");
        return {{{ {m[1][1]/d, -m[0][1]/d}, {-m[1][0]/d, m[0][0]/d} }}};
    }
};

} // namespace uzpp::Matematika
