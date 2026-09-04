# 14 · Standart kutubxona {#14-stdlib}

## uz++ standart kutubxonasi haqida

uz++ — bu C++23 ustida qurilgan. Demak, sizning ixtiyoringizda C++
ning **butun** standart kutubxonasi bor. Bundan tashqari, uz++ o'zining
o'zbek tilidagi modullarini taqdim etadi.

Bu bobda biz uz++ standart kutubxonasini va shu bilan birga foydali
C++ modullarini umumiy ko'rib chiqamiz. Bu — ma'lumot bobi. Har bir
moduldan amalda foydalanganingiz sayin chuqurroq tushunasiz.

### uz++ standart modullari

Hozirda o'zbeklashtirilgan modullar:

| Modul | Tavsif |
|-------|--------|
| `matn` | Matn (string) bilan ishlash |
| `xatoliklar` | Xatolik turlari va istisnolar |
| `vaqt` | Vaqt va sana bilan ishlash |
| `matematika` | Matematik funksiyalar |
| `sinov` | Test framework |
| `jurnal` | Logging (jurnallash) |

---

## `matematika` moduli

Matematik funksiyalar va konstantalar.

```cpp
ulash "matematika.uzpp"

// Konstantalar
yozish << matematika::PI; // 3.14159...
yozish << matematika::E; // 2.71828...

// Asosiy funksiyalar
yozish << matematika::ildiz(16); // 4 (kvadrat ildiz)
yozish << matematika::quvvat(2, 10); // 1024 (2^10)
yozish << matematika::log(100); // 4.605... (tabiiy logarifm)
yozish << matematika::log10(1000); // 3 (o'nlik logarifm)

// Trigonometriya
yozish << matematika::sinus(matematika::PI / 2); // 1.0
yozish << matematika::kosinus(0); // 1.0
yozish << matematika::tangens(matematika::PI / 4); // 1.0

// Yaxlitlash
yozish << matematika::yaxlitla(3.7); // 4
yozish << matematika::pastga(3.7); // 3
yozish << matematika::yuqoriga(3.2); // 4

// Min/max/abs
yozish << matematika::min(3, 7); // 3
yozish << matematika::maks(3, 7); // 7
yozish << matematika::absolyut(-5); // 5
```

---

## `vaqt` moduli

Vaqt va sana bilan ishlash.

```cpp
ulash "vaqt.uzpp"

// Joriy vaqt
o'zgaruvchan hozir = uzpp::Vaqt::hozir();
yozish << "Hozir: " << hozir.formatla("%Y-%m-%d %H:%M:%S");

// Vaqt elementlari
yozish << "Yil: " << hozir.yil();
yozish << "Oy: " << hozir.oy();
yozish << "Kun: " << hozir.kun();
yozish << "Soat: " << hozir.soat();

// Sana yaratish
o'zgaruvchan sana = uzpp::Vaqt::Sana(2026, 5, 21);

// Farq
o'zgaruvchan kelajak = sana.qoshish_kun(30);
yozish << kelajak.formatla("%Y-%m-%d");

// Vaqt o'lchash
o'zgaruvchan bosh = uzpp::Vaqt::hozir();
// ... biron ish ...
o'zgaruvchan oxir = uzpp::Vaqt::hozir();
yozish << "O'tdi: " << (oxir - bosh).millisekund() << " ms";
```

---

## `sinov` moduli — Test framework

uz++ da test yozish uchun sodda framework. Ikkita yo'l bor.

### 1-yo'l: `@sinov` atributi (eng qulay)

`@sinov` — bu **funksiya atributi**. Uni `bosh` qaytaruvchi funksiya
oldiga qo'yasiz, funksiya nomi esa test nomi bo'ladi:

```cpp
ulash "uzpp_runtime.hpp"

butun yigindi(butun a, butun b) {
    qaytarish a + b;
}

@sinov bosh yigindi_musbat_sonlar() {
    uzpp::Sinov::tasdiqlash(yigindi(2, 3) == 5, "2 + 3 = 5 bo'lishi kerak");
    uzpp::Sinov::tasdiqlash_teng(0, yigindi(0, 0));
}

@sinov bosh yigindi_manfiy_sonlar() {
    uzpp::Sinov::tasdiqlash_teng(0, yigindi(-1, 1));
    uzpp::Sinov::tasdiqlash_teng(-5, yigindi(-2, -3));
}

butun asosiy() {
    yozish << "Oddiy ishga tushirish" << qator_oxiri;
    qaytarish 0;
}
```

Testlarni ishga tushirish:

```bash
uzpp sinov mening_testlarim.uzpp
```

`uzpp sinov` `asosiy` ni chetlab o'tadi va `@sinov` funksiyalarini
yig'ib, o'z test yurituvchisini quradi. Natija:

```
=============================================
  uz++ sinovlari
=============================================
  [1/2] yigindi_musbat_sonlar ... OTDI
  [2/2] yigindi_manfiy_sonlar ... OTDI
---------------------------------------------
  Jami:  2
  O'tdi: 2
  Quladi:0
=============================================
```

