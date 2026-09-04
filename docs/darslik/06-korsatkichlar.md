# 06 · Ko'rsatkichlar va havolalar {#06-korsatkichlar}

## Bu bobning maqsadi

Ko'rsatkichlar (pointers) — C++ (va shu sababli uz++) ning eng kuchli,
shu bilan birga eng qiyin tushunarli xususiyatidan biri. Ko'p
boshlovchilar bu yerda to'xtab qoladi. Bu bobni o'tib bo'lganingizdan
keyin, siz quyidagilarga ega bo'lasiz:

- Xotira qanday tashkil etilganini chuqur tushunish
- Funksiyalarni samarali yozish ko'nikmasi (havolalar bilan)
- Dinamik xotirani boshqarish — kerakli paytda olib, kerakli paytda
  qaytarish
- Klassik dasturlash strukturalarini yaratish (linked list, daraxt)

Boshlanglar — bu boshlanish, oxiri emas. Sabr-toqat bilan!

---

## Xotira tashkil etilishi

Avval xotirani umumiy ko'rib chiqaylik. Sizning kompyuteringizdagi
xotira (RAM) — bu juda uzun lenta. Har bir katakda 1 bayt (8 bit)
saqlanadi. Har bir katakning **manzili** (address) bor — bu son.

```
Manzil:    0x1000  0x1001  0x1002  0x1003  0x1004  ...
Mazmun:      72      0       0       0      'A'    ...
            ↑                              ↑
       butun (4 bayt)                belgi (1 bayt)
```

Yuqorida:
- `0x1000` — `0x1003` manzillarida butun `72` saqlanadi (`butun` 4 bayt).
- `0x1004` da `'A'` belgisi (`belgi` 1 bayt).

Manzillar **o'n oltilik** (hexadecimal) sistemada yoziladi — shuning
uchun `0x` prefiks bor. Bu shunchaki katta sonlarni qisqaroq yozish
usuli.

Endi savol: agar siz **o'zgaruvchining manzilini bilsangiz**, unga
to'g'ridan-to'g'ri kira olasizmi? Javob — **ha**. Bu — **ko'rsatkich**
deganda nazarda tutilgan narsa.

---

## Havola (Reference) — sodda boshlash

Havolaga (`&`) biz funksiyalar bobida (5-bob) qisqacha tegingan edik.
Endi batafsil ko'rib chiqaylik.

**Havola** — bu o'zgaruvchining yangi nomi (psevdonimi). Tasavvur
qiling, sizning ismingiz "Akmal", lekin do'stlaringiz sizni "Aki"
deyishadi. Aki = Akmal. Ikkala nom — _bir xil odam_.

```cpp
butun yosh = 25;
butun& yosh_psevdonim = yosh;

yosh_psevdonim = 30; // bu yosh ni ham o'zgartiradi!
yozish << yosh; // 30
yozish << yosh_psevdonim; // 30 (bir xil narsa!)
```

### Havola qoidalari

1. **Havola e'lon paytida shart initsializatsiya qilinishi kerak**:
   ```cpp
   butun& y; // ← XATO: nimaga havola?
   butun& y = yosh; // TO'G'RI
   ```

2. **Havola boshqa o'zgaruvchiga "ko'chirilmaydi"**:
   ```cpp
   butun a = 5, b = 10;
   butun& r = a;
   r = b; // Bu — a ga b ning qiymatini saqlaydi (a = 10)
                             // r endi b ga ko'rsatadi DEYIL
   ```

3. **`null` havola yo'q** — havola doim biror narsani ko'rsatadi.

### Havolalar nima uchun foydali?

**1. Funksiyalarda parametr o'zgartirish:**
```cpp
bosh almashtirish(butun& a, butun& b) {
    butun temp = a;
    a = b;
    b = temp;
}

butun asosiy() {
    butun x = 5, y = 10;
    almashtirish(x, y);
    yozish << x << " " << y; // 10 5
}
```

`&` siz: `almashtirish(x, y)` ichida `a` va `b` — `x` va `y` ning
nusxalari. Ularni o'zgartirsangiz, asl o'zgaruvchilarga ta'sir qilmaydi.
`&` bilan — bu xuddi bir xil o'zgaruvchilar.

**2. Katta o'zgaruvchilarni samarali uzatish:**
```cpp
butun matn_uzunligi(o'zgarmas matn& s) {
    qaytarish s.length();
}
```

