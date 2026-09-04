<div align="center">

# uz++ — O'zbek Dasturlash Tili

**Ona tilingizda yozing. C++23 tezligida ishlating.**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![VS Code](https://img.shields.io/visual-studio-marketplace/v/uzpp.uzpp?label=VS%20Code&color=007ACC)](https://marketplace.visualstudio.com/items?itemName=uzpp.uzpp)
[![GitHub release](https://img.shields.io/github/v/release/timetolivechk-spec/uzpp)](https://github.com/timetolivechk-spec/uzpp/releases/latest)

</div>

---

## uz++ nima?

**uz++** — barcha kalit so'zlari va standart kutubxonasi O'zbek tilida yoziladigan,
C++23 ga `transpile` qilinuvchi til. Siz toza O'zbekcha sintaksisda yozasiz —
kompilyator uni C++23 ga aylantirib `g++` orqali mahalliy `binary` yig'adi.
Hech qanday ish vaqti ortiqchaligi yo'q.

```uzpp
ulash "uzpp_runtime.hpp"

butun asosiy() {
    yozish << "Salom, Dunyo!" << qator_oxiri;
    qaytarish 0;
}
```

## Tezkor o'rnatish

| Platforma | Bir-qatorli o'rnatish |
|---|---|
| 🪟 **Windows** | [`uzpp-setup.exe`](https://github.com/timetolivechk-spec/uzpp/releases/latest/download/uzpp-setup.exe) ni yuklab oling va ishga tushiring (~150 MB, MinGW GCC 15.2 ichida) |
| 🐧 **Linux** | `curl -fsSL https://raw.githubusercontent.com/timetolivechk-spec/uzpp/main/installer/unix/install.sh \| bash` |
| 🍎 **macOS** | (yuqoridagi `install.sh` macOS-da ham ishlaydi) |
| 🧩 **VS Code** | Extensions oynasidan `uzpp.uzpp` ni o'rnating — u kompilyatorni avtomatik yuklaydi |

Birinchi dasturgacha 2 daqiqa: o'rnating → VS Code da `.uzpp` fayl yarating → **F5**.

## Birinchi dasturingiz

PowerShell yoki Bash da:

```bash
mkdir uzpp-test && cd uzpp-test
```

`salom.uzpp` faylini yarating:

```uzpp
ulash "uzpp_runtime.hpp"

butun asosiy() {
    yozish << "Salom, Dunyo!" << qator_oxiri;
    qaytarish 0;
}
```

Ishga tushiring:

```bash
uzpp ishga-tushirish salom.uzpp
```

Faqat yig'ish (`binary` yaratish, ishga tushirmaslik):

```bash
uzpp qurish salom.uzpp        # build/salom.exe
```

Hosil qilingan C++ kodni ko'rish (`debug` uchun foydali):

```bash
uzpp transpile salom.uzpp --show-cpp
```

## Misollar

**1. Lambda va yopiq o'zgaruvchilar** (C++ "closures"):

```uzpp
butun asosiy() {
    butun asos = 10;
    o'zgaruvchan qoshuvchi = [asos](butun x) -> butun {
        qaytarish x + asos;
    };
    yozish << qoshuvchi(5) << qator_oxiri;  // 15
    qaytarish 0;
}
```

**2. Xavfsiz xatolik boshqaruvi** (`Result` / `std::expected` analogi):

```uzpp
uzpp::Natija<haqiqiy> xavfsiz_bo'lish(haqiqiy a, haqiqiy b) {
    agar (b == 0.0)
        qaytarish uzpp::Natija<haqiqiy>::xato("Nolga bo'lish mumkin emas");
    qaytarish uzpp::Natija<haqiqiy>::muvaffaqiyat(a / b);
}

butun asosiy() {
    o'zgaruvchan r = xavfsiz_bo'lish(10.0, 0.0);
    agar (!r.yaroqliMi())
        yozish << "Xato: " << r.xatoMazmun() << qator_oxiri;
    qaytarish 0;
}
```

**3. Patternlarni moslashtirish** (C++23 `if`/`switch with init`):

```uzpp
butun asosiy() {
    butun x = 5;
    moslash (x) {
        holat 0:    yozish << "nol" << qator_oxiri;
        holat 1, 2: yozish << "bir yoki ikki" << qator_oxiri;
        boshqa:     yozish << "boshqa son" << qator_oxiri;
    }
    qaytarish 0;
}
```

`moslash` — bu C++ `switch` emas: har bir `holat` avtomatik tugaydi,
`to'xtatish` (`break`) yozish shart emas. Bir nechta qiymatni vergul
bilan sanang.

Yana 15 ta misol — [`misollar/`](misollar/) papkasida.

## Standart kutubxona

`stdlib/` ichida 30+ modul. Ulardan **5 tasi self-hosted** — uz++ tilida yozilgan:

- `matn` — matn manipulyatsiyasi (`std::string` ustida, UTF-8 yordamchilari bilan)
- `xatoliklar` — Rust uslubidagi xatolik turlari
- `vaqt` — vaqt/sana, `<chrono>` `wrapper`
- `matematika` — trig, statistika, vektor, matritsa
- `sinov` — `unit test` `framework`
- `jurnal` — `logging` (DEBUG/INFO/WARN/ERROR)

Qolganlari hozircha qo'lda yozilgan C++ — to'liq ro'yxat va statuslari:
[docs/stdlib-status.md](docs/stdlib-status.md).

## VS Code kengaytmasi

[Marketplace `uzpp.uzpp`](https://marketplace.visualstudio.com/items?itemName=uzpp.uzpp)
ni o'rnating — quyidagi imkoniyatlar avtomatik faollashadi:

- Sintaksis `highlight`, sinf a'zolarini ajratib ko'rsatish (LSP `semantic tokens`)
- `o'zgaruvchan x = ...` uchun `inlay hints` (xulosalangan tur)
- Diagnostikalar (`autocomplete`, `hover`, `go-to-definition`, `rename`)
- `F5` — fayl ishga tushiriladi, `Ctrl+F5` — faqat yig'ish

Welcome ekranidagi **"Hammasini o'rnatish"** tugmasi `uzpp-setup.exe` ni avtomatik
yuklab o'rnatadi (Windows) yoki `install.sh` ni ishga tushiradi (Linux/macOS).

## Manba koddan qurish

```bash
git clone https://github.com/timetolivechk-spec/uzpp && cd uzpp
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
./build/uzpp ishga-tushirish misollar/01_salom_dunyo.uzpp
```

Talablar: `g++` ≥ 13 (C++23 uchun), `cmake` ≥ 3.20, `ninja`.

## Hujjatlar

**Darslik — noldan boshlab o'rganish uchun 16 bob:**

- [📘 To'liq darslik (PDF)](docs/darslik/uzpp-darslik.pdf) — bir faylda, chop etishga tayyor
- [Mundarija va boblar (Markdown)](docs/darslik/00-mundarija.md) — brauzerda o'qish uchun

Darslikdagi har bir kod misoli CI da haqiqiy kompilyator orqali
tekshiriladi (`tests/darslik_tekshir.py`) — ular ishlashi kafolatlangan.

**Qolgan hujjatlar:**

- [`docs/getting-started.md`](docs/getting-started.md) — 5 daqiqada birinchi dastur
- [`docs/stdlib-status.md`](docs/stdlib-status.md) — har bir stdlib modulning holati
- [`CHANGELOG.md`](CHANGELOG.md) — versiyalararo o'zgarishlar
- [`CONTRIBUTING.md`](CONTRIBUTING.md) — loyihaga hissa qo'shish

## Litsenziya

MIT — [`LICENSE`](LICENSE) ga qarang.
