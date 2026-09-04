# uz++ — Boshlang'ich qo'llanma

**O'qish vaqti: ~15 daqiqa**

---

## 1. O'rnatish

### 🪟 Windows — tavsiya etiladi

**Eng oson yo'l:** [`uzpp-setup.exe`](https://github.com/timetolivechk-spec/uzpp/releases/latest/download/uzpp-setup.exe)
ni yuklab oling va ishga tushiring (~150 MB).

Ichida MinGW GCC 15.2 ham bor — boshqa hech narsa o'rnatish kerak emas. Admin
huquqlari talab qilinmaydi. Default joy:
`%LOCALAPPDATA%\Programs\uzpp\`. PATH ga avtomatik qo'shiladi.

O'rnatish tugagach yangi PowerShell oynasini oching va tekshiring:

```powershell
uzpp --version
```

### 🐧 Linux / 🍎 macOS

Bir-qatorli o'rnatuvchi (rasmiy):

```bash
curl -fsSL https://raw.githubusercontent.com/timetolivechk-spec/uzpp/main/installer/unix/install.sh | bash
```

macOS da skript karantin belgisini (`com.apple.quarantine`) olib tashlaydi va
ad-hoc imzo qo'yadi — Gatekeeper ning "ishlab chiquvchini tekshirib bo'lmadi"
xatosi shundan. Arxivni qo'lda ochsangiz, shu ikki buyruqni o'zingiz bajaring:

```bash
sudo xattr -dr com.apple.quarantine /usr/local/bin/uzpp
sudo codesign --force --sign - /usr/local/bin/uzpp
```

Yoki manba koddan qurish (~3 daqiqa):

**Ubuntu 24.04:**
```bash
sudo apt-get install gcc-15 g++-15 cmake ninja-build
git clone https://github.com/timetolivechk-spec/uzpp && cd uzpp
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B build -DCMAKE_CXX_COMPILER=g++-15
cmake --build build
export PATH="$PWD/build:$PATH"   # `.bashrc` ga ko'chiring
```

**macOS (AppleClang Xcode 16+):**
```bash
xcode-select --install
brew install cmake ninja
git clone https://github.com/timetolivechk-spec/uzpp && cd uzpp
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
export PATH="$PWD/build:$PATH"
```

**Windows (MSYS2 UCRT64) — agar `uzpp-setup.exe` ishlatmasangiz:**
```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja
git clone https://github.com/timetolivechk-spec/uzpp && cd uzpp
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
```

### 🧩 VS Code — har qanday OS

1. Extensions panelida `uz++` ni qidiring (publisher `uzpp`).
2. **Install** bossangiz, status bar pastida `⬇ uz++ install` tugmasi paydo
   bo'ladi — bosing, kengaytma `uzpp-setup.exe` ni avtomatik yuklab oladi va
   ishga tushiradi (Windows uchun).
3. Har qanday `.uzpp` faylida **F5** → ishga tushirish, **Ctrl+F5** → qurish.

---

## 2. Birinchi dastur

`salom.uzpp` nomli fayl yarating:

```uzpp
ulash "uzpp_runtime.hpp"

butun asosiy() {
    yozish << "Salom, Dunyo!" << qator_oxiri;
    qaytarish 0;
}
```

Ishga tushiring:

```bash
# uzpp-setup.exe orqali o'rnatgan bo'lsangiz (PATH'da bor):
uzpp ishga-tushirish salom.uzpp

# Manba koddan qurgan bo'lsangiz:
./build/uzpp ishga-tushirish salom.uzpp
```

Natija:
```
Salom, Dunyo!
```

**Faqat kompilyatsiya** (binary yaratish, ishga tushirmaslik):
```bash
uzpp qurish salom.uzpp                 # build/salom.exe yaratiladi
./build/salom.exe                      # qo'lda ishga tushirish
```

**C++ ga tarjima qilingan kodni ko'rish** (debug uchun foydali):
```bash
uzpp transpile salom.uzpp --show-cpp
```

---

## 3. O'zgaruvchilar

uz++ da barcha asosiy C++ turlari o'zbek kalit so'zlari bilan yoziladi:

```uzpp
ulash "uzpp_runtime.hpp"