`o'zgarmas matn&` — havola (tez), lekin o'zgartirib bo'lmaydi (xavfsiz).
Eng yaxshi parametr berish usuli.

---

## Ko'rsatkich (Pointer)

Endi keling, ko'rsatkichga o'tamiz. **Ko'rsatkich** — bu boshqa
o'zgaruvchining **manzilini** saqlovchi o'zgaruvchi.

### Ko'rsatkichni e'lon qilish
```cpp
butun a = 42;
butun* p = &a; // p — a ning manzilini saqlaydi
```

Tahlil:
- `butun*` — "butun turidagi qiymatga ko'rsatuvchi ko'rsatkich"
- `&a` — "a ning manzili" (e'tibor: bu `&` boshqa kontekstda — havola
  emas, **manzilni olish** operatori)
- `p` — endi `a` ning manziliga ko'rsatadi

### `*` (dereference) — qiymatga kirish

Ko'rsatkichdan qiymatga kirish uchun `*` ishlatamiz:
```cpp
butun a = 42;
butun* p = &a;

yozish << p; // a ning manzili (qandaydir son)
yozish << *p; // a ning qiymati (42)

*p = 100; // a ga 100 ni saqlash
yozish << a; // 100
```

`*p` — "p ko'rsatadigan joydagi qiymat" demakdir.

### Visualization
```
Xotira:
┌──────────┬──────────┐
│ Manzil   │ Mazmun   │
├──────────┼──────────┤
│ 0x1000   │   42     │  ← a
├──────────┼──────────┤
│ 0x2000   │  0x1000  │  ← p (a ning manzili)
└──────────┴──────────┘

p     = 0x1000  (a ning manzili)
*p    = 42      (manzildagi qiymat)
&a    = 0x1000  (a ning manzili)
&p    = 0x2000  (p ning manzili)
```

---

## `nullptr` — bo'sh ko'rsatkich

`nullptr` — bu **hech narsaga ko'rsatmaslik** qiymati. Yangi ko'rsatkich
yaratayotganda, qiymat berishni unutmang:
```cpp
butun* p = nullptr; // bo'sh ko'rsatkich
```

`nullptr` ni `*` qilish — **xato**! Dastur ishdan chiqadi:
```cpp
butun* p = nullptr;
*p = 5; // ← XATO: dastur "crash" qiladi
```

Doim tekshiring:
```cpp
agar (p != nullptr) {
    *p = 5;
}
```

---

## Dinamik xotira: `yangi` va `o'chirish`

Hozirgacha biz e'lon qilgan barcha o'zgaruvchilar **stack** xotirasida
yashardi. Stack — bu funksiya tugashi bilan tozalanadigan vaqtinchalik
xotira.

Lekin ba'zan bizga **dinamik xotira** kerak — funksiya tugagandan
keyin ham yashaydigan. Bu **heap** xotirasi.

### `yangi` (new) — heap'dan xotira olish
```cpp
butun* p = yangi butun(42); // heap'da yangi butun, qiymati 42
yozish << *p; // 42
```

`yangi butun(42)` — "heap'da yangi butun yarat, qiymati 42 bo'lsin,
manzilini qaytar".

### `o'chirish` (delete) — xotirani qaytarish
```cpp
butun* p = yangi butun(42);
*p = 100;
o'chirish p; // xotirani qaytarish (free)
p = nullptr; // havsiz ko'rsatkich qoldirma
```

**Muhim qoida:** Har bir `yangi` ga **bitta** `o'chirish` mos kelishi
kerak. Aks holda — **xotira sizmasi** (memory leak)!

### Massivlar uchun
```cpp
butun* sonlar = yangi butun[10]; // 10 ta butun
sonlar[0] = 1;
sonlar[1] = 2;
// ...

o'chirish[] sonlar; // diqqat — [] qo'shing!
```

> **Real hayotdagi misol:** Stack — restoranning stoli (taom keldi,
> yedingiz, ketdingiz — stol tozalanadi). Heap — sizning oshxonangiz
> (siz nima qo'ysangiz, o'zingiz tozalashingiz kerak). Agar siz
> qo'ydingiz va unutsangiz — bardoshli iflos joy hosil bo'ladi.

### Memory leak — eng yomon kelishuv
```cpp
bosh funksiya() {
    butun* p = yangi butun(42);
    // p ni `o'chirish` qilmadik!
}

// Bu funksiya har chaqirilganda 4 bayt xotira "yo'qoladi"
// Agar 1 milliard marta chaqirsangiz — 4 GB yo'qolgan!
```

---

## Aqlli ko'rsatkichlar (Smart Pointers)

`yangi/o'chirish` xato manbai. Zamonaviy C++ (C++11+) bu masalani hal
qiladi: **aqlli ko'rsatkichlar**.

### `yagona_korsatkich` (unique_ptr)

Avtomatik tozalanadigan ko'rsatkich. Faqat bitta egasi bor:
```cpp
ulash "uzpp_runtime.hpp"
ulash <memory>

butun asosiy() {
    yagona_korsatkich<butun> p = std::make_unique<butun>(42);
    yozish << *p << qator_oxiri; // 42

    // funksiya tugagach, p avtomatik tozalanadi
    qaytarish 0;
}
```

`o'chirish` ni yozish kerak emas! Tozalash avtomatik.

### `umumiy_korsatkich` (shared_ptr)

Bir necha egasi bo'lishi mumkin. Oxirgi egasi yo'qolganda — xotira
tozalanadi:
```cpp
umumiy_korsatkich<butun> p1 = std::make_shared<butun>(42);
umumiy_korsatkich<butun> p2 = p1; // ikkala egasi
yozish << *p1 << " " << *p2; // 42 42

// p1 va p2 ikkalasi yo'qolgach — xotira tozalanadi
```

**Tavsiya:** Yangi kodda har doim aqlli ko'rsatkichlar ishlating, oddiy
`yangi/o'chirish` emas. Bu siz uchun katta foyda — xato kamayadi.

---

## Ko'rsatkich arifmetikasi

Massiv elementlariga `*` orqali kirish mumkin:
```cpp
butun massiv[5] = {10, 20, 30, 40, 50}
butun* p = massiv; // birinchi elementga ko'rsatadi

yozish << *p; // 10
p++; // keyingi elementga o'tdi
yozish << *p; // 20
p += 2; // 2 ta oldinga
yozish << *p; // 40
```

`p++` faqat 1 oshmaydi — u **`butun` hajmi**ga (4 bayt) oshadi! Bu
nima uchun ishlaydi: kompilyator ko'rsatkich turidan kelib chiqib,
qanchalik siljitish kerakligini biladi.

> **Ehtiyot bo'ling:** Ko'rsatkich arifmetikasi xavfli — chegaradan
> tashqari joyga o'tib qolishingiz mumkin (out-of-bounds). Buni `vektor`
> kabi konteynerlar (7-bob) hal qiladi.

---

## Ko'rsatkich va `const`

Uchta xil "const ko'rsatkich" bor:
```cpp
// 1. O'zgarmas qiymatga ko'rsatadigan
o'zgarmas butun* p1 = &a;
// *p1 = 10   // XATO: qiymatni o'zgartirib bo'lmaydi
// p1 = &b    // OK: p1 ni boshqa joyga qaratish mumkin

// 2. O'zgarmas ko'rsatkich
butun* o'zgarmas p2 = &a;
// *p2 = 10   // OK
// p2 = &b    // XATO

// 3. Ikkala
o'zgarmas butun* o'zgarmas p3 = &a;
// *p3 = 10   // XATO
// p3 = &b    // XATO
```

Bu narsa yangi boshlovchilar uchun chalkash, lekin amaliyotda foydali.
Kompilyator sizning niyatingizni tushunadi va xatolardan saqlaydi.

---

## Amaliy misol: Bog'langan ro'yxat (Linked List)

Ko'rsatkichlarning kuchli qo'llanish maydoni — **bog'langan ro'yxat**.
Massivdan farqi: elementlar xotirada ketma-ket emas, har biri
keyingisining manzilini bilib turadi.
```cpp
tuzilma Tugun {
    butun qiymat;
    Tugun* keyingi;
};

butun asosiy() {
    // Uchta tugun yaratamiz
    Tugun* birinchi = yangi Tugun{10, nullptr};
    Tugun* ikkinchi = yangi Tugun{20, nullptr};
    Tugun* uchinchi = yangi Tugun{30, nullptr};

    // Bog'laymiz: 10 → 20 → 30
    birinchi->keyingi = ikkinchi;
    ikkinchi->keyingi = uchinchi;

    // Bo'ylab yuramiz
    Tugun* joriy = birinchi;
    toki (joriy != nullptr) {
        yozish << joriy->qiymat << " ";
        joriy = joriy->keyingi;
    }
    yozish << qator_oxiri; // 10 20 30

    // Tozalash
    o'chirish birinchi;
    o'chirish ikkinchi;
    o'chirish uchinchi;

    qaytarish 0;
}
```

