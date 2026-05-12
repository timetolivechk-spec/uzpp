# uz++ — VS Code Extension

**uz++** — o'zbek tilidagi dasturlash tili. C++ ning barcha kuchi, o'z tilida.

[![Version](https://img.shields.io/visual-studio-marketplace/v/uzpp.uzpp)](https://marketplace.visualstudio.com/items?itemName=uzpp.uzpp)
[![Installs](https://img.shields.io/visual-studio-marketplace/i/uzpp.uzpp)](https://marketplace.visualstudio.com/items?itemName=uzpp.uzpp)

---

## ⚡ O'rnatish — Windows uchun bitta qadam

### 👉 [`uzpp-setup.exe`](https://github.com/timetolivechk-spec/uzpp/releases/latest/download/uzpp-setup.exe) ni yuklab oling va ishga tushiring (~115 MB).

Bitta o'rnatuvchi ichida bor:

| Komponent | Hajm | Tavsif |
|---|---|---|
| **uz++ kompilyatori** | ~4 MB | Transpayler: `.uzpp` → C++ → binary |
| **Standart kutubxona** | ~2 MB | `stdlib/uzpp_runtime.hpp` va boshqa modullar |
| **MinGW GCC 14.2** | ~700 MB (113 MB zip) | C++ kompilyatori — boshqa hech narsa o'rnatish kerak emas |

Default joy: `%LOCALAPPDATA%\Programs\uzpp\` — admin huquqlari shart emas.
PATH ga avtomatik qo'shiladi.

O'rnatish tugagach VS Code'ni qayta ishga tushiring → har qanday `.uzpp` faylda **F5** bossangiz, dastur ishlaydi.

---

### Ko'p marta bosib turgan bo'lsangiz...

VS Code Extensions panelida shu kengaytma sahifasida **Install** bosgandan keyin:

- **Status bar pastida `⬇ uz++ install` tugmasi har doim turadi** — bosing, "Xush kelibsiz" ekrani ochiladi.
- "Xush kelibsiz" ekranida **"Hammasini o'rnatish — uzpp-setup.exe (~115 MB)"** primary tugma har doim ko'rinib turadi (siz uz++ ni allaqachon o'rnatgan bo'lsangiz ham, qayta o'rnatish uchun).
- Tugma `uzpp-setup.exe` ni avtomatik yuklab oladi va ishga tushiradi.

Yoki to'g'ridan-to'g'ri brauzerdan yuklab oling: [GitHub Release v2.1.1](https://github.com/timetolivechk-spec/uzpp/releases/tag/v2.1.1).

---

### Linux / macOS

Hozircha rasmiy paket yo'q — manba koddan quring:
[Boshlash qo'llanmasi](https://github.com/timetolivechk-spec/uzpp/blob/main/docs/getting-started.md).

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
