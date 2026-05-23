# 07 · Massivlar va to'plamlar {#07-tuplamlar}

## Nima uchun to'plamlar kerak?

Tasavvur qiling, siz maktab darajachisiz va sizga 30 ta talaba
bahosini saqlash kerak. 30 ta o'zgaruvchini alohida e'lon qilasizmi?

```cpp
butun baho_1 = 85
butun baho_2 = 92
butun baho_3 = 78
// ... 27 ta yana
butun baho_30 = 88
```

Bu juda noqulay. Eng kichik vazifani bajarish — masalan, **o'rtacha
bahoni hisoblash** — 30 qator kod talab qiladi. Yangi talaba qo'shish
— yangi o'zgaruvchi yozish, hamma joyda yangilash.

**Yaxshi yechim — to'plam.** Bitta o'zgaruvchida 30 ta (yoki 30 ming)
qiymatni saqlash. uz++ da to'plamlarning ko'p turlari bor, har biri
o'zining maqsadi uchun. Bu bobda eng asosiylarini ko'rib chiqamiz:

- **Massiv (`array`)** — qat'iy o'lchamli to'plam
- **Vektor (`vektor`)** — dinamik o'lchamli (o'sib boradigan) massiv
- **Tartiblangan xarita (`map`)** — kalit-qiymat juftlari
- **To'plam (`set`)** — takrorlanmas elementlar
- **Juftlik va uchlik (`pair`, `tuple`)** — qisqacha guruhlash
- **Ixtiyoriy (`optional`)** — bo'lishi yoki bo'lmasligi mumkin qiymat

---

## Massivlar (Statik)

Eng oddiy to'plam. **Qat'iy uzunlikdagi** ketma-ket joylashgan
elementlar.

### E'lon qilish

```cpp
butun baholar[5] = {85, 92, 78, 90, 88}
```

Tahlil:
- `butun` — element turi
- `baholar` — to'plam nomi
- `[5]` — uzunligi (5 ta element)
- `{...}` — boshlang'ich qiymatlar

### Elementga kirish — indeks orqali

```cpp
yozish << baholar[0]          // 85 (birinchi)
yozish << baholar[2]          // 78 (uchinchi)
yozish << baholar[4]          // 88 (oxirgi)
```

**Diqqat!** Indeks **0 dan boshlanadi**, `n-1` da tugaydi (n — uzunlik).

```
baholar:    [85][92][78][90][88]
indeks:      0   1   2   3   4
```

### Element o'zgartirish

```cpp
baholar[2] = 80               // uchinchi bahoni o'zgartirdik
yozish << baholar[2]          // 80
```

### Bo'ylab yurish

```cpp
uchun (butun i = 0; i < 5; i++) {
    yozish << baholar[i] << " "
}
```

Yoki **range-based for**:

```cpp
uchun (butun baho : baholar) {
    yozish << baho << " "
}
```

Ikkinchi variant **soddaroq va xavfsizroq** — indeks haqida o'ylash
kerak emas.

### Chegaradan tashqari kirish — XATO

```cpp
butun baholar[5]
baholar[10] = 100             // ← XATO: out of bounds!
```

Kompilyator ba'zan buni ushlamaydi, lekin dastur ishga tushganda
kutilmagan natija beradi yoki ishdan chiqadi. C++ da bu chegarani
**siz** tekshirishingiz kerak.

### Massiv hajmi

```cpp
butun baholar[5]
butun n = sizeof(baholar) / sizeof(baholar[0])    // 5
```

Bu — `total_size / element_size`. Lekin bu mexanizm noqulay. Zamonaviy
yo'l: `std::array`:

```cpp
#include <array>
std::array<butun, 5> baholar = {85, 92, 78, 90, 88}
yozish << baholar.size()      // 5 — sodda!
```

---

## Vektor (`vektor`) — dinamik massiv

`vektor` (C++ da `std::vector`) — bu o'sib bora oladigan massiv. Hajm
oldindan ma'lum bo'lmasa, vektordan foydalaning.

### E'lon qilish

