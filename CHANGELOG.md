# Changelog

Barcha muhim o'zgarishlar shu yerda hujjatlashtiriladi.

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
