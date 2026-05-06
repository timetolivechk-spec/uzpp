# uz++ — VS Code Extension

**uz++** — o'zbek tilidagi dasturlash tili. C++ ning barcha kuchi, o'z tilida.

[![Version](https://img.shields.io/visual-studio-marketplace/v/uzpp.uzpp)](https://marketplace.visualstudio.com/items?itemName=uzpp.uzpp)
[![Installs](https://img.shields.io/visual-studio-marketplace/i/uzpp.uzpp)](https://marketplace.visualstudio.com/items?itemName=uzpp.uzpp)

---

## O'rnatish — 2 qadam

**1.** VS Code → Extensions → "uz++" → **Install**

**2.** Xush kelibsiz ekranida **"Hammasini o'rnatish"** tugmasini bosing

Shundan so'ng F5 bilan dastur ishga tushadi. Boshqa hech narsa kerak emas.

### Nima yuklanadi?

| Komponent | Hajm | Tavsif |
|-----------|------|--------|
| uz++ kompilyatori | ~10 MB | Transpayler: `.uzpp` → C++ → binary |
| Standart kutubxona | ~2 MB | `stdlib/uzpp_runtime.hpp` va boshqa modullar |
| MinGW (faqat Windows) | ~150 MB | Portable C++ kompilyatori (g++), bir marta yuklanadi |

Barcha komponentlar `~/.vscode/extensions-storage/uzpp.uzpp/` papkasida saqlanadi va kengaytma yangilanishlarida saqlanib qoladi.

### Internet yo'q? (Offline o'rnatish)

Arxivlarni oldindan yuklab, `Ctrl+Shift+P` → **"uz++: Fayldan o'rnatish"** orqali o'rnating.

---

## Imkoniyatlar

- **Sintaksis yorqinligi** — barcha uz++ kalit so'zlari rangli
- **Real-vaqt diagnostika** — xatolar yozayotganda ko'rsatiladi (LSP)
- **Avtoto'ldirish** — kalit so'zlar va funksiyalar
- **F5 → ishga tushirish** — bir tugma bilan
- **C++ kodini ko'rish** — transpile qilingan C++ ni yonida ochish
- **Status bar** — kompilyator holati va versiyasi

---

## Buyruqlar

| Tugma / Buyruq | Amal |
|----------------|------|
| `F5` | Faylni ishga tushirish |
| `Ctrl+F5` | Faylni qurish (binary) |
| `Ctrl+Shift+P` → `uz++ yangi loyiha` | Yangi loyiha yaratish |
| `Ctrl+Shift+P` → `uz++ C++ kodi` | Generatsiya qilingan C++ ni ko'rish |
| `Ctrl+Shift+P` → `uz++ komponentlar holati` | O'rnatilgan komponentlarni ko'rish |
| `Ctrl+Shift+P` → `uz++ kompilyatorni yangilash` | Yangi versiyani GitHub'dan yuklash |
| `Ctrl+Shift+P` → `uz++ fayldan o'rnatish` | Offline o'rnatish |
| `Ctrl+Shift+P` → `uz++ reset` | Barcha komponentlarni o'chirish |

---

## Birinchi dastur

```uz++
// salom.uzpp
ulash "uzpp_runtime.hpp"

butun asosiy() {
    yozish << "Salom, Dunyo!" << qator_oxiri;
    qaytarish 0;
}
```

Saqlang → `F5` → Natija terminala chiqadi.

---

## Kalit so'zlar (qisqacha)

| uz++ | C++ | uz++ | C++ |
|------|-----|------|-----|
| `butun` | `int` | `yozish` | `std::cout` |
| `haqiqiy` | `double` | `o'qish` | `std::cin` |
| `matn` | `std::string` | `qaytarish` | `return` |
| `mantiqiy` | `bool` | `agar` | `if` |
| `ozgaruvchan` | `auto` | `aks holda` | `else` |
| `ozgarmas` | `const` | `uchun` | `for` |
| `sinf` | `class` | `holda` | `while` |
| `vektor` | `std::vector` | `urinish` | `try` |
| `qator_oxiri` | `std::endl` | `ushlash` | `catch` |

---

## Sozlamalar

**`uzpp.compilerPath`** — Kompilyatorga maxsus yo'l (ixtiyoriy). Odatda bo'sh qoldiriladi — kengaytma o'zi topadi.

---

## Xato tuzatish

**Komponent yuklanmadi:** Internetni tekshiring, keyin `Ctrl+Shift+P` → `uz++ komponentlarni o'rnatish`.

**"command not found":** `Ctrl+Shift+P` → `uz++ komponentlar holati` — nima yetishmayotganini ko'rsatadi.

**VPN/proxy bilan muammo:** [GitHub Releases](https://github.com/timetolivechk-spec/uzpp/releases/latest) dan qo'lda yuklab, `uz++ fayldan o'rnatish` orqali o'rnating.

---

## Maxfiylik

Kengaytma faqat `github.com/timetolivechk-spec/uzpp` manzilidan komponentlar yuklab oladi. Foydalanuvchi ma'lumotlari hech qayerga yuborilmaydi.

---

## Resurslar

- [GitHub](https://github.com/timetolivechk-spec/uzpp)
- [Boshlash qo'llanmasi](https://github.com/timetolivechk-spec/uzpp/blob/main/docs/getting-started.md)
- [Misollar](https://github.com/timetolivechk-spec/uzpp/tree/main/examples)
- [Muammo bildirish](https://github.com/timetolivechk-spec/uzpp/issues)
