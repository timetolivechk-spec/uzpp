#pragma once

#include "tuzilmalar.hpp"
#include "hodisalar.hpp"
#include <string>
#include <functional>
#include <vector>
#include <memory>

namespace uzpp::Grafika {

class Oyna; // forward declaration

class Vidjet {
public:
    virtual void chizish(Oyna* oyna, Turtburchak hudud) = 0;
    virtual bool hodisa(SichqonchaHolati sh) { return false; }
    virtual ~Vidjet() = default;
};

class MatnVidjeti : public Vidjet {
    std::function<std::string()> matnFunc_;
public:
    explicit MatnVidjeti(std::function<std::string()> funk) : matnFunc_(std::move(funk)) {}
    explicit MatnVidjeti(const std::string& matn) : matnFunc_([matn]() { return matn; }) {}
    
    void chizish(Oyna* oyna, Turtburchak hudud) override;
};

class TugmaVidjeti : public Vidjet {
    std::string matn_;
    std::function<void()> onClick_;
    bool hover_ = false;
    float hoverAnim_ = 0.0f;
    Turtburchak oxirgiHudud_ = {0,0,0,0};
public:
    TugmaVidjeti(const std::string& matn, std::function<void()> onClick) 
        : matn_(matn), onClick_(std::move(onClick)) {}
        
    void chizish(Oyna* oyna, Turtburchak hudud) override;
    bool hodisa(SichqonchaHolati sh) override;
};

class VertikalQutiVidjeti : public Vidjet {
    std::vector<std::shared_ptr<Vidjet>> bolalar_;
public:
    explicit VertikalQutiVidjeti(std::vector<std::shared_ptr<Vidjet>> bolalar) 
        : bolalar_(std::move(bolalar)) {}
        
    void chizish(Oyna* oyna, Turtburchak hudud) override {
        int count = bolalar_.empty() ? 1 : bolalar_.size();
        float elementH = hudud.balandlik / count;
        for (size_t i = 0; i < bolalar_.size(); ++i) {
            Turtburchak bolaHudud = { hudud.x, hudud.y + float(i * elementH), hudud.kenglik, float(elementH) };
            bolalar_[i]->chizish(oyna, bolaHudud);
        }
    }
    
    bool hodisa(SichqonchaHolati sh) override {
        bool ushlandi = false;
        for (auto& bola : bolalar_) {
            if (bola->hodisa(sh)) ushlandi = true;
        }
        return ushlandi;
    }
};

class KiritishVidjeti : public Vidjet {
    std::string matn_;
    std::string sarlavha_;
    bool fokus_ = false;
    Turtburchak oxirgiHudud_ = {0,0,0,0};
public:
    explicit KiritishVidjeti(const std::string& sarlavha) : sarlavha_(sarlavha) {}
    
    std::string olish() const { return matn_; }
    void ornatish(const std::string& m) { matn_ = m; }
    
    void chizish(Oyna* oyna, Turtburchak hudud) override {
        oxirgiHudud_ = hudud;
        if (fokus_) {
            char c = oyna->oxirgiBelgi();
            if (c == '\b' && !matn_.empty()) matn_.pop_back();
            else if (c >= 32 && c <= 126) matn_ += c;
        }
        
        Rang fonRangi = fokus_ ? Rang{255, 255, 255, 255} : Rang{200, 200, 200, 255};
        oyna->turtburchakChizish(hudud, fonRangi);
        
        std::string krsatish = matn_.empty() && !fokus_ ? sarlavha_ : matn_;
        Rang matnRangi = matn_.empty() && !fokus_ ? Rang{150, 150, 150, 255} : Rang{0, 0, 0, 255};
        
        if (fokus_ && static_cast<int>(oyna->umumiyVaqt() * 2) % 2 == 0) krsatish += "|";
        oyna->matnChizish(hudud.x + 5, hudud.y + (hudud.balandlik / 2.0f) - 8.0f, krsatish, matnRangi);
    }
    
    bool hodisa(SichqonchaHolati sh) override {
        if (sh.chapTugmaQoyibYuborildi) {
            fokus_ = (sh.x >= oxirgiHudud_.x && sh.x <= oxirgiHudud_.x + oxirgiHudud_.kenglik &&
                      sh.y >= oxirgiHudud_.y && sh.y <= oxirgiHudud_.y + oxirgiHudud_.balandlik);
            return fokus_;
        }
        return false;
    }
};

class RasmVidjeti : public Vidjet {
    unsigned int tekstura_;
public:
    explicit RasmVidjeti(unsigned int t) : tekstura_(t) {}
    void chizish(Oyna* oyna, Turtburchak hudud) override {
        // Oyna rasm chizishni o'z bo'yniga oladi
    }
};

// === ANIMATSIYA VIDJETLARI ===

class AylanuvchiVidjet : public Vidjet {
    std::shared_ptr<Vidjet> bola_;
    float tezlik_;
    float burchak_ = 0.0f;
public:
    AylanuvchiVidjet(std::shared_ptr<Vidjet> bola, float tezlik);
    void chizish(Oyna* oyna, Turtburchak hudud) override;
    bool hodisa(SichqonchaHolati sh) override;
};

class SuzuvchiVidjet : public Vidjet {
    std::shared_ptr<Vidjet> bola_;
    float amplituda_;
    float chastota_;
public:
    SuzuvchiVidjet(std::shared_ptr<Vidjet> bola, float amplituda, float chastota = 1.0f);
    void chizish(Oyna* oyna, Turtburchak hudud) override;
    bool hodisa(SichqonchaHolati sh) override;
};

class PulsatsiyaVidjeti : public Vidjet {
    std::shared_ptr<Vidjet> bola_;
    float tezlik_;
public:
    PulsatsiyaVidjeti(std::shared_ptr<Vidjet> bola, float tezlik);
    void chizish(Oyna* oyna, Turtburchak hudud) override;
    bool hodisa(SichqonchaHolati sh) override;
};

// Yordamchi metod (Yangi = std::make_shared o'rniga qisqa)
template<typename T, typename... Args>
std::shared_ptr<T> Yangi(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

} // namespace uzpp::Grafika
