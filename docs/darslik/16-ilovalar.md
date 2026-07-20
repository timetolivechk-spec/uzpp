# 16 · Ilovalar {#16-ilovalar}

## A · Kalit so'zlar lug'ati

uz++ ning barcha kalit so'zlari va ularning C++ ekvivalentlari.

### Asosiy turlar

| uz++ | C++ | Ma'nosi |
|------|-----|---------|
| `butun` | `int` | Butun son (32 bit) |
| `uzun` | `long long` | Katta butun son (64 bit) |
| `kasr` | `float` | Haqiqiy son (32 bit) |
| `haqiqiy` | `double` | Haqiqiy son (64 bit) |
| `mantiqiy` | `bool` | Rost/yolg'on (boolean) |
| `belgi` | `char` | Bitta belgi |
| `matn` | `std::string` | Matn satri |
| `bosh` | `void` | Bo'sh (qiymatsiz) |

### Konteynerlar

| uz++ | C++ | Tavsif |
|------|-----|--------|
| `vektor` | `std::vector` | Dinamik massiv |
| `roy'xat` | `std::list` | Bog'langan ro'yxat |
| `tartiblangan_xarita` | `std::map` | Tartiblangan kalit-qiymat |
| `lug'at` | `std::unordered_map` | Hash lug'at (map) |
| `tartib_to'plam` | `std::set` | Tartiblangan to'plam (set) |
| `massiv` | `std::array` | Statik massiv |
| `juftlik` | `std::pair` | Juftlik |
| `uchlik` | `std::tuple` | Uchlik |

### Qo'shimcha turlar

| uz++ | C++ | Tavsif |
|------|-----|--------|
| `ixtiyoriy` | `std::optional` | Bo'lishi mumkin yoki yo'q |
| `Natija` | `std::expected` | Qiymat yoki xato |
| `Tanlov` | `std::optional` | Tanlov |
| `yagona_korsatkich` | `std::unique_ptr` | Yagona ko'rsatkich |
| `umumiy_korsatkich` | `std::shared_ptr` | Umumiy ko'rsatkich |
| `funksiya` | `std::function` | Funksiya obyekti |
| `kelajak` | `std::future` | Kelajakdagi qiymat |
| `vada` | `std::promise` | Va'da (promise) |
| `qulf` | `std::mutex` | Mutex |
| `atomik` | `std::atomic` | Atomar o'zgaruvchi |

### Boshqaruv tuzilmalari

| uz++ | C++ | Ma'nosi |
|------|-----|---------|
| `agar` | `if` | Agar |
| `aks_holda` / `yoki` | `else` | Aks holda |
| `uchun` | `for` | Takrorlash sikli |
| `toki` | `while` | Shartli takrorlash (while) |
| `moslash` | `switch` | Moslash (switch) |
| `holat` | `case` | Holat |
| `boshqa` | `default` | Standart |
| `to'xtatish` | `break` | Sikldan chiqish |
| `davom_etish` | `continue` | Keyingi takrorlashga o'tish |
| `qaytarish` | `return` | Qiymat qaytarish |

### Xatolik bilan ishlash

| uz++ | C++ | Ma'nosi |
|------|-----|--------|
| `urinish` | `try` | Urinib ko'rish |
| `ushlash` | `catch` | Ushlash |
| `irgitish` | `throw` | Irgitish |
| `tasdiqlash` | `assert` | Tasdiqlash (debug) |

### Sinf va tuzilmalar

| uz++ | C++ | Ma'nosi |
|------|-----|---------|
| `sinf` | `class` | Sinf |
| `tuzilma` | `struct` | Tuzilma |
| `ochiq` | `public` | Ochiq |
| `yopiq` | `private` | Yopiq |
| `himoyalangan` | `protected` | Himoyalangan |
| `mavhum` | `virtual` | Virtual |
| `ustidan_yozish` | `override` | Qayta yozish |
| `dost` | `friend` | Do'st |
| `shablon` | `template` | Shablon |
| `tur` | `typename` | Tur nomi |
| `turdash` | `typename` | Turdosh |

### Boshqa kalit so'zlar

| uz++ | C++ | Tavsif |
|------|-----|--------|
| `o'zgarmas` | `const` | O'zgarmas |
| `o'zgaruvchan` | `auto` | Avtomatik tur |
| `qator_ichi` | `inline` | Ichki funksiya |
| `yangi` | `new` | Dinamik xotira |
| `o'chirish` | `delete` | Xotirani qaytarish |
| `rost` | `true` | Mantiqiy rost |
| `yolg'on` | `false` | Mantiqiy yolg'on |
| `nullptr` | `nullptr` | Bo'sh ko'rsatkich |