```cpp
#include <vector>

vektor<butun> sonlar                              // bo'sh
vektor<butun> baholar = {85, 92, 78, 90, 88}      // qiymat bilan
vektor<butun> nollar(10, 0)                        // 10 ta nol
```

### Element qo'shish — `.push_back()`

```cpp
vektor<butun> sonlar
sonlar.push_back(10)
sonlar.push_back(20)
sonlar.push_back(30)
// sonlar = {10, 20, 30}
```

`push_back` — vektorning **oxiriga** element qo'shadi.

### Element o'chirish — `.pop_back()`

```cpp
sonlar.pop_back()             // oxirgini olib tashlaydi
```

### Element soni — `.size()`

```cpp
yozish << sonlar.size()       // 2 (yuqorisidan keyin)
```

### Bo'sh ekanligini tekshirish

```cpp
agar (sonlar.empty()) {
    yozish << "Bo'sh"
}
```

### Elementga kirish

```cpp
sonlar[0]                     // tezroq, lekin chegaradan tashqari xavfli
sonlar.at(0)                  // sekinroq, lekin chegaradan tashqari exception
```

`.at()` — agar chegaradan tashqari bo'lsa, **xatolik ko'taradi**
(crash o'rniga). Xavfsizroq.

### Bo'ylab yurish

```cpp
// Indeks bilan
uchun (butun i = 0; i < sonlar.size(); i++) {
    yozish << sonlar[i] << " "
}

// Range-based for (afzal)
uchun (butun son : sonlar) {
    yozish << son << " "
}

// Havola bilan (o'zgartirish uchun)
uchun (butun& son : sonlar) {
    son *= 2                  // har birini ikkilantir
}
```

### Vektor tozalash

```cpp
sonlar.clear()                // hamma elementlarni olib tashlaydi
```

### Misol: Ortacha baho

```cpp
vektor<butun> baholar = {85, 92, 78, 90, 88, 95}

butun yigindi = 0
uchun (butun b : baholar) {
    yigindi += b
}
ikkilangan ortacha = ikkilangan(yigindi) / baholar.size()
yozish << "O'rtacha: " << ortacha << qator_oxiri
```

---

## `std::map` — tartiblangan xarita

`map` — bu **kalit-qiymat juftlari** to'plami. Tartibga keltirilgan.

### Misol — talaba bahosi xaritasi

```cpp
#include <map>

std::map<matn, butun> baholar
baholar["Aziza"] = 92
baholar["Bobur"] = 85
baholar["Karim"] = 78

yozish << baholar["Aziza"]    // 92
```

Bu yerda:
- Kalit: `matn` (ism)
- Qiymat: `butun` (baho)

### Element bor yoki yo'qligini tekshirish

```cpp
agar (baholar.count("Aziza") > 0) {
    yozish << "Aziza topildi"
}

// Yoki — find orqali
agar (baholar.find("Aziza") != baholar.end()) {
    yozish << "Topildi"
}
```

> **Diqqat!** `baholar["Dilshoda"]` — agar "Dilshoda" yo'q bo'lsa,
> **avtomatik yaratadi** (qiymati 0 bilan)! Buni faqat o'qish uchun
> ishlatmang.

### Bo'ylab yurish

```cpp
uchun (o'zgarmas avto& juft : baholar) {
    yozish << juft.first << ": " << juft.second << qator_oxiri
}
```

**Natija** (tartiblangan):

```
Aziza: 92
Bobur: 85
Karim: 78
```

Map elementlari **kalit bo'yicha** avtomatik tartibga keltiriladi.

### Element o'chirish

```cpp
baholar.erase("Bobur")
```

---

## `std::set` — takrorlanmas to'plam

`set` — bu **takrorsiz** elementlar to'plami. Tartiblangan.

```cpp
#include <set>

std::set<butun> raqamlar
raqamlar.insert(5)
raqamlar.insert(3)
raqamlar.insert(5)            // takror — qo'shilmaydi
raqamlar.insert(7)

yozish << raqamlar.size()      // 3 (5, 3, 7)
```

### Foydali amallar

```cpp
agar (raqamlar.count(5) > 0) {
    yozish << "5 bor"
}

raqamlar.erase(3)              // 3 ni o'chirish
```

### Misol: noyob so'zlarni sanash

```cpp
vektor<matn> matnlar = {"olma", "anor", "olma", "anor", "uzum"}
std::set<matn> noyob(matnlar.begin(), matnlar.end())

yozish << "Noyob so'zlar soni: " << noyob.size()   // 3
```

---

## Boshqa ketma-ketlik to'plamlari

### `std::deque` (juft tomonlama navbat)

Vektor kabi, lekin **ikkala tomondan** ham qo'shish/olib tashlash tez:

```cpp
#include <deque>
std::deque<butun> dq
dq.push_back(1)
dq.push_front(0)              // boshiga qo'shish
// dq = {0, 1}
```

### `std::list` (bog'langan ro'yxat)

Element qo'shish/olib tashlash o'rtada ham tez. Lekin indeks bilan
kirish sekin.

### Qaysi birini qachon ishlatish?

| Holat | To'plam |
|-------|---------|
| Hajm aniq, kichik | `array` |
| Hajm o'zgaruvchan, oxiriga qo'shish | `vektor` |
| Ikkala tomondan qo'shish | `deque` |
| O'rtada ko'p qo'shish/olib tashlash | `list` |
| Kalit-qiymat | `map` |
| Takrorsiz qiymatlar | `set` |

---

## `std::pair` — juftlik

Ikki qiymatni birga saqlash uchun:

```cpp
std::pair<matn, butun> talaba = {"Aziza", 92}
yozish << talaba.first         // "Aziza"
yozish << talaba.second        // 92
```

### Foydali misol

```cpp
vektor<std::pair<matn, butun>> baholar = {
    {"Aziza", 92},
    {"Bobur", 85},
    {"Karim", 78}
}

uchun (o'zgarmas avto& p : baholar) {
    yozish << p.first << ": " << p.second << qator_oxiri
}
```

`pair` map dan farqli — tartiblanmagan va kalit qaytaruvi mumkin
(takror).

---

## `std::tuple` — uchlik va undan ko'p

Uchta yoki undan ko'p qiymat birga:

```cpp
std::tuple<matn, butun, ikkilangan> talaba = {"Aziza", 22, 4.7}
yozish << std::get<0>(talaba)         // "Aziza"
yozish << std::get<1>(talaba)         // 22
yozish << std::get<2>(talaba)         // 4.7
```

Yoki `structured binding`:

```cpp
o'zgarmas avto [ism, yosh, baho] = talaba
yozish << ism << " " << yosh << " " << baho
```

---

## `std::optional` — bo'lishi mumkin

Ba'zan qiymat **bo'lmasligi** mumkin. Masalan, qidirish funksiyasi:

```cpp
std::optional<butun> qidirish(vektor<butun> sonlar, butun maqsad) {
    uchun (butun i = 0; i < sonlar.size(); i++) {
        agar (sonlar[i] == maqsad) {
            qaytarish i               // topildi
        }
    }
    qaytarish std::nullopt           // topilmadi
}

butun asosiy() {
    vektor<butun> v = {10, 20, 30}
    avto natija = qidirish(v, 20)

    agar (natija) {
        yozish << "Topildi, indeks: " << *natija
    }
    aks_holda {
        yozish << "Topilmadi"
    }
}
```

`std::optional` — modern C++ ning eng foydali xususiyatlaridan biri.
"qiymat bor yoki yo'q" holatini tabiiy ko'rsatish uchun.

---

## Amaliy misol: Talabalar jurnali

```cpp
#include <vector>
#include <map>

tuzilma Talaba {
    matn ism
    butun yoshi
    vektor<butun> baholar
}

butun asosiy() {
    Talaba t1 = {"Aziza", 22, {85, 92, 78}}
    Talaba t2 = {"Bobur", 21, {90, 88, 95}}
    Talaba t3 = {"Karim", 23, {65, 70, 75}}

    vektor<Talaba> guruh = {t1, t2, t3}

    uchun (o'zgarmas Talaba& t : guruh) {
        butun yigindi = 0
        uchun (butun b : t.baholar) yigindi += b
        ikkilangan ortacha = ikkilangan(yigindi) / t.baholar.size()

        yozish << t.ism << " (" << t.yoshi << " yosh): "
               << "o'rtacha = " << ortacha << qator_oxiri
    }

    qaytarish 0
}
```

**Natija:**

```
Aziza (22 yosh): o'rtacha = 85
Bobur (21 yosh): o'rtacha = 91
Karim (23 yosh): o'rtacha = 70
```

Bu — real dasturning kichik fragmenti. Kollej, ma'lumotlar bazasi,
talaba boshqaruv tizimi.

---

## Eng ko'p uchraydigan xatolar

### 1. Chegaradan tashqari kirish

```cpp
vektor<butun> v = {1, 2, 3}
v[5] = 10                     // ← XATO yoki crash
```

### 2. Bo'sh vektordan element olish

```cpp
vektor<butun> v
yozish << v[0]                // ← XATO
```

Tekshiring: `agar (!v.empty())`.

### 3. Map elementiga kirib unga avtomatik element qo'shish

```cpp
std::map<matn, butun> m
agar (m["yangi"] == 0) {      // ← "yangi" ni yaratdi (0 qiymat bilan)!
    // ...
}
// To'g'risi: m.count("yangi") yoki m.find()
```

### 4. Iteratorni o'chirib davom etish

```cpp
vektor<butun> v = {1, 2, 3, 4, 5}
uchun (avto it = v.begin(); it != v.end(); it++) {
    agar (*it == 3) {
        v.erase(it)            // ← XATO: it endi tutashmagan
    }
}
```

To'g'risi:
```cpp
uchun (avto it = v.begin(); it != v.end(); ) {
    agar (*it == 3) {
        it = v.erase(it)       // erase yangi iterator qaytaradi
    }
    aks_holda {
        it++
    }
}
```

### 5. Vektor nusxasini funksiyaga uzatish

```cpp
butun yigindi(vektor<butun> v) {       // nusxa — sekin!
    // ...
}

butun yigindi(o'zgarmas vektor<butun>& v) {   // havola — tez
    // ...
}
```

---

## Bob bo'yicha mashqlar

### Mashq 1: Massivning yig'indisi va o'rtachasi (oson)

Foydalanuvchidan 10 ta son so'rang. Yig'indisi va o'rtachasini
chiqaring.

### Mashq 2: Eng katta va kichik (oson)

Vektor ichida eng katta va eng kichik elementlarni toping.

### Mashq 3: Sonlarni teskari tartibda chiqarish (o'rta)

Foydalanuvchidan vektor olin va uni teskari tartibda chiqaring.

### Mashq 4: Takrorlarni topish (o'rta)

Vektor berilgan — undagi takror keladigan sonlarni topib chiqaring.
Maslahat: `map` ishlating, har bir sonni necha marta ko'rganini
sanang.

### Mashq 5: So'zlar chastotasi (o'rtacha)

Matn berilgan. Har bir so'z necha marta uchrayotganini sanang va
chiqaring.

### Mashq 6: Tartiblash (qiyinroq)

Vektor sonlarini ostidan kattagacha tartiblang. `std::sort` ishlatmasdan
— bubble sort yoki insertion sort algoritmini o'zingiz yozing.

### Mashq 7: To'plamlar amallar (qiyin)

Ikkita `set` berilgan. Ularning birlashishi (union), kesishishi
(intersection) va farqi (difference) ni hisoblang.

---

## Xulosa

- **Massiv** — qat'iy uzunlikdagi to'plam.
- **Vektor** — dinamik o'sadigan massiv. Eng ko'p ishlatiladigan.
- **Map** — kalit-qiymat juftlari, tartiblangan.
- **Set** — takrorsiz to'plam.
- **Pair, tuple** — bir necha qiymatni guruhlash.
- **Optional** — qiymat bo'lishi yoki bo'lmasligi mumkin.
- **Bo'ylab yurish** — `uchun` orqali, indeks yoki range-based.
- **Eng ko'p xato** — chegaradan tashqari kirish.

Keyingi bobda biz **sinflar (klasses)** va **obyektga yo'naltirilgan
dasturlash** (OOP) ni o'rganamiz. Bu — zamonaviy dasturlashning eng
muhim asoslaridan biri.