butun asosiy() {
    // Asosiy turlar
    butun   son    = 42;           // int
    haqiqiy kasr   = 3.14;         // double
    matn    ism    = "Akbar";      // string
    mantiqiy togri = rost;         // bool (true)
    mantiqiy noto  = yolg'on;      // bool (false)

    // Avtomatik tur (auto)
    o'zgaruvchan x = 100;
    o'zgaruvchan s = "Salom";

    // O'zgarmas (const auto)
    o'zgarmas PI = 3.14159265;

    yozish << ism << ": " << son << qator_oxiri;
    yozish << "PI = " << PI << qator_oxiri;

    qaytarish 0;
}
```

---

## 4. Shartli ifodalar

```uzpp
ulash "uzpp_runtime.hpp"

butun asosiy() {
    butun ball = 75;

    // if / else if / else
    agar (ball >= 90) {
        yozish << "A'lo" << qator_oxiri;
    } aks_holda agar (ball >= 70) {
        yozish << "Yaxshi" << qator_oxiri;
    } aks_holda agar (ball >= 50) {
        yozish << "Qoniqarli" << qator_oxiri;
    } aks_holda {
        yozish << "Qoniqarsiz" << qator_oxiri;
    }

    // Ternary operator
    // DIQQAT: `holat` — kalit so'z (`moslash` ichida `case`), shuning uchun
    // o'zgaruvchini boshqacha nomlaymiz.
    matn natija = ball >= 60 ? "O'tdi" : "Qoldi";
    yozish << natija << qator_oxiri;

    // Mantiqiy operatorlar
    mantiqiy a = rost, b = yolg'on;
    agar (a va !b) {
        yozish << "Shart bajarildi" << qator_oxiri;
    }

    qaytarish 0;
}
```

---

## 5. Tsikllar

```uzpp
ulash "uzpp_runtime.hpp"

butun asosiy() {
    // for tsikli
    uchun (butun i = 1; i <= 5; i++) {
        yozish << i << " ";
    }
    yozish << qator_oxiri;

    // while tsikli
    butun n = 10;
    toki (n > 0) {
        yozish << n << " ";
        n--;
    }
    yozish << qator_oxiri;

    // for-each (range-based for)
    vektor<matn> ismlar = {"Ali", "Vali", "Gani"};
    uchun (matn ism : ismlar) {
        yozish << ism << qator_oxiri;
    }

    qaytarish 0;
}
```

---

## 6. Funksiyalar

```uzpp
ulash "uzpp_runtime.hpp"

// Oddiy funksiya
butun kvadrat(butun x) {
    qaytarish x * x;
}

// Rekursiya
butun faktorial(butun n) {
    agar (n <= 1) qaytarish 1;
    qaytarish n * faktorial(n - 1);
}

// Lambda (yopiq funksiya)
butun asosiy() {
    yozish << "5^2 = " << kvadrat(5) << qator_oxiri;
    yozish << "7! = " << faktorial(7) << qator_oxiri;

    // Lambda
    o'zgaruvchan qosh = [](butun a, butun b) -> butun {
        qaytarish a + b;
    };
    yozish << "3 + 4 = " << qosh(3, 4) << qator_oxiri;

    // Yopiq o'zgaruvchi bilan lambda
    butun asos = 10;
    o'zgaruvchan kopayt = [asos](butun x) -> butun {
        qaytarish x * asos;
    };
    yozish << "5 * 10 = " << kopayt(5) << qator_oxiri;

    qaytarish 0;
}
```

---

## 7. Kolleksiyalar

```uzpp
ulash "uzpp_runtime.hpp"