E'tibor bering: `joriy->qiymat` = `(*joriy).qiymat` ning qisqartmasi.

> **Real hayotdagi misol:** Tuxum sotuvchining ro'yxati. Har bir
> mijozning kartochkasida quyidagi mijoz nomi yozilgan. Birinchi
> kartochkadan boshlab, har bir kartochkadan "keyingi" maydonini
> kuzatib borasiz va oxiri (`nullptr`) ga yetganingizda to'xtaysiz.

Bog'langan ro'yxat afzalliklari va kamchiliklari:

| Afzallik | Kamchilik |
|----------|-----------|
| Element qo'shish/o'chirish tez | Element qidirish sekin |
| Hajm o'zgaruvchan | Xotira sarflanishi ko'p (har bir tugun uchun) |
| Xotira bo'sh joylariga moslashadi | CPU cache uchun yomon |

---

## Eng ko'p uchraydigan xatolar

### 1. `nullptr` ni dereference qilish
```cpp
butun* p = nullptr;
*p = 5; // ← XATO: dastur crash
```

### 2. O'chirilgan ko'rsatkichni ishlatish
```cpp
butun* p = yangi butun(42);
o'chirish p;
*p = 5; // ← XATO: dangling pointer
```

### 3. Bir xil joyni ikki marta `o'chirish`
```cpp
butun* p = yangi butun(42);
o'chirish p;
o'chirish p; // ← XATO: double-free
```

### 4. Massivni `o'chirish` (without `[]`)
```cpp
butun* arr = yangi butun[10];
o'chirish arr; // ← XATO: yo'qolgan
o'chirish[] arr; // TO'G'RI
```

### 5. Memory leak
```cpp
butun* p = yangi butun(42);
// o'chirish ni unutdik!
```

**Hammaning yechimi:** Aqlli ko'rsatkichlardan foydalaning.

---

## Bob bo'yicha mashqlar

### Mashq 1: Ikkita o'zgaruvchini almashtirish (oson)

`almashtirish_havola(butun& a, butun& b)` va `almashtirish_korsatkich(butun* a, butun* b)`
— ikki versiyani yozing.

### Mashq 2: Massivning yig'indisi (o'rta)

`yigindi(butun* arr, butun n)` — massiv elementlarining yig'indisini
qaytaring. Ko'rsatkich arifmetikasini ishlating.

### Mashq 3: Eng katta element (o'rta)

`maks(butun* arr, butun n)` — massivdagi eng kattasini toping.

### Mashq 4: String uzunligi (o'rtacha)

`uzunlik(o'zgarmas belgi* s)` — Cstring uzunligini hisoblang
(`'\0'` gacha).

### Mashq 5: Massivni teskari aylantirish (qiyinroq)

`teskari(butun* arr, butun n)` — massiv elementlarini joyida
(in-place) teskari aylantiring.

### Mashq 6: Linked list (qiyin)

Bog'langan ro'yxatga element qo'shish va olib tashlash funksiyalarini
yozing.

---

## Xulosa

- **Xotira** — manzilli baytlar to'plami.
- **Havola (`&`)** — o'zgaruvchining boshqa nomi. Sodda va xavfsiz.
- **Ko'rsatkich (`*`)** — o'zgaruvchining manzilini saqlaydi.
- **`&` (manzil olish)** — o'zgaruvchining manzilini qaytaradi.
- **`*` (dereference)** — ko'rsatkichdagi qiymatga kiradi.
- **`nullptr`** — bo'sh ko'rsatkich.
- **`yangi/o'chirish`** — dinamik xotira (heap).
- **Aqlli ko'rsatkichlar** — avtomatik tozalanadigan zamonaviy
  yondashuv.
- **Memory leak** — `yangi` qilib `o'chirish` unutish.
- **Linked list** — ko'rsatkichlar bilan tuzilgan dinamik struktura.

Keyingi bobda biz **massivlar va to'plamlar** ni o'rganamiz —
ma'lumotlarni guruhlash va saqlashning eng samarali usullarini ko'rib
chiqamiz.
