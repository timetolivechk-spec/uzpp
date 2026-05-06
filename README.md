<div align="center">

# uz++ — O'zbek Dasturlash Tili

**Ona tilingizda yozing. C++23 tezligida ishlating.**

[![CI](https://github.com/timetolivechk-spec/uzpp/actions/workflows/ci.yml/badge.svg)](https://github.com/timetolivechk-spec/uzpp/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)](.github/workflows/ci.yml)

<br>

**uz++** — C++23 asosida qurilgan, butunlay O'zbek tilida yoziladigan kompilyatsiya tili.

*O'zbek dasturchilar uchun, o'zbek dasturchilar tomonidan.*

---

[O'zbekcha](#-uzbekcha) &nbsp;|&nbsp; [Русский](#-russian) &nbsp;|&nbsp; [English](#-english)

</div>

---

## 🇺🇿 O'zbekcha

### uz++ nima?

**uz++** — barcha kalit so'zlari O'zbek tilida yoziladigan, C++23 ga transpilatsiya qilinadigan til.
Siz oddiy O'zbek sintaksisida kod yozasiz — kompilyator uni to'liq C++23 ga aylantiradi va
mahalliy mashina kodi sifatida ishlatadi. **Hech qanday ish vaqti ortiqligi yo'q.**

### Tezkor boshlash — 3 ta buyruq

```bash
git clone https://github.com/timetolivechk-spec/uzpp && cd uz-plus-plus
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B build && cmake --build build
./build/uzpp ishga-tushirish examples/01_salom_dunyo.uzpp
```

**Natija:**
```
Salom, Dunyo!
uz++ tilida xush kelibsiz!
```

### 5 ta misol

**1. Salom, Dunyo!**
```uzpp
ulash "uzpp_runtime.hpp"

butun asosiy() {
    yozish << "Salom, Dunyo!" << qator_oxiri;
    qaytarish 0;
}
```

**2. Lambda va yopiq o'zgaruvchilar**
```uzpp
butun asosiy() {
    butun asos = 10;
    ozgaruvchan qoshuvchi = [asos](butun x) -> butun {
        qaytarish x + asos;
    };
    yozish << qoshuvchi(5) << qator_oxiri;  // 15
    qaytarish 0;
}
```

**3. Xavfsiz xatolik boshqaruvi (Result type)**
```uzpp
uzpp::Natija<haqiqiy> xavfsiz_bolish(haqiqiy a, haqiqiy b) {
    agar (b == 0.0)
        qaytarish uzpp::Natija<haqiqiy>::xato("Nolga bolish mumkin emas!");
    qaytarish uzpp::Natija<haqiqiy>::muvaffaqiyat(a / b);
}

butun asosiy() {
    ozgaruvchan r = xavfsiz_bolish(10.0, 0.0);
    agar (!r.yaroqliMi())
        yozish << "Xato: " << r.xatoMazmun() << qator_oxiri;
    qaytarish 0;
}
```

**4. Pattern Matching**
```uzpp
matn bahola(butun ball) {
    moslash (ball / 10) {
        holat 10: qaytarish "A+";
        holat 9:  qaytarish "A";
        holat 8:  qaytarish "B";
        holat 7:  qaytarish "C";
        boshqa:   qaytarish "F";
    }
}
```

**5. Asinxron parallel hisoblash**
```uzpp
butun asosiy() {
    ozgaruvchan v1 = std::async(std::launch::async, []() { qaytarish 6 * 7; });
    ozgaruvchan v2 = std::async(std::launch::async, []() { qaytarish 10 * 10; });
    yozish << "6×7 = " << v1.get() << qator_oxiri;   // 42
    yozish << "10² = " << v2.get() << qator_oxiri;   // 100
    qaytarish 0;
}
```

### uz++ vs C++ solishtirma

| Vazifa | uz++ | C++ |
|--------|------|-----|
| Hello World | `yozish << "Salom!" << qator_oxiri;` | `std::cout << "Hello!" << std::endl;` |
| For tsikli | `uchun (butun i = 0; i < 5; i++)` | `for (int i = 0; i < 5; i++)` |
| Shartli | `agar (x > 0) { ... } aks_holda { ... }` | `if (x > 0) { ... } else { ... }` |
| Sinf | `sinf Shahs { ochiq: ... }` | `class Shahs { public: ... };` |
| Lambda | `[x](butun y) -> butun { ... }` | `[x](int y) -> int { ... }` |
| Result type | `uzpp::Natija<butun>` | `std::expected<int, std::string>` |

### ✅ Xususiyatlar

| Xususiyat | Holat |
|-----------|-------|
| 80+ o'zbek kalit so'zlari | ✅ |
| Sinflar va meros | ✅ |
| Shablonlar (generics) | ✅ |
| Lambda funksiyalar | ✅ |
| `Natija<T>` (Result type) | ✅ |
| `Tanlov<T>` (Option type) | ✅ |
| Pattern matching (`moslash`) | ✅ |
| Enum turlari | ✅ |
| Asinxron/parallel | ✅ |
| OqimPool (thread pool) | ✅ |
| JSON kutubxonasi | ✅ |
| Xatoliklar (try/catch) | ✅ |
| LSP (VSCode qo'llab-quvvatlash) | ✅ |
| Kod formatlash | ✅ |

### Kalit so'zlar jadvali

| uz++          | C++           | Ma'no           |
|---------------|---------------|-----------------|
| `butun`       | `int`         | Butun son       |
| `haqiqiy`     | `double`      | Kasr son        |
| `matn`        | `string`      | Matn            |
| `mantiqiy`    | `bool`        | Mantiqiy        |
| `rost`        | `true`        | To'g'ri         |
| `yolg'on`     | `false`       | Noto'g'ri       |
| `bosh`        | `void`        | Bo'sh tur       |
| `agar`        | `if`          | Agar            |
| `aks_holda`   | `else`        | Aks holda       |
| `uchun`       | `for`         | Uchun (tsikl)   |
| `toki`        | `while`       | Toki (tsikl)    |
| `qaytarish`   | `return`      | Qaytarish       |
| `sinf`        | `class`       | Sinf            |
| `tuzilma`     | `struct`      | Tuzilma         |
| `ochiq`       | `public`      | Ochiq           |
| `yopiq`       | `private`     | Yopiq           |
| `himoyalangan`| `protected`   | Himoyalangan    |
| `mavhum`      | `virtual`     | Mavhum (virtual)|
| `ustidan_yozish` | `override` | Ustidan yozish  |
| `shablon`     | `template`    | Shablon         |
| `ozgaruvchan` | `auto`        | Avtomatik tur   |
| `ozgarmas`    | `const auto`  | O'zgarmas       |
| `statik`      | `static`      | Statik          |
| `sanab_olish` | `enum class`  | Raqamlash       |
| `vektor`      | `vector`      | Vektor          |
| `lug'at`      | `unordered_map`| Lug'at         |
| `ulash`       | `#include`    | Kutubxona ulash |
| `yozish`      | `cout`        | Chiqarish       |
| `qator_oxiri` | `endl`        | Yangi qator     |
| `urinish`     | `try`         | Urinish         |
| `ushlash`     | `catch`       | Ushlash         |
| `irgitish`    | `throw`       | Irgitish        |
| `va`          | `&&`          | Va (AND)        |
| `yoki`        | `\|\|`        | Yoki (OR)       |

### O'rnatish

```bash
# Talablar: CMake 3.20+, GCC 14+ yoki Clang 18+, Ninja
git clone https://github.com/timetolivechk-spec/uzpp
cd uz-plus-plus
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
./build/uzpp --version
```

**Windows (MSYS2):**
```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja
```

**Linux (Ubuntu 24.04):**
```bash
sudo apt-get install gcc-14 g++-14 cmake ninja-build libx11-dev libgl1-mesa-dev
```

### Buyruqlar

| Buyruq | Tavsif |
|--------|--------|
| `uzpp qurish fayl.uzpp` | Kompilyatsiya |
| `uzpp ishga-tushirish fayl.uzpp` | Kompilyatsiya + ishga tushirish |
| `uzpp format fayl.uzpp` | Kodni formatlash |
| `uzpp lsp` | LSP server (editor uchun) |
| `uzpp --version` | Versiyani ko'rish |

### Yo'l xaritasi | Roadmap

| Xususiyat | Holat |
|-----------|-------|
| ✅ Asosiy sintaksis (o'zgaruvchilar, funksiyalar, sinflar) | Tayyor |
| ✅ Shablonlar va generics | Tayyor |
| ✅ Lambda va closures | Tayyor |
| ✅ Natija/Tanlov turlari | Tayyor |
| ✅ Pattern matching | Tayyor |
| ✅ Enum va mavhum sinflar | Tayyor |
| ✅ LSP + VSCode kengaytmasi | Tayyor |
| ✅ GitHub Actions CI | Tayyor |
| 🔄 Paket menejeri (uzpm) | Ishlanmoqda |
| 🔄 Hujjatlar generatori | Ishlanmoqda |
| 📋 `uz++` standart kutubxona kengaytmasi | Rejalashtirilgan |
| 📋 Xato xabarlari yaxshilash | Rejalashtirilgan |
| 📋 Debugger integratsiyasi (DAP) | Rejalashtirilgan |
| 📋 Interaktiv REPL | Rejalashtirilgan |

### Loyiha tuzilmasi

```
uz++/
├── src/               # Kompilyator manba kodi (C++)
│   ├── lexer.cpp      # Tokenizator
│   ├── parser.cpp     # AST quruvchi
│   ├── codegen.cpp    # C++23 kod generatori
│   ├── type_checker.hpp # Semantik tahlil
│   ├── lsp_server.cpp # Language Server Protocol
│   └── main.cpp       # CLI kirish nuqtasi
├── stdlib/            # uz++ standart kutubxonasi (header-only C++)
├── examples/          # 10 ta misol dastur
├── misollar/          # Qo'shimcha misollar
├── tests/             # Integratsiya testlari (23+ ta)
├── vscode-uzpp/       # VSCode kengaytmasi
└── .github/workflows/ # CI (Windows + Linux)
```

### Hissa qo'shish

[CONTRIBUTING.md](CONTRIBUTING.md) ni o'qing.
[GitHub Issues](https://github.com/timetolivechk-spec/uzpp/issues) da xato yoki taklif qoldiring.

---

## 🇷🇺 Russian

### Что такое uz++?

**uz++** — язык программирования, в котором весь код пишется на узбекском языке.
Транспилируется в C++23 и производит нативные бинарные файлы с **полной производительностью C++**.

### Быстрый старт

```bash
git clone https://github.com/timetolivechk-spec/uzpp && cd uz-plus-plus
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B build && cmake --build build
./build/uzpp ishga-tushirish examples/01_salom_dunyo.uzpp
```

### Пример кода

```uzpp
// FizzBuzz на uz++
ulash "uzpp_runtime.hpp"

butun asosiy() {
    uchun (butun i = 1; i <= 20; i++) {
        agar (i % 15 == 0)           yozish << "FizzBuzz";
        aks_holda agar (i % 3 == 0)  yozish << "Fizz";
        aks_holda agar (i % 5 == 0)  yozish << "Buzz";
        aks_holda                    yozish << i;
        yozish << qator_oxiri;
    }
    qaytarish 0;
}
```

### Обработка ошибок — Result type

```uzpp
uzpp::Natija<haqiqiy> xavfsiz_bolish(haqiqiy a, haqiqiy b) {
    agar (b == 0.0)
        qaytarish uzpp::Natija<haqiqiy>::xato("Нельзя делить на ноль!");
    qaytarish uzpp::Natija<haqiqiy>::muvaffaqiyat(a / b);
}

butun asosiy() {
    ozgaruvchan r = xavfsiz_bolish(10.0, 0.0);
    agar (r.yaroqliMi())
        yozish << r.qiymat() << qator_oxiri;
    aks_holda
        yozish << "Xato: " << r.xatoMazmun() << qator_oxiri;
    qaytarish 0;
}
```

### Стандартная библиотека

| Модуль               | Возможности                               |
|----------------------|-------------------------------------------|
| `uzpp_runtime.hpp`   | Natija, Tanlov, OqimPool, потоки          |
| `matematika.hpp`     | sin, cos, sqrt, pow, среднее, факториал   |
| `matn.hpp`           | обрезка, разбивка, замена, конвертация    |
| `json.hpp`           | разбор и сериализация JSON                |
| `tarmoq.hpp`         | сетевые операции                          |
| `fayl_tizimi.hpp`    | файловая система                          |
| `asinxron.hpp`       | async/await примитивы                     |

### Сборка

```bash
# Зависимости: CMake 3.20+, GCC 14+ или Clang 18+, Ninja
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
```

---

## 🇬🇧 English

### What is uz++?

**uz++** is a systems programming language where all keywords and identifiers are written in Uzbek.
It **transpiles to C++23** and produces native binaries with full C++ performance — no runtime overhead.

Uzbek speakers can now program in their native language, with the same power as C++23.

### Quick start

```bash
git clone https://github.com/timetolivechk-spec/uzpp && cd uz-plus-plus
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B build && cmake --build build
./build/uzpp ishga-tushirish examples/01_salom_dunyo.uzpp
```

### Language overview

```uzpp
ulash "uzpp_runtime.hpp"

// Generic template function
shablon <tur T>
bosh chop_et(matn nom, vektor<T>& v) {
    yozish << nom << ": ";
    uchun (ozgaruvchan& x : v) yozish << x << " ";
    yozish << qator_oxiri;
}

butun asosiy() {
    // Vector with lambda sort
    vektor<butun> sonlar = {5, 3, 8, 1, 9, 2};
    std::sort(sonlar.begin(), sonlar.end());
    chop_et("Sorted", sonlar);

    // Pattern matching
    moslash (sonlar[0]) {
        holat 1: yozish << "Minimal!" << qator_oxiri;
        boshqa:  yozish << "Not minimum" << qator_oxiri;
    }

    // Result type for safe error handling
    ozgaruvchan r = uzpp::Natija<butun>::muvaffaqiyat(42);
    agar (r.yaroqliMi())
        yozish << "Result: " << r.qiymat() << qator_oxiri;

    qaytarish 0;
}
```

### Key features

- **80+ Uzbek keywords** — `butun`→`int`, `agar`→`if`, `uchun`→`for`, `sinf`→`class`, ...
- **Full C++23 feature set** — templates, lambdas, ranges, structured bindings, concepts
- **`Natija<T>`** — Rust-inspired Result type (`std::expected` wrapper)
- **`Tanlov<T>`** — Option/Maybe type
- **`OqimPool`** — built-in thread pool executor
- **Pattern matching** — `moslash/holat/boshqa` (switch on any type)
- **Enum types** — `sanab_olish` with auto-generated name helpers
- **Abstract classes** — `mavhum`, `ustidan_yozish` (virtual/override)
- **VSCode extension** — syntax highlighting, LSP, hover docs
- **GitHub Actions CI** — Windows (MSYS2/GCC) + Linux (GCC 14)
- **25+ stdlib modules** — math, strings, JSON, async, networking, graphics

### Keyword reference

| uz++          | C++            | Meaning        |
|---------------|----------------|----------------|
| `butun`       | `int`          | Integer        |
| `haqiqiy`     | `double`       | Float/double   |
| `matn`        | `string`       | String         |
| `mantiqiy`    | `bool`         | Boolean        |
| `rost`        | `true`         | True           |
| `yolg'on`     | `false`        | False          |
| `agar`        | `if`           | If             |
| `aks_holda`   | `else`         | Else           |
| `uchun`       | `for`          | For loop       |
| `toki`        | `while`        | While loop     |
| `qaytarish`   | `return`       | Return         |
| `sinf`        | `class`        | Class          |
| `tuzilma`     | `struct`       | Struct         |
| `ochiq`       | `public`       | Public         |
| `yopiq`       | `private`      | Private        |
| `mavhum`      | `virtual`      | Virtual        |
| `ustidan_yozish`| `override`   | Override       |
| `shablon`     | `template`     | Template       |
| `ozgaruvchan` | `auto`         | Auto           |
| `ozgarmas`    | `const auto`   | Const auto     |
| `sanab_olish` | `enum class`   | Enum           |
| `moslash`     | `switch`       | Switch/match   |
| `holat`       | `case`         | Case           |
| `boshqa`      | `default`      | Default        |
| `urinish`     | `try`          | Try            |
| `ushlash`     | `catch`        | Catch          |
| `irgitish`    | `throw`        | Throw          |
| `va`          | `&&`           | Logical AND    |
| `yoki`        | `\|\|`         | Logical OR     |
| `ulash`       | `#include`     | Include        |
| `vektor`      | `vector`       | Vector         |
| `yozish`      | `cout`         | Output stream  |
| `qator_oxiri` | `endl`         | Newline        |

### Examples

The `examples/` directory contains 10 runnable programs:

| File | Topic |
|------|-------|
| `01_salom_dunyo.uzpp`  | Hello World + variables |
| `02_ozgaruvchilar.uzpp`| All types + arithmetic |
| `03_funksiyalar.uzpp`  | Functions, recursion, lambdas |
| `04_sinflar.uzpp`      | Classes, inheritance, polymorphism |
| `05_natija.uzpp`       | Result<T> and Option<T> types |
| `06_moslik.uzpp`       | Pattern matching |
| `07_asinxron.uzpp`     | Async + OqimPool thread pool |
| `08_kolleksiyalar.uzpp`| Vectors, maps, sorting |
| `09_json.uzpp`         | JSON create/parse |
| `10_ilgor.uzpp`        | Enums, generics, abstract classes |

### Requirements

| Platform | Compiler     | Minimum version |
|----------|--------------|-----------------|
| Windows  | GCC (MSYS2)  | GCC 13          |
| Linux    | GCC or Clang | GCC 14 / Clang 18 |
| macOS    | AppleClang   | Xcode 16        |

CMake 3.20+ and Ninja (recommended)

### Project structure

```
uz++/
├── src/                    # Compiler source (C++)
│   ├── lexer.cpp           # Tokenizer — Uzbek keywords
│   ├── parser.cpp          # AST builder — full semantic parser
│   ├── ast.h               # Abstract syntax tree types
│   ├── codegen.cpp         # C++23 code generator
│   ├── type_checker.hpp    # Semantic analysis + type inference
│   ├── formatter.cpp       # Auto-formatter
│   ├── lsp_server.cpp      # Language Server Protocol
│   └── main.cpp            # CLI entry point
├── stdlib/                 # uz++ standard library (header-only C++)
│   ├── uzpp_runtime.hpp    # Core: Natija, Tanlov, OqimPool
│   ├── matematika.hpp      # Math
│   ├── matn.hpp            # String utilities
│   ├── json.hpp            # JSON
│   └── ...                 # 25+ modules
├── examples/               # 10 runnable example programs
├── misollar/               # Additional example programs
├── tests/                  # Integration tests (23+ passing)
├── vscode-uzpp/            # VSCode extension
└── .github/workflows/      # CI: Windows + Linux
```

### Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md). Issues and PRs welcome.

### License

MIT License — see [LICENSE](LICENSE) for details.

Copyright (c) 2026 timetolivechk-spec

