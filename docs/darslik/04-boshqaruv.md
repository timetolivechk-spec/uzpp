# 04 · Boshqaruv tuzilmalari {#04-boshqaruv}

## Dastur oqimi nima?

Hozirgacha bizning dasturlarimiz oddiy edi — har bir qator yuqoridan
pastga, ketma-ket bajarilar edi. Lekin haqiqiy dasturlar shunday
ishlamaydi. Kompyuter sharoitlarga qarab boshqacha yo'l tutadi,
takroriy ishlarni avtomatik qiladi, va kerakli paytda to'xtab biror
ish qiladi.

Tasavvur qiling, siz mehmonni kutib olyapsiz. Sizning xatti-harakatingiz
ham sodda emas:

- **Agar** eshik qo'ng'irog'i jiringlasa — eshikni oching.
- **Agar** mehmon yaqin do'st bo'lsa — quchoqlang. **Aks holda** —
  qo'l berib salomlashing.
- Choy quying. **Bu jarayonni** har bir mehmon uchun **takror**lang.
- Mehmonlar ketgancha — ketishni kutib turing. **To'xtang** faqat
  hammasi ketganidan keyin.

Bu — **boshqaruv mantiqi**. Dasturda ham xuddi shunday: biz
kompyuterga ayrim shartlarda boshqacha amal qilishni, ba'zi ishlarni
takrorlashni, kerak bo'lganda to'xtashni buyurishimiz kerak.

Bu boshqaruvga uchta asosiy mexanizm xizmat qiladi:

1. **Tanlov (shart)** — agar shart bajarilsa, biror kodni bajar (`agar`)
2. **Takrorlash (sikl)** — kodni bir necha marta bajar (`uchun`, `davr`)
3. **Tartibni o'zgartirish** — sikldan chiqish, keyingi takrorlashga
   o'tish (`to'xtatish`, `davom_ettirish`)

---

## Shart — `agar` operatori

### Eng oddiy shakl

```cpp
agar (shart) {
    // shart rost bo'lsa, bu blok bajariladi
}
```

Misol:

```cpp
butun yosh = 20

agar (yosh >= 18) {
    yozish << "Siz voyaga yetgansiz" << qator_oxiri
}
```

Bu yerda:
- `(yosh >= 18)` — **shart**. Bu mantiqiy ifoda — uning natijasi
  `rost` yoki `yolg'on`.
- `{ ... }` — **blok**. Shart `rost` bo'lsa, ichidagi kod bajariladi.

### `agar/aks_holda`

Ko'pincha biz "agar bu — buni qil, aks holda — buni" demoqchimiz:

```cpp
agar (yosh >= 18) {
    yozish << "Voyaga yetgan" << qator_oxiri
}
aks_holda {
    yozish << "Voyaga yetmagan" << qator_oxiri
}
```

`aks_holda` (C++ da `else`) ni keng tilda "boshqa", "agar bu shartni
qondirmasa" deb tarjima qilish mumkin.

### `aks_holda agar` — bir necha shart

Ko'p marta biz uchta yoki undan ko'p tanlovga duch kelamiz:

```cpp
butun baho = 87

agar (baho >= 90) {
    yozish << "A'lo" << qator_oxiri
}
aks_holda agar (baho >= 80) {
    yozish << "Yaxshi" << qator_oxiri
}
aks_holda agar (baho >= 60) {
    yozish << "Qoniqarli" << qator_oxiri
}
aks_holda {
    yozish << "Qoniqarsiz" << qator_oxiri
}
```

**Muhim:** Shartlar **yuqoridan pastga** tekshiriladi. Birinchi rost
shart topilsa, qolganlari tekshirilmaydi.