Bitta test qulasa, jarayon nolga teng bo'lmagan kod bilan tugaydi —
CI da shuning o'zi yetarli.

### Tasdiqlash funksiyalari

| Funksiya | Nima tekshiradi |
|----------|-----------------|
| `uzpp::Sinov::tasdiqlash(shart, xabar)` | shart `rost` ekanini |
| `uzpp::Sinov::tasdiqlash_teng(kutilgan, qiymat, xabar)` | `kutilgan == qiymat` |
| `uzpp::Sinov::tasdiqlash_yaqin(kutilgan, qiymat, epsilon, xabar)` | haqiqiy sonlar yaqinligini |

`xabar` — ixtiyoriy; test qulaganda shu matn chiqadi.

### 2-yo'l: `TestTo'plami` — to'plamni qo'lda yig'ish

Testlarni dastur ichida o'zingiz boshqarmoqchi bo'lsangiz:

```cpp
ulash "uzpp_runtime.hpp"

butun asosiy() {
    uzpp::Sinov::TestTo'plami sinovlar("Matematika testlari");

    sinovlar.test_qoshish("qo'shish", []() {
        uzpp::Sinov::tasdiqlash(2 + 2 == 4);
    });

    sinovlar.test_qoshish("bo'lish", []() {
        uzpp::Sinov::tasdiqlash_yaqin(2.5, 5.0 / 2.0, 0.0001);
    });

    qaytarish sinovlar.ishga_tushirish();
}
```

Bu shaklda `uzpp ishga-tushirish` yetarli — maxsus buyruq kerak emas.

---

## `@bench` — tezlikni o'lchash

`@sinov` bilan bir xil ishlaydi, lekin funksiyani ming marta
takrorlab, bitta amalga ketgan o'rtacha vaqtni chiqaradi:

```cpp
ulash "uzpp_runtime.hpp"

butun fib(butun n) {
    agar (n < 2) qaytarish n;
    qaytarish fib(n - 1) + fib(n - 2);
}

@bench bosh fib_20() {
    o'zgaruvchan natija = fib(20);
    agar (natija < 0) {
        yozish << natija;   // natija ishlatilmasa, kompilyator uni tashlab yuborishi mumkin
    }
}

butun asosiy() { qaytarish 0; }
```

```bash
uzpp bench mening_olchovlarim.uzpp
```

```
=============================================
  Benchmark (1000 marta takrorlash)
=============================================
  [1/1] fib_20 ... 11054.2 ns/amal  (jami 11.0542 ms)
=============================================
```

Birinchi chaqiruv "isitish" uchun ishlatiladi va o'lchovga
kirmaydi — kesh va lazy-init ta'siri natijani buzmasligi uchun.

---

## `jurnal` moduli — Logging

Dasturning ishini kuzatish uchun:

```cpp
ulash "jurnal.uzpp"

butun asosiy() {
    uzpp::Jurnal::malumot("Dastur boshlandi");
    uzpp::Jurnal::ogohlantirish("Bu  ogohlantirish");
    uzpp::Jurnal::xato("Xato yuz berdi");

    butun yosh = 25;
    uzpp::Jurnal::malumot("Foydalanuvchi yoshi: " + std::to_string(yosh));
    qaytarish 0;
}
```

**Natija:**

```
[2026-05-21 14:30:00] [MA'LUMOT] Dastur boshlandi
[2026-05-21 14:30:00] [OGOHLANTIRISH] Bu — ogohlantirish
[2026-05-21 14:30:00] [XATO] Xato yuz berdi
[2026-05-21 14:30:00] [MA'LUMOT] Foydalanuvchi yoshi: 25
```

Faylga yozish ham mumkin:

```cpp
uzpp::Jurnal::fayl("app.log");
uzpp::Jurnal::malumot("Bu fayl ga yoziladi");
```

---

## `kripto` moduli — Kriptografiya

Hash, shifrlash, parol himoyasi:

```cpp
ulash "kripto.uzpp"

// MD5 / SHA-256 hash
matn s = "Salom, dunyo!";
yozish << kripto::md5(s);
yozish << kripto::sha256(s);

// Parolni xavfsiz saqlash
matn parol = "qiyin_parol_123";
matn hash = kripto::parol_hash(parol);

// Tekshirish
mantiqiy togri = kripto::parol_tekshirish("qiyin_parol_123", hash);
```

---

## `tarmoq` moduli — Tarmoq aloqalari

HTTP so'rovlari, socket'lar:

```cpp
ulash "tarmoq.uzpp"

// HTTP GET so'rov
o'zgaruvchan javob = tarmoq::http_oluvchi("https://api.example.com/data")

agar (javob.holat == 200) {
    yozish << javob.matn;
}

// POST so'rov
tarmoq::http_yuborish("https://api.example.com/post", json_data)
```

---

## `suniy_intellekt` moduli — AI integratsiyasi