---

## B · O'rnatish bo'yicha batafsil qo'llanma

### Windows

1. **Yuklab olish:** [GitHub Releases](https://github.com/timetolivechk-spec/uzpp/releases)
   sahifasidan eng so'nggi `uzpp-setup.exe` faylini yuklab oling.

2. **O'rnatish:** Yuklangan faylni ishga tushiring. O'rnatuvchi
   quyidagilarni avtomatik o'rnatadi:
   - `uzpp.exe` — transpilyator
   - MinGW-w64 `g++` — C++ kompilyatori
   - VS Code kengaytmasi (`uzpp.uzpp`)

3. **Tekshirish:**
   ```bash
   uzpp --yordam
   ```
   Agar yordam matni chiqsa, o'rnatish muvaffaqiyatli.

### Linux
```bash
curl -fsSL https://raw.githubusercontent.com/timetolivechk-spec/uzpp/main/install.sh | bash
```

`install.sh` skripti:
- `g++` o'rnatilganligini tekshiradi (yo'q bo'lsa — apt/yum/pacman
  orqali o'rnatadi)
- `uzpp` ni `/usr/local/bin/` ga joylashtiradi
- VS Code mavjud bo'lsa, kengaytmani ham o'rnatadi

### macOS
```bash
curl -fsSL https://raw.githubusercontent.com/timetolivechk-spec/uzpp/main/install.sh | bash
```

Yoki Homebrew orqali (kelajakda):
```bash
brew install uzpp
```

`g++` o'rniga `clang++` ishlatiladi.

---

## C · Fayl kengaytmalari

| Kengaytma | Tavsif |
|-----------|--------|
| `.uzpp` | uz++ manba fayli |
| `.cpp` / `.cxx` / `.cc` | C++ manba fayli |
| `.hpp` / `.h` | C++ sarlavha fayli |
| `.exe` | Bajarib bo'ladigan fayl (Windows) |
| `.so` | Dinamik kutubxona (Linux) |
| `.dylib` | Dinamik kutubxona (macOS) |
| `.dll` | Dinamik kutubxona (Windows) |

---

## D · CLI buyruqlari

### Asosiy buyruqlar

| Buyruq | Tavsif |
|--------|--------|
| `uzpp qurish fayl.uzpp` | Dasturni kompilyatsiya qilish |
| `uzpp ishga-tushirish fayl.uzpp` | Kompilyatsiya + ishga tushirish |
| `uzpp sinov` | Testlarni bajarish |
| `uzpp formatlash` | Kodni formatlash |
| `uzpp lsp` | LSP server ishga tushirish |
| `uzpp hujjat` | Hujjat generatsiya qilish |
| `uzpp --yordam` | Yordam |
| `uzpp --versiya` | Versiya |

### Bayroqlar

| Bayroq | Tavsif |
|--------|--------|
| `--debug` | Debug rejimida kompilyatsiya |
| `--bare` | `uzpp_runtime.hpp` siz qurish (Qt/CopperSpice uchun) |
| `-I yo'l` | Include katalogi |
| `-l yo'l` | Kutubxona fayli |
| `--target=os` | Maqsadli platforma |
| `--show-cpp` | C++ kodini ko'rsatish |
| `--optimize=N` | Optimizatsiya darajasi (0-3) |

---

## E · C++23 mosligi

uz++ C++23 standartini to'liq qo'llab-quvvatlaydi:

| C++23 xususiyati | uz++ da mavjud |
|------------------|----------------|
| `std::expected` (`Natija<T>`) | ✅ |
| `std::format` | ✅ |
| `std::ranges` | ✅ |
| `std::generator` (coroutines) | ✅ |
| `auto(x)` decay copy | ✅ |
| `if consteval` | ✅ |
| Deducing `this` | ✅ |
| `static operator()` | ✅ |
| `std::print` / `std::println` | ✅ |
| `std::mdspan` | ✅ |

---

## F · Foydali resurslar

### Rasmiy

- **Repozitoriy:** [github.com/timetolivechk-spec/uzpp](https://github.com/timetolivechk-spec/uzpp)
- **Hujjatlar:** `docs/` papkasi
- **Misollar:** `misollar/` papkasi (15 ta misol)
- **Xatoliklar:** [GitHub Issues](https://github.com/timetolivechk-spec/uzpp/issues)
- **VS Code Marketplace:** [uzpp.uzpp](https://marketplace.visualstudio.com/items?itemName=uzpp.uzpp)

### Tashqi

- **CopperSpice:** [copperspice.com](https://www.copperspice.com)
- **C++23 standarti:** [eel.is/c++draft](https://eel.is/c++draft)
- **cppreference:** [en.cppreference.com](https://en.cppreference.com)

---

## G · Eng ko'p uchraydigan xatoliklar

| Xatolik | Sababi | Yechim |
|---------|--------|--------|
| `Xato: Fayl topilmadi` | Noto'g'ri fayl yo'li | Absolyut yo'ldan foydalaning yoki CWD ni tekshiring |
| `asosiy() qaytish turi butun bo'lishi kerak` | `bosh asosiy()` | `butun asosiy()` deb yozing |
| `Kalit so'z o'zgaruvchi nomi sifatida ishlatilishi mumkin emas` | O'zgaruvchi nomi kalit so'z bilan ustma-ust tushdi | Boshqa nom tanlang |
| `Yopilmagan belgi literal` | Apostrof noto'g'ri yopilgan | `'` juftligini tekshiring |
| `ld returned 1 exit status` | Havola xatosi yoki disk to'lgan | Disk joyini tekshiring, `--bare` flag'ini sinab ko'ring |
| `uzpp_runtime.hpp: No such file` | `--bare` flag'i kerak | `--bare` qo'shing yoki `-I stdlib` |
| `CsSignal::connect xatosi` | CopperSpice sarlavhalari topilmadi | `-I C:/CopperSpice/include/...` qo'shing |
| `0 ga bo'lib bo'lmadi` | Bo'luvchi 0 | `agar (b != 0)` tekshiring |
| `out_of_range` | Vektor/massiv chegaradan tashqari | Indeks va hajmni tekshiring |
| `bad_alloc` | Xotira tugadi | Memory leak yoki katta hajm |

---

## H · Kitob bo'yicha xulosa

Tabriklayman! Siz uz++ kitobini o'qib tamomladingiz. Endi sizning
oldingizda dasturlash dunyosining keng maydoni ochildi.

### Siz nimani o'rgandingiz?

- **Dasturlash asoslari** — o'zgaruvchilar, turlar, operatorlar
- **Mantiq va boshqaruv** — `agar`, sikllar, funksiyalar
- **Xotira boshqaruvi** — ko'rsatkichlar, havolalar, aqlli ko'rsatkichlar
- **Ma'lumotlar tuzilmalari** — vektor, xarita, to'plam, ro'yxat
- **OOP** — sinflar, meros, polimorfizm
- **Umumlashgan dasturlash** — shablonlar, STL
- **Xatoliklar boshqaruvi** — istisnolar, `Natija`, `Tanlov`
- **Matn va fayllar** — kuchli matn ishlash, JSON, fayl tizimi
- **Ko'p oqimlilik** — parallel dasturlash, sinxronizatsiya
- **GUI** — CopperSpice bilan grafik dastur yaratish

### Endi nima qilish?

1. **Loyihalar qiling.** Kitobni o'qish — bu birinchi qadam. Mahorat
   amaliyot orqali keladi.
2. **Boshqalarning kodini o'qing.** [misollar/](https://github.com/timetolivechk-spec/uzpp/tree/main/misollar)
   papkasidagi 15 ta misolni ko'rib chiqing.
3. **O'z kutubxonangizni yarating.** Foydali bo'lgan funksiyalarni
   to'plang.
4. **Hamjamiyatga qo'shiling.** Savol bering, javob bering, ko'maklashing.
5. **Open source ga hissa qo'shing.** uz++ ning o'zini ham
   yaxshilashga yordam berishingiz mumkin!

### So'nggi maslahat

Yaxshi dasturchining sirlari:

- **Har kuni kod yozing.** Hatto 30 daqiqa.
- **Xato qilishdan qo'rqmang.** Har bir xato — bu o'rganish.
- **Tezroq emas, aniqroq.** "Tez ishlaydigan xato kod" — bu yomon kod.
- **Boshqalardan o'rganing.** Hech kim hamma narsani bilmaydi.
- **Boshqalarga o'rgating.** O'rgatish — eng yaxshi o'rganish.

**uz++ — bu siz uchun. O'z tilingizda, o'z istalgan jarayonda, o'z
loyihalaringizni yarating!**

---

*Kitob yakuni. Ushbu qo'llanma sizga uz++ dasturlash tilini o'rganishda
yordam berdi degan umiddamiz. Savol, taklif yoki xatolik bo'lsa —
[GitHub Issues](https://github.com/timetolivechk-spec/uzpp/issues)
ga yozing.*

**uz++ — o'zbek tilida zamonaviy dasturlash!**

*Kitob versiyasi: v2.2.1 | 2026-yil may | © Shaxriyor Malikov | MIT
Litsenziyasi*
