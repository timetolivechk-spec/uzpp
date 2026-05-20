# uz++ ga hissa qo'shish

uz++ loyihasiga xush kelibsiz. Har qanday hissa — xato tuzatish, yangi
xususiyat, hujjat yoki tarjima — qadrlanadi.

## Loyihani qurish

**Talablar:**
- CMake 3.20+
- GCC 15+ yoki Clang 18+ (C++23 qo'llab-quvvatlashi kerak)
- Ninja (tavsiya etiladi)
- Windows: MSYS2/UCRT64 muhiti

```bash
# 1. Nusxa olish
git clone https://github.com/timetolivechk-spec/uzpp
cd uzpp

# 2. Qurish
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build

# 3. To'liq test to'plamini ishga tushirish (PR yuborishdan oldin)
./build/uzpp --version
./build/uzpp_frontend_tests                                     # frontend smoke
for f in tests/*.uzpp; do ./build/uzpp qurish "$f"; done        # 75 pozitiv
bash tests/negative/run.sh ./build/uzpp                         # 51 salbiy

# 4. Misol dasturni ishga tushirish
./build/uzpp ishga-tushirish misollar/01_salom_dunyo.uzpp
```

## Loyiha tuzilmasi

```
uz++/
├── src/
│   ├── lexer.{h,cpp}          # `Tokenizer` — kalit so'zlarni aniqlaydi
│   ├── parser.{h,cpp}         # `AST` quruvchi
│   ├── ast.h                  # Abstrakt sintaksis daraxti turlari
│   ├── type_checker.hpp       # Semantik tahlil, tur xulosasi
│   ├── codegen.{h,cpp}        # `AST` → C++23 generatori
│   ├── lsp_server.{h,cpp}     # `LSP` server (editor integratsiyasi)
│   └── main.cpp               # CLI: `qurish`, `ishga-tushirish`, `lsp`
├── stdlib/                    # Standart kutubxona (6 ta self-hosted + 25 qo'lda)
├── tests/                     # Regression testlar (75 pozitiv + 51 salbiy)
├── misollar/                  # 15 ta tayyor dastur (foydalanuvchi uchun)
├── vscode-uzpp/               # VS Code kengaytmasi (TypeScript)
├── installer/                 # Windows Inno Setup + Unix install.sh
└── .github/workflows/         # CI/CD (Windows MSYS2 + Linux + macOS)
```

## Kompilyator quvuri

`Lexer` → `Parser` → `TypeChecker` → `CodeGen` → `g++`

## Test darajalari

PR yuborishdan oldin uchchala daraja yashil bo'lishi shart:

1. **`tests/*.uzpp`** — integratsiya testlari. Har bir yangi til xususiyati
   uchun bittadan qo'shing.
2. **`tests/negative/*.uzpp`** — kompilyator yomon kodni rad etishini
   tasdiqlovchi testlar. Birinchi qatorda `// XATOLIK_KUTILMOQDA: <kategoriya>`
   qoldiring. `bash tests/negative/run.sh` har birini ishga tushirib,
   `exit != 0` qaytarishini tekshiradi.
3. **`tests/frontend_smoke.cpp`** — `Lexer`/`Parser`/`TypeChecker`/`CodeGen`
   ichki invariantlari. Yangi `assert` blokini qo'shing va
   `cmake --build` ni qayta ishga tushiring.

## Kod uslubi

**C++ (kompilyator kodi):**
- `snake_case` o'zgaruvchilar va funksiyalar uchun
- `PascalCase` sinflar uchun
- Sharh faqat noaniq joylarda — "nima qilyapti" emas, "nima uchun"
- `auto` ni tur xulosasi xavfsiz bo'lganda ishlatish mumkin
- Diagnostikalar Aniq × Aniq qiyoslashda chiqishi kerak (Phase 1+2 keynisini ko'ring)

**uz++ (til misollari):**
- Kalit so'zlar faqat o'zbekcha
- Misollar to'liq va ishlaydigan bo'lishi kerak
- Sharhlar o'zbek tilida
- Har bir C++ tushunchasi uchun bitta uz++ so'z (sinonim qo'shmang)

## PR jarayoni

1. `main` dan yangi tarmoq yarating:
   `git checkout -b feature/mening-xususiyatim`
2. O'zgarishlaringizni kiriting (testlar bilan)
3. Barcha testlar o'tishiga ishonch hosil qiling
4. PR yuboring va `.github/PULL_REQUEST_TEMPLATE.md` shablonini to'ldiring

## Xato topildi?

[GitHub Issues](https://github.com/timetolivechk-spec/uzpp/issues) orqali
xabarlang. Bug report shablonini to'ldiring.

## Litsenziya

Hissa qo'shsangiz, kodingiz MIT litsenziyasi ostida tarqatilishiga rozilik
bildirgan bo'lasiz.
