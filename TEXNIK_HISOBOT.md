# uz++ Dasturlash Tili — To'liq Texnik Hisobot

**Sana:** 2026-yil, 5-may  
**Versiya:** 2.0.0  
**Platforma:** Windows 11 / Linux (Ubuntu 24.04)  
**Kompilyator talabi:** GCC 13+ (Windows), GCC 14+ (Linux)

---

## Mundarija

1. [Loyiha haqida umumiy ma'lumot](#1-loyiha-haqida-umumiy-malumot)
2. [Arxitektura](#2-arxitektura)
3. [Komponentlar tavsifi](#3-komponentlar-tavsifi)
4. [Standart kutubxona](#4-standart-kutubxona)
5. [Kalit so'zlar jadvali](#5-kalit-sozlar-jadvali)
6. [Qurilish tizimi](#6-qurilish-tizimi)
7. [CI/CD (Uzluksiz integratsiya)](#7-cicd-uzluksiz-integratsiya)
8. [VSCode kengaytmasi](#8-vscode-kengaytmasi)
9. [Misollar to'plami](#9-misollar-toplami)
10. [Testlar](#10-testlar)
11. [Statistika va metrikalar](#11-statistika-va-metrikalar)
12. [Cheklovlar va ma'lum xatolar](#12-cheklovlar-va-malum-xatolar)
13. [Xulosa](#13-xulosa)

---

## 1. Loyiha haqida umumiy ma'lumot

**uz++** — O'zbek tilida yoziladigan kompilyatsiya tili. Barcha kalit so'zlar, operatorlar va standart kutubxona nomlari o'zbek tilida bo'lib, transpilatsiya orqali C++23 ga aylantiriladi va native ikkilik fayl hosil qilinadi.

### Maqsad

Dasturlash tilini o'zbek ona tilida o'rganish imkonini berish. uz++ to'liq C++23 imkoniyatlarini — shablonlar, lambdalar, asinxron hisoblash, ranges, konseptlar — o'zbek sintaksisi orqali qo'lga kiritadi.

### Transpilatsiya zanjiri

```
.uzpp fayl
    │
    ▼
Leksik tahlil (Lexer)
    │  Tokenlar oqimi
    ▼
Sintaktik tahlil (Parser)
    │  AST (Abstrakt Sintaksis Daraxti)
    ▼
Semantik tahlil (TypeChecker)
    │  Tekshirilgan AST + ogohlantirishlar
    ▼
Kod generatsiyasi (CodeGen)
    │  C++23 manba kodi
    ▼
GCC/Clang kompilyatori
    │
    ▼
Native ikkilik fayl (.exe / ELF)
```

### Asosiy xususiyatlar

| Xususiyat | Tavsif |
|-----------|--------|
| **Sintaksis** | 89+ o'zbek kalit so'zi |
| **Tezlik** | Native C++ tezligi (zero-overhead) |
| **Xotira** | To'liq C++ xotira boshqaruvi |
| **Xatolik boshqaruvi** | `Natija<T>` — Rust'dagi `Result<T,E>` analogi |
| **OOP** | Sinflar, meros, virtual metodlar, shablonlar |
| **Parallellik** | `std::async`, `OqimPool` (thread pool) |
| **LSP** | VS Code uchun Language Server Protocol |
| **Formatlash** | `uzpp format` buyrug'i |
| **Lint** | `uzpp tekshirish` buyrug'i |

---

## 2. Arxitektura

```
uz++/
├── src/                    # Kompilyator manba kodi (C++)
│   ├── lexer.cpp           # Leksik tahlilchi            (345  qator)
│   ├── lexer.h
│   ├── parser.cpp          # Sintaktik tahlilchi          (1996 qator)
│   ├── parser.h
│   ├── ast.h               # AST tugunlar ta'rifi
│   ├── codegen.cpp         # C++ kod generatori           (1586 qator)
│   ├── codegen.h
│   ├── type_checker.hpp    # Semantik tahlil              (622  qator)
│   ├── lsp_server.cpp      # Language Server Protocol     (727  qator)
│   ├── main.cpp            # CLI kirish nuqtasi           (1125 qator)
│   ├── formatter.cpp       # Kod formatlash               (624  qator)
│   ├── dap_server.cpp      # Debug Adapter Protocol       (310  qator)
│   └── docgen.hpp          # Hujjat generatori            (96   qator)
│
├── stdlib/                 # O'zbek standart kutubxonasi (header-only)
│   ├── uzpp_runtime.hpp    # Natija<T>, OqimPool, Tanlov (367  qator)
│   ├── matematika.hpp      # sin, cos, sqrt, faktorial   (312  qator)
│   ├── matn.hpp            # Matn ishlash funksiyalari   (275  qator)
│   └── json.hpp            # JSON parse/serializatsiya   (324  qator)
│
├── misollar/               # 10 ta namuna dastur
├── tests/                  # Birlik va integratsiya testlari
├── vscode-uzpp/            # VSCode kengaytmasi
└── .github/workflows/      # GitHub Actions CI
```

### Modul bog'liqliklari

```
main.cpp
  ├── lexer.cpp          (mustaqil)
  ├── parser.cpp         ← lexer.cpp
  ├── type_checker.hpp   ← parser.cpp (AST)
  ├── codegen.cpp        ← parser.cpp (AST)
  ├── formatter.cpp      ← lexer.cpp
  └── lsp_server.cpp     ← lexer.cpp, parser.cpp, type_checker.hpp
```

---

## 3. Komponentlar tavsifi

### 3.1 Lexer (leksik tahlilchi)

**Fayl:** `src/lexer.cpp` — 345 qator

Leksik tahlilchi manba matnini tokenlar ketma-ketligiga aylantiradi.

**Token turlari:**

| Token turi | Misol |
|------------|-------|
| `Identifier` | `asosiy`, `Shahs`, `lug'at` |
| `Integer` | `42`, `0xFF`, `0b1010` |
| `Float` | `3.14`, `2.71e8` |
| `StringLiteral` | `"Salom dunyo"` |
| `Symbol` | `{`, `}`, `<<`, `::`, `->` |
| `EndOfFile` | — |

**Muhim xususiyat:** Identifikatorlarda apostrof (`'`) ruxsat etilgan — `lug'at`, `yolg'on`, `og'ir`. `isIdentifierPart()` funksiyasi `'` va `` ` `` belgilarini identifikator tarkibida qabul qiladi.

```cpp
bool isIdentifierSuffix(char value) {
    return std::isalnum(value) || value == '_' || value == '\'' || value == '`';
}
```

### 3.2 Parser (sintaktik tahlilchi)

**Fayl:** `src/parser.cpp` — **1996 qator** (loyihadagi eng katta fayl)

Rekursiv-tushuvchi parser. Tokenlar ketma-ketligidan AST (Abstrakt Sintaksis Daraxti) quradi.

**Qo'llab-quvvatlanadigan konstruktsiyalar:**

| Konstruktsiya | uz++ sintaksisi | C++ | Holat |
|---------------|-----------------|-----|-------|
| O'zgaruvchi | `butun son = 42;` | `int son = 42;` | ✓ |
| Funksiya | `butun kvadrat(butun x) { ... }` | `int kvadrat(int x)` | ✓ |
| Shart | `agar (x > 0) { ... } aks_holda { ... }` | `if/else` | ✓ |
| `uchun` sikli | `uchun (butun i = 0; i < n; i++)` | `for` | ✓ |
| `holda` sikli | `holda (shart) { ... }` | `while` | ✓ |
| Sinf | `sinf Shahs { ochiq: ... };` | `class` | ✓ |
| Meros | `sinf Talaba : Shahs { ... }` | `: public` | ✓ |
| Konstruktor init-list | `Shahs(matn i) : ism(i) {}` | `: ism(i)` | ✓* |
| Shablon | `shablon <tur T> T funksiya(T x)` | `template<typename T>` | ✓ |
| Lambda | `[&](butun x) { qaytarish x*x; }` | same | ✓ |
| Lambda `&` params | `[](matn& a, matn& b) { ... }` | same | ✓* |
| Vektor literal | `[1, 2, 3]` → `std::vector{1,2,3}` | — | ✓ |
| `Natija<T>` | `uzpp::Natija<haqiqiy>::xato(...)` | `std::expected` | ✓ |
| Namespace | `nomlar_fazosi uzpp { ... }` | `namespace` | ✓ |
| Interfeys | `shartnoma IShahs { ... }` | abstract class | ✓ |
| Enum | `sanab_olish Rang { Qizil, Yashil }` | `enum class` | ✓ |
| `asinxron` | `asinxron butun hisob() { ... }` | `std::async` | ✓ |

> *— bu sessiyada tuzatildi (ilgari parser xatosi bor edi)

**AST tugunlari (ast.h):**

- `Program` — ildiz tugun
- `FunctionDeclaration` — funksiya ta'rifi
- `ClassDeclaration` — sinf ta'rifi (ichida `Member`, `Method` tuzilmalar)
- `VariableDeclaration` — o'zgaruvchi
- `IfStatement` / `ForStatement` / `WhileStatement`
- `ReturnStatement` / `ExpressionStatement`
- `Block` — `{ ... }` blok
- `BinaryExpression` / `UnaryExpression`
- `CallExpression` — funksiya chaqiruvi
- `LambdaExpression` — lambda
- `LiteralExpression` — son, matn, mantiqiy literal
- `IncludeStatement` — `ulash "fayl.hpp"`
- `NamespaceDeclaration`
- `InterfaceDeclaration`

### 3.3 TypeChecker (semantik tahlilchi)

**Fayl:** `src/type_checker.hpp` — 622 qator

AST ustida ishlaydi va quyidagilarni tekshiradi:

| Tekshiruv | Misol |
|-----------|-------|
| O'zgaruvchi e'lon qilinganmi | `noma'lum o'zgaruvchi` ogohlantirishlar |
| Qaytarish turi | `butun` funksiya `mantiqiy` qaytarsa — ogohlantirish |
| Erişilmas kod | `qaytarish`dan keyin kod — ogohlantirish |
| Ishlatilmagan o'zgaruvchi | `butun x = 5;` — ogohlantirish |
| Noma'lum o'zgaruvchi ishlatish | `y = x + 1;` (y e'lon qilinmagan) |

Xatoliklar **rang-barang terminal formati** bilan chiqariladi:
```
demo.uzpp:5:10: ogohlantirish: O'zgaruvchi 'x' e'lon qilingan, lekin ishlatilmagan.
  5 |     butun x = 42;
    |          ^
```

### 3.4 CodeGen (kod generatori)

**Fayl:** `src/codegen.cpp` — 1586 qator

AST'ni C++23 manba kodiga aylantiradi.

**Preamble (har bir fayl boshiga qo'shiladi):**
```cpp
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <memory>
#include <future>
#include <thread>
#include <mutex>
#include "../stdlib/uzpp_runtime.hpp"
// ...

int main() { return _uzpp_user_main(); }
int _uzpp_user_main() { /* foydalanuvchi kodi */ }
```

**Maxsus transformatsiyalar:**

1. **Vektor literali:** `[1, 2, 3]` → `std::vector{1, 2, 3}`
2. **Matn vektori:** `["a", "b"]` → `std::vector<std::string>{"a", "b"}` (CTAD xatosini oldini oladi)
3. **Identifikator tarjimasi:** ikki xaritada (funksiyalar va umumiy)
4. **Konstruktor init-list:** `ism(n)` → `ism(n)` (to'g'ridan-to'g'ri uzatiladi)

### 3.5 Formatter

**Fayl:** `src/formatter.cpp` — 624 qator

`uzpp format fayl.uzpp` buyrug'i bilan ishga tushiriladi. Tokenlarni qayta o'qib, standart uz++ uslubida formatlash amalga oshiriladi.

### 3.6 LSP Server

**Fayl:** `src/lsp_server.cpp` — 727 qator

JSON-RPC 2.0 protokoli orqali VS Code bilan muloqot qiladi.

**Qo'llab-quvvatlanadigan LSP metodlari:**

| Metod | Funksiya |
|-------|---------|
| `initialize` | LSP ulanishni boshlash |
| `textDocument/didOpen` | Fayl ochilganda tahlil |
| `textDocument/didChange` | O'zgartirilganda qayta tahlil |
| `textDocument/didSave` | Saqlanganda diagnostika |
| `textDocument/publishDiagnostics` | Ogohlantirishlarni yuborish |
| `textDocument/hover` | Kalit so'z ustiga sichqon bosganda izoh |
| `textDocument/completion` | Avtoto'ldirish (kalit so'zlar) |

### 3.7 CLI (main.cpp)

**Fayl:** `src/main.cpp` — 1125 qator

**Buyruqlar:**

| Buyruq | Tavsif |
|--------|--------|
| `uzpp ishga-tushirish fayl.uzpp` | Kompilyatsiya va ishga tushirish |
| `uzpp transpile fayl.uzpp` | Faqat C++ ga transpilatsiya |
| `uzpp transpile --show-cpp fayl.uzpp` | uz++ va C++ yonma-yon ko'rsatish |
| `uzpp tekshirish fayl.uzpp` | Faqat lint/type-check |
| `uzpp format fayl.uzpp` | Kodni formatlash |
| `uzpp lsp` | LSP serverini ishga tushirish |
| `uzpp --version` | Versiyani ko'rsatish |
| `uzpp --yordam` | Yordam ma'lumoti |

---

## 4. Standart kutubxona

Barcha stdlib fayllari **header-only** (`.hpp`) formatida — alohida kompilyatsiya talab qilinmaydi.

### 4.1 uzpp_runtime.hpp (367 qator)

**`uzpp::Natija<T>`** — Rust'dagi `Result<T,E>` yoki C++23 `std::expected<T,E>` analogi:

```uzpp
uzpp::Natija<haqiqiy> bolish(haqiqiy a, haqiqiy b) {
    agar (b == 0.0) {
        qaytarish uzpp::Natija<haqiqiy>::xato("Nolga bolish mumkin emas!");
    }
    qaytarish uzpp::Natija<haqiqiy>::muvaffaqiyat(a / b);
}

ozgaruvchan r = bolish(10.0, 3.0);
agar (r.yaroqliMi()) {
    yozish << r.qiymat() << qator_oxiri;   // 3.33333
} aks_holda {
    yozish << r.xatoMazmun() << qator_oxiri;
}
```

**`uzpp::OqimPool`** — Thread pool (ishchi oqimlar havzasi):

```uzpp
uzpp::OqimPool havza(4);           // 4 ta ishchi oqim
havza.vazifaQosh([i]() {
    yozish << "Vazifa #" << i << qator_oxiri;
});
havza.toxtatish();                 // barcha vazifalar tugashini kutish
```

**`uzpp::Tanlov<T...>`** — `std::variant` analogi (bir necha turdan biri).

**VazifaJavob** — asinxron vazifadan natija olish uchun.

### 4.2 matematika.hpp (312 qator)

`namespace uzpp::Matematika` ichida:

| Funksiya | C++ analogi | Tavsif |
|----------|-------------|--------|
| `ildiz(x)` | `std::sqrt` | Kvadrat ildiz |
| `kub_ildiz(x)` | `std::cbrt` | Kub ildiz |
| `quvvat(a, b)` | `std::pow` | Daraja |
| `mutlaq(x)` | `std::abs` | Mutlaq qiymat |
| `sinus(x)` | `std::sin` | Sinus |
| `kosinus(x)` | `std::cos` | Kosinus |
| `tangens(x)` | `std::tan` | Tangens |
| `log(x)` | `std::log` | Tabiiy logarifm |
| `log10(x)` | `std::log10` | O'nli logarifm |
| `yaxlitla(x)` | `std::round` | Yaxlitlash |
| `pastga(x)` | `std::floor` | Pastga yaxlitlash |
| `tepaga(x)` | `std::ceil` | Tepaga yaxlitlash |
| `oʻrta(v)` | — | Vektorning o'rtachasi |
| `faktorial(n)` | — | n! |
| `kombinatsiya(n,k)` | — | C(n,k) binomial koeffitsient |
| `umumiy_boluv(a,b)` | `std::gcd` | EKUB |
| `umumiy_karrali(a,b)` | `std::lcm` | EKUK |

### 4.3 matn.hpp (275 qator)

`namespace uzpp::Matn` ichida:

| Funksiya | Tavsif |
|----------|--------|
| `ajratish(m, ajr)` | Matni ajratuvchi bo'yicha bo'lish |
| `qatorlarga_ajratish(m)` | Qatorlarga bo'lish |
| `birlashtirish(v, biriktiruvchi)` | Elementlarni birlashtirish |
| `boshlanganda(m, prefiks)` | Boshlanishini tekshirish |
| `tugaganda(m, sufiks)` | Tugashini tekshirish |
| `ichida(m, qidiriluvchi)` | Ichida borligini tekshirish |
| `topish(m, naqsh)` | Pozitsiyani topish |
| `almashtirish(m, eski, yangi)` | Barchasini almashtirish |
| `kichik_harfga(m)` | Kichik harfga o'tkazish |
| `katta_harfga(m)` | Katta harfga o'tkazish |
| `chapdan_qirqish(m)` | Chap bo'shliqlarni olib tashlash |
| `ongdan_qirqish(m)` | O'ng bo'shliqlarni olib tashlash |
| `qirqish(m)` | Ikkala tomondagi bo'shliqlarni olib tashlash |
| `raqammi(m)` | Raqam ekanligini tekshirish |
| `butun_songa(m)` | Matni butun songa aylantirish |
| `haqiqiy_songa(m)` | Matni kasr songa aylantirish |
| `RegEx` | Muntazam ifodalar (regex) |

### 4.4 json.hpp (324 qator)

```uzpp
// JSON yaratish
ozgaruvchan ob = uzpp::Json::ob();
ob->belgi("ism", "Ali");
ob->son("yosh", 30);

// JSON parse qilish
ozgaruvchan tuzilma = uzpp::Json::parse(matn_json);
ozgaruvchan ism = tuzilma->maydon("ism")->matn();
ozgaruvchan yosh = tuzilma->maydon("yosh")->butun();
```

---

## 5. Kalit so'zlar jadvali

Loyihada **89 ta** o'zbek → C++ tarjima mavjud. Asosiylaridan:

### Turlar (Types)

| uz++ | C++ | Ma'no |
|------|-----|-------|
| `butun` | `int` | Butun son |
| `haqiqiy` | `double` | Kasr son (64-bit) |
| `haqiqiy32` | `float` | Kasr son (32-bit) |
| `matn` | `std::string` | Matn |
| `mantiqiy` | `bool` | Mantiqiy qiymat |
| `bosh` | `void` | Qaytarish turi yo'q |
| `uzun` | `long` | Uzun butun son |
| `belgi` | `char` | Bitta belgi |
| `ulkan` | `std::size_t` | Katta hajm turi |

### Kolleksiyalar

| uz++ | C++ | Ma'no |
|------|-----|-------|
| `vektor<T>` | `std::vector<T>` | O'lchanadigan massiv |
| `lug'at<K,V>` | `std::unordered_map<K,V>` | Xesh-jadval |
| `tartiblangan_xarita<K,V>` | `std::map<K,V>` | Tartiblangan jadval |
| `o'ziga_xos<T>` | `std::set<T>` | Noyob elementlar to'plami |
| `juftlik<A,B>` | `std::pair<A,B>` | Juftlik |
| `uchlik<A,B,C>` | `std::tuple<A,B,C>` | Uchlik |
| `ixtiyoriy<T>` | `std::optional<T>` | Ixtiyoriy qiymat |

### Boshqaruv oqimi

| uz++ | C++ | Ma'no |
|------|-----|-------|
| `agar` | `if` | Shart |
| `aks_holda` | `else` | Aks holda |
| `uchun` | `for` | Tsikl (for) |
| `holda` | `while` | Tsikl (while) |
| `qaytarish` | `return` | Qaytarish |
| `davom` | `continue` | Davom etish |
| `tuxta` | `break` | To'xtatish |
| `urinish` | `try` | Urinib ko'rish |
| `ushlash` | `catch` | Xatolikni ushlash |
| `irgitish` | `throw` | Xatolik otish |

### OOP

| uz++ | C++ | Ma'no |
|------|-----|-------|
| `sinf` | `class` | Sinf |
| `tuzilma` | `struct` | Tuzilma |
| `shablon` | `template` | Shablon |
| `tur` | `typename` | Tur parametri |
| `ochiq` | `public` | Ochiq bo'lim |
| `yopiq` | `private` | Yopiq bo'lim |
| `himoyalangan` | `protected` | Himoyalangan bo'lim |
| `mavhum` | `virtual` | Virtual metod |
| `ustidan_yozish` | `override` | Qayta aniqlash |
| `statik` | `static` | Statik |

### Xotira boshqaruvi

| uz++ | C++ | Ma'no |
|------|-----|-------|
| `yagona<T>` | `std::unique_ptr<T>` | Yagona ko'rsatkich |
| `umumiy<T>` | `std::shared_ptr<T>` | Umumiy ko'rsatkich |
| `kuchsiz_korsatkich<T>` | `std::weak_ptr<T>` | Kuchsiz ko'rsatkich |
| `yangi_yagona<T>(...)` | `std::make_unique<T>(...)` | Yagona hosil qilish |
| `yangi_umumiy<T>(...)` | `std::make_shared<T>(...)` | Umumiy hosil qilish |

### I/U (Kirish/Chiqish)

| uz++ | C++ | Ma'no |
|------|-----|-------|
| `yozish` | `std::cout` | Konsolga yozish |
| `kiritish` | `std::cin` | Konsoldan o'qish |
| `qator_oxiri` | `std::endl` | Yangi qator |

### Oldindan qayta ishlash

| uz++ | C++ | Ma'no |
|------|-----|-------|
| `ulash "fayl.hpp"` | `#include "fayl.hpp"` | Fayl ulash |
| `makro NOMI = ...` | `#define NOMI ...` | Makros |
| `nomlar_fazosi` | `namespace` | Nomlar fazosi |

---

## 6. Qurilish tizimi

**Fayl:** `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.20)
project(uzpp CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

### Maqsadlar (Targets)

| Maqsad | Tur | Tavsif |
|--------|-----|--------|
| `uzpp_frontend` | Static library | Lexer, Parser, TypeChecker, CodeGen, Formatter, LSP |
| `uzpp` | Executable | CLI kompilyator |
| `uzpp_frontend_tests` | Executable | Birlik testlari |

### Qurilish jarayoni

```bash
# Release rejimida qurilish
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build

# Debug rejimida qurilish
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -B build-debug
cmake --build build-debug
```

### Platform bog'liqliklari

| Platform | Kutubxonalar |
|----------|-------------|
| Windows | `-static`, `ws2_32`, `user32`, `gdi32`, `opengl32` |
| Linux/macOS | `X11`, `GL` (ixtiyoriy) |

### Kutubxona talablari

| Vosita | Minimal versiya |
|--------|----------------|
| CMake | 3.20+ |
| GCC (Windows/MSYS2) | 13+ |
| GCC (Linux) | 14+ |
| Clang (Linux/macOS) | 18+ |
| Ninja | istalgan |

---

## 7. CI/CD (Uzluksiz integratsiya)

**Fayl:** `.github/workflows/ci.yml`

### Ishga tushirish shartlari

| Hodisa | Tarmoqlar |
|--------|----------|
| `push` | `main`, `master`, `dev` |
| `pull_request` | `main`, `master` |

### Windows ish jarayoni

```yaml
Platform: windows-latest (MSYS2/UCRT64)
Compiler: GCC (g++)
Steps:
  1. MSYS2 o'rnatish
  2. cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B build
  3. cmake --build build
  4. C++ unit testlarni ishga tushirish
  5. 7 ta .uzpp misolni sinash
  6. Artefakt yuklash: uzpp-windows-x64 (uzpp.exe)
```

### Linux ish jarayoni

```yaml
Platform: ubuntu-24.04
Compiler: GCC-14
Steps:
  1. sudo apt-get install g++-14 cmake ninja-build
  2. cmake -DCMAKE_CXX_COMPILER=g++-14 -G Ninja -B build
  3. cmake --build build
  4. C++ unit testlarni ishga tushirish
  5. 7 ta .uzpp misolni sinash
  6. Artefakt yuklash: uzpp-linux-x64
```

### Sinab ko'rilgan misollar (CI da)

1. `01_salom_dunyo.uzpp`
2. `02_fizzbuzz.uzpp`
3. `03_rekursiya.uzpp`
4. `04_sinflar.uzpp`
5. `05_kolleksiyalar.uzpp`
6. `06_xatolik_boshqaruvi.uzpp`
7. `07_matn_ishlash.uzpp`

---

## 8. VSCode kengaytmasi

**Joylashuv:** `vscode-uzpp/`

### Fayllar

| Fayl | Qator | Tavsif |
|------|-------|--------|
| `extension.js` | 97 | LSP mijoz, buyruqlar |
| `syntaxes/uzpp.tmLanguage.json` | 210 | TextMate grammatikasi |
| `language-configuration.json` | 30 | Til konfiguratsiyasi |
| `package.json` | 59 | Kengaytma manifestasi |

### Sintaksis podsvеtka (TextMate grammatika)

Joriy sessiyada qayta yozilgan grammatika quyidagi elementlarni ajratib ko'rsatadi:

| Element | Rang (mavzu bog'liq) | TextMate scope |
|---------|---------------------|----------------|
| `agar`, `qaytarish`, `uchun` | Ko'k (control) | `keyword.control.uzpp` |
| `sinf`, `tuzilma`, `shablon` | Ko'k (declaration) | `storage.type.class.uzpp` |
| `ochiq`, `ozgaruvchan` | Ko'k (modifier) | `storage.modifier.uzpp` |
| `butun`, `haqiqiy`, `matn`, `vektor` | Yashil/ko'k | `support.type.uzpp` |
| `rost`, `yolg'on` | Ko'k | `constant.language.boolean.uzpp` |
| `asosiy(...)`, `tanishtir()` | Sariq | `entity.name.function.uzpp` |
| `Shahs`, `Talaba` (sinf nomi) | Ko'k-yashil | `entity.name.type.uzpp` |
| `uzpp`, `std` (namespace) | Qoʻshimcha rang | `entity.name.namespace.uzpp` |
| `"matn"` | To'q sariq | `string.quoted.double.uzpp` |
| `// izoh` | Kulrang/yashil | `comment.line.double-slash.uzpp` |
| Sonlar | Yashil/to'q sariq | `constant.numeric.uzpp` |
| `==`, `!=`, `->`, `::` | Oq | `keyword.operator.uzpp` |

**Muhim tuzatish:** Oldingi grammatikada apostrof (`'`) matn belgisi sifatida belgilangan edi — bu `lug'at`, `yolg'on` kabi identifikatorlarni noto'g'ri rangga bo'yardi. Yangi grammatikada bu qoida olib tashlandi.

### LSP (Language Server Protocol)

**Server topish ustuvorligi (4 bosqich):**

1. Kengaytma ichiga joylashtirilgan `uzpp.exe` (agar mavjud bo'lsa)
2. Foydalanuvchi sozlamasi: `uzpp.serverPath`
3. Ish joy papkasida `uzpp.exe`
4. Tizim `PATH`idan qidirish

**Buyruqlar:**
- `uzpp.restartServer` — LSP serverni qayta ishga tushirish

---

## 9. Misollar to'plami

**Joylashuv:** `misollar/`

| # | Fayl | Qator | Ko'rsatiladigan mavzu |
|---|------|-------|----------------------|
| 01 | `01_salom_dunyo.uzpp` | 6 | "Salom, Dunyo!" — birinchi dastur |
| 02 | `02_fizzbuzz.uzpp` | 16 | FizzBuzz — shart va tsikl |
| 03 | `03_rekursiya.uzpp` | 24 | Rekursiya: Fibonacci, faktorial |
| 04 | `04_sinflar.uzpp` | 48 | OOP: sinflar va meros |
| 05 | `05_kolleksiyalar.uzpp` | 37 | Vektor, lug'at, lambda tartiblash |
| 06 | `06_xatolik_boshqaruvi.uzpp` | 41 | `Natija<T>` bilan xatolik boshqaruvi |
| 07 | `07_matn_ishlash.uzpp` | 38 | Matn funksiyalari (`matn.hpp`) |
| 08 | `08_json_ishlash.uzpp` | 41 | JSON parse va yaratish |
| 09 | `09_kop_oqimlilik.uzpp` | 47 | Parallel hisoblash, `OqimPool` |
| 10 | `10_ilgor_dasturlash.uzpp` | 84 | Generiklar, lambdalar, funksional |

### Katta demo dastur

**`demo_tizim.uzpp`** — 175 qator, 10 bo'lim:

```
====================================
   uz++ DEMO DASTURI
====================================
--- 1. OOP va Meros ---
Shahs: Ali, 30 yosh
Talaba: Vali (Informatika), 22 yosh

--- 2. Shablon funksiyalar ---
Tartiblanmagan: [ 5 3 8 1 9 2 7 4 6 ]
Tartiblangan  : [ 1 2 3 4 5 6 7 8 9 ]
Yigindi: 45

--- 3. Lambda va sort ---
Tartiblanmagan: [ Zafar Malika Bobur Aziz ]
Tartiblangan  : [ Aziz Bobur Malika Zafar ]

--- 4. Xatolik boshqaruvi (Natija<T>) ---
10 / 3 = 3.33333
Xato: Nolga bolish mumkin emas!

--- 5. Lugat (unordered_map) ---
Ali yoshi: 30  |  Malika yoshi: 25

--- 6. Matematika kutubxonasi ---
sqrt(4)=2  sqrt(9)=3  sqrt(16)=4  sqrt(25)=5

--- 7. Fibonacci rekursiyasi ---
fib(0)=0  fib(1)=1  ...  fib(9)=34

--- 8. Matn ishlash ---
Tozalangan: [salom dunyo uz++]
Bosh harf:  [  SALOM DUNYO UZ++  ]

--- 9. Parallel hisoblash (std::async) ---
1..100 yigindisi: 5050
1..50 kvadratlari yigindisi: 42925

--- 10. OqimPool (Thread Pool) ---
Vazifa #1 bajarildi  ...  Vazifa #5 bajarildi
====================================
   DEMO MUVAFFAQIYATLI YAKUNLANDI!
====================================
```

---

## 10. Testlar

**Joylashuv:** `tests/`

### C++ birlik testlari

**`frontend_smoke.cpp`** (135 qator) — `uzpp_frontend` kutubxonasini bevosita sinaydi:
- Lexer: tokenlar to'g'ri ajratiladimi
- Parser: AST to'g'ri quriladimi
- TypeChecker: ogohlantirishlar chiqadilatmi
- CodeGen: C++ kodi to'g'ri generiladimi

### uz++ integratsiya testlari

| Fayl | Sinayotgan mavzu |
|------|-----------------|
| `test_api.uzpp` | HTTP API ulanish |
| `test_asinxron.uzpp` | `asinxron` funksiyalar |
| `test_baza.uzpp` | Ma'lumotlar bazasi |
| `test_math.uzpp` | `matematika.hpp` funksiyalar |
| `test_modern.uzpp` | Zamonaviy C++23 xususiyatlar |
| `test_modules.uzpp` | Modullar tizimi |
| `test_reaktiv.uzpp` | Reaktiv dasturlash |
| `test_shartnoma.uzpp` | Interfeys (shartnoma) |
| `test_threads.uzpp` | Ko'p oqimlilik |
| `test_yangi_imkoniyatlar.uzpp` | Yangi xususiyatlar (112 qator) |

---

## 11. Statistika va metrikalar

### Manba kodi hajmi

| Kategoriya | Fayllar | Qatorlar |
|------------|---------|---------|
| **Kompilyator** (`src/`) | 9 ta | **7 431** |
| **Standart kutubxona** (`stdlib/`) | 23 ta | **5 149** |
| **Misollar** (`misollar/`) | 10 ta | **517** |
| **Testlar** (`tests/`) | 13 ta | **505** |
| **VSCode kengaytmasi** | 4 ta | **393** |
| **Jami** | — | **~14 000** |

### Kompilyator komponentlari

| Komponent | Qatorlar | Ulushi |
|-----------|---------|--------|
| Parser | 1 996 | 26.8% |
| CodeGen | 1 586 | 21.3% |
| main.cpp (CLI) | 1 125 | 15.1% |
| LSP Server | 727 | 9.8% |
| TypeChecker | 622 | 8.4% |
| Formatter | 624 | 8.4% |
| Lexer | 345 | 4.6% |
| DAP Server | 310 | 4.2% |
| DocGen | 96 | 1.3% |

### Kalit so'zlar

| Kategoriya | Soni |
|------------|-----|
| Jami tarjima (uz++ → C++) | **89** |
| Boshqaruv oqimi kalit so'zlari | 12 |
| Tur kalit so'zlari | 18 |
| OOP kalit so'zlari | 10 |
| Kutubxona funksiyalari | 49+ |

### Ishlash ko'rsatkichlari

| Ko'rsatkich | Qiymat |
|-------------|--------|
| `01_salom_dunyo.uzpp` kompilyatsiya vaqti | ~2 s (birinchi marta) |
| Qayta kompilyatsiya (o'zgarmagan) | ~0.5 s |
| `demo_tizim.uzpp` (175 qator) kompilyatsiya | ~4-6 s |
| Thread pool (5 vazifa, 3 oqim) | parallel ijro |
| `std::async` parallel natija | deterministik |

---

## 12. Cheklovlar va ma'lum xatolar

Ushbu sessiyada quyidagi **3 ta parser xatosi** topildi va tuzatildi:

### ✅ Tuzatildi: Konstruktor initializer list

**Muammo:** `Shahs(matn n) : ism(n) {}` — parser `:` belgisini qayta ishlamas edi, natijada keyingi `}` (konstruktor tanasini yopuvchi) sinf tanasini yopuvchi sifatida talqin qilinib, qolgan metodlar global darajaga chiqib ketardi.

**Tuzatish:** `parseFunctionParameters()` dan keyin `:` topilsa, `{` yoki `;` gacha barcha tokenlarni `initializerList` satriga yig'ib, kod generatsiyasida `method->returnType.empty()` holatida chiqarish.

### ✅ Tuzatildi: Lambda parametrlarida `&`

**Muammo:** `[](matn& a, matn& b) { ... }` — lambda parametrini parse qilishda `p.type = advance().value` faqat bitta tokenni (`matn`) olardi, `&` tokeni qolardi. Keyingi iteratsiyada `&` uchun hech qanday branch topilmay, cheksiz tsikl (`std::bad_alloc`) yuzaga kelardi.

**Tuzatish:** Lambda parametrini o'qishda `advance().value` o'rniga to'liq `parseTypeString` mantiq (namespace `::`, template `<>`, `&`/`*`/`&&`) qo'llanildi. Tanib bo'lmagan token uchun `advance()` saqlash mexanizmi qo'shildi.

### ✅ Tuzatildi: Voris sinfda noto'g'ri `override`

**Muammo:** Agar sinf `sinf A : B` shaklida baza sinfi bor bo'lsa, `visitClassDeclaration` barcha metodlarga — konstruktor va destruktorlar jumladan — `override` qo'shar edi. Bu C++ da kompilyatsiya xatosi berardi.

**Tuzatish:** `method->isVirtual` faqat `true` bo'lganda `override` chiqariladi. Konstruktor/destruktorlar uchun bu maydon hech qachon o'rnatilmaydi.

### ⚠️ Mavjud cheklovlar (tuzatilmagan)

| Cheklov | Tavsif |
|---------|--------|
| Template lookahead | `agar (x < 0.0 \|\| x > 100.0)` — `x<...>` ni shablon sifatida talqin qilishi mumkin. Yechim: ifodani bo'lish |
| C-uslub castlar | `(butun)x` — parse qilinmaydi. Yechim: `static_cast<butun>(x)` ishlatish |
| Ko'p qatorli matn | `"..."` `"..."` — qatorlar birlashtirilmaydi, alohida iboralar sifatida ko'riladi |
| Ichma-ich `>>` | `vektor<juft<butun, butun>>` — `>>` operator sifatida talqin etilishi mumkin |
| TypeChecker (sinflar) | Sinf a'zolari TypeChecker ixtisoslashtiruvchiga hali ulangan emas — "noma'lum o'zgaruvchi" ogohlantirishlari chiqadi (lekin kompilyatsiya ishlaydi) |

---

## 13. Xulosa

**uz++** — C++23 asosida qurilgan, to'liq funksional transpilatsiya tili. Loyiha quyidagi asosiy maqsadlarga erishdi:

### Erishilgan natijalar

| # | Natija |
|---|--------|
| 1 | **89+ kalit so'z** bilan to'liq o'zbek sintaksisi |
| 2 | **10 ta namuna dastur** — hamma ishlaydi |
| 3 | **`Natija<T>`** xatolik boshqaruvi Rust uslubida |
| 4 | **`OqimPool`** — thread pool parallellik uchun |
| 5 | **JSON, Matematika, Matn** standart kutubxonalari |
| 6 | **LSP server** — VS Code integratsiyasi |
| 7 | **GitHub Actions CI** — Windows va Linux |
| 8 | **Sintaksis podsvеtka** — to'g'ri kalit so'zlar bilan |
| 9 | **`uzpp tekshirish`** — lint buyrug'i |
| 10 | **`uzpp transpile --show-cpp`** — yonma-yon ko'rish |

### Texnik asoslar

- **Zero-overhead:** Transpilatsiya native C++23 ga → GCC/Clang optimizatsiyasi to'liq ishlaydi
- **Header-only stdlib:** Alohida yig'ish talab qilinmaydi
- **~14 000 qator kod** (kompilyator + stdlib + testlar + misollar)
- **Ikki platformada ishlaydi:** Windows 11 (MSYS2/GCC) va Linux (Ubuntu/GCC-14)

---

*Hisobot avtomatik ravishda yig'ildi — 2026-yil, 5-may.*  
*uz++ versiyasi 2.0.0 asosida.*