> **Real hayotdagi misol:** Avtomobil yo'lidagi yo'l harakati boshqaruvi.
> Avval qizil chiroqni tekshiramiz (to'xtang), keyin sariqni (sekinlashing),
> keyin yashilni (yuring). Sariq chiroqni qizil borligida ham yondiramiz.
> Har bir shart oldingisini istisno qiladi.

### Ichma-ich shartlar

Shart ichida yana shart bo'lishi mumkin:

```cpp
agar (yosh >= 18) {
    agar (haydovchilik_guvohnomasi) {
        yozish << "Avtomobil haydashga ruxsat" << qator_oxiri
    }
    aks_holda {
        yozish << "Avval guvohnoma olishingiz kerak" << qator_oxiri
    }
}
aks_holda {
    yozish << "Voyaga yetmaganlar avtomobil haydolmaydi" << qator_oxiri
}
```

Buni `&&` bilan ham yozish mumkin:

```cpp
agar (yosh >= 18 && haydovchilik_guvohnomasi) {
    yozish << "Avtomobil haydashga ruxsat" << qator_oxiri
}
aks_holda agar (yosh >= 18) {
    yozish << "Avval guvohnoma olishingiz kerak" << qator_oxiri
}
aks_holda {
    yozish << "Voyaga yetmaganlar avtomobil haydolmaydi" << qator_oxiri
}
```

Ikkalasi ham to'g'ri, lekin kontekstga qarab biri yoki ikkinchisi
o'qish osonroq bo'ladi.

### Bir qatorli shart

Agar shart blokida bitta ko'rsatma bo'lsa, qavslarsiz yozish mumkin:

```cpp
agar (yosh >= 18) yozish << "Voyaga yetgan" << qator_oxiri
```

Lekin **biz buni tavsiya qilmaymiz**. Qavslar bo'lsa, kod tartibga
keladi va kelajakda yangi qator qo'shish oson bo'ladi.

---

## `tanlash` — bir nechta yo'naltirishlardan tanlash

Agar siz bir nechta aniq qiymatlarni tekshirayotgan bo'lsangiz, `tanlash`
(C++ da `switch`) qulayroq:

```cpp
butun kun = 3

tanlash (kun) {
    holat 1: yozish << "Dushanba" << qator_oxiri; to'xtatish
    holat 2: yozish << "Seshanba" << qator_oxiri; to'xtatish
    holat 3: yozish << "Chorshanba" << qator_oxiri; to'xtatish
    holat 4: yozish << "Payshanba" << qator_oxiri; to'xtatish
    holat 5: yozish << "Juma" << qator_oxiri; to'xtatish
    holat 6: yozish << "Shanba" << qator_oxiri; to'xtatish
    holat 7: yozish << "Yakshanba" << qator_oxiri; to'xtatish
    boshqa: yozish << "Noto'g'ri kun" << qator_oxiri
}
```

### Sintaksis va qoidalar

- `tanlash (ifoda)` — qaysi qiymatni tekshirayapmiz
- `holat qiymat:` — agar `ifoda == qiymat` bo'lsa, bu bo'limga
  o'tamiz
- `to'xtatish` — `tanlash` dan chiqamiz (boshqasini ham tekshirmaymiz)
- `boshqa:` — agar hech bir `holat` mos kelmasa, bu bajariladi (C++
  da `default`)

> **Diqqat — `to'xtatish` ni unutmang!** Agar `to'xtatish` yo'q bo'lsa,
> dastur **keyingi `holat`ga ham o'tib ketadi**. Bu ba'zan ataylab
> qilinadi, lekin ko'pincha xato.

### `to'xtatish` siz "fallthrough"

```cpp
butun oy = 2

tanlash (oy) {
    holat 12:
    holat 1:
    holat 2:
        yozish << "Qish" << qator_oxiri
        to'xtatish
    holat 3:
    holat 4:
    holat 5:
        yozish << "Bahor" << qator_oxiri
        to'xtatish
    holat 6:
    holat 7:
    holat 8:
        yozish << "Yoz" << qator_oxiri
        to'xtatish
    holat 9:
    holat 10:
    holat 11:
        yozish << "Kuz" << qator_oxiri
        to'xtatish
}
```

12, 1, va 2-oylar uchun bitta natija — "Qish". `to'xtatish` qo'yilmagani
sababli kompilyator pastki `holat`larga oqib boradi.

### `tanlash` ning chegaralari

`tanlash` faqat **butun va belgi** turi bilan ishlay oladi. Matn,
kasr yoki `mantiq` bilan ishlamaydi. Shu bilan bir qatorda — har bir
`holat` **aniq qiymat** bo'lishi kerak, oraliq emas.

```cpp
tanlash (baho) {
    holat (baho >= 90):       // ← XATO! Bunday yozib bo'lmaydi
    ...
}
```

Bu hollarda `agar/aks_holda` ishlating.

---

## Sikl — `davr` (while)

Sikl — bu kod blokini bir necha marta takrorlovchi konstruksiya.
**`davr`** (C++ da `while`) eng oddiy sikl:

```cpp
davr (shart) {
    // shart rost bo'lguncha takrorlanadi
}
```

Misol — 1 dan 5 gacha sonlarni chiqarish:

```cpp
butun i = 1
davr (i <= 5) {
    yozish << i << qator_oxiri
    i++
}
```

**Natija:**
```
1
2
3
4
5
```

### Qanday ishlaydi?

Har bir iteratsiyada:

1. Shart tekshiriladi (`i <= 5`)
2. Agar rost — blok bajariladi
3. Blok tugagandan keyin yana 1-bosqichga qaytamiz
4. Agar yolg'on — sikldan chiqamiz

```
   ┌──→ Shart? (i <= 5) ──── yolg'on ──→ Sikldan chiqish
   │       │
   │      rost
   │       │
   │       ▼
   │     Blok bajarish
   │       │
   └───────┘
```

### Sikl o'zgaruvchisi (counter)

Yuqoridagi `i` o'zgaruvchisi — **sikl o'zgaruvchisi** yoki **counter**.
U sikl davomida o'zgaradi va sikl qachon to'xtashini belgilaydi.

**Diqqat!** Agar siz `i++` ni unutsangiz, sikl **cheksiz** bo'ladi —
dastur hech qachon to'xtamaydi:

```cpp
butun i = 1
davr (i <= 5) {
    yozish << i << qator_oxiri
    // i++ unutilgan — cheksiz sikl!
}
```

Bunday holatlarda dasturni `Ctrl+C` bilan to'xtatishingiz kerak.

### `bajar/davr` — kamida bir marta bajariladi

```cpp
butun javob

bajar {
    yozish << "0 dan katta son kiriting: "
    kiritish >> javob
} davr (javob <= 0)

yozish << "Rahmat! " << javob << " qabul qilindi" << qator_oxiri
```

Bu yerda farq: blok **birinchi marta har holda** bajariladi, keyin
shart tekshiriladi.

> **Qachon `bajar/davr` ishlatish kerak?** Foydalanuvchidan ma'lumot
> so'raganda — siz birinchi marta savol berishingiz **shart**, keyin
> javobiga qarab davom etish yoki to'xtash.

---

## Sikl — `uchun` (for)

`uchun` — eng ko'p ishlatiladigan sikl. U `davr` ga qaraganda
qisqaroq va aniqroq.

### Sintaksis

```cpp
uchun (ishga tushirish; shart; o'zgartirish) {
    // takrorlanadigan blok
}
```

Uchta qism — `;` bilan ajratiladi:

1. **Ishga tushirish** — sikl boshlanishidan oldin bir marta bajariladi
2. **Shart** — har iteratsiyada tekshiriladi
3. **O'zgartirish** — har iteratsiya **oxirida** bajariladi

Misol — 1 dan 5 gacha sonlarni chiqarish (yuqoridagi `davr` kabi,
lekin qisqaroq):

```cpp
uchun (butun i = 1; i <= 5; i++) {
    yozish << i << qator_oxiri
}
```

Bu xuddi `davr` kabi ishlaydi, lekin 3 qator o'rniga 1 qator.

### `uchun` qanday ishlaydi?

```
   ┌──→ Boshlang'ich: i = 1 (bir marta)
   │
   ├──→ Shart? (i <= 5) ──── yolg'on ──→ Sikldan chiqish
   │       │
   │      rost
   │       │
   │       ▼
   │     Blok bajarish
   │       │
   │       ▼
   │     O'zgartirish: i++
   │       │
   └───────┘
```

### Murakkab `uchun` siklilari

```cpp
// 10 dan 1 gacha teskari hisoblash
uchun (butun i = 10; i >= 1; i--) {
    yozish << i << " "
}
// Natija: 10 9 8 7 6 5 4 3 2 1

// 2 lik qadam bilan
uchun (butun i = 0; i <= 20; i += 2) {
    yozish << i << " "
}
// Natija: 0 2 4 6 8 10 12 14 16 18 20

// Quvvatlash — 1, 2, 4, 8, 16, ...
uchun (butun i = 1; i <= 1000; i *= 2) {
    yozish << i << " "
}
// Natija: 1 2 4 8 16 32 64 128 256 512
```

### Range-based `uchun` (uchun har)

Agar siz to'plam (massiv, vector) bo'yicha takrorlamoqchi bo'lsangiz,
oddiy sintaksis:

```cpp
vektor<butun> sonlar = {10, 20, 30, 40, 50}

uchun (butun son : sonlar) {
    yozish << son << qator_oxiri
}
```

Bu — _range-based for_ deb ataladi. Buni to'plamlar haqida 7-bobda
ko'ramiz.

---

## Sikl ichida sikl (Nested loops)

Bir sikl ichida boshqa sikl bo'lishi mumkin. Bu **ichma-ich sikllar**.

### Misol — ko'paytirish jadvali

```cpp
uchun (butun i = 1; i <= 5; i++) {
    uchun (butun j = 1; j <= 5; j++) {
        yozish << (i * j) << "\t"
    }
    yozish << qator_oxiri
}
```

**Natija:**
```
1   2   3   4   5
2   4   6   8   10
3   6   9   12  15
4   8   12  16  20
5   10  15  20  25
```

### Qanday ishlaydi?

- Tashqi sikl (`i`) 1 dan 5 gacha boradi.
- Har bir `i` qiymati uchun, ichki sikl (`j`) 1 dan 5 gacha boradi.
- Jami 5 × 5 = **25** marta `(i * j)` chop etiladi.

### Ehtiyot bo'ling — murakkablik

Ichma-ich sikllar **tez sekinlashadi**:

- 1 sikl × 1000 iteratsiya = 1000 amal
- 2 ichma-ich sikl × 1000 iteratsiya = 1,000,000 amal
- 3 ichma-ich sikl × 1000 iteratsiya = 1,000,000,000 amal (1 milliard)

Agar siz katta ma'lumotlar bilan ishlasangiz, ehtiyot bo'ling: 3
ichma-ich sikl bilan dastur soatlab ishlashi mumkin.

---

## Sikldan chiqish va o'tkazib yuborish

### `to'xtatish` (break)

Sikldan **darhol chiqish**. Sikldagi qolgan iteratsiyalar bekor
qilinadi.

```cpp
uchun (butun i = 1; i <= 100; i++) {
    agar (i == 5) {
        to'xtatish
    }
    yozish << i << " "
}
// Natija: 1 2 3 4
```

**Real qo'llanish:** Foydalanuvchidan parol so'rash — agar to'g'ri
bo'lsa, sikldan chiqish:

```cpp
davr (rost) {
    matn parol
    yozish << "Parolni kiriting: "
    kiritish >> parol

    agar (parol == "salom123") {
        yozish << "Tabriklayman! Tizimga kirdingiz" << qator_oxiri
        to'xtatish
    }
    aks_holda {
        yozish << "Noto'g'ri parol. Qayta urinib ko'ring" << qator_oxiri
    }
}
```

`davr (rost)` — bu **cheksiz sikl**. U faqat `to'xtatish` orqali
to'xtaydi.

### `davom_ettirish` (continue)

Joriy iteratsiyani **o'tkazib yuborish**, lekin sikldan chiqmaslik —
keyingi iteratsiyaga o'tish.

```cpp
uchun (butun i = 1; i <= 10; i++) {
    agar (i % 2 == 0) {
        davom_ettirish      // juft sonlarni o'tkazib yuboramiz
    }
    yozish << i << " "
}
// Natija: 1 3 5 7 9 (faqat toq sonlar)
```

> **Eng ko'p uchraydigan xato:** `to'xtatish` va `davom_ettirish` ni
> chalkashtirish. `to'xtatish` — sikldan **butunlay** chiqish.
> `davom_ettirish` — faqat joriy iteratsiyani o'tkazib, keyingisiga
> o'tish.

---

## Cheksiz sikllar va ulardan qutilish

### `davr (rost)`

Bu eng tipik cheksiz sikl. Faqat `to'xtatish` orqali chiqish mumkin.

### Cheksiz `uchun`

```cpp
uchun (;;) {
    // ...
}
```

Bo'sh shart `rost` deb hisoblanadi.

### Qachon cheksiz sikl foydali?

- Foydalanuvchidan kirim olganda (xato bo'lsa qayta so'rash uchun)
- Server dasturlarida — har doim mijozlardan ulanishlarni kutish
- O'yin "loop"larida — har bir kadrni qayta-qayta chizish

---

## Amaliy misollar

### 1-misol: Mukammal son tekshiruvchi

Mukammal son — bu uning barcha bo'luvchilari (o'zi tashqari)
yig'indisi o'ziga teng bo'lgan son. Masalan: 6 = 1 + 2 + 3.