butun asosiy() {
    // Vektor
    vektor<butun> sonlar = {1, 2, 3, 4, 5};
    sonlar.push_back(6);
    yozish << "Uzunlik: " << sonlar.size() << qator_oxiri;

    // For-each bilan yig'indi hisoblash
    butun jami = 0;
    uchun (butun s : sonlar) jami += s;
    yozish << "Jami: " << jami << qator_oxiri;

    // Lug'at (hash map)
    lug'at<matn, butun> yoshlar;
    yoshlar["Ali"]  = 25;
    yoshlar["Vali"] = 30;
    yoshlar["Gani"] = 28;

    uchun (o'zgaruvchan& juft : yoshlar) {
        yozish << juft.first << ": " << juft.second << qator_oxiri;
    }

    qaytarish 0;
}
```

---

## 8. Sinflar

```uzpp
ulash "uzpp_runtime.hpp"

sinf BankHisobi {
yopiq:
    matn    egasi;
    haqiqiy balans;

ochiq:
    BankHisobi(matn ism, haqiqiy boshlangich)
        : egasi(ism), balans(boshlangich) {}

    bosh depozit(haqiqiy summa) {
        agar (summa > 0.0) balans += summa;
    }

    mantiqiy yechish(haqiqiy summa) {
        agar (summa > 0.0 && summa <= balans) {
            balans -= summa;
            qaytarish rost;
        }
        qaytarish yolg'on;
    }

    haqiqiy balansOlish() o'zgarmas { qaytarish balans; }
    matn    egasiniOlish() o'zgarmas { qaytarish egasi; }
};

butun asosiy() {
    BankHisobi hisob("Akbar", 1000.0);

    hisob.depozit(500.0);
    yozish << hisob.egasiniOlish() << ": "
           << hisob.balansOlish() << " so'm" << qator_oxiri;

    agar (hisob.yechish(200.0)) {
        yozish << "Yechildi. Qolgan: " << hisob.balansOlish() << qator_oxiri;
    }

    qaytarish 0;
}
```

---

## 9. Xatolik boshqaruvi

uz++ da ikkita usul mavjud:

### 9.1 try/catch

```uzpp
ulash "uzpp_runtime.hpp"

haqiqiy bolish(haqiqiy a, haqiqiy b) {
    agar (b == 0.0) {
        irgitish std::runtime_error("Nolga bolish mumkin emas!");
    }
    qaytarish a / b;
}

butun asosiy() {
    urinish {
        yozish << bolish(10.0, 2.0) << qator_oxiri;
        yozish << bolish(5.0, 0.0) << qator_oxiri;  // exception
    } ushlash (std::exception& e) {
        yozish << "Xatolik: " << e.what() << qator_oxiri;
    }
    qaytarish 0;
}
```

### 9.2 Natija turi (Result type)

```uzpp
ulash "uzpp_runtime.hpp"

uzpp::Natija<haqiqiy> xavfsiz_bolish(haqiqiy a, haqiqiy b) {
    agar (b == 0.0) {
        qaytarish uzpp::Natija<haqiqiy>::xato("Nolga bolish mumkin emas!");
    }
    qaytarish uzpp::Natija<haqiqiy>::muvaffaqiyat(a / b);
}

butun asosiy() {
    o'zgaruvchan r = xavfsiz_bolish(10.0, 2.0);
    agar (r.yaroqliMi()) {
        yozish << "Natija: " << r.qiymat() << qator_oxiri;
    } aks_holda {
        yozish << "Xato: " << r.xatoMazmun() << qator_oxiri;
    }

    // Nolga bolish
    o'zgaruvchan r2 = xavfsiz_bolish(10.0, 0.0);
    agar (!r2.yaroqliMi()) {
        yozish << r2.xatoMazmun() << qator_oxiri;
    }

    qaytarish 0;
}
```

---

## 10. Naqsh moslashtirish (`pattern matching`)

```uzpp
ulash "uzpp_runtime.hpp"

matn kun_nomi(butun kun) {
    moslash (kun) {
        holat 1: qaytarish "Dushanba";
        holat 2: qaytarish "Seshanba";
        holat 3: qaytarish "Chorshanba";
        holat 4: qaytarish "Payshanba";
        holat 5: qaytarish "Juma";
        holat 6: qaytarish "Shanba";
        holat 7: qaytarish "Yakshanba";
        boshqa:  qaytarish "Noto'g'ri kun";
    }
}

