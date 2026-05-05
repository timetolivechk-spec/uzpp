#pragma once

#include <vector>
#include <functional>
#include <memory>

namespace uzpp::Grafika {
    template<typename T>
    class Holat {
        T qiymat_;
        std::vector<std::function<void(const T&)>> kuzatuvchilar_;
    public:
        Holat(T boshlangich) : qiymat_(boshlangich) {}
        
        T olish() const { return qiymat_; }
        
        void ornatish(T yangiQiymat) {
            qiymat_ = yangiQiymat;
            for(auto& kuzatuvchi : kuzatuvchilar_) {
                kuzatuvchi(qiymat_);
            }
        }
        
        void obunaBolish(std::function<void(const T&)> callback) {
            kuzatuvchilar_.push_back(callback);
        }
    };
}