```cpp
butun asosiy() {
    butun son
    yozish << "Sonni kiriting: "
    kiritish >> son

    butun yigindi = 0
    uchun (butun i = 1; i < son; i++) {
        agar (son % i == 0) {
            yigindi += i
        }
    }

    agar (yigindi == son) {
        yozish << son << " — mukammal son!" << qator_oxiri
    }
    aks_holda {
        yozish << son << " — mukammal son emas" << qator_oxiri
    }

    qaytarish 0
}
```

### 2-misol: Fibonachchi sonlari

Fibonachchi sonlari: 0, 1, 1, 2, 3, 5, 8, 13, 21, ... — har bir keyingi
son oldingi ikkitasining yig'indisi.

```cpp
butun asosiy() {
    butun n
    yozish << "Necha ta Fibonachchi soni? "
    kiritish >> n

    butun a = 0
    butun b = 1

    uchun (butun i = 0; i < n; i++) {
        yozish << a << " "
        butun temp = a + b
        a = b
        b = temp
    }
    yozish << qator_oxiri

    qaytarish 0
}
```

### 3-misol: Tub sonlarni topish

Tub son — bu faqat 1 va o'ziga bo'linadigan son. 2, 3, 5, 7, 11, 13...

```cpp
butun asosiy() {
    butun n
    yozish << "N gacha tub sonlar (N): "
    kiritish >> n

    uchun (butun son = 2; son <= n; son++) {
        mantiq tub = rost
        uchun (butun i = 2; i * i <= son; i++) {
            agar (son % i == 0) {
                tub = yolg'on
                to'xtatish
            }
        }
        agar (tub) {
            yozish << son << " "
        }
    }
    yozish << qator_oxiri

    qaytarish 0
}
```

