# uz++ — O'zbek Dasturlash Tili

<div align="center">

**uz++** — C++23 asosida qurilgan, O'zbek tilida yoziladigan kompilyatsiya tili.  
**uz++** — язык программирования на узбекском, транспилируется в C++23.  
**uz++** — an Uzbek-language programming language that transpiles to C++23.

[![CI](https://github.com/YOUR_USERNAME/uz-plus-plus/actions/workflows/ci.yml/badge.svg)](https://github.com/YOUR_USERNAME/uz-plus-plus/actions/workflows/ci.yml)

</div>

---

## O'zbekcha | Русский | English

- [O'zbekcha](#uzbekcha)
- [Русский](#russian)
- [English](#english)

---

## O'zbekcha

### uz++ nima?

**uz++** — dasturlash tilini o'z ona tilingizda o'rganishga imkon beruvchi kompilyatsiya tiliga asoslangan til. uz++ kodi C++23 ga transpilatsiya qilinadi va to'liq tezlik hamda xotira nazoratiga ega.

### Tezkor boshlash

```bash
# Birinchi dasturingiz
cat > salom.uzpp << 'EOF'
ulash "uzpp_runtime.hpp"

butun asosiy() {
    yozish << "Salom, Dunyo!" << qator_oxiri;
    qaytarish 0;
}
EOF

uzpp ishga-tushirish salom.uzpp
```

### Asosiy sintaksis

```uzpp
// O'zgaruvchilar
butun son = 42;          // int
haqiqiy kasr = 3.14;     // double
matn ism = "Ali";        // string
mantiqiy togri = rost;   // bool

// Shartli ifoda
agar (son > 10) {
    yozish << "Katta" << qator_oxiri;
} aks_holda agar (son > 5) {
    yozish << "O'rta" << qator_oxiri;
} aks_holda {
    yozish << "Kichik" << qator_oxiri;
}

// Tsikllar
uchun (butun i = 0; i < 5; i++) {
    yozish << i << " ";
}

// Funksiyalar
butun kvadrat(butun x) {
    qaytarish x * x;
}
```

### Kolleksiyalar

```uzpp
// Vektor
vektor<butun> sonlar = [1, 2, 3, 4, 5];
uchun (butun n : sonlar) {
    yozish << n << " ";
}

// Lug'at (Map)
lug'at<matn, butun> yoshlar;
yoshlar["Ali"] = 25;
yoshlar["Vali"] = 30;
```

### Xatolik boshqaruvi

```uzpp
uzpp::Natija<haqiqiy> xavfsiz_bolish(haqiqiy a, haqiqiy b) {
    agar (b == 0.0) {
        qaytarish uzpp::Natija<haqiqiy>::xato("Nolga bolish mumkin emas!");
    }
    qaytarish uzpp::Natija<haqiqiy>::muvaffaqiyat(a / b);
}

ozgaruvchan natija = xavfsiz_bolish(10.0, 3.0);
agar (natija.yaroqliMi()) {
    yozish << natija.qiymat() << qator_oxiri;
} aks_holda {
    yozish << "Xato: " << natija.xatoMazmun() << qator_oxiri;
}
```

### Sinflar

```uzpp
sinf Shahs {
ochiq:
    matn ism;
    butun yosh;

    Shahs(matn i, butun y) : ism(i), yosh(y) {}

    bosh tanishtir() {
        yozish << ism << " (" << yosh << " yosh)" << qator_oxiri;
    }
};

Shahs s("Akbar", 25);
s.tanishtir();
```

### Kalit so'zlar jadvali

| uz++          | C++        | Ma'no           |
|---------------|------------|-----------------|
| `butun`       | `int`      | Butun son       |
| `haqiqiy`     | `double`   | Kasr son        |
| `matn`        | `string`   | Matn            |
| `mantiqiy`    | `bool`     | Mantiqiy qiymat |
| `rost`        | `true`     | To'g'ri         |
| `yolg'on`     | `false`    | Noto'g'ri       |
| `agar`        | `if`       | Agar            |
| `aks_holda`   | `else`     | Aks holda       |
| `uchun`       | `for`      | Uchun (tsikl)   |
| `qaytarish`   | `return`   | Qaytarish       |
| `sinf`        | `class`    | Sinf            |
| `ozgaruvchan` | `auto`     | Avtomatik tur   |
| `yozish`      | `cout`     | Chiqarish       |
| `qator_oxiri` | `endl`     | Yangi qator     |

### O'rnatish

```bash
git clone https://github.com/YOUR_USERNAME/uz-plus-plus
cd uz-plus-plus
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
./build/uzpp --version
```

---

## Russian

### Что такое uz++?

**uz++** — язык программирования, в котором весь код пишется на узбекском языке. Транспилируется в C++23 — получаете полную производительность нативного кода.

### Быстрый старт

```bash
uzpp ishga-tushirish mening_dastur.uzpp  # запустить
uzpp transpile kod.uzpp                  # только транспилировать в C++
uzpp tekshirish kod.uzpp                 # только проверить (lint)
```

### Пример кода

```uzpp
// FizzBuzz на uz++
ulash "uzpp_runtime.hpp"

butun asosiy() {
    uchun (butun i = 1; i <= 20; i++) {
        agar (i % 15 == 0)      yozish << "FizzBuzz";
        aks_holda agar (i % 3 == 0) yozish << "Fizz";
        aks_holda agar (i % 5 == 0) yozish << "Buzz";
        aks_holda               yozish << i;
        yozish << qator_oxiri;
    }
    qaytarish 0;
}
```

### Обработка ошибок

Тип `uzpp::Natija<T>` — аналог `Result<T, E>` из Rust или `std::expected` из C++23:

```uzpp
ozgaruvchan r = xavfsiz_bolish(10.0, 0.0);
agar (r.yaroqliMi()) {
    yozish << r.qiymat();         // значение
} aks_holda {
    yozish << r.xatoMazmun();    // сообщение об ошибке
}
```

### Стандартная библиотека

| Модуль            | Возможности                              |
|-------------------|------------------------------------------|
| `matematika.hpp`  | sin, cos, sqrt, pow, среднее, факториал  |
| `matn.hpp`        | обрезка, разбивка, замена, конвертация   |
| `json.hpp`        | разбор и сериализация JSON               |
| `uzpp_runtime.hpp`| Natija, Tanlov, OqimPool, форматирование |

### Сборка

```bash
# Зависимости: CMake 3.20+, GCC-14+ или Clang-18+, Ninja
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
```

---

## English

### What is uz++?

**uz++** is a compiled programming language where all keywords and identifiers are written in Uzbek. It transpiles to C++23 and produces native binaries with full C++ performance.

**Key features:**
- All keywords in Uzbek (80+ translations: `butun`→`int`, `agar`→`if`, `uchun`→`for`, ...)
- Full C++23 feature set: templates, lambdas, ranges, async, concepts
- Result type (`Natija<T>`) for safe error handling
- Thread pool (`OqimPool`) for parallel programming
- JSON parsing stdlib
- VSCode extension with syntax highlighting, LSP and hover docs
- GitHub Actions CI for Windows and Linux

### Quick start

```bash
git clone https://github.com/YOUR_USERNAME/uz-plus-plus
cd uz-plus-plus
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
./build/uzpp ishga-tushirish misollar/01_salom_dunyo.uzpp
```

### Language overview

```uzpp
ulash "uzpp_runtime.hpp"

// Generic template function
shablon <tur T>
bosh chop_et(matn nom, vektor<T>& v) {
    yozish << nom << ": ";
    uchun (ozgaruvchan x : v) yozish << x << " ";
    yozish << qator_oxiri;
}

butun asosiy() {
    // Vector literals
    vektor<butun> sonlar = [3, 1, 4, 1, 5, 9, 2, 6];

    // Lambda + sort
    ozgaruvchan taqqosla = [&](butun a, butun b) { qaytarish a < b; };
    std::sort(sonlar.begin(), sonlar.end(), taqqosla);

    chop_et("Tartiblangan", sonlar);

    // Async parallel computation
    ozgaruvchan oqim = std::async(std::launch::async, []() {
        qaytarish 42;
    });
    yozish << "Natija: " << oqim.get() << qator_oxiri;

    qaytarish 0;
}
```

### Commands

| Command                              | Description                    |
|--------------------------------------|--------------------------------|
| `uzpp ishga-tushirish file.uzpp`     | Compile and run                |
| `uzpp transpile file.uzpp`           | Transpile to C++ only          |
| `uzpp tekshirish file.uzpp`          | Lint / type-check only         |
| `uzpp format file.uzpp`              | Auto-format source             |
| `uzpp lsp`                           | Start LSP server (for editors) |

### Keyword reference

| uz++          | C++           | Meaning              |
|---------------|---------------|----------------------|
| `butun`       | `int`         | Integer              |
| `haqiqiy`     | `double`      | Float/double         |
| `matn`        | `string`      | String               |
| `mantiqiy`    | `bool`        | Boolean              |
| `rost`        | `true`        | True                 |
| `yolg'on`     | `false`       | False                |
| `agar`        | `if`          | If                   |
| `aks_holda`   | `else`        | Else                 |
| `uchun`       | `for`         | For loop             |
| `holda`       | `while`       | While loop           |
| `qaytarish`   | `return`      | Return               |
| `sinf`        | `class`       | Class                |
| `sinf_nomi`   | `struct`      | Struct               |
| `ozgaruvchan` | `auto`        | Auto type            |
| `ozgarmas`    | `const auto`  | Const auto           |
| `bosh`        | `void`        | Void                 |
| `shablon`     | `template`    | Template             |
| `tur`         | `typename`    | Typename             |
| `ochiq`       | `public`      | Public               |
| `yopiq`       | `private`     | Private              |
| `himoyalangan`| `protected`   | Protected            |
| `ulash`       | `#include`    | Include              |
| `vektor`      | `vector`      | Vector               |
| `lug'at`      | `unordered_map`| Hash map            |
| `yozish`      | `cout`        | Output stream        |
| `qator_oxiri` | `endl`        | Newline              |

### Project structure

```
uz++/
├── src/               # Compiler source (C++)
│   ├── lexer.cpp      # Tokenizer
│   ├── parser.cpp     # AST builder
│   ├── codegen.cpp    # C++ code generator
│   ├── type_checker.hpp # Semantic analysis
│   ├── lsp_server.cpp # Language Server Protocol
│   └── main.cpp       # CLI entry point
├── stdlib/            # uz++ standard library (header-only C++)
│   ├── uzpp_runtime.hpp  # Core: Natija, OqimPool, Xatoliklar
│   ├── matematika.hpp    # Math functions
│   ├── matn.hpp          # String utilities
│   └── json.hpp          # JSON parser/serializer
├── misollar/          # 10 canonical example programs
├── tests/             # Unit and integration tests
├── vscode-uzpp/       # VSCode extension
└── .github/workflows/ # CI (Windows + Linux)
```

### Requirements

| Platform | Compiler       | Minimum |
|----------|----------------|---------|
| Windows  | GCC (MSYS2)    | GCC 13  |
| Linux    | GCC or Clang   | GCC 14 / Clang 18 |
| macOS    | AppleClang     | Xcode 16 |

CMake 3.20+, Ninja (recommended)

### Examples

The `misollar/` directory contains 10 example programs covering:

| File | Topic |
|------|-------|
| `01_salom_dunyo.uzpp`    | Hello World |
| `02_fizzbuzz.uzpp`       | FizzBuzz (else-if chain) |
| `03_rekursiya.uzpp`      | Recursion: Fibonacci, Factorial |
| `04_sinflar.uzpp`        | Classes and inheritance |
| `05_kolleksiyalar.uzpp`  | Vectors, maps, lambda sort |
| `06_xatolik_boshqaruvi.uzpp` | Error handling with `Natija<T>` |
| `07_matn_ishlash.uzpp`   | String operations |
| `08_json_ishlash.uzpp`   | JSON create/parse/query |
| `09_kop_oqimlilik.uzpp`  | Parallel async + OqimPool |
| `10_ilgor_dasturlash.uzpp` | Generics, lambdas, functional |

### License

MIT License — see [LICENSE](LICENSE) for details.
