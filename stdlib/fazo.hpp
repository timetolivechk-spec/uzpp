#pragma once

#include "platforma.hpp"
#include "xatoliklar.hpp"

#include <vector>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <string>

namespace uzpp::Fazo {

// Ma'lumotlar tahlili va Sun'iy intellekt uchun 2D Matritsa
class Matritsa {
    std::size_t qatorlar_;
    std::size_t ustunlar_;
    std::vector<double> malumotlar_;

public:
    Matritsa(std::size_t qatorlar, std::size_t ustunlar, double boshlangich = 0.0)
        : qatorlar_(qatorlar), ustunlar_(ustunlar), malumotlar_(qatorlar * ustunlar, boshlangich) {}

    Matritsa(std::size_t qatorlar, std::size_t ustunlar, const std::vector<double>& malumotlar)
        : qatorlar_(qatorlar), ustunlar_(ustunlar), malumotlar_(malumotlar) {
        if (malumotlar.size() != qatorlar * ustunlar) {
            throw std::invalid_argument("Matritsa o'lchamlari va malumotlar soni mos kelmaydi.");
        }
    }

    [[nodiscard]] std::size_t qatorlar() const { return qatorlar_; }
    [[nodiscard]] std::size_t ustunlar() const { return ustunlar_; }

    double& olish(std::size_t q, std::size_t u) { return malumotlar_[q * ustunlar_ + u]; }
    const double& olish(std::size_t q, std::size_t u) const { return malumotlar_[q * ustunlar_ + u]; }

    // Matritsalarni qoshish
    Matritsa operator+(const Matritsa& b) const {
        if (qatorlar_ != b.qatorlar_ || ustunlar_ != b.ustunlar_) throw std::invalid_argument("Matritsa o'lchamlari mos emas");
        Matritsa natija(qatorlar_, ustunlar_);
        for (std::size_t i = 0; i < malumotlar_.size(); ++i) natija.malumotlar_[i] = malumotlar_[i] + b.malumotlar_[i];
        return natija;
    }

    // Matritsalarni kopaytirish (Dot product)
    Matritsa operator*(const Matritsa& b) const {
        if (ustunlar_ != b.qatorlar_) throw std::invalid_argument("Ko'paytirish uchun ustun va qatorlar soni mos emas");
        Matritsa natija(qatorlar_, b.ustunlar_);
        for (std::size_t i = 0; i < qatorlar_; ++i) {
            for (std::size_t j = 0; j < b.ustunlar_; ++j) {
                double yigindi = 0.0;
                for (std::size_t k = 0; k < ustunlar_; ++k) {
                    yigindi += olish(i, k) * b.olish(k, j);
                }
                natija.olish(i, j) = yigindi;
            }
        }
        return natija;
    }

    // Transponirlash (A^T)
    Matritsa transponir() const {
        Matritsa natija(ustunlar_, qatorlar_);
        for (std::size_t i = 0; i < qatorlar_; ++i) {
            for (std::size_t j = 0; j < ustunlar_; ++j) {
                natija.olish(j, i) = olish(i, j);
            }
        }
        return natija;
    }
};

// Statistika moduli
namespace Statistika {
    inline double ortacha(const std::vector<double>& v) {
        if (v.empty()) return 0.0;
        double yigindi = 0.0;
        for (double d : v) yigindi += d;
        return yigindi / v.size();
    }

    inline double mediana(std::vector<double> v) {
        if (v.empty()) return 0.0;
        std::sort(v.begin(), v.end());
        std::size_t n = v.size();
        if (n % 2 == 0) return (v[n / 2 - 1] + v[n / 2]) / 2.0;
        return v[n / 2];
    }
}

} // namespace uzpp::Fazo