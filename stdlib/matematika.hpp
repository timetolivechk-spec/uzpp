#pragma once
#ifndef UZPP_GEN_MATEMATIKA_HPP_
#define UZPP_GEN_MATEMATIKA_HPP_
#line 1 "C:\\Users\\MSN\\uz++\\stdlib\\matematika.uzpp"
#include <cmath>
#include <random>
#include <algorithm>
#include <numeric>
#include <vector>
#include <stdexcept>
#include <limits>
#include <array>
namespace uzpp::Matematika {
    inline auto PI()->double
    {
        return 3.14159265358979323846;
    }
    inline auto E_soni()->double
    {
        return 2.71828182845904523536;
    }
    inline auto PHI()->double
    {
        return 1.61803398874989484820;
    }
    inline auto SQRT2()->double
    {
        return 1.41421356237309504880;
    }
    inline auto LN2()->double
    {
        return 0.69314718055994530942;
    }
    inline auto LN10()->double
    {
        return 2.30258509299404568402;
    }
    inline auto SONSIZLIK()->double
    {
        return std::numeric_limits<double>::infinity();
    }
    inline auto NAN_SON()->double
    {
        return std::numeric_limits<double>::quiet_NaN();
    }
    template <typename T >
 inline auto modul(T x)->T
    {
        return std::abs(x);
    }
    template <typename T , typename U >
 inline auto daraja(T x, U y)->auto
    {
        return std::pow(x, y);
    }
    template <typename T >
 inline auto ildiz(T x)->auto
    {
        return std::sqrt(x);
    }
    template <typename T >
 inline auto kub_ildiz(T x)->auto
    {
        return std::cbrt(x);
    }
    template <typename T >
 inline auto yuqori_yaxlitlash(T x)->T
    {
        return static_cast < T >(std::ceil(x));
    }
    template <typename T >
 inline auto quyi_yaxlitlash(T x)->T
    {
        return static_cast < T >(std::floor(x));
    }
    template <typename T >
 inline auto yaxlitlash(T x)->T
    {
        return static_cast < T >(std::round(x));
    }
    template <typename T >
 inline auto kesish(T x)->T
    {
        return static_cast < T >(std::trunc(x));
    }
    template <typename T >
 inline auto ishorat(T x)->int
    {
        if((x > T(0)))
            {
                return 1;
            }
        if((x < T(0)))
            {
                return - 1;
            }
        return 0;
    }
    template <typename T >
 inline auto sonMi(T x)->bool
    {
        return ! std::isnan(x);
    }
    template <typename T >
 inline auto cheksizMi(T x)->bool
    {
        return std::isinf(x);
    }
    template <typename T >
 inline auto ln(T x)->double
    {
        return std::log(static_cast < double >(x));
    }
    template <typename T >
 inline auto log10(T x)->double
    {
        return std::log10(static_cast < double >(x));
    }
    template <typename T >
 inline auto log2(T x)->double
    {
        return std::log2(static_cast < double >(x));
    }
    template <typename T , typename B >
 inline auto log_asosi(T x, B base)->double
    {
        return(std::log(static_cast < double >(x)) / std::log(static_cast < double >(base)));
    }
    template <typename T >
 inline auto exp(T x)->double
    {
        return std::exp(static_cast < double >(x));
    }
    template <typename T >
 inline auto sin(T x)->double
    {
        return std::sin(static_cast < double >(x));
    }
    template <typename T >
 inline auto cos(T x)->double
    {
        return std::cos(static_cast < double >(x));
    }
    template <typename T >
 inline auto tan(T x)->double
    {
        return std::tan(static_cast < double >(x));
    }
    template <typename T >
 inline auto asin(T x)->double
    {
        return std::asin(static_cast < double >(x));
    }
    template <typename T >
 inline auto acos(T x)->double
    {
        return std::acos(static_cast < double >(x));
    }
    template <typename T >
 inline auto atan(T x)->double
    {
        return std::atan(static_cast < double >(x));
    }
    template <typename T , typename U >
 inline auto atan2(T y, U x)->double
    {
        return std::atan2(static_cast < double >(y), static_cast < double >(x));
    }
    inline auto darajadan_radianga(double daraja)->double
    {
        return((daraja * PI()) / 180.0);
    }
    inline auto radiandan_darajaga(double radian)->double
    {
        return((radian * 180.0) / PI());
    }
    template <typename T >
 inline auto sinh(T x)->double
    {
        return std::sinh(static_cast < double >(x));
    }
    template <typename T >
 inline auto cosh(T x)->double
    {
        return std::cosh(static_cast < double >(x));
    }
    template <typename T >
 inline auto tanh(T x)->double
    {
        return std::tanh(static_cast < double >(x));
    }
    template <typename T , typename U >
 inline auto eng_katta(T x, U y)->auto
    {
        return((x > y) ? x: y);
    }
    template <typename T , typename U >
 inline auto eng_kichik(T x, U y)->auto
    {
        return((x < y) ? x: y);
    }
    template <typename T >
 inline auto oraliqda_cheklash(T x, T quyi, T yuqori)->T
    {
        return std::clamp(x, quyi, yuqori);
    }
    template <typename T >
 inline auto chiziqli_interpolatsiya(T a, T b, double t)->T
    {
        return static_cast < T >((a +((b - a) * t)));
    }
    inline auto ekub(long long a, long long b)->long long
    {
        while((b != 0))
            {
                auto t = b;
                (b =(a % b));
                (a = t);
            }
        return a;
    }
    inline auto ekuk(long long a, long long b)->long long
    {
        return((a / ekub(a, b)) * b);
    }
    inline auto toqMi(long long n)->bool
    {
        return((n % 2) != 0);
    }
    inline auto juftMi(long long n)->bool
    {
        return((n % 2) == 0);
    }
    inline auto tub_sonMi(long long n)->bool
    {
        if((n < 2))
            {
                return false;
            }
        if(((n == 2) ||(n == 3)))
            {
                return true;
            }
        if((((n % 2) == 0) ||((n % 3) == 0)))
            {
                return false;
            }
        long long i = 5;
        while(((i * i) <= n))
            {
                if((((n % i) == 0) ||((n %(i + 2)) == 0)))
                    {
                        return false;
                    }
                (i =(i + 6));
            }
        return true;
    }
    inline auto faktorial(int n)->long long
    {
        if((n < 0))
            {
                throw std::invalid_argument("Faktorial manfiy son uchun aniqlanmagan");
            }
        if((n > 20))
            {
                throw std::overflow_error("Faktorial juda katta");
            }
        long long natija = 1;
        for(int i = 2;(i <= n);(i =(i + 1)))
            {
                (natija =(natija * i));
            }
        return natija;
    }
    inline auto kombinatsiya(int n, int k)->long long
    {
        if(((k < 0) ||(k > n)))
            {
                return 0;
            }
        if(((k == 0) ||(k == n)))
            {
                return 1;
            }
        (k = std::min(k,(n - k)));
        long long natija = 1;
        for(int i = 0;(i < k);(i =(i + 1)))
            {
                (natija =((natija *(n - i)) /(i + 1)));
            }
        return natija;
    }
    inline auto tasodifiy_dvigatel()->std::mt19937&
    {
        static std::random_device rd;
        static std::mt19937 gen = std::mt19937(rd());
        return gen;
    }
    inline auto tasodifiy_son(int min, int max)->int
    {
        std::uniform_int_distribution<> d = std::uniform_int_distribution<>(min, max);
        return d(tasodifiy_dvigatel());
    }
    inline auto tasodifiy_kasr(double min = 0.0, double max = 1.0)->double
    {
        std::uniform_real_distribution<> d = std::uniform_real_distribution<>(min, max);
        return d(tasodifiy_dvigatel());
    }
    inline auto normal_taqsimot(double ort = 0.0, double sigma = 1.0)->double
    {
        std::normal_distribution<> d = std::normal_distribution<>(ort, sigma);
        return d(tasodifiy_dvigatel());
    }
    template <typename T >
 inline auto aralashtir(std::vector<T>& v)->void
    {
        std::shuffle(v.begin(), v.end(), tasodifiy_dvigatel());
    }
    template <typename Container >
 inline auto ortacha(const Container& c)->double
    {
        if(c.empty())
            {
                throw std::invalid_argument("Bo'sh to'plamning o'rtachasi yo'q");
            }
        double yigindi = 0.0;
        for(std::size_t i = 0;(i < c.size());(i =(i + 1)))
            {
                (yigindi =(yigindi + static_cast < double >(c[i])));
            }
        return(yigindi / static_cast < double >(c.size()));
    }
    template <typename Container >
 inline auto dispersiya(const Container& c)->double
    {
        if((c.size() < 2))
            {
                throw std::invalid_argument("Dispersiya kamida 2 element talab qiladi");
            }
        double m = ortacha(c);
        double yigindi = 0.0;
        for(std::size_t i = 0;(i < c.size());(i =(i + 1)))
            {
                double d =(static_cast < double >(c[i]) - m);
                (yigindi =(yigindi +(d * d)));
            }
        return(yigindi / static_cast < double >(c.size()));
    }
    template <typename Container >
 inline auto standart_chetlanish(const Container& c)->double
    {
        return std::sqrt(dispersiya(c));
    }
    template <typename Container >
 inline auto eng_katta_el(const Container& c)->auto
    {
        if(c.empty())
            {
                throw std::invalid_argument("Bo'sh to'plam");
            }
        return * std::max_element(c.begin(), c.end());
    }
    template <typename Container >
 inline auto eng_kichik_el(const Container& c)->auto
    {
        if(c.empty())
            {
                throw std::invalid_argument("Bo'sh to'plam");
            }
        return * std::min_element(c.begin(), c.end());
    }
    template <typename Container >
 inline auto yigindisi(const Container& c)->double
    {
        double s = 0.0;
        for(std::size_t i = 0;(i < c.size());(i =(i + 1)))
            {
                (s =(s + static_cast < double >(c[i])));
            }
        return s;
    }
    struct Vektor2 {
    public:
        double x;
        double y;
        Vektor2 operator+(const Vektor2& b) const
        {
            Vektor2 natija;
            (natija.x =(x + b.x));
            (natija.y =(y + b.y));
            return natija;
        }
        Vektor2 operator-(const Vektor2& b) const
        {
            Vektor2 natija;
            (natija.x =(x - b.x));
            (natija.y =(y - b.y));
            return natija;
        }
        Vektor2 operator*(double s) const
        {
            Vektor2 natija;
            (natija.x =(x * s));
            (natija.y =(y * s));
            return natija;
        }
        Vektor2 operator/(double s) const
        {
            Vektor2 natija;
            (natija.x =(x / s));
            (natija.y =(y / s));
            return natija;
        }
        double uzunlik() const
        {
            return std::sqrt(((x * x) +(y * y)));
        }
        double uzunlik_sq() const
        {
            return((x * x) +(y * y));
        }
        Vektor2 normallashtirish() const
        {
            double l = uzunlik();
            return((l > 0) ?(* this / l): * this);
        }
        double skalyar_kopaytma(const Vektor2& b) const
        {
            return((x * b.x) +(y * b.y));
        }
        double vektor_kopaytma(const Vektor2& b) const
        {
            return((x * b.y) -(y * b.x));
        }
        double masofaGacha(const Vektor2& b) const
        {
            return(* this - b).uzunlik();
        }
    };
    struct Vektor3 {
    public:
        double x;
        double y;
        double z;
        Vektor3 operator+(const Vektor3& b) const
        {
            Vektor3 natija;
            (natija.x =(x + b.x));
            (natija.y =(y + b.y));
            (natija.z =(z + b.z));
            return natija;
        }
        Vektor3 operator-(const Vektor3& b) const
        {
            Vektor3 natija;
            (natija.x =(x - b.x));
            (natija.y =(y - b.y));
            (natija.z =(z - b.z));
            return natija;
        }
        Vektor3 operator*(double s) const
        {
            Vektor3 natija;
            (natija.x =(x * s));
            (natija.y =(y * s));
            (natija.z =(z * s));
            return natija;
        }
        Vektor3 operator/(double s) const
        {
            Vektor3 natija;
            (natija.x =(x / s));
            (natija.y =(y / s));
            (natija.z =(z / s));
            return natija;
        }
        double uzunlik() const
        {
            return std::sqrt((((x * x) +(y * y)) +(z * z)));
        }
        Vektor3 normallashtirish() const
        {
            double l = uzunlik();
            return((l > 0) ?(* this / l): * this);
        }
        double skalyar_kopaytma(const Vektor3& b) const
        {
            return(((x * b.x) +(y * b.y)) +(z * b.z));
        }
        Vektor3 vektor_kopaytma(const Vektor3& b) const
        {
            Vektor3 natija;
            (natija.x =((y * b.z) -(z * b.y)));
            (natija.y =((z * b.x) -(x * b.z)));
            (natija.z =((x * b.y) -(y * b.x)));
            return natija;
        }
        double masofaGacha(const Vektor3& b) const
        {
            return(* this - b).uzunlik();
        }
    };
    struct Matritsa2x2 {
    public:
        std::array<std::array<double, 2>, 2> m;
        double determinant() const
        {
            return((m[0][0] * m[1][1]) -(m[0][1] * m[1][0]));
        }
        Matritsa2x2 operator*(const Matritsa2x2& b) const
        {
            Matritsa2x2 r;
            for(int i = 0;(i < 2);(i =(i + 1)))
                {
                    for(int j = 0;(j < 2);(j =(j + 1)))
                        {
                            for(int k = 0;(k < 2);(k =(k + 1)))
                                {
                                    (r.m[i][j] =(r.m[i][j] +(m[i][k] * b.m[k][j])));
                                }
                        }
                }
            return r;
        }
        Matritsa2x2 teskari() const
        {
            double d = determinant();
            if((d == 0.0))
                {
                    throw std::runtime_error("Teskari matritsa yo'q (det = 0)");
                }
            Matritsa2x2 natija;
            (natija.m[0][0] =(m[1][1] / d));
            (natija.m[0][1] =(- m[0][1] / d));
            (natija.m[1][0] =(- m[1][0] / d));
            (natija.m[1][1] =(m[0][0] / d));
            return natija;
        }
    };
}
#endif
