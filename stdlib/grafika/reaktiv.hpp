#pragma once
// status: REAL — Observer pattern (kuzatuvchi).

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
                if (kuzatuvchi) kuzatuvchi(qiymat_);
            }
        }
        
        size_t obunaBolish(std::function<void(const T&)> callback) {
            kuzatuvchilar_.push_back(std::move(callback));
            return kuzatuvchilar_.size() - 1;
        }

        void obunaniBekorQilish(size_t id) {
            if (id < kuzatuvchilar_.size()) {
                kuzatuvchilar_[id] = nullptr;
            }
        }
    };
}
