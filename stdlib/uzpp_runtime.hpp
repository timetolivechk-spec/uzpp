#pragma once

#include "platforma.hpp"
#include "xatoliklar.hpp"
#include "xavfsizlik.hpp"
#include "matematika.hpp"
#include "matn.hpp"
#include "kripto.hpp"
#include "apparat.hpp"

#if !defined(UZPP_EMBEDDED)
#include <ranges>
#include <future>
#include "jurnal.hpp"
#include "tizim.hpp"
#include "kesh.hpp"
#include "sinov.hpp"
#include "asinxron.hpp"
#include "fayl_tizimi.hpp"
#include "tarmoq.hpp"
#include "vaqt.hpp"
#include "oyna.hpp"
#include "tarjima.hpp"
#include "xotira.hpp"
#include "veb_ui.hpp"
#include "suniy_intellekt.hpp"
#include "fazo.hpp"
#include "koinot.hpp"

#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>
#endif

namespace uzpp {

// ===== NATIJA (Result<T,E>) — xatolikni qiymat sifatida ifodalash =====
template <typename T, typename E = std::string>
class Natija {
public:
    static Natija muvaffaqiyat(T val) { return Natija(std::move(val), true); }
    static Natija xato(E err)         { return Natija(std::move(err), false); }

    [[nodiscard]] bool yaroqliMi()   const noexcept { return ok_; }
    [[nodiscard]] bool xatoliMi()    const noexcept { return !ok_; }
    [[nodiscard]] const T& qiymat()  const { if (!ok_) throw std::runtime_error("Natija xato!"); return val_; }
    [[nodiscard]] const E& xatoMazmun() const { if (ok_) throw std::runtime_error("Natija muvaffaqiyatli!"); return err_; }

    T qiymat_yoki(T def) const noexcept { return ok_ ? val_ : std::move(def); }

    template <typename F>
    auto va_keyin(F&& f) const {
        using R = decltype(f(std::declval<const T&>()));
        if (!ok_) return Natija<R, E>::xato(err_);
        return Natija<R, E>::muvaffaqiyat(f(val_));
    }

    explicit operator bool() const noexcept { return ok_; }

private:
    Natija(T val, bool ok) : val_(std::move(val)), ok_(ok) {}
    Natija(E err, bool ok) : err_(std::move(err)), ok_(ok) {}

    T val_{};
    E err_{};
    bool ok_;
};

// ===== TANLOV (Option<T>) — mavjud yoki yo'q =====
template <typename T>
class Tanlov {
public:
    static Tanlov bor(T val)  { return Tanlov(std::move(val)); }
    static Tanlov yoq()       { return Tanlov(); }

    [[nodiscard]] bool borMi()   const noexcept { return has_; }
    [[nodiscard]] bool yoqMi()   const noexcept { return !has_; }
    [[nodiscard]] const T& qiymat() const { if (!has_) throw std::runtime_error("Tanlov bo'sh!"); return val_; }

    T qiymat_yoki(T def) const noexcept { return has_ ? val_ : std::move(def); }

    template <typename F>
    auto va_keyin(F&& f) const {
        using R = decltype(f(std::declval<const T&>()));
        if (!has_) return Tanlov<R>::yoq();
        return Tanlov<R>::bor(f(val_));
    }

    explicit operator bool() const noexcept { return has_; }

private:
    Tanlov() : has_(false) {}
    explicit Tanlov(T val) : val_(std::move(val)), has_(true) {}
    T val_{};
    bool has_ = false;
};

// ===== Qulay yaratuvchi funksiyalar =====
template <typename T>
[[nodiscard]] inline Natija<T> muvaffaqiyat(T val) { return Natija<T>::muvaffaqiyat(std::move(val)); }

template <typename T = std::monostate, typename E>
[[nodiscard]] inline auto xato(E err) { return Natija<T, E>::xato(std::move(err)); }

template <typename T>
[[nodiscard]] inline Tanlov<T> bor(T val) { return Tanlov<T>::bor(std::move(val)); }

template <typename T>
[[nodiscard]] inline Tanlov<T> yoq() { return Tanlov<T>::yoq(); }

#if !defined(UZPP_EMBEDDED)

class Oqim {
public:
    Oqim() = default;

    template <typename Callable, typename... Args>
    explicit Oqim(Callable&& callable, Args&&... args)
        : thread_(std::forward<Callable>(callable), std::forward<Args>(args)...) {}

    Oqim(const Oqim&) = delete;
    Oqim& operator=(const Oqim&) = delete;
    Oqim(Oqim&&) noexcept = default;
    Oqim& operator=(Oqim&&) noexcept = default;
    ~Oqim() = default;

    void kutish() {
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    [[nodiscard]] bool faol() const noexcept {
        return thread_.joinable();
    }

    [[nodiscard]] std::jthread::native_handle_type native_handle() {
        return thread_.native_handle();
    }

private:
    std::jthread thread_;
};

template <typename T>
using AqlliKorsatkich = std::shared_ptr<T>;

template <typename T>
using YagonaKorsatkich = std::unique_ptr<T>;

template <typename Mutex>
using QulflanganGuard = std::lock_guard<Mutex>;

// ===== PHASE 11-C: Null-coalescing and Error handling operator overrides =====

template <typename T, typename U>
T operator^(const std::optional<T>& opt, U&& def) {
    return opt.value_or(std::forward<U>(def));
}

// ===== PHASE 11-D: THREAD POOL EXPANSION =====

class TasqTask {
public:
    using Function = std::function<void()>;
    
    TasqTask() = default;
    explicit TasqTask(Function func) : func_(std::move(func)) {}
    
    void chiqarish() {
        if (func_) {
            func_();
        }
    }
    
    [[nodiscard]] bool bosMi() const {
        return !func_;
    }

private:
    Function func_;
};

class VazifaJavob {
public:
    enum class Holat {
        Kutilmoqda,
        Tugallandi,
        XatoBilan
    };
    
    VazifaJavob() : holat_(Holat::Kutilmoqda) {}
    
    Holat holatOlish() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return holat_;
    }
    
    void jelish() const {
        std::unique_lock<std::mutex> lock(mutex_);
        aytishnomasi_.wait(lock, [this] { return holat_ != Holat::Kutilmoqda; });
    }
    
    bool tugallanganMi() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return holat_ == Holat::Tugallandi;
    }

private:
    friend class OqimPooliPrivate;
    friend class OqimPool;
    
    mutable std::mutex mutex_;
    mutable std::condition_variable aytishnomasi_;
    Holat holat_ = Holat::Kutilmoqda;
    std::string xato_;
    
    void tugallanganQilish() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            holat_ = Holat::Tugallandi;
        }
        aytishnomasi_.notify_all();
    }
    
    void xatoBilan(const std::string& xato) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            holat_ = Holat::XatoBilan;
            xato_ = xato;
        }
        aytishnomasi_.notify_all();
    }
};

class OqimPooliPrivate {
public:
    struct VazifaNavbati: public TasqTask {
        std::shared_ptr<VazifaJavob> javob;
        
        VazifaNavbati() = default;
        VazifaNavbati(TasqTask::Function func, std::shared_ptr<VazifaJavob> j)
            : TasqTask(std::move(func)), javob(j) {}
    };
};

class OqimPool {
public:
    explicit OqimPool(std::size_t ischilarSoni = 4) {
        if (ischilarSoni == 0) {
            ischilarSoni = std::thread::hardware_concurrency();
        }
        
        for (std::size_t i = 0; i < ischilarSoni; ++i) {
            ishchilar_.emplace_back([this] { ishchiSikli(); });
        }
    }
    
    ~OqimPool() {
        toxtatish();
    }
    
    OqimPool(const OqimPool&) = delete;
    OqimPool& operator=(const OqimPool&) = delete;
    OqimPool(OqimPool&&) = delete;
    OqimPool& operator=(OqimPool&&) = delete;
    
    // Vazifani qo'shish (blocking qilmay)
    std::shared_ptr<VazifaJavob> vazifaQosh(TasqTask::Function func) {
        auto javob = std::make_shared<VazifaJavob>();
        {
            std::lock_guard<std::mutex> lock(vazifaMutex_);
            
            if (toxtab_) {
                javob->xatoBilan("Thread pool.pool butun taqsim qilingan.");
                return javob;
            }
            
            OqimPooliPrivate::VazifaNavbati taskQo(func, javob);
            vazifalar_.push_back(taskQo);
        }
        
        aytishnomasi_.notify_one();
        return javob;
    }
    
    // Barcha vazifalarni tugatish uchun kutish
    void kutish() {
        for (auto& worker : ishchilar_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
    
    // Thread poolni to'xtatish
    void toxtatish() {
        {
            std::lock_guard<std::mutex> lock(vazifaMutex_);
            toxtab_ = true;
        }
        aytishnomasi_.notify_all();
        kutish();
    }
    
    [[nodiscard]] std::size_t ishchilarSoni() const {
        return ishchilar_.size();
    }
    
    [[nodiscard]] std::size_t kutilayotganVazifalarSoni() const {
        std::lock_guard<std::mutex> lock(vazifaMutex_);
        return vazifalar_.size();
    }

private:
    std::vector<std::jthread> ishchilar_;
    std::deque<OqimPooliPrivate::VazifaNavbati> vazifalar_;
    mutable std::mutex vazifaMutex_;
    std::condition_variable aytishnomasi_;
    bool toxtab_ = false;
    
    void ishchiSikli() {
        while (true) {
            OqimPooliPrivate::VazifaNavbati vazifa;
            
            {
                std::unique_lock<std::mutex> lock(vazifaMutex_);
                aytishnomasi_.wait(lock, [this] {
                    return toxtab_ || !vazifalar_.empty();
                });
                
                if (toxtab_ && vazifalar_.empty()) {
                    break;
                }
                
                if (vazifalar_.empty()) {
                    continue;
                }
                
                vazifa = std::move(vazifalar_.front());
                vazifalar_.pop_front();
            }
            
            if (!vazifa.bosMi()) {
                try {
                    vazifa.chiqarish();
                    if (vazifa.javob) {
                        vazifa.javob->tugallanganQilish();
                    }
                } catch (const std::exception& e) {
                    if (vazifa.javob) {
                        vazifa.javob->xatoBilan(std::string("Vazifa xatosi: ") + e.what());
                    }
                }
            }
        }
    }
};

#endif // !UZPP_EMBEDDED

} // namespace uzpp
