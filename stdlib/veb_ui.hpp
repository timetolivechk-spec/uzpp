#pragma once

#include "xotira.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <functional>

namespace uzpp::VebUI {

    // Barcha UI elementlar uchun asosiy interfeys
    class Element {
    public:
        virtual std::string chizish() const = 0;
        virtual ~Element() = default;
    };

    // XSS Himoyasi uchun HTML Qochirish (HTML Escaping)
    inline std::string htmlQochirish(const std::string& matn) {
        std::string natija;
        natija.reserve(matn.size());
        for (char c : matn) {
            switch (c) {
                case '&':  natija += "&amp;";  break;
                case '\"': natija += "&quot;"; break;
                case '\'': natija += "&#39;";  break;
                case '<':  natija += "&lt;";   break;
                case '>':  natija += "&gt;";   break;
                default:   natija += c;        break;
            }
        }
        return natija;
    }

    // Oddiy matn tuguni
    class Matn : public Element {
        std::string matn_;
    public:
        explicit Matn(std::string m) : matn_(std::move(m)) {}
        std::string chizish() const override { return htmlQochirish(matn_); }
    };

    // HTML/UI Teg tuguni (Fluent Interface arxitekturasi bilan)
    class Teg : public Element, public std::enable_shared_from_this<Teg> {
        std::string nomi_;
        std::unordered_map<std::string, std::string> xususiyatlar_;
        std::vector<Xotira::Ulashilgan<Element>> bolalar_;
    public:
        explicit Teg(std::string nomi) : nomi_(std::move(nomi)) {}

        // Xususiyatlarni zanjirli (chaining) ulash
        Xotira::Ulashilgan<Teg> id(const std::string& val) { xususiyatlar_["id"] = val; return shared_from_this(); }
        Xotira::Ulashilgan<Teg> klass(const std::string& val) { xususiyatlar_["class"] = val; return shared_from_this(); }
        Xotira::Ulashilgan<Teg> uslub(const std::string& val) { xususiyatlar_["style"] = val; return shared_from_this(); }
        Xotira::Ulashilgan<Teg> xususiyat(const std::string& k, const std::string& v) { xususiyatlar_[k] = v; return shared_from_this(); }
        
        // Hodisalar (Events) - onlick, oninput va hkz. JS kod ulanadi.
        Xotira::Ulashilgan<Teg> hodisa(const std::string& nomi, const std::string& js_kod) { xususiyatlar_[nomi] = js_kod; return shared_from_this(); }

        // HTMX integratsiyasi (Interaktiv frontend uchun)
        Xotira::Ulashilgan<Teg> hxGet(const std::string& url) { xususiyatlar_["hx-get"] = url; return shared_from_this(); }
        Xotira::Ulashilgan<Teg> hxPost(const std::string& url) { xususiyatlar_["hx-post"] = url; return shared_from_this(); }
        Xotira::Ulashilgan<Teg> hxTarget(const std::string& target) { xususiyatlar_["hx-target"] = target; return shared_from_this(); }
        Xotira::Ulashilgan<Teg> hxSwap(const std::string& swap) { xususiyatlar_["hx-swap"] = swap; return shared_from_this(); }
        Xotira::Ulashilgan<Teg> hxTrigger(const std::string& trigger) { xususiyatlar_["hx-trigger"] = trigger; return shared_from_this(); }

        // Bolalarni (children) qo'shish
        Xotira::Ulashilgan<Teg> bola(Xotira::Ulashilgan<Element> b) { bolalar_.push_back(b); return shared_from_this(); }
        Xotira::Ulashilgan<Teg> matn(const std::string& m) { bolalar_.push_back(Xotira::yangi<Matn>(m)); return shared_from_this(); }

        // HTML kodni generatsiya qilish
        std::string chizish() const override {
            std::ostringstream ss;
            ss << "<" << nomi_;
            for (const auto& [k, v] : xususiyatlar_) {
                ss << " " << htmlQochirish(k) << "=\"" << htmlQochirish(v) << "\"";
            }
            if (bolalar_.empty() && nomi_ != "div" && nomi_ != "script" && nomi_ != "textarea") {
                ss << " />";
            } else {
                ss << ">";
                for (const auto& b : bolalar_) ss << b->chizish();
                ss << "</" << nomi_ << ">";
            }
            return ss.str();
        }
    };

    // --- State Management (Redux Architecture) ---
    template<typename T, typename A>
    class Shtor {
        T holat_;
        std::function<T(const T&, const A&)> redyuser_;
        std::vector<std::function<void(const T&)>> obunachilar_;
    public:
        Shtor(std::function<T(const T&, const A&)> redyuser, T bosh_holat) 
            : redyuser_(std::move(redyuser)), holat_(std::move(bosh_holat)) {}

        T olish() const { return holat_; }

        void yuborish(const A& amal) {
            holat_ = redyuser_(holat_, amal);
            for (auto& kuzatuvchi : obunachilar_) kuzatuvchi(holat_);
        }

        void obuna(std::function<void(const T&)> callback) {
            obunachilar_.push_back(callback);
        }
    };

    // --- Reaktiv UI Komponentlari (Tayyor Funksiyalar) ---
    inline Xotira::Ulashilgan<Teg> teg(const std::string& nom) { return Xotira::yangi<Teg>(nom); }
    inline Xotira::Ulashilgan<Teg> div() { return teg("div"); }
    inline Xotira::Ulashilgan<Teg> span() { return teg("span"); }
    inline Xotira::Ulashilgan<Teg> kiritish() { return teg("input"); }
    
    inline Xotira::Ulashilgan<Teg> tugma(const std::string& m = "") { 
        auto t = teg("button"); 
        if (!m.empty()) t->matn(m);
        return t; 
    }
    inline Xotira::Ulashilgan<Teg> sarlavha(int daraja, const std::string& m) { return teg("h" + std::to_string(daraja))->matn(m); }
    inline Xotira::Ulashilgan<Teg> rasm(const std::string& url) { return teg("img")->xususiyat("src", url); }
    inline Xotira::Ulashilgan<Teg> havola(const std::string& url, const std::string& m) { return teg("a")->xususiyat("href", url)->matn(m); }

} // namespace uzpp::VebUI