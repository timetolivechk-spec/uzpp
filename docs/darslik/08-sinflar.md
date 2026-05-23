# 08 · Sinflar — Obyektga Yo'naltirilgan Dasturlash {#08-sinflar}

## OOP nima va nima uchun kerak?

Hozirgacha siz dasturlarni **funksiya-tomonidan** o'rganib keldingiz —
funksiyalar va o'zgaruvchilarni alohida-alohida tashkil etib. Lekin
katta dasturlarda bu yondashuv yetarsiz. **Obyektga yo'naltirilgan
dasturlash** (Object-Oriented Programming, OOP) — kodingizni
**obyektlar** atrofida tashkil qilish usuli.

### Real hayotdan misol

Tasavvur qiling, siz mashina haqida gapirayapsiz. Mashinaning **xususiyatlari**
bor (rang, model, yili, dvigatel hajmi) va **harakatlari** (yurish,
to'xtash, signal berish, eshikni ochish). _Mashina_ — bu shu xususiyat va
harakatlarni birgalikda saqlovchi **obyekt**.

Mashinaning **chizmasi** (sxemasi) — bu **sinf**. Sxemadan minglab
mashinalar yaratish mumkin, har biri o'zining shaxsiy xususiyatlari
bilan, lekin barchasi bir xil sxemaga binoan.

```
[Mashina sinfi]          ← chizma
     ↓ yaratamiz
[Mashina 1] [Mashina 2] [Mashina 3]    ← obyektlar
   rangi=qora    rangi=oq      rangi=qizil
   yili=2020     yili=2022     yili=2024
```

### OOP afzalliklari

1. **Inkapsulyatsiya** — ma'lumot va kod birga, ichki tafsilotlar
   yashirin.
2. **Meros** — bir sinfdan boshqasini "merosga olish" — kodni qayta
   ishlatish.
3. **Polimorfizm** — bitta nom, ko'p versiya — kod moslashuvchan.
4. **Abstraktsiya** — qanday ishlashini bilmasdan ishlatish (mashina
   ichini bilmaysiz, lekin haydaysiz).

---

## Sinf e'lon qilish

```cpp
sinf Talaba {
ochiq:
    matn ism
    butun yoshi
    ikkilangan ortacha_baho

    bosh malumotni_chiqarish() {
        yozish << "Ism: " << ism << qator_oxiri
        yozish << "Yoshi: " << yoshi << qator_oxiri
        yozish << "O'rtacha: " << ortacha_baho << qator_oxiri
    }
}
```

Tahlil:
- `sinf Talaba` — sinf e'loni
- `ochiq:` — bu qismni hammaga ko'rinadi qiladi (default `yopiq`)
- `matn ism`, `butun yoshi` — bu sinfning **maydonlari** (fields)
- `malumotni_chiqarish()` — bu sinfning **metodi** (member function)

### Obyekt yaratish va ishlatish

```cpp
butun asosiy() {
    Talaba t1
    t1.ism = "Aziza"
    t1.yoshi = 22
    t1.ortacha_baho = 4.7

    t1.malumotni_chiqarish()

    Talaba t2
    t2.ism = "Bobur"
    t2.yoshi = 21
    t2.ortacha_baho = 4.2

    t2.malumotni_chiqarish()

    qaytarish 0
}
```

**Natija:**

```
Ism: Aziza
Yoshi: 22
O'rtacha: 4.7
Ism: Bobur
Yoshi: 21
O'rtacha: 4.2
```

`t1` va `t2` — ikkita alohida `Talaba` obyekti. Har biri o'zining
ma'lumotlari bilan.

### Nuqta operatori

`.` (nuqta) operatori — obyektning maydoni yoki metodiga kirish uchun:

```cpp
t1.ism                        // maydon
t1.malumotni_chiqarish()      // metod
```

---

## Inkapsulyatsiya — `yopiq` va `ochiq`

Klassda ikki xil ko'rinish darajasi:

- **`ochiq:`** (public) — tashqaridan ko'rinadi
- **`yopiq:`** (private) — faqat sinf ichidan ko'rinadi

### Misol

```cpp
sinf BankHisobi {
yopiq:
    butun balans = 0

ochiq:
    bosh pul_qoyish(butun miqdor) {
        agar (miqdor > 0) {
            balans += miqdor
        }
    }

    bosh pul_olish(butun miqdor) {
        agar (miqdor > 0 && miqdor <= balans) {
            balans -= miqdor
        }
        aks_holda {
            yozish << "Yetarli mablag' yo'q!" << qator_oxiri
        }
    }

    butun joriy_balans() {
        qaytarish balans
    }
}
```

Endi:

```cpp
BankHisobi hisob
hisob.pul_qoyish(1000)
hisob.pul_olish(500)
yozish << hisob.joriy_balans()    // 500

hisob.balans = 1000000             // ← XATO: yopiq
```

Foydalanuvchi `balans` ga **to'g'ridan-to'g'ri** kira olmaydi. Faqat
biz bergan metodlar orqali. Bu — **inkapsulyatsiya**.

### Nima uchun bu muhim?

Tasavvur qiling, bank dasturida `balans` ochiq bo'lsa, har qanday kod
uni o'zgartira oladi:

```cpp
hisob.balans = -1000              // manfiy balans?
hisob.balans = 999999999          // oddiy foydalanuvchida million?
```

`yopiq` bilan — **biz qoidalarni nazorat qilamiz**. Faqat `pul_qoyish`
va `pul_olish` metodlari orqali balans o'zgaradi.

> **Real hayotdagi misol:** ATM bankomat. Siz pulni to'g'ridan-to'g'ri
> hisobingizdan olib chiqolmaysiz — siz tugmachalarni bosasiz, bankomat
> qoidalarni tekshiradi (parol, miqdor, balans), keyin pul beradi. Bu
> — inkapsulyatsiya.

---

## Konstruktor va destruktor

### Konstruktor

**Konstruktor** — obyekt yaratilganida avtomatik chaqiriladigan maxsus
metod. Maydonlarni boshlang'ich qiymat bilan to'ldirish uchun.

```cpp
sinf Talaba {
ochiq:
    matn ism
    butun yoshi

    // Konstruktor — sinf nomi bilan bir xil, qaytarish turi yo'q
    Talaba(matn yangi_ism, butun yangi_yoshi) {
        ism = yangi_ism
        yoshi = yangi_yoshi
    }
}

butun asosiy() {
    Talaba t1("Aziza", 22)             // konstruktor chaqirildi
    Talaba t2("Bobur", 21)

    yozish << t1.ism << " " << t1.yoshi
}
```

Konstruktor — sinf nomi bilan bir xil. Qaytarish turi yo'q.

### Standart konstruktor

Agar konstruktorni yozmasangiz, kompilyator avtomatik yaratadi
(parametrsiz). Lekin agar siz biror konstruktor yozsangiz, standartini
ham siz yozishingiz kerak:

```cpp
sinf Talaba {
ochiq:
    Talaba() {                        // bo'sh, default
        ism = ""
        yoshi = 0
    }

    Talaba(matn yangi_ism, butun yangi_yoshi) {   // parametrli
        ism = yangi_ism
        yoshi = yangi_yoshi
    }
}

Talaba t1                            // standart konstruktor
Talaba t2("Aziza", 22)               // parametrli
```

### Initialization list

Maydonlarni qiymat bilan ishga tushirish samaraliroq usul:

```cpp
sinf Talaba {
ochiq:
    matn ism
    butun yoshi

    Talaba(matn yangi_ism, butun yangi_yoshi)
        : ism(yangi_ism), yoshi(yangi_yoshi)
    { }
}
```

`: ism(yangi_ism), yoshi(yangi_yoshi)` — bu **initialization list**.
U maydonlarni qiymat bilan **e'lon paytida** boshlash imkonini beradi.

### Destruktor

**Destruktor** — obyekt yo'q qilinganida avtomatik chaqiriladi.
Tozalash uchun:

```cpp
sinf Fayl {
yopiq:
    matn nom

ochiq:
    Fayl(matn n) : nom(n) {
        yozish << "Faylni ochildi: " << nom << qator_oxiri
    }

    ~Fayl() {                         // destruktor — ~ bilan
        yozish << "Faylni yopildi: " << nom << qator_oxiri
    }
}

butun asosiy() {
    Fayl f("ma'lumot.txt")
    // ... ish bilan ...
}                                    // bu yerda destruktor chaqiriladi
```

**Natija:**

```
Faylni ochildi: ma'lumot.txt
Faylni yopildi: ma'lumot.txt
```

Destruktor xotira, fayl, tarmoq aloqasi kabi resurslarni tozalash
uchun ishlatiladi.

---

## Meros (Inheritance)

**Meros** — bir sinfdan boshqasini "merosga olish". Yangi sinf —
ota-sinfning hamma xususiyatlarini oladi va o'zinikini qo'shadi.

### Misol — hayvonlar

```cpp
sinf Hayvon {
ochiq:
    matn ism
    butun yoshi

    bosh nafas_olish() {
        yozish << ism << " nafas oladi" << qator_oxiri
    }
}

sinf It : ochiq Hayvon {              // : ochiq — meros olish
ochiq:
    bosh hurish() {
        yozish << ism << " huriydi: Vov-vov!" << qator_oxiri
    }
}

butun asosiy() {
    It mening_itim
    mening_itim.ism = "Rex"
    mening_itim.nafas_olish()         // Hayvondan meros
    mening_itim.hurish()              // It ning o'zi
}
```

**Natija:**

```
Rex nafas oladi
Rex huriydi: Vov-vov!
```

`It` `Hayvon` ning hamma `ochiq` xususiyatlarini oldi.

### Konstruktor va meros

```cpp
sinf Hayvon {
ochiq:
    matn ism
    Hayvon(matn n) : ism(n) {}
}

sinf It : ochiq Hayvon {
ochiq:
    It(matn n) : Hayvon(n) {           // ota konstruktorini chaqirish
        // ...
    }
}
```

### Meros nima uchun foydali?

Bir nechta yaqin xil obyektlar bo'lsa — umumiy qismini ota-sinfga
chiqaring:

```cpp
sinf Transport {
ochiq:
    butun tezlik
    bosh yurish() { ... }
}

sinf Mashina : ochiq Transport { ... }
sinf Motosikl : ochiq Transport { ... }
sinf Velosiped : ochiq Transport { ... }
```

Hammasi `tezlik` va `yurish` ni `Transport` dan oladi. Faqat o'ziga
xos qismini yozadi.

---

## Virtual metodlar va polimorfizm

### Polimorfizm muammosi

```cpp
sinf Hayvon {
ochiq:
    bosh ovoz() {
        yozish << "Qandaydir ovoz" << qator_oxiri
    }
}

sinf It : ochiq Hayvon {
ochiq:
    bosh ovoz() {                     // override
        yozish << "Vov-vov!" << qator_oxiri
    }
}

butun asosiy() {
    It mening_itim
    Hayvon* h = &mening_itim
    h->ovoz()                          // "Qandaydir ovoz" — XATO!
}
```

Biz `It` ning ovozini xohlardik, lekin `Hayvon` ovozi chiqdi. Buni
**virtual metod** hal qiladi:

```cpp
sinf Hayvon {
ochiq:
    virtual bosh ovoz() {              // virtual qiling!
        yozish << "Qandaydir ovoz" << qator_oxiri
    }
}

sinf It : ochiq Hayvon {
ochiq:
    bosh ovoz() ustidan_yozish {       // override deb belgilang
        yozish << "Vov-vov!" << qator_oxiri
    }
}

butun asosiy() {
    It mening_itim
    Hayvon* h = &mening_itim
    h->ovoz()                          // "Vov-vov!" — TO'G'RI
}
```

### Bu nima uchun kuchli?

```cpp
vektor<Hayvon*> hayvonlar
hayvonlar.push_back(yangi It())
hayvonlar.push_back(yangi Mushuk())
hayvonlar.push_back(yangi Ot())

uchun (Hayvon* h : hayvonlar) {
    h->ovoz()                          // har biri o'zinikini aytadi!
}
```

**Natija:**

```
Vov-vov!
Miyov-miyov!
Pir-pir!
```

Bitta sikl — uchta turli ovoz. Bu — **polimorfizm**.

---

## Abstrakt sinf

Ba'zan ota-sinfdan obyekt yaratish ma'nosiz bo'ladi. Masalan,
`Hayvon` — bu abstrakt tushuncha. Aslida `It`, `Mushuk` bor, "umumiy
hayvon" yo'q.

**Pure virtual** funksiya orqali sinfni abstrakt qilamiz:

```cpp
sinf Hayvon {
ochiq:
    virtual bosh ovoz() = 0           // = 0 — pure virtual
}

butun asosiy() {
    Hayvon h                           // ← XATO: abstrakt sinfdan obyekt yaratib bo'lmaydi
    It mening_itim                     // TO'G'RI
}
```

Pure virtual funksiyani **albatta** voris sinfda implement qilish
kerak.

---

## Operatorlarni qayta yuklash (Operator Overloading)

Sinflar uchun operatorlarni o'z xohlagancha aniqlay olasiz:

```cpp
sinf Vektor2D {
ochiq:
    ikkilangan x, y

    Vektor2D(ikkilangan a, ikkilangan b) : x(a), y(b) {}

    // + operatorini qayta yuklaymiz
    Vektor2D operator+(o'zgarmas Vektor2D& boshqa) {
        qaytarish Vektor2D(x + boshqa.x, y + boshqa.y)
    }
}

butun asosiy() {
    Vektor2D a(1.0, 2.0)
    Vektor2D b(3.0, 4.0)
    Vektor2D c = a + b                 // qayta yuklash ishlaydi!
    yozish << c.x << " " << c.y        // 4.0 5.0
}
```

Bu — matematik tushunchalarni dasturlashda tabiiy ifodalash imkoni.

---

## `static` a'zolar

Sinf bo'yicha umumiy a'zo (har bir obyektda emas, sinfning o'zida):

```cpp
sinf Talaba {
ochiq:
    static butun jami_talaba             // umumiy
    matn ism

    Talaba(matn n) : ism(n) {
        jami_talaba++
    }
}

butun Talaba::jami_talaba = 0            // initsializatsiya

butun asosiy() {
    Talaba t1("Aziza")
    Talaba t2("Bobur")
    Talaba t3("Karim")

    yozish << Talaba::jami_talaba         // 3
}
```

`jami_talaba` — bu obyektga emas, sinfga tegishli.

---

## `dost` (friend) deklaratsiyasi

`dost` — boshqa funksiya yoki sinfga `yopiq` a'zolarga kirish ruxsati
beradi:

```cpp
sinf Hisob {
yopiq:
    butun balans

    dost bosh balansni_chiqarish(Hisob& h)
}

bosh balansni_chiqarish(Hisob& h) {
    yozish << h.balans                    // yopiq ga kira oladi!
}
```

Ehtiyot bo'lib ishlating — inkapsulyatsiyani buzadi.

---

## Amaliy misol: Bank hisobi tizimi

```cpp
sinf Hisob {
yopiq:
    matn egasi
    butun raqami
    ikkilangan balans

ochiq:
    Hisob(matn ism, butun n) : egasi(ism), raqami(n), balans(0) {}

    bosh qoyish(ikkilangan miqdor) {
        agar (miqdor > 0) {
            balans += miqdor
            yozish << miqdor << " so'm qo'yildi. "
                   << "Yangi balans: " << balans << qator_oxiri
        }
    }

    mantiq olish(ikkilangan miqdor) {
        agar (miqdor > 0 && miqdor <= balans) {
            balans -= miqdor
            yozish << miqdor << " so'm olindi. "
                   << "Yangi balans: " << balans << qator_oxiri
            qaytarish rost
        }
        yozish << "Yetarli mablag' yo'q!" << qator_oxiri
        qaytarish yolg'on
    }

    ikkilangan joriy_balans() o'zgarmas {
        qaytarish balans
    }

    bosh malumot() o'zgarmas {
        yozish << "Hisob #" << raqami << " (egasi: " << egasi << ")"
               << ", balans: " << balans << qator_oxiri
    }
}

butun asosiy() {
    Hisob h("Aziza Karimova", 12345)
    h.qoyish(1000000)
    h.olish(250000)
    h.olish(2000000)              // mablag' yetmaydi
    h.malumot()

    qaytarish 0
}
```

**Natija:**

```
1000000 so'm qo'yildi. Yangi balans: 1000000
250000 so'm olindi. Yangi balans: 750000
Yetarli mablag' yo'q!
Hisob #12345 (egasi: Aziza Karimova), balans: 750000
```

---

## Eng ko'p uchraydigan xatolar

### 1. Yopiq a'zolarga to'g'ridan-to'g'ri kirishga harakat

```cpp
sinf X { yopiq: butun a }
X x
x.a = 5                          // ← XATO
```

### 2. Konstruktorda `qaytarish` qilish

```cpp
sinf X {
ochiq:
    X() {
        qaytarish 5              // ← XATO: konstruktor qaytarmaydi
    }
}
```

### 3. Virtual destruktorni unutish

```cpp
sinf Ota { ochiq: ~Ota() {} }    // virtual emas
sinf Voris : ochiq Ota { ... }

Ota* p = yangi Voris()
o'chirish p                       // ← XATO: Voris destruktori chaqirilmaydi
```

To'g'risi: `virtual ~Ota() {}`.

### 4. Slicing

```cpp
Voris v
Ota o = v                         // Ota qismi nusxalandi, Voris qismi yo'qoldi!
```

Polimorfizm uchun ko'rsatkich yoki havola ishlating:

```cpp
Ota& o = v                        // havola — slicing yo'q
```

### 5. Memory leak — destruktorda `o'chirish` qilmaslik

```cpp
sinf X {
ochiq:
    butun* p
    X() { p = yangi butun(0) }
    // destruktorda p ni o'chirish kerak!
    ~X() { o'chirish p }
}
```

---

## Bob bo'yicha mashqlar

### Mashq 1: Aylana sinfi (oson)

`Aylana` sinfi yarating. Maydonlari: `radius`. Metodlari: `yuza()`,
`uzunlik()`.

### Mashq 2: Kalkulyator sinfi (o'rta)

`Kalkulyator` sinfini yozing. Metodlari: `qoshish`, `ayirish`,
`koPaytirish`, `bolish`. Saqlanadigan maydon — joriy natija.

### Mashq 3: Talaba va guruh (o'rta)

`Talaba` sinfi (ism, baholar vektori, o'rtacha). `Guruh` sinfi —
`vektor<Talaba>`. Guruh metodlari: eng yaxshi talabani topish,
o'rtacha bahoni hisoblash.

### Mashq 4: Shakl sinflari ierarxiyasi (o'rtacha)

Abstrakt `Shakl` (pure virtual `yuza()`). Voris sinflar: `Aylana`,
`Toqrtburchak`, `Uchburchak`. Polimorfizm bilan ishlating.

### Mashq 5: Vektor2D bilan operatorlar (qiyinroq)

`Vektor2D` ga `+`, `-`, `*` (skalyar bilan ko'paytirish) operatorlarini
qayta yuklang.

### Mashq 6: Bog'langan ro'yxat — sinf sifatida (qiyin)

`BoglanganRoyxat` sinfini yozing. Metodlari: `qoshish`, `ochirish`,
`chiqarish`, `uzunlik`.

---

## Xulosa

- **Sinf** — bu chizma. **Obyekt** — bu chizma asosida yaratilgan
  narsa.
- **`ochiq` va `yopiq`** — ko'rinish nazorati (inkapsulyatsiya).
- **Konstruktor** — obyektni yaratish va boshlash.
- **Destruktor** — obyektni yo'qotish va tozalash.
- **Meros** — bir sinfdan boshqasini "merosga olish".
- **Virtual va polimorfizm** — bir nom, ko'p versiya.
- **Abstrakt sinf** — obyekt yaratib bo'lmaydigan, faqat meros uchun
  bo'lgan sinf.
- **Operator overloading** — `+`, `-`, va boshqa operatorlarni o'z
  sinfingiz uchun aniqlash.
- **`static` a'zolar** — sinfning o'zi uchun, obyekti uchun emas.

Keyingi bobda biz **shablonlar** ni o'rganamiz — `vektor<butun>` va
`vektor<matn>` qanday bir kod bilan ishlay olishi qiziq emasmi?
Bularning hammasi shablonlar yordamida.
