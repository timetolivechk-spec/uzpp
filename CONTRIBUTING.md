# uz++ ga hissa qo'shish | Contributing to uz++

<div align="center">

**O'zbekcha** | [English](#english)

</div>

---

## O'zbekcha

uz++ loyihasiga xush kelibsiz! Har qanday hissa — xato tuzatish, yangi xususiyat, hujjat yoki tarjima — juda qadrlanadi.

### Loyihani qurish

**Talablar:**
- CMake 3.20+
- GCC 14+ yoki Clang 18+ (C++23 qo'llab-quvvatlashi kerak)
- Ninja (tavsiya etiladi)
- Windows: MSYS2/UCRT64 muhiti

```bash
# 1. Nusxa olish
git clone https://github.com/timetolivechk-spec/uzpp
cd uzpp

# 2. Qurish
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build

# 3. Tekshirish
./build/uzpp --version
./build/uzpp_frontend_tests   # birlik testlar

# 4. Misol dasturni ishga tushirish
./build/uzpp ishga-tushirish misollar/01_salom_dunyo.uzpp
```

### Loyiha tuzilmasi

```
uz++/
├── src/
│   ├── lexer.h / lexer.cpp          # Tokenizator — kalit so'zlarni aniqlaydi
│   ├── parser.h / parser.cpp        # AST quruvchi — sintaksisni tahlil qiladi
│   ├── ast.h                        # Abstrakt sintaksis daraxti turlari
│   ├── codegen.h / codegen.cpp      # C++23 kod generatori
│   ├── type_checker.hpp             # Semantik tahlil va tur tekshiruvi
│   ├── formatter.h / formatter.cpp  # Kod formatlash
│   ├── lsp_server.h / lsp_server.cpp# Language Server Protocol
│   └── main.cpp                     # CLI kirish nuqtasi
├── stdlib/                          # uz++ standart kutubxonasi (header-only)
├── tests/                           # Integratsiya testlari (*.uzpp)
├── misollar/                        # 10 ta kanon misol
└── vscode-uzpp/                     # VSCode kengaytmasi
```

### Kompilyator arxitekturasi

```
.uzpp fayl
    ↓  Lexer (lexer.cpp)
Token oqimi
    ↓  Parser (parser.cpp)
AST (ast.h)
    ↓  TypeChecker (type_checker.hpp)
Tekshirilgan AST
    ↓  CodeGen (codegen.cpp)
.generated.cpp
    ↓  g++ / clang++
Ikkilik fayl
```

### Test yozish

Har bir yangi xususiyat uchun `tests/` papkasida test fayli yarating:

```bash
# Yangi test yaratish
cat > tests/test_mening_xususiyatim.uzpp << 'EOF'
ulash "uzpp_runtime.hpp"

butun asosiy() {
    // Xususiyatingizni bu yerda sinab ko'ring
    yozish << "Test muvaffaqiyatli!" << qator_oxiri;
    qaytarish 0;
}
EOF

# Testni ishga tushirish
./build/uzpp qurish tests/test_mening_xususiyatim.uzpp
```

### Kod uslubi

**C++ (kompilyator kodi):**
- `snake_case` o'zgaruvchilar va funksiyalar uchun
- `PascalCase` sinflar uchun
- Ko'rsatmalar faqat non-obvious kod uchun
- `auto` turni xavfsiz tarzda qo'llash mumkin bo'lsa

**uz++ (til misollari):**
- Kalit so'zlar faqat o'zbek tilida
- Misollar to'liq va ishlaydigan bo'lishi kerak
- Izohlar o'zbek tilida

### PR jarayoni

1. `main` dan yangi tarmoq yarating: `git checkout -b feature/mening-xususiyatim`
2. O'zgarishlaringizni kiriting
3. Barcha testlar o'tishiga ishonch hosil qiling: `./build/uzpp_frontend_tests`
4. Tegishli integratsiya testlari o'tishi kerak
5. PR yuboring — `.github/PULL_REQUEST_TEMPLATE.md` shablonini to'ldiring

### Xato topildi?

[GitHub Issues](https://github.com/timetolivechk-spec/uzpp/issues) orqali xabarlang. Bug report shablonini to'ldiring.

---

## English

Welcome to uz++! All contributions — bug fixes, features, documentation, or translations — are warmly appreciated.

### Building the project

**Requirements:**
- CMake 3.20+
- GCC 14+ or Clang 18+ (C++23 support required)
- Ninja (recommended)
- Windows: MSYS2/UCRT64 environment

```bash
# Clone
git clone https://github.com/timetolivechk-spec/uzpp
cd uzpp

# Build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build

# Test
./build/uzpp_frontend_tests
./build/uzpp ishga-tushirish misollar/01_salom_dunyo.uzpp
```

### Code style

**C++ (compiler source):**
- `snake_case` for variables and functions
- `PascalCase` for classes
- Comments only for non-obvious WHY, not WHAT
- Prefer `auto` where type is deducible

**uz++ (language examples):**
- All keywords must be Uzbek
- Examples must be complete and runnable
- Comments in Uzbek

### PR process

1. Branch from `main`: `git checkout -b feature/my-feature`
2. Make your changes with tests
3. Verify all tests pass: `./build/uzpp_frontend_tests`
4. Open a PR using the provided template

### Architecture overview

The compiler pipeline is: **Lexer → Parser → TypeChecker → CodeGen → g++**

Key files:
- `src/lexer.cpp` — tokenizes Uzbek source into tokens
- `src/parser.cpp` — builds typed AST from token stream
- `src/type_checker.hpp` — semantic analysis, scope tracking, type inference
- `src/codegen.cpp` — emits valid C++23 from AST nodes
- `src/main.cpp` — CLI: `uzpp qurish`, `uzpp ishga-tushirish`, `uzpp lsp`

### License

By contributing, you agree your contributions will be licensed under the MIT License.
