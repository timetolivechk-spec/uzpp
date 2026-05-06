# uz++ — VS Code Extension

**uz++** — o'zbek tilidagi dasturlash tili. C++ ning barcha kuchi, o'z tilida.

[![Version](https://img.shields.io/visual-studio-marketplace/v/uzpp.uzpp)](https://marketplace.visualstudio.com/items?itemName=uzpp.uzpp)
[![Installs](https://img.shields.io/visual-studio-marketplace/i/uzpp.uzpp)](https://marketplace.visualstudio.com/items?itemName=uzpp.uzpp)

---

## Imkoniyatlar

- **Sintaksis yorqinligi** — barcha uz++ kalit so'zlari rangli
- **Real-vaqt diagnostika** — xatolar yozayotgan paytda ko'rsatiladi
- **Avtoto'ldirish** — kalit so'zlar va funksiyalar uchun
- **F5 bilan ishga tushirish** — bir tugma bilan dasturni bajarish
- **C++ kodini ko'rish** — transpile qilingan C++ ni yonida ko'rsatish
- **Status bar** — kompilyator versiyasi doim ko'zda
- **Yangi loyiha** — bir buyruq bilan shablon yaratish

---

## Tezkor boshlash

1. Kengaytmani o'rnating
2. `.uzpp` faylini oching yoki yarating
3. **F5** bosing — dastur ishga tushadi

```uz++
// salom.uzpp
ulash "uzpp_runtime.hpp"

butun asosiy() {
    yozish << "Salom, Dunyo!" << qator_oxiri;
    qaytarish 0;
}
```

---

## Buyruqlar

| Buyruq / Tugma | Tavsif |
|---|---|
| `F5` | Joriy faylni ishga tushirish |
| `Ctrl+F5` | Faylni qurish (binary hosil qilish) |
| `Ctrl+Shift+P` → `uz++ yangi loyiha` | Yangi loyiha papkasi va shablon yaratish |
| `Ctrl+Shift+P` → `uz++ C++ kodi` | Generatsiya qilingan C++ ni yonida ko'rish |
| `Ctrl+Shift+P` → `uz++ kompilyatorni yangilash` | GitHub Releases'dan eng yangi versiyani yuklash |
| `Ctrl+Shift+P` → `uz++ faylni formatlash` | Faylni formatlash |

---

## Kalit so'zlar

| uz++ | C++ | uz++ | C++ |
|---|---|---|---|
| `butun` | `int` | `yozish` | `std::cout` |
| `haqiqiy` | `double` | `o'qish` | `std::cin` |
| `matn` | `std::string` | `qaytarish` | `return` |
| `mantiqiy` | `bool` | `agar` | `if` |
| `ozgaruvchan` | `auto` | `aks holda` | `else` |
| `ozgarmas` | `const` | `uchun` | `for` |
| `sinf` | `class` | `holda` | `while` |
| `tuzilma` | `struct` | `urinish` | `try` |
| `vektor` | `std::vector` | `ushlash` | `catch` |
| `qator_oxiri` | `std::endl` | `asosiy` | `main` |

---

## Sozlamalar

**`uzpp.compilerPath`** — uzpp.exe fayliga to'liq yo'l.  
Bo'sh bo'lsa, kengaytma quyidagi tartibda qidiradi:
1. Kengaytma ichidagi `bin/` papkasi (o'rnatilgan binar)
2. Sozlamadagi yo'l
3. Workspace papkasi
4. Tizim PATH

---

## Talablar

- Windows 10/11, Linux, macOS
- uz++ kompilyatori (kengaytma bilan birga yoki alohida o'rnatiladi)

---

## Muammo va takliflar

[GitHub Issues](https://github.com/timetolivechk-spec/uz-plus-plus/issues)