LLM va AI bilan ishlash:

```cpp
ulash "suniy_intellekt.uzpp"

o'zgaruvchan suxbat = suniy_intellekt::Suhbat();
suxbat.tizim("Sen foydali yordamchisan");

matn savol = "C++ da const ko'rsatkich qanday yaratiladi?";
matn javob = suxbat.sorash(savol);
yozish << javob;
```

---

## C++ standart kutubxonasidan foydalanish

Eng ko'p ishlatiladigan modullar:

### `<algorithm>` — Algoritmlar

```cpp
ulash <algorithm>
vektor<butun> v = {3, 1, 4, 1, 5, 9, 2, 6}

std::sort(v.begin(), v.end());
std::reverse(v.begin(), v.end());
butun joy = std::count(v.begin(), v.end(), 1);
o'zgaruvchan max_el = std::max_element(v.begin(), v.end());

// Filtering
vektor<butun> juft_lar;
std::copy_if(v.begin(), v.end(), std::back_inserter(juft_lar),
    [](butun x) { qaytarish x % 2 == 0 });

// Map (transform)
vektor<butun> kvadratlar;
std::transform(v.begin(), v.end(), std::back_inserter(kvadratlar),
    [](butun x) { qaytarish x * x });
```

### `<random>` — Tasodifiy sonlar

```cpp
ulash <random>

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dist(1, 100);

butun tasodif = dist(gen); // 1 dan 100 gacha
```

### `<chrono>` — Vaqt o'lchovi

```cpp
ulash <chrono>

o'zgaruvchan bosh = std::chrono::high_resolution_clock::now();
// ... ish ...
o'zgaruvchan oxir = std::chrono::high_resolution_clock::now();

o'zgaruvchan millisekund = std::chrono::duration_cast<std::chrono::milliseconds>(oxir - bosh);
yozish << millisekund.count() << " ms";
```

### `<regex>` — Regular expressions

```cpp
ulash <regex>

matn s = "Telefon: +998901234567, email: aziza@example.com";
std::regex telefon(R"(\+\d{12})");
std::smatch m;

agar (std::regex_search(s, m, telefon)) {
    yozish << "Telefon topildi: " << m[0];
}
```

### `<thread>`, `<mutex>`, `<future>` — Ko'p oqimlilik

13-bobda ko'rdik.

### `<filesystem>` — Fayl tizimi

12-bobda ko'rdik.

---

## Amaliy misol: Vaqt hisoblagichi bilan testlash

```cpp
ulash "vaqt.uzpp"
ulash "jurnal.uzpp"
ulash <vector>
ulash <algorithm>
ulash <chrono>

butun asosiy() {
    uzpp::Jurnal::malumot("Test boshlandi");

    o'zgaruvchan bosh = std::chrono::high_resolution_clock::now();

    // Vazifa: 1 million tasodifiy son yaratib, tartiblash
    vektor<butun> v(1000000);
    std::generate(v.begin(), v.end(), []() { qaytarish std::rand(); });

    std::sort(v.begin(), v.end());

    o'zgaruvchan oxir = std::chrono::high_resolution_clock::now();
    o'zgaruvchan ms = std::chrono::duration_cast<std::chrono::milliseconds>(oxir - bosh).count();

    uzpp::Jurnal::malumot("Tartiblash vaqti: " + std::to_string(ms) + " ms");

    qaytarish 0;
}
```

Bu dasturning chiqishi:

```
[2026-05-21 14:30:00] [MA'LUMOT] Test boshlandi
[2026-05-21 14:30:01] [MA'LUMOT] Tartiblash vaqti: 234 ms
```

---

## Bob bo'yicha mashqlar

### Mashq 1: Tasodifiy son (oson)

`<random>` orqali 1 dan 100 gacha tasodifiy 10 ta son chiqaring.

### Mashq 2: Vaqt o'lchash (oson)

`<chrono>` orqali Fibonachchi 30-elementni hisoblash vaqtini o'lchang.

### Mashq 3: Regex (o'rta)

Berilgan matnda barcha email manzillarni toping.

### Mashq 4: Log fayl tahlili (o'rtacha)

Log fayldan barcha xato satrlarini boshqa faylga yozing.

### Mashq 5: Test framework (qiyinroq)

O'zingizning matematik funksiyalaringiz uchun testlar yozing.

---

## Xulosa

- uz++ standart kutubxonasi (`matn`, `vaqt`, `matematika`, `sinov`,
  `jurnal`, `kripto`, `tarmoq`, `suniy_intellekt`) — o'zbek tilida.
- C++ standart kutubxonasi (`<algorithm>`, `<random>`, `<regex>`,
  `<chrono>`, `<thread>`, `<filesystem>`) — to'liq mavjud.
- Har bir modul aniq vazifa uchun mo'ljallangan.
- Yangi modullar doim qo'shilmoqda.

Keyingi bobda biz **GUI dasturlash** ni o'rganamiz — grafik
interfeysli dasturlar yaratish.
