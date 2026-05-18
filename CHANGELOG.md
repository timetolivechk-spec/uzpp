# Changelog

Barcha muhim o'zgarishlar shu yerda hujjatlashtiriladi.

## [2.1.8] — 2026-05-18

### Til (uz++ kompilyatori)
- **Apostrof identifikatorlarda haqiqatan ishlaydi.** `o'lcham`, `G'oya`,
  `to'lov` kabi haqiqiy o'zbek so'zlari endi `_` ga aylantirilmaydi — C++23
  da U+02BC MODIFIER LETTER APOSTROPHE sifatida saqlanadi (`oʼlcham`).
- **Keyword shadowing.** `yangi`, `bor`, `kasr`, `uzun`, `bosh` kabi
  alias-keywordlarni endi lokal o'zgaruvchi nomi sifatida ishlatish mumkin —
  codegen scope-tracking shadow qilingan nomlarni alias jadvalidan o'tkazmaydi.
- **`} yoki { ... }`** — `yoki` (logical OR) endi `if`-`then`dan keyin
  `else` sifatida ham qabul qilinadi.
- **`funksiya X() o'zgarmas -> T { ... }`** — sinf metodlarida `funksiya`
  uslubida `o'zgarmas` modifier va trailing return type birga ishlaydi.
- **Bitwise operatorlar `& | ^`** — parser ularni binary sifatida tan oladi
  (C-precedence: `&&` va `==` orasida). Unary `&` (address-of) o'zgarmagan.
- **`yangi`/`o'chirish` lookahead** — endi `new`/`delete` sifatida faqat
  type/expression davom etganda olinadi. `butun yangi = 5;` ishlaydi.

### Stdlib
- **`stdlib/matn.uzpp`** — `matn` kutubxonasi to'liq uz++ tilida qayta
  yozildi (626 LOC). Eski C++ `matn.hpp` artefakt sifatida shu fayldan
  generatsiya qilinadi (header-mode transpilatsiyasi).
- **41 funksiya** `matn` da: barcha 27 ta eski + 14 ta yangi:
  `qadar_qirqish`, `qator_son`, `belgi_almashtirish`, `bosh_belgi` /
  `oxir_belgi`, `katta_yokib` / `kichik_yokib`, `formatlash_indeksli`,
  `oxir_topish`, `bir_marta_ajratish`.
- **UTF-8 yordamchilari** (`belgilar_soni`, `belgi_pozitsiyasi`,
  `chapdan_belgi_bo'yicha`, `qism_matn_belgi_bo'yicha`, `belgi_olish`,
  `belgi_uzunligi`) — kirill/o'zbek matnlari uchun kritik.

### Til — `ulash "*.uzpp"`
- Uz++ fayllarni boshqa uz++ fayldan to'g'ridan-to'g'ri `ulash` qilish
  mumkin. Transpilator rekursiv ravishda `.uzpp` ni `.hpp` ga aylantiradi
  (header-mode) va build papkasiga joylashtiradi.
- Header-mode: `#pragma once` + `#ifndef UZPP_GEN_<NAME>_HPP_` guard.
- `manba_joyi` — `std::source_location` ning o'zbekcha aliasi.

### LSP
- **Inlay hints**: `o'zgaruvchan x = ...` dan keyin xulosa qilingan tur ko'rsatiladi.
- **Code actions**: `unused variable` warning uchun ikkita quick-fix:
  `_` prefiks qo'shish yoki e'lonni o'chirish.

### Codegen tuzatishlar
- Char literal default qiymatlar (`' '`) endi mangle qilinmaydi.
- `(pos = s.find(...)) != npos` formulasi to'g'ri qavslar bilan
  emit qilinadi — avval cheksiz tsikl yaratardi.
- Error remap: g++ xato xabarlaridagi `.cpp:N:M` joylar `.uzpp` ga
  qaytariladi (`#line` direktivalari orqali).

### Sinov natijasi
- Local regression: **68/68** (avval 64/64).


## [2.1.1] — 2026-05-06

### VS Code Extension
- MinGW WinLibs GCC 14.2.0 UCRT64 dan to'g'ridan-to'g'ri yuklanadi
- GitHub Release `uzpp-windows-x64.zip` chop etildi (`uzpp.exe` + `stdlib/`)
- Extension v2.1.1 Marketplace'da

## [2.1.0] — 2026-05-06

### VS Code Extension — Zero-friction o'rnatish
- **One-Click Install**: Welcome ekranida bir tugma — barcha komponentlar yuklanadi
- 7 ta yangi buyruq: `F5` ishga tushirish, `Ctrl+F5` qurish, yangi loyiha, C++ kodi ko'rish va boshqalar
- Status bar: kompilyator versiyasi va holati
- Offline o'rnatish: `uz++ fayldan o'rnatish` buyrug'i
- `componentManager.js`: komponentlarni yuklash, o'rnatish, tekshirish
- Wrapper script (`uzpp-run.bat`): MinGW PATH va stdlib CWD ni avtomatik sozlaydi

### Kompilyator / Transpayler
- `StatementList` AST node: vergul bilan ajratilgan o'zgaruvchi e'lonlari
- TypeChecker: `ushlash` blokida o'zgaruvchi ro'yxatdan o'tkaziladi
- `va`/`yoki` mantiqiy operatorlar to'g'ri tahlil qilinadi

### Misollar
- `examples/` papkasida 10 ta to'liq ishlaydigan misol
- CI/CD: `examples/` integratsion testlari qo'shildi

## [2.0.0] — 2025-11-01

### VS Code Extension
- LSP (Language Server Protocol) integratsiyasi
- Real-vaqt diagnostika — yozayotganda xatolar ko'rsatiladi
- Avtoto'ldirish qo'shildi

### Kompilyator
- C++23 transpilatsiya mexanizmi
- Type checker semantik tahlil bilan
- DAP (Debug Adapter Protocol) serveri
- `uzpp lsp`, `uzpp dap` serverlari
- Paket menejeri (`uzpp ornatish`)

## [1.0.0] — 2025-06-01

### Birinchi chiqarilish
- Asosiy sintaksis: o'zgaruvchilar, funksiyalar, sinflar, tsikllar
- Leksik va sintaktik tahlilchi
- Kod generatori (C++ chiqarish)
- VS Code sintaksis yorqinligi
