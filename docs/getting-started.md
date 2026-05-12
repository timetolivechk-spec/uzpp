# uz++ — Boshlang'ich qo'llanma | Getting Started with uz++

**O'qish vaqti: ~15 daqiqa | Reading time: ~15 minutes**

---

## 1. O'rnatish | Installation

### 🪟 Windows — tavsiya etiladi | Recommended

**Eng oson yo'l:** [`uzpp-setup.exe`](https://github.com/timetolivechk-spec/uzpp/releases/latest/download/uzpp-setup.exe)
ni yuklab oling va ishga tushiring (~115 MB).

Ichida MinGW GCC 14.2 ham bor — boshqa hech narsa o'rnatish kerak emas. Admin
huquqlari talab qilinmaydi. Default joy:
`%LOCALAPPDATA%\Programs\uzpp\`. PATH ga avtomatik qo'shiladi.

O'rnatish tugagach yangi PowerShell oynasini oching va tekshiring:

```powershell
uzpp --version
```

### 🐧 Linux / 🍎 macOS — manba koddan qurish | Build from source

Hozircha rasmiy paket yo'q, manba koddan qurish kerak (~3 daqiqa):

**Ubuntu 24.04:**
```bash
sudo apt-get install gcc-14 g++-14 cmake ninja-build
git clone https://github.com/timetolivechk-spec/uzpp && cd uzpp
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B build -DCMAKE_CXX_COMPILER=g++-14
cmake --build build
export PATH="$PWD/build:$PATH"   # PATH ga qo'shing (.bashrc ga ko'chiring)
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

## 2. Birinchi dastur | Your First Program

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

## 3. O'zgaruvchilar | Variables

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
    ozgaruvchan x = 100;
    ozgaruvchan s = "Salom";

    // O'zgarmas (const auto)
    ozgarmas PI = 3.14159265;

    yozish << ism << ": " << son << qator_oxiri;
    yozish << "PI = " << PI << qator_oxiri;

    qaytarish 0;
}
```

---

## 4. Shartli ifodalar | Conditionals

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
    matn holat = ball >= 60 ? "O'tdi" : "Qoldi";
    yozish << holat << qator_oxiri;

    // Mantiqiy operatorlar
    mantiqiy a = rost, b = yolg'on;
    agar (a va !b) {
        yozish << "Shart bajarildi" << qator_oxiri;
    }

    qaytarish 0;
}
```

---

## 5. Tsikllar | Loops

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

## 6. Funksiyalar | Functions

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
    ozgaruvchan qosh = [](butun a, butun b) -> butun {
        qaytarish a + b;
    };
    yozish << "3 + 4 = " << qosh(3, 4) << qator_oxiri;

    // Yopiq o'zgaruvchi bilan lambda
    butun asos = 10;
    ozgaruvchan kopayt = [asos](butun x) -> butun {
        qaytarish x * asos;
    };
    yozish << "5 * 10 = " << kopayt(5) << qator_oxiri;

    qaytarish 0;
}
```

---

## 7. Kolleksiyalar | Collections

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

    uchun (ozgaruvchan& juft : yoshlar) {
        yozish << juft.first << ": " << juft.second << qator_oxiri;
    }

    qaytarish 0;
}
```

---

## 8. Sinflar | Classes

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

    haqiqiy balansOlish() sabit { qaytarish balans; }
    matn    egasiniOlish() sabit { qaytarish egasi; }
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

## 9. Xatolik boshqaruvi | Error Handling

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
    ozgaruvchan r = xavfsiz_bolish(10.0, 2.0);
    agar (r.yaroqliMi()) {
        yozish << "Natija: " << r.qiymat() << qator_oxiri;
    } aks_holda {
        yozish << "Xato: " << r.xatoMazmun() << qator_oxiri;
    }

    // Nolga bolish
    ozgaruvchan r2 = xavfsiz_bolish(10.0, 0.0);
    agar (!r2.yaroqliMi()) {
        yozish << r2.xatoMazmun() << qator_oxiri;
    }

    qaytarish 0;
}
```

---

## 10. Pattern Matching | Naqsh moslashtirish

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

## 11. Shablonlar | Templates

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
        ozgaruvchan yuqori = ma'lumot.back();
        ma'lumot.pop_back();
        qaytarish yuqori;
    }
    mantiqiy bo'shMi() sabit { qaytarish ma'lumot.empty(); }
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

## 12. Asinxron dasturlash | Async Programming

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
    ozgaruvchan v1 = std::async(std::launch::async, uzoq_hisob, 1000);
    ozgaruvchan v2 = std::async(std::launch::async, uzoq_hisob, 2000);

    yozish << "1000 ning yig'indisi: " << v1.get() << qator_oxiri;
    yozish << "2000 ning yig'indisi: " << v2.get() << qator_oxiri;

    qaytarish 0;
}
```

---

## 13. Buyruqlar | Commands Reference

| Buyruq | Tavsif |
|--------|--------|
| `uzpp qurish fayl.uzpp` | Kompilyatsiya qilish |
| `uzpp ishga-tushirish fayl.uzpp` | Kompilyatsiya va ishga tushirish |
| `uzpp format fayl.uzpp` | Kodni formatlash |
| `uzpp lsp` | LSP serverini ishga tushirish (editor uchun) |
| `uzpp --version` | Versiyani ko'rish |

---

## Keyingi qadamlar | Next Steps

- `misollar/` papkasidagi 10 ta misolni ko'ring
- `tests/` papkasidagi testlarni o'rganing
- [CONTRIBUTING.md](../CONTRIBUTING.md) orqali hissa qo'shing
- [GitHub Issues](https://github.com/timetolivechk-spec/uzpp/issues) da savol bering
