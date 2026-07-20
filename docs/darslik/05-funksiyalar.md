# 05 · Funksiyalar {#05-funksiyalar}

## Funksiya nima va nima uchun kerak?

Tasavvur qiling, siz oshpazsiz. Restoraningizda 50 ta xil taom bor.
Lekin har bir taomda — chunin, palov, manti — _kartoshkani arrachilash_
kerak. Bu — bir xil ish, lekin u 50 marta takrorlanadi. Yomon strategiya:
har taom retseptida "kartoshkani arrachilash" bo'limini qayta-qayta
yozish. Yaxshi strategiya: "kartoshkani arrachilash" usulini bitta
joyda batafsil yozib qo'yish, keyin har taom retseptida "kartoshkani
arrachilash" deb havola qilish.

Dasturlashda **funksiya** — aynan shu havola qilingan ish bo'lagi.
Bir marta yozilgan, keyin _istalgan joyda_ chaqirilishi mumkin.

Funksiyaning afzalliklari:

1. **Kodni qayta ishlatish** — bir marta yoz, ko'p marta foydalan.
2. **Tushunarli kod** — katta dastur kichik bo'laklarga ajraladi.
3. **Tuzatishni osonlashtirish** — bitta joyni o'zgartirsangiz, hamma
   chaqirish o'zgaradi.
4. **Mantiqiy izolyatsiya** — har bir funksiyaning bir aniq vazifasi
   bor.

### Birinchi funksiya — oddiy misol

```cpp
// salomlashish funksiyasi
bosh salom_ber(matn ism) {
    yozish << "Salom, " << ism << "!" << qator_oxiri;
}

butun asosiy() {
    salom_ber("Aziza"); // funksiyani chaqirish
    salom_ber("Bobur");
    salom_ber("Karim");
    qaytarish 0;
}
```

**Natija:**

```
Salom, Aziza!
Salom, Bobur!
Salom, Karim!
```

Bitta funksiya — uchta chaqirish. Tasavvur qiling, 100 ta foydalanuvchi
bo'lsa, har biri uchun "yozish << ..." ni qaytaqayta yozish kerak emas.

---

## Funksiyaning anatomiyasi

```cpp
qaytarish_turi  funksiya_nomi  ( parametrlar )  {
    // funksiya tanasi
    qaytarish qiymat;
}
```

Misol uchun:

```cpp
butun kvadrat(butun x) {
    qaytarish x * x;
}
```

Tahlil qilamiz:

| Qism | Misol | Ma'nosi |
|------|-------|---------|
| Qaytarish turi | `butun` | Funksiya **qaytaradigan** qiymat turi |
| Nom | `kvadrat` | Funksiyaning nomi |
| Parametrlar | `butun x` | Kirish qiymatlari (ko'p bo'lishi mumkin) |
| Tanasi | `{ ... }` | Bajariladigan kod |
| `qaytarish` | `qaytarish x * x` | Qaytadigan qiymat |

### Funksiya chaqirish

```cpp
butun natija = kvadrat(5); // 25
butun ikkinchi = kvadrat(10); // 100
```

Bu yerda biz `kvadrat` funksiyasini ikki marta chaqirdik, har safar
boshqa qiymat berib.

---

## Parametrlar

**Parametr** — bu funksiyaga kiritiluvchi qiymat. Funksiyaga _kirish
nuqtasi_.

### Parametrsiz funksiya

```cpp
bosh salom_ber() {
    yozish << "Salom!" << qator_oxiri;
}

salom_ber(); // chaqirish
salom_ber(); // yana
```

Hech qanday kirish kerak emas — har safar bir xil ish.

### Bir nechta parametr

```cpp
butun yigindi(butun a, butun b) {
    qaytarish a + b;
}

butun n1 = yigindi(5, 3); // 8
butun n2 = yigindi(10, 20); // 30
butun n3 = yigindi(n1, n2); // 38 (8 + 30)
```

Parametrlar **`,` (vergul) bilan ajratiladi**. Har bir parametrning
turi alohida yozilishi kerak — `yigindi(butun a, b)` deb yozib bo'lmaydi.

### Turli xil parametrlar

```cpp
matn formatlash(matn ism, butun yosh, mantiqiy talaba) {
    matn natija = ism + " (" + std::to_string(yosh) + " yosh)";
    agar (talaba) {
        natija += ", talaba";
    }
    qaytarish natija;
}

yozish << formatlash("Aziza", 22, rost);
// Natija: Aziza (22 yosh), talaba
```

---

## Qaytarish (return)

**`qaytarish`** kalit so'zi orqali funksiya o'z natijasini chaqiruvchiga
qaytaradi.

```cpp
butun kub(butun x) {
    qaytarish x * x * x; // qaytar va funksiyadan chiq
}
```

### `qaytarish` darhol funksiyadan chiqaradi

```cpp
butun absolyut(butun x) {
    agar (x >= 0) {
        qaytarish x; // bu yerda chiqamiz
    }
    qaytarish -x; // bu satr faqat x < 0 bo'lsa bajariladi
}
```

### Bir nechta `qaytarish`

```cpp
matn baho_nomi(butun ball) {
    agar (ball >= 90) qaytarish "A'lo";
    agar (ball >= 70) qaytarish "Yaxshi";
    agar (ball >= 60) qaytarish "Qoniqarli";
    qaytarish "Qoniqarsiz";
}
```

Birinchi `qaytarish` bajarilishi bilanoq, qolganlari ishlamaydi.

### Qaytarmaslik — `bosh`

Agar funksiya qiymat qaytarmasa, qaytarish turi **`bosh`** (C++ da
`void`) bo'ladi:

```cpp
bosh chiqarish(matn xabar) {
    yozish << xabar << qator_oxiri;
    // qaytarish yo'q
}
```

`bosh` funksiyalarda `qaytarish` ni yozish shart emas, lekin yozish
mumkin (faqat qiymatsiz):

```cpp
bosh tekshirish(butun x) {
    agar (x < 0) {
        yozish << "Xato!" << qator_oxiri;
        qaytarish; // funksiyadan darhol chiqamiz
    }
    yozish << "OK" << qator_oxiri;
}
```

---

## Parametr berish usullari

Bu juda muhim mavzu. uz++ (va C++) da parametrlarni funksiyaga **uch
xil usulda** berish mumkin.

### 1. Qiymat orqali (by value)

Eng oddiy va xavfsiz usul. Funksiyaga _nusxa_ uzatiladi:

```cpp
bosh oshiring(butun x) {
    x = x + 100;
    yozish << "Funksiyada: " << x << qator_oxiri;
}

butun asosiy() {
    butun a = 5;
    oshiring(a);
    yozish << "Asosiyda: " << a << qator_oxiri;
}
```

**Natija:**

```
Funksiyada: 105
Asosiyda: 5
```

`a` o'zgarmadi! Chunki `oshiring` funksiyasi `a` ning _nusxasini_ oldi.
Aslida ikkita alohida o'zgaruvchi bo'ldi: `asosiy` dagi `a` va
`oshiring` dagi `x`.

> **Real hayotdagi misol:** Siz qog'ozga raqam yozdingiz. Do'stingizga
> bu qog'ozning **photocopy** ini berdingiz. Do'stingiz photocopy
> ustida nimani o'zgartirsa, sizning asl qog'ozingiz buzilmaydi.

### 2. Havola orqali (by reference)

Agar siz funksiyada **asl o'zgaruvchini** o'zgartirmoqchi bo'lsangiz,
parametr nomidan oldin `&` qo'ying:

```cpp
bosh oshiring(butun& x) {        // & belgisiga e'tibor!
    x = x + 100;
}

butun asosiy() {
    butun a = 5;
    oshiring(a);
    yozish << a << qator_oxiri; // 105
}
```

`&` — "asl narsani ber" demakdir. Endi `oshiring` ichidagi `x` —
bu xuddi `a`. Ularni o'zgartirsangiz, `a` ham o'zgaradi.

### 3. Const havola orqali

Agar funksiya **katta** o'zgaruvchini olsa (matn, vektor), nusxa olish
sekin. Lekin siz uni o'zgartirmoqchi emassiz. Bu holatda `o'zgarmas&`
ishlating:

```cpp
butun matn_uzunligi(o'zgarmas matn& s) {     // havola, lekin o'zgartirib bo'lmaydi
    qaytarish s.length();
}

matn katta_matn = "Juda uzun matn...";
butun uzunlik = matn_uzunligi(katta_matn);
```

Bu eng tez va xavfsiz usul.

### Qaysi usulni qachon ishlatish kerak?

| Holat | Tavsiya |
|-------|---------|
| Kichik tur (`butun`, `kasr`, `mantiqiy`) | Qiymat orqali (`butun x`) |
| Funksiya parametrni o'zgartirishi kerak | Havola orqali (`butun& x`) |
| Katta tur, o'zgartirmaslik kerak | Const havola (`o'zgarmas matn& s`) |

---

## Standart parametr qiymatlari

Parametrga **standart qiymat** belgilash mumkin:

```cpp
bosh salom_ber(matn ism = "Mehmon") {
    yozish << "Salom, " << ism << "!" << qator_oxiri;
}

salom_ber(); // "Salom, Mehmon!"
salom_ber("Aziza"); // "Salom, Aziza!"
```

Standart qiymatli parametrlar **oxirida** bo'lishi kerak:

```cpp
bosh f(butun a, butun b = 0, butun c = 0); // TO'G'RI
bosh f(butun a = 0, butun b, butun c); // XATO
```

---

## Funksiya overloading (qayta yuklash)

Bir xil nomli funksiyaning **bir nechta versiyasi** bo'lishi mumkin —
agar ularning parametrlari farqli bo'lsa:

```cpp
butun yigindi(butun a, butun b) {
    qaytarish a + b;
}

haqiqiy yigindi(haqiqiy a, haqiqiy b) {
    qaytarish a + b;
}

butun yigindi(butun a, butun b, butun c) {
    qaytarish a + b + c;
}

yigindi(5, 3); // 1-versiya: butun
yigindi(2.5, 3.7); // 2-versiya: haqiqiy
yigindi(1, 2, 3); // 3-versiya: 3 ta parametr
```

Kompilyator parametrlarga qarab qaysi versiyani chaqirishni aniqlaydi.

---

## Rekursiya — funksiya o'zini chaqiradi

**Rekursiya** — bu funksiyaning o'zini chaqirishi. Ko'pincha matematik
muammolarni yechish uchun foydali.

### Klassik misol: Faktoriyal

`n!` = `1 × 2 × 3 × ... × n`. Rekursiv ta'rif: `n!` = `n × (n-1)!`.

```cpp
butun faktoriyal(butun n) {
    agar (n <= 1) {
        qaytarish 1; // bazaviy holat
    }
    qaytarish n * faktoriyal(n - 1); // rekursiya
}
```

Qanday ishlaydi?
- `faktoriyal(5)` → `5 * faktoriyal(4)`
- `faktoriyal(4)` → `4 * faktoriyal(3)`
- `faktoriyal(3)` → `3 * faktoriyal(2)`
- `faktoriyal(2)` → `2 * faktoriyal(1)`
- `faktoriyal(1)` → `1` (bazaviy)
- Qaytib: `2*1=2`, `3*2=6`, `4*6=24`, `5*24=120`

### Rekursiyaning ikki shart

Har bir rekursiv funksiya kerak:

1. **Bazaviy holat** — to'xtash sharti. Agar bu bo'lmasa — cheksiz
   rekursiya, dastur "stack overflow" bilan ishdan chiqadi.
2. **Rekursiv chaqiruv** — o'zini chaqirish, lekin **boshqacha
   parametr bilan**.

### Yana bir misol: Fibonachchi

```cpp
butun fibonachchi(butun n) {
    agar (n <= 1) {
        qaytarish n; // F(0) = 0, F(1) = 1
    }
    qaytarish fibonachchi(n-1) + fibonachchi(n-2);
}
```

> **Diqqat — rekursiya sekin bo'lishi mumkin.** Yuqoridagi
> `fibonachchi(40)` ni hisoblash juda uzoq vaqt oladi, chunki bir xil
> qiymatlar qayta-qayta hisoblanadi. Bunday hollarda **memoizatsiya**
> yoki **iterativ** yechim ishlatiladi.

---

## Lambda ifodalar

Lambda — bu **nomsiz funksiya**. Qachon kerak? Faqat bir joyda ishlatib,
keyin tashlamoqchi bo'lganda.

### Sintaksis

```cpp
[ qo'lga olish ] ( parametrlar ) -> qaytarish_turi { tanasi }
```

### Misol

```cpp
o'zgaruvchan kvadrat = [](butun x) { qaytarish x * x; }
yozish << kvadrat(5); // 25
```

### Foydali misol — to'plamlarda tartiblash

```cpp
vektor<butun> sonlar = {3, 1, 4, 1, 5, 9, 2, 6}

// Pasayish bo'yicha tartiblash
std::sort(sonlar.begin(), sonlar.end(), [](butun a, butun b) {
    qaytarish a > b;
});
```

Bu yerda lambda — taqqoslash funksiyasi sifatida `sort` ga uzatildi.

### O'rab olish (capture)

Lambda tashqi o'zgaruvchilarni "qo'lga olishi" mumkin:

```cpp
butun ko'paytma = 3;
o'zgaruvchan ko'p = [ko'paytma](butun x) { qaytarish x * ko'paytma; }

yozish << ko'p(5); // 15
```

`[ko'paytma]` — "tashqi `ko'paytma` ni qiymat orqali qo'lga ol" demakdir.

Lambda haqida 6-bobda batafsil ko'ramiz.

---

## Inline funksiyalar

`qator_ichi` (C++ da `inline`) — funksiyani chaqirish o'rniga, uning
tanasini chaqirish joyiga **kiritib qo'yish** kompilyatorga tavsiya:

```cpp
qator_ichi butun maks(butun a, butun b) {
    qaytarish (a > b) ? a : b;
}
```

Bu nima beradi? Tez, chunki funksiya chaqirish overhead'i yo'q. Lekin
kompyuterning ishi ko'payadi va kompilatsiya sekinlashadi.

**Qoida:** Faqat juda kichik va tez funksiyalar uchun `qator_ichi`
ishlating.

---

## Funksiya shablonlari

Agar siz `yigindi` ni faqat `butun` uchun emas, har xil tur uchun
yozmoqchi bo'lsangiz, **shablon** ishlatasiz:

```cpp
shablon<tur T>
T yigindi(T a, T b) {
    qaytarish a + b;
}

yigindi(5, 3); // butun
yigindi(2.5, 3.7); // haqiqiy
yigindi("Salom ", "dunyo"); // matn
```

Shablonlarni 9-bobda batafsil ko'ramiz.

---

## Amaliy misol: Matematik funksiyalar to'plami

Mana, bir nechta matematik funksiyalarni birlashtirgan dastur:

```cpp
// matematika.uzpp

butun yigindi(butun a, butun b) {
    qaytarish a + b;
}

butun ko'paytma(butun a, butun b) {
    qaytarish a * b;
}

butun maks(butun a, butun b) {
    qaytarish (a > b) ? a : b;
}

butun min(butun a, butun b) {
    qaytarish (a < b) ? a : b;
}

butun absolyut(butun x) {
    qaytarish (x >= 0) ? x : -x;
}

butun faktoriyal(butun n) {
    agar (n <= 1) qaytarish 1;
    qaytarish n * faktoriyal(n - 1);
}

mantiqiy tubmi(butun n) {
    agar (n < 2) qaytarish yolg'on;
    uchun (butun i = 2; i * i <= n; i++) {
        agar (n % i == 0) qaytarish yolg'on;
    }
    qaytarish rost;
}

butun asosiy() {
    yozish << "5 + 3 = " << yigindi(5, 3) << qator_oxiri;
    yozish << "5 * 3 = " << ko'paytma(5, 3) << qator_oxiri;
    yozish << "maks(5, 3) = " << maks(5, 3) << qator_oxiri;
    yozish << "|-7| = " << absolyut(-7) << qator_oxiri;
    yozish << "5! = " << faktoriyal(5) << qator_oxiri;
    yozish << "7 tub? " << (tubmi(7) ? "ha" : "yo'q") << qator_oxiri;

    qaytarish 0;
}
```

**Natija:**

```
5 + 3 = 8
5 * 3 = 15
maks(5, 3) = 5
|-7| = 7
5! = 120
7 tub? ha
```

Har bir funksiya **bir aniq vazifa** bajaradi. `asosiy` faqat ularni
chaqiradi. Bu — _yaxshi dasturlash_ namunasi.

---

## Eng ko'p uchraydigan xatolar

### 1. Qaytarmaslik

```cpp
butun yigindi(butun a, butun b) {
    butun c = a + b;
    // qaytarish yo'q!
}
// ← XATO: butun qaytarmadi
```

### 2. Noto'g'ri tur

```cpp
butun yigindi(butun a, butun b) {
    qaytarish "natija"; // ← XATO: matn qaytarmoqda
}
```

### 3. Parametr berish unutilgan

```cpp
butun yigindi(butun a, butun b) {
    qaytarish a + b;
}

yigindi(5); // ← XATO: ikkita parametr kerak
```

### 4. Cheksiz rekursiya

```cpp
butun fibonachchi(butun n) {
    qaytarish fibonachchi(n-1) + fibonachchi(n-2);
    // bazaviy holat yo'q!
}
```

### 5. Funksiya nomini kalit so'z bilan bir xil qilish

```cpp
butun agar(butun x) { ... }   // ← XATO: agar — kalit so'z
```

---

## Bob bo'yicha mashqlar

### Mashq 1: Doira yuzasi (oson)

`doira_yuzasi(haqiqiy r)` funksiyasini yozing, u doira yuzasini
qaytaradi.

### Mashq 2: Eng katta (oson)

`maks_uch(butun a, butun b, butun c)` — uchta sondan eng kattasini
qaytaring.

### Mashq 3: Sonni teskari aylantirish (o'rta)

`teskari(butun n)` — sonni teskari aylantiring (123 → 321).

### Mashq 4: Sonning raqamlari yig'indisi (o'rta)

`raqam_yigindisi(butun n)` — sonning barcha raqamlari yig'indisini
qaytaring (rekursiv).

### Mashq 5: Eng katta umumiy bo'luvchi (o'rtacha)

`gcd(butun a, butun b)` — Euclid algoritmi bilan. Rekursiv yozing.

```cpp
gcd(a, b) = (b == 0) ? a : gcd(b, a % b);
```

### Mashq 6: Quvvatga ko'tarish (o'rtacha)

`quvvat(haqiqiy asos, butun daraja)` — `asos^daraja`. Rekursiv
versiya yozing.

### Mashq 7: Stringni teskari aylantirish (qiyinroq)

`teskari_matn(matn s)` — matnni teskari aylantiring ("salom" →
"molas").

---

## Xulosa

- **Funksiya** — bir necha marta ishlatish uchun nomlangan kod bloki.
- **Sintaksis:** `qaytarish_turi nom(parametrlar) { tanasi }`.
- **Parametr berish usullari**: qiymat orqali, havola orqali, const
  havola orqali.
- **`qaytarish`** — funksiyadan qiymat qaytarish va chiqish.
- **Standart parametrlar** — kerak bo'lmasa berilmasa, standart
  qiymat ishlatiladi.
- **Funksiya overloading** — bir xil nomli, har xil parametrli
  funksiyalar.
- **Rekursiya** — funksiya o'zini chaqiradi. Bazaviy holat shart.
- **Lambda** — nomsiz funksiya.
- **Shablonlar** — bitta funksiya, ko'p tur uchun (9-bobda).

Keyingi bobda biz **ko'rsatkichlar va havolalar** ni o'rganamiz —
bu C++ ning eng kuchli va eng murakkab xususiyatlaridan biri. Ular
orqali siz xotirani to'g'ridan-to'g'ri boshqarishni o'rganasiz.