Kichik tushuntirish: nima uchun `i * i <= son`? Chunki agar `son`
bo'luvchisi `sqrt(son)` dan kichik bo'lsa, biz uni topamiz. Aks holda
— u tub son. Bu siklning ishlash tezligini sezilarli oshiradi.

---

## Eng ko'p uchraydigan xatolar

### 1. Cheksiz sikl

```cpp
butun i = 1
davr (i <= 10) {
    yozish << i << qator_oxiri
    // i++ ni unutdi! Cheksiz sikl.
}
```

### 2. Off-by-one xato

```cpp
// "1 dan 10 gacha" deb yozmoqchimiz
uchun (butun i = 1; i < 10; i++)     // 1 dan 9 gacha (faqat 9!)
uchun (butun i = 1; i <= 10; i++)    // 1 dan 10 gacha (TO'G'RI)
uchun (butun i = 0; i < 10; i++)     // 0 dan 9 gacha (10 ta son)
```

Off-by-one — bu eng ko'p uchraydigan xato. Doim aniq tekshiring:
"Birinchi qiymat nima? Oxirgi qiymat nima?"

### 3. `to'xtatish` faqat **bitta** sikldan chiqaradi

```cpp
uchun (butun i = 0; i < 10; i++) {
    uchun (butun j = 0; j < 10; j++) {
        agar (j == 5) {
            to'xtatish      // Faqat ichki sikldan chiqadi!
        }
    }
    // Tashqi sikl davom etadi
}
```

