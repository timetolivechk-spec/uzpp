#pragma once

#include <memory>
#include <utility>
#include <vector>
#include <new>

namespace uzpp::Xotira {

    // Ulashilgan (Shared Pointer) - Xotirani RAII asosida xavfsiz boshqarish (GC emas!)
    template<typename T>
    using Ulashilgan = std::shared_ptr<T>;

    // Yangi obyekt yaratish va uni Ulashilgan ko'rsatkichga topshirish
    template<typename T, typename... Args>
    Ulashilgan<T> yangi(Args&&... args) {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    // Arena Allocator - O'yinlar va yuqori tezlik talab qilinadigan joylar uchun
    // xotirani bitta katta blokda olib, juda tez (O(1)) ajratib beradi.
    class Arena {
        std::vector<unsigned char> bufer_;
        std::size_t offset_ = 0;
    public:
        explicit Arena(std::size_t hajm_baytlarda) : bufer_(hajm_baytlarda) {}
        
        template<typename T, typename... Args>
        T* ajratish(Args&&... args) {
            std::size_t qoldiq = offset_ % alignof(T);
            std::size_t padding = qoldiq == 0 ? 0 : alignof(T) - qoldiq;
            
            if (offset_ + padding + sizeof(T) > bufer_.size()) {
                return nullptr; // Xotira yetarli emas
            }
            
            offset_ += padding;
            T* ptr = reinterpret_cast<T*>(bufer_.data() + offset_);
            offset_ += sizeof(T);
            
            return new(ptr) T(std::forward<Args>(args)...);
        }
        
        // Barcha ajratilgan xotirani bir zumda bo'shatish
        void tozalash() { offset_ = 0; }
        
        [[nodiscard]] std::size_t ishlatilgan() const { return offset_; }
        [[nodiscard]] std::size_t sigimi() const { return bufer_.size(); }
    };

} // namespace uzpp::Xotira