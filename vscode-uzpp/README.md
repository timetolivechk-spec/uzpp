# uz++ — VS Code Extension

**uz++** — o'zbek tilidagi dasturlash tili. C++ ning barcha kuchi, o'z tilida.

[![Version](https://img.shields.io/visual-studio-marketplace/v/uzpp.uzpp)](https://marketplace.visualstudio.com/items?itemName=uzpp.uzpp)
[![Installs](https://img.shields.io/visual-studio-marketplace/i/uzpp.uzpp)](https://marketplace.visualstudio.com/items?itemName=uzpp.uzpp)

---

## ⚡ Tezda boshlash — sizning platformangiz

### 🪟 Windows — bitta fayl

👉 **[uzpp-setup.exe](https://github.com/timetolivechk-spec/uzpp/releases/latest/download/uzpp-setup.exe)** ni yuklab oling va ishga tushiring (~152 MB).

Bitta o'rnatuvchi ichida bor:

| Komponent | Tavsif |
|---|---|
| **uz++ kompilyatori** | Transpayler: `.uzpp` → C++ → binary |
| **Standart kutubxona** | `stdlib/` — 30+ modul (matn, vaqt, json, tarmoq, ...) |
| **Misollar** | `misollar/` — 10 ta tayyor `.uzpp` dastur |
| **MinGW-w64 GCC 15.2 UCRT** | Bundle C++ kompilyatori — alohida yuklash kerak emas |

- Default joy: `%LOCALAPPDATA%\Programs\uzpp\` (admin huquqlari shart emas).
- PATH ga avtomatik qo'shiladi; `.uzpp` fayllar uzpp bilan ochiladi.
- Tugagach: VS Code'ni qayta ishga tushiring → har qanday `.uzpp` faylda **F5**.

---

### 🐧 Linux / 🍎 macOS — bitta buyruq

```bash
curl -fsSL https://github.com/timetolivechk-spec/uzpp/releases/latest/download/install.sh | bash
```

Skript:
- Tizimni aniqlaydi (Linux x64 yoki macOS Apple Silicon).
- `g++` / `clang++` borligini tekshiradi; yo'q bo'lsa kerakli `apt`/`dnf`/`brew` buyrug'ini ko'rsatadi.
- `uzpp` ni `/usr/local/bin/` ga, stdlib ni `/usr/local/lib/uzpp/` ga o'rnatadi.
- Smoke-test: `uzpp --version`.

Pin qilingan versiya kerak bo'lsa: `UZPP_VERSION=v2.1.8 ./install.sh` (yoki `$HOME` ga: `UZPP_PREFIX=$HOME ./install.sh`).

---

### 💻 VS Code Welcome ekrani (har uchchala platforma uchun)

Extension o'rnatilgandan keyin status bar pastida **`⬇ uz++ install`** tugmasi paydo bo'ladi — bosing va "Xush kelibsiz" ekrani ochiladi:

- **Windows**: tugma `uzpp-setup.exe` ni avtomatik yuklab oladi va ishga tushiradi.
- **Linux/macOS**: tugma terminal ochib `install.sh` ni `curl|bash` orqali ishga tushiradi.

---

## ✨ Imkoniyatlar

- **Sintaksis yorqinligi** — barcha uz++ kalit so'zlari rangli (semantic tokens orqali AST-aware).
- **Real-vaqt diagnostika** — xatolar yozayotganda ko'rsatiladi (LSP, TypeChecker).
- **Inlay hints** — `o'zgaruvchan x = ...` keyin xulosa qilingan tur ko'rsatiladi.
- **Code actions** — "ishlatilmagan o'zgaruvchi" uchun quick-fix: `_` prefiks yoki o'chirish.
- **Avtoto'ldirish** — 100+ kalit so'zlar va stdlib funksiyalari.
- **Hover** — kursor ostidagi belgilar haqida ma'lumot (uz++ → C++ tarjima, signature).
- **Definition / References / Rename** — barcha asosiy LSP navigatsiya buyruqlari.
- **F5 → ishga tushirish** — bir tugma bilan.
- **C++ kodini ko'rish** — transpile qilingan C++ ni yonida ochish.
- **Status bar** — kompilyator holati va versiyasi.

---

## ⌨️ Buyruqlar

| Tugma / Buyruq | Amal |
|----------------|------|
| `F5` | Faylni ishga tushirish |
| `Ctrl+F5` | Faylni qurish (binary) |
| `Ctrl+Shift+P` → `uz++ yangi loyiha` | Yangi loyiha yaratish |
| `Ctrl+Shift+P` → `uz++ C++ kodi` | Generatsiya qilingan C++ ni ko'rish |
| `Ctrl+Shift+P` → `uz++ komponentlar holati` | O'rnatilgan komponentlarni ko'rish |
| `Ctrl+Shift+P` → `uz++ kompilyatorni yangilash` | Yangi versiyani GitHub'dan yuklash |
| `Ctrl+Shift+P` → `uz++ fayldan o'rnatish` | Offline `.vsix` / `.exe` orqali o'rnatish |
| `Ctrl+Shift+P` → `uz++ reset` | Barcha komponentlarni o'chirish |

---

## 🚀 Birinchi dastur

```uz++
// salom.uzpp
ulash "uzpp_runtime.hpp"

butun asosiy() {
    matn o'lcham = "kichik";        // apostrof haqiqiy o'zbek so'zida
    yozish << "Salom, " << o'lcham << " dunyo!" << qator_oxiri;
    qaytarish 0;
}
```

Saqlang → `F5` → Natija terminalda chiqadi.

---

## 🔑 Kalit so'zlar (qisqacha)

| uz++ | C++ | uz++ | C++ |
|------|-----|------|-----|
| `butun` | `int` | `yozish` | `std::cout` |
| `haqiqiy` | `double` | `o'qish` | `std::cin` |
| `matn` | `std::string` | `qaytarish` | `return` |
| `mantiqiy` | `bool` | `agar` | `if` |
| `o'zgaruvchan` | `auto` | `aks` / `yoki` | `else` |
| `o'zgarmas` | `const` | `uchun` | `for` |
| `sinf` | `class` | `toki` | `while` |
| `vektor` | `std::vector` | `urinish` | `try` |
| `qator_oxiri` | `std::endl` | `ushlash` | `catch` |
| `manba_joyi` | `std::source_location` | `funksiya` | `auto fn(...) -> T` |

Apostrof identifikatorlar (`o'lcham`, `G'oya`, `to'lov`) C++23 ga U+02BC MODIFIER LETTER APOSTROPHE sifatida ko'chiriladi — `_` ga mangle qilinmaydi.

---

## ⚙️ Sozlamalar

- **`uzpp.compilerPath`** — Kompilyatorga maxsus yo'l (ixtiyoriy). Odatda bo'sh qoldiriladi — kengaytma o'zi topadi.

---

## 🐛 Xato tuzatish

- **Komponent yuklanmadi:** Internetni tekshiring, keyin `Ctrl+Shift+P` → `uz++ komponentlarni o'rnatish`.
- **"command not found":** `Ctrl+Shift+P` → `uz++ komponentlar holati` — nima yetishmayotganini ko'rsatadi.
- **VPN/proxy bilan muammo:** [GitHub Releases](https://github.com/timetolivechk-spec/uzpp/releases/latest) dan qo'lda yuklab, `uz++ fayldan o'rnatish` orqali o'rnating.
- **Linux: `<generator>` topilmadi:** `sudo apt install libstdc++-15-dev` (Ubuntu 24.04 + ubuntu-toolchain-r/test PPA).

---

## 🔒 Maxfiylik

Kengaytma faqat `github.com/timetolivechk-spec/uzpp` manzilidan komponentlar yuklab oladi. Foydalanuvchi ma'lumotlari hech qayerga yuborilmaydi. Telemetriya yo'q.

---

## 📚 Resurslar

- [GitHub](https://github.com/timetolivechk-spec/uzpp) — manba kodi, issues, releases
- [Boshlash qo'llanmasi](https://github.com/timetolivechk-spec/uzpp/blob/main/docs/getting-started.md)
- [Misollar](https://github.com/timetolivechk-spec/uzpp/tree/main/examples)
- [Muammo bildirish](https://github.com/timetolivechk-spec/uzpp/issues)
- [So'nggi release (v2.1.8)](https://github.com/timetolivechk-spec/uzpp/releases/tag/v2.1.8)