### 4. Switch ichida `to'xtatish` ni unutish

```cpp
tanlash (kun) {
    holat 1: yozish << "Dush"
    holat 2: yozish << "Sesh"
}
// "Dush" ham, "Sesh" ham chiqadi (fallthrough)!
```

### 5. `=` ni shart ichida ishlatish

```cpp
agar (a = 5) { ... }     // ← XATO: 5 ni saqladi, har doim rost
agar (a == 5) { ... }    // TO'G'RI
```

---

## Bob bo'yicha mashqlar

### Mashq 1: Eng katta son (oson)

Foydalanuvchidan 3 ta son so'rang. `agar/aks_holda` orqali eng kattasini
toping.

### Mashq 2: Baho hisoblovchi (oson)

Talabadan ball so'rang (0-100). Bahoni chiqaring:
- 90+: A'lo
- 70-89: Yaxshi
- 60-69: Qoniqarli
- <60: Qoniqarsiz

### Mashq 3: Faktoriyal (o'rta)

`n!` ni hisoblang (`5! = 1*2*3*4*5 = 120`). Foydalanuvchidan `n` so'rang.

```cpp
butun faktorial = 1
uchun (butun i = 1; i <= n; i++) {
    faktorial *= i
}
```

### Mashq 4: Sonni teskari aylantirish (o'rta)