butun asosiy() {
    uchun (butun k = 1; k <= 7; k++) {
        yozish << k << ": " << kun_nomi(k) << qator_oxiri;
    }
    qaytarish 0;
}
```

---

## 11. Shablonlar

```uzpp
ulash "uzpp_runtime.hpp"

// Generic funksiya
shablon <tur T>
T maksimal(T a, T b) {
    qaytarish a > b ? a : b;
}

// Generic sinf
shablon <tur T>
sinf Stek {
yopiq:
    vektor<T> ma'lumot;
ochiq:
    bosh qo'sh(T qiymat) { ma'lumot.push_back(qiymat); }
    T olish() {
        o'zgaruvchan yuqori = ma'lumot.back();
        ma'lumot.pop_back();
        qaytarish yuqori;
    }
    mantiqiy bo'shMi() o'zgarmas { qaytarish ma'lumot.empty(); }
};

butun asosiy() {
    yozish << maksimal(10, 20) << qator_oxiri;       // 20
    yozish << maksimal(3.14, 2.72) << qator_oxiri;   // 3.14

    Stek<butun> stek;
    stek.qo'sh(1);
    stek.qo'sh(2);
    stek.qo'sh(3);
    yozish << stek.olish() << qator_oxiri;  // 3

    qaytarish 0;
}
```

---

## 12. Asinxron dasturlash

```uzpp
ulash "uzpp_runtime.hpp"

butun uzoq_hisob(butun n) {
    // Murakkab hisob-kitob simulyatsiyasi
    butun natija = 0;
    uchun (butun i = 0; i < n; i++) natija += i;
    qaytarish natija;
}

butun asosiy() {
    // Parallel vazifalar
    o'zgaruvchan v1 = std::async(std::launch::async, uzoq_hisob, 1000);
    o'zgaruvchan v2 = std::async(std::launch::async, uzoq_hisob, 2000);

    yozish << "1000 ning yig'indisi: " << v1.get() << qator_oxiri;
    yozish << "2000 ning yig'indisi: " << v2.get() << qator_oxiri;

    qaytarish 0;
}
```

---

## 13. Buyruqlar

| Buyruq | Tavsif |
|--------|--------|
| `uzpp ishga-tushirish fayl.uzpp` | Kompilyatsiya va darhol ishga tushirish |
| `uzpp qurish fayl.uzpp` | Faqat kompilyatsiya → `build/<nom>` |
| `uzpp tekshirish fayl.uzpp` | Faqat tekshirish, hech narsa qurmaydi (lint) |
| `uzpp transpile fayl.uzpp --show-cpp` | uz++ va hosil bo'lgan C++ ni yonma-yon ko'rsatish |
| `uzpp sinov fayl.uzpp` | `@sinov` funksiyalarini yugurtirish |
| `uzpp bench fayl.uzpp` | `@bench` funksiyalari tezligini o'lchash |
| `uzpp formatlash fayl.uzpp` | Kodni formatlash |
| `uzpp init <nom>` | Yangi loyiha skeleti yaratish |
| `uzpp lsp` | LSP serverini ishga tushirish (muharrir uchun) |
| `uzpp --version` | Versiyani ko'rish |
| `uzpp --yordam` | To'liq ro'yxat va izohlar |

Loyiha ichida (`uzpp.toml` bor papkada) fayl nomini yozish shart emas —
`uzpp ishga-tushirish` yetarli.

---

## Keyingi qadamlar

- [To'liq darslik](darslik/00-mundarija.md) — 16 bob, noldan boshlab
  ([PDF](darslik/uzpp-darslik.pdf))
- [`misollar/`](../misollar/README.md) — 15 ta izohlangan misol
- `tests/` papkasidagi 89 ta kichik dastur — har biri bitta til
  imkoniyatini ko'rsatadi
- [CONTRIBUTING.md](../CONTRIBUTING.md) orqali hissa qo'shing
- [GitHub Issues](https://github.com/timetolivechk-spec/uzpp/issues) da savol bering