Foydalanuvchidan butun son so'rang. Uni teskari aylantiring (123 →
321).

**Maslahat:**
```cpp
butun teskari = 0
davr (son > 0) {
    teskari = teskari * 10 + son % 10
    son /= 10
}
```

### Mashq 5: Yulduzlar piramidasi (o'rtacha)

Quyidagicha piramida chiqaring (`n=5` uchun):
```
    *
   ***
  *****
 *******
*********
```

**Maslahat:** Ichma-ich sikllar. Tashqi sikl — qatorlar uchun. Ichki —
bo'sh joylar va yulduzlar uchun.

### Mashq 6: Tasodifiy sonlar o'yini (qiyinroq)

Kompyuter 1 dan 100 gacha tasodifiy son tanlasin. Foydalanuvchi
taxmin qiladi. Har taxminga "kattaroq" yoki "kichikroq" deb yozsin.
Foydalanuvchi topgancha davom eting va necha urinish kerakligini
chiqaring.

**Maslahat:** Tasodifiy son uchun:
```cpp
#include <random>
butun sirli_son = std::rand() % 100 + 1
```

### Mashq 7: Eng katta umumiy bo'luvchi (qiyin)

Ikkita son uchun GCD (greatest common divisor) ni Euclid algoritmi
bilan toping.

**Maslahat:**
```cpp
davr (b != 0) {
    butun temp = b
    b = a % b
    a = temp
}
// a — javob
```

---

## Xulosa

Bu bobda biz dastur oqimini boshqarishni o'rgandik:

- **`agar/aks_holda`** — shartga qarab boshqacha kod bajarish.
- **`tanlash`** — bir nechta aniq qiymatdan tanlash, `tanlash` ning
  `holat` va `to'xtatish` qoidalari.
- **`davr`** — shart rost bo'lguncha takrorlash.
- **`bajar/davr`** — kamida bir marta bajaradigan sikl.
- **`uchun`** — ishga tushirish, shart va o'zgartirish bilan eng ko'p
  ishlatiladigan sikl.
- **Sikl ichida sikl** — ichma-ich sikllar va ularning murakkabligi.
- **`to'xtatish`** va **`davom_ettirish`** — sikldan chiqish va
  iteratsiyani o'tkazib yuborish.
- **Amaliyotda** — Fibonachchi, mukammal son, tub sonlar dasturlarini
  yozdik.

Keyingi bobda biz **funksiyalar** ni o'rganamiz — kodni qayta
ishlatish va dasturni mantiqiy bo'laklarga ajratish eng muhim usul.
Hozirgi katta dasturlaringizni o'nlab funksiyalarga bo'lib, har birini
alohida tushunarli qila olasiz.
