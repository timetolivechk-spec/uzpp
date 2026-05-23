# 02 · O'zgaruvchilar va ma'lumot turlari {#02-ozgaruvchilar-va-turlar}

## O'zgaruvchi nima?

Tasavvur qiling, siz omborxonada ishlaysiz. Omborxona — bu katta zal,
unda minglab javon va qutilar bor. Har bir qutida bir narsa saqlanadi:
biriga "Olma", boshqasiga "Sabzi", uchinchisiga "Un" deb yorliq
yopishtirilgan. Siz "Olma" kerak bo'lsa, "olma" yozilgan qutiga
borasiz, undan olasiz. "Olma"ni qaytarish kerak bo'lsa — yana o'sha
qutiga qo'yasiz.

Kompyuter xotirasi — xuddi shu omborxona. **O'zgaruvchi** — bu xotirada
nomlangan quti. Siz unga qiymat saqlaysiz, keyin nom orqali murojaat
qilasiz va qiymatni o'qiysiz yoki o'zgartirasiz.

```cpp
butun yosh = 25            // "yosh" deb nomlangan qutiga 25 ni saqla
yozish << yosh             // "yosh" qutisidan qiymatni o'qib chiqar
yosh = 26                  // qiymatni o'zgartir
```

E'tibor bering: `yosh` — bu nom (etiketka). `25` — qiymat (mazmun). Quti
o'zi — xotira manzili. Bu uchta narsa bir-biriga bog'liq, lekin alohida
tushunchalar.

> **Nima uchun "o'zgaruvchi" deyiladi?** Chunki uning qiymati _o'zgarishi
> mumkin_. Yuqoridagi misolda biz `yosh` ni 25 dan 26 ga o'zgartirdik.
> Bunga taqqoslash uchun, _doimiy_ (`o'zgarmas`) qiymat ham bor — uni
> bir marta belgilab, keyin o'zgartirib bo'lmaydi. Buni keyinroq ko'ramiz.

### Xotiradagi haqiqat

Yuqoridagi kod ishga tushganida, xotirada nima sodir bo'ladi?

Tasavvur qiling, xotira — bu juda uzun lenta, har bir katakda bitta
**bayt** (8 bit, ya'ni 0-255 oraliqdagi son) saqlanadi:

```
Manzil:  ... 0x7FF8  0x7FF9  0x7FFA  0x7FFB  ...
Mazmun:  ...   25       0       0       0     ...
                ^
                └──── "yosh" nomi shu yerga ko'rsatadi
```

`butun` (32-bitlik son) 4 ta baytni egallaydi. `25` raqami xotirada
quyidagicha saqlanadi: `25` (qiymatning eng past bayti), keyin `0`,
`0`, `0`. Kompyuter "yosh" nomini hisobga olib, 4 baytni o'qib bittadek
ko'radi: 25.

Bu darajada chuqur tushunish hozir shart emas — lekin **muhim narsa
shu**: o'zgaruvchi xotirada aniq bir joyni egallaydi va aniq miqdordagi
baytni iste'mol qiladi.

---

## Asosiy ma'lumot turlari

uz++ da ma'lumotning ko'p turlari bor. Avval eng asosiylarini tanitamiz,
keyin har biriga batafsil to'xtalamiz.

### Tezkor jadval

| Tur | C++ ekvivalenti | Hajm | Diapazon | Misol |
|-----|-----------------|------|----------|-------|
| `butun` | `int` | 4 bayt | -2,147,483,648 dan 2,147,483,647 gacha | `25`, `-7`, `1000000` |
| `uzun` | `long long` | 8 bayt | -9×10¹⁸ dan 9×10¹⁸ gacha | `9999999999` |
| `kasr` | `float` | 4 bayt | ±3.4×10³⁸ (taxminan 7 ta aniq raqam) | `3.14`, `-0.5` |
| `ikkilangan` | `double` | 8 bayt | ±1.8×10³⁰⁸ (taxminan 15 ta aniq raqam) | `3.141592653589` |
| `mantiq` | `bool` | 1 bayt | `rost` yoki `yolg'on` | `rost`, `yolg'on` |
| `belgi` | `char` | 1 bayt | 0 dan 255 gacha (ASCII) | `'A'`, `'?'`, `'5'` |
| `matn` | `std::string` | o'zgaruvchan | har qanday matn | `"Salom"`, `"Aziza"` |

### `butun` — butun sonlar

Bu eng ko'p ishlatiladigan tur. `butun` 4 baytni egallaydi va manfiy
hamda musbat butun sonlarni saqlay oladi.

```cpp
butun yosh = 25
butun temperatura = -15
butun aholisi = 35000000      // Toshkent aholisi
```

**Real hayotdagi misol:** Talabaning yoshi, mahsulot soni, do'kondagi
narx (so'mlarda), kompyuter xotirasidagi piksel koordinatasi — bularning
hammasi `butun`.

**Diqqat — diapazon!** `butun` 4 baytlik bo'lgani uchun, u **taxminan
±2.1 milliard** oralig'idagi sonlarni saqlay oladi. Agar siz bundan
katta son saqlamoqchi bo'lsangiz (masalan, dunyo aholisi 8 milliard),
`uzun` ishlatishingiz kerak.

```cpp
butun dunyo_aholisi = 8000000000      // ← XATO! butun chegaradan tashqari
uzun dunyo_aholisi = 8000000000       // ← TO'G'RI
```

### `uzun` — katta butun sonlar

8 bayt, ±9 kvintillion (kvintillion = 10¹⁸). Bu son shu darajada katta
— u koinotdagi yulduzlar sonidan ko'p!

```cpp
uzun galaktika_yulduzlari = 100000000000    // 10¹¹
uzun fayl_hajmi_baytda = 5368709120          // 5 GB
```

### `kasr` va `ikkilangan` — haqiqiy sonlar

Agar son kasrli bo'lsa (`3.14`, `0.5`, `-2.7`), unda `kasr` yoki
`ikkilangan` ishlatiladi.

| Tur | Hajm | Aniqligi |
|-----|------|----------|
| `kasr` | 4 bayt | taxminan 7 ta raqam |
| `ikkilangan` | 8 bayt | taxminan 15 ta raqam |

```cpp
kasr balandlik = 1.75           // metrlarda
ikkilangan pi = 3.14159265358979
```

**Qaysi birini ishlatish kerak?** Ko'p hollarda — `ikkilangan`. U
aniqroq, va zamonaviy protsessorlar `ikkilangan` bilan `kasr` kabi
tez ishlaydi. `kasr` ni faqat juda ko'p sonlar saqlash kerak bo'lsa
ishlating (grafika, mashina o'rganishi).

> **Diqqat! Kasr aniqligi.** Kompyuter `0.1` ni aniq saqlay olmaydi! U
> binar (ikkilik) sistemada ishlaydi, va `0.1` o'nlik kasr ikkilik
> kasrda **cheksiz takrorlanuvchi** son. Shuning uchun:
> ```cpp
> ikkilangan a = 0.1 + 0.2
> yozish << a << qator_oxiri      // 0.30000000000000004
> ```
> Aniqlik talab qilinadigan hisoblashlarda (pul, narx) — siz `butun`
> ishlatib so'mlarda hisoblashingiz yaxshiroq, yoki maxsus kutubxonalardan
> foydalanishingiz mumkin.

### `mantiq` — rost yoki yolg'on

`mantiq` faqat ikkita qiymatni saqlay oladi: `rost` (true) va `yolg'on`
(false). Bu shartlarni va bayroqlarni ifodalash uchun ishlatiladi.

```cpp
mantiq foydalanuvchi_kirgan = rost
mantiq parol_togri = yolg'on
mantiq tugma_bosilgan = (yosh > 18)
```

**Real hayotdagi misol:** Lift tugmasi bosilganmi yoki yo'qmi, foydalanuvchi
tizimga kirganmi, hujjat saqlandimi — barchasi `mantiq`.

### `belgi` — bitta belgi

Bitta belgini saqlash uchun. Qo'shtirnoqlar yagona apostrofda:

```cpp
belgi birinchi = 'A'
belgi savol = '?'
belgi raqam = '5'        // diqqat — bu son emas, BELGI!
```

`belgi` aslida 1 baytlik son sifatida saqlanadi. `'A'` ning ichki
qiymati 65 (ASCII jadvalida). Shu sababli siz unga arifmetik amallar
ham qila olasiz:

```cpp
belgi A = 'A'
belgi B = A + 1          // 'B' bo'ladi (65 + 1 = 66)
```

> **Real hayotda:** Eski Caesar shifri — har bir harfni 3 ta oldinga
> siljitish. `belgi` arifmetikasi bilan bunga 2 qator kod yetadi!

### `matn` — matn satrlari

Matn (so'z, jumla, paragraf) `matn` turi orqali saqlanadi:

```cpp
matn ism = "Aziza"
matn salom = "Salom, dunyo!"
matn paragraf = "Bu uzun matn ham bo'lishi mumkin."
```

`matn` — bu maxsus tur, u xotirada o'zgaruvchan miqdorda joy egallaydi
(matn uzunligiga qarab). 11-bobda biz matnlarning ichini batafsil
o'rganamiz.

---

## O'zgaruvchi e'lon qilish usullari

### 1-usul: aniq tur belgisi bilan

```cpp
butun yosh = 25
matn ism = "Aziza"
ikkilangan PI = 3.14159
```

Bu eng aniq usul. Siz kompyuterga: "men butun son saqlayman" deb
aytasiz.

### 2-usul: tip xulosalash (`o'zgaruvchan`)

```cpp
o'zgaruvchan yosh = 25                // butun deb xulosa qilinadi
o'zgaruvchan ism = "Aziza"            // matn deb xulosa qilinadi
o'zgaruvchan PI = 3.14159             // ikkilangan deb xulosa qilinadi
```

`o'zgaruvchan` (C++ da `auto`) — bu "qiymatdan kelib chiqib tipni o'zing
aniqla" demakdir. Bu uz++ ning juda qulay xususiyati — siz qisqaroq
kod yozasiz.

**Qachon `o'zgaruvchan` ishlatish kerak?**
- Tip ko'rinib turibdi va aniq (`o'zgaruvchan x = 5` — albatta butun)
- Tip uzoq va batafsil yozish noqulay (`std::map<matn, butun>` kabi)

**Qachon aniq tip yozish kerak?**
- Boshqalar uchun kod o'qish oson bo'lsin
- Tip muhim va aniq belgilash kerak

### 3-usul: qiymatsiz e'lon

```cpp
butun yosh                  // butun e'lon qilindi, lekin qiymat yo'q
matn ism
```

**Diqqat!** Qiymatsiz e'lon qilingan butun son **noma'lum qiymat**
oladi (xotirada qaysi bayt bo'lsa, o'sha). Buni "garbage value" deyiladi
va bu xato manbai. Doim qiymat bering:

```cpp
butun yosh = 0              // yaxshi
matn ism = ""               // yaxshi (bo'sh matn)
```

### Bir nechta o'zgaruvchini bir satrda

```cpp
butun a = 5, b = 10, c = 15
```

Yoki:

```cpp
butun a, b, c
a = 5
b = 10
c = 15
```

Bu uslub kichik o'zgaruvchilar uchun qulay, lekin har bir o'zgaruvchi
muhim bo'lsa, alohida qator bering — kod o'qish oson bo'ladi.

---

## O'zgarmaslar (Konstantalar)

Ba'zi qiymatlar **hech qachon o'zgarmasligi kerak**. Masalan:
- π (pi) son — 3.14159...
- Bir yildagi kunlar soni — 365
- Soatdagi soniyalar — 3600

Bunday qiymatlar uchun `o'zgarmas` (const) kalit so'zi ishlatiladi:

```cpp
o'zgarmas ikkilangan PI = 3.14159
o'zgarmas butun YIL_KUNLARI = 365
o'zgarmas matn DAVLAT = "O'zbekiston"
```

Endi agar dasturda siz PI ni o'zgartirmoqchi bo'lsangiz, kompilyator
xato beradi:

```cpp
PI = 3.14                   // ← XATO! o'zgarmasni o'zgartirib bo'lmaydi
```

> **Nima uchun o'zgarmaslar muhim?** Birinchidan, ular kodingizni
> _himoyalaydi_ — siz tasodifan muhim qiymatni o'zgartirib qo'yolmaysiz.
> Ikkinchidan, ular kodingizni _o'qishni osonlashtiradi_ — `YIL_KUNLARI`
> ko'rsa, hamma tushunadi nima nazarda tutilayotganini. `365` ni ko'rsa
> esa o'ylab qolish kerak. Uchinchidan, kompilyator o'zgarmas qiymatlar
> bilan tezroq ishlay oladi.

**Nomlash konvensiyasi:** O'zgarmaslar nomlari odatda **BARCHA HARFLAR
KATTA** yozilab, so'zlar `_` (pastki chiziq) bilan ajratiladi. Bu
o'zgaruvchilardan ko'rinish jihatidan ajratish uchun.

```cpp
o'zgarmas butun MAKS_FOYDALANUVCHILAR = 1000      // konstanta
butun joriy_foydalanuvchilar = 25                 // o'zgaruvchi
```

---

## Tur o'zgartirish (Type casting)

Ba'zan bir turdagi qiymatni boshqa turga aylantirish kerak bo'ladi. Bu
**tur o'zgartirish** (yoki **casting**) deyiladi.

### Yashirin (implicit) o'zgartirish

uz++ ba'zan turlarni avtomatik o'zgartiradi:

```cpp
butun a = 5
ikkilangan b = a            // butun → ikkilangan, ma'lumot yo'qolmaydi
yozish << b                 // 5
```

Lekin ba'zan ma'lumot yo'qoladi:

```cpp
ikkilangan x = 3.7
butun y = x                 // ikkilangan → butun, KASR YO'QOLADI!
yozish << y                 // 3 (3.7 emas!)
```

### Aniq (explicit) o'zgartirish

Yaxshiroq — siz aniq aytasiz nimani qilmoqchisiz:

```cpp
ikkilangan x = 3.7
butun y = butun(x)          // aniq aytamiz: butunga aylantir
```

Yoki C++ uslubida:

```cpp
butun y = static_cast<butun>(x)
```

**Real hayotdagi misol:** Sizda 3.7 metr arqon bor. Lekin sizga
faqat butun metr o'lchaydigan asbob berildi. Asbob "3 metr" deydi —
qolgan 0.7 metr "yo'qoladi".

```cpp
ikkilangan arqon_uzunligi = 3.7
butun ulchov = butun(arqon_uzunligi)     // 3
yozish << "Asbob ko'rsatdi: " << ulchov << " metr"
```

### Bo'lish va integer division

uz++ da (C++ kabi) **butun ÷ butun = butun**. Kasr qismi yo'qoladi:

```cpp
butun a = 7
butun b = 2
butun natija = a / b            // 3 (3.5 emas!)
```

Agar kasrli natija kerak bo'lsa, kamida bittasi `ikkilangan` bo'lsin:

```cpp
butun a = 7
butun b = 2
ikkilangan natija = ikkilangan(a) / b     // 3.5
```

Yoki:

```cpp
ikkilangan natija = 7.0 / 2               // 3.5 (7.0 — kasr)
```

> **Eng ko'p uchraydigan xato:** Yangi boshlovchilar `5 / 2` ni `2.5`
> deb o'ylashadi, lekin natija `2` chiqadi. Doim eslatib qo'ying: agar
> har ikki operand ham butun bo'lsa, natija ham butun!

---

## O'zgaruvchining ko'rinish sohasi (Scope)

Har bir o'zgaruvchining **yashash davri** va **ko'rinish sohasi** bor.
Bu nimani bildiradi?

Tasavvur qiling, omborxonada uchta zal bor: katta, o'rta, kichik. Katta
zaldan siz hamma zallarni ko'rasiz. O'rta zaldan — faqat o'rta va
kichik. Kichik zaldan — faqat kichik.

Xuddi shunday, o'zgaruvchi qaysi blokda e'lon qilingan bo'lsa, faqat
o'sha blokda va uning ichidagi bloklarda ko'rinadi.

```cpp
butun asosiy() {
    butun x = 10                    // x — asosiy ichida ko'rinadi

    agar (x > 0) {
        butun y = 20                 // y — faqat shu blokda
        yozish << x << " " << y     // ikkalasi ham ko'rinadi
    }

    yozish << x                      // x ko'rinadi
    yozish << y                      // ← XATO! y ko'rinmaydi bu yerda
}
```

### Mahalliy (Local) o'zgaruvchilar

Funksiya yoki blok ichida e'lon qilingan o'zgaruvchilar **mahalliy**
hisoblanadi. Ular blok tugashi bilan _yo'qoladi_.

### Global o'zgaruvchilar

Funksiyalardan _tashqarida_ e'lon qilingan o'zgaruvchilar **global**
hisoblanadi. Ular butun dastur davomida yashaydi va hamma joydan
ko'rinadi:

```cpp
butun global_x = 100                  // global, hamma joydan ko'rinadi

butun asosiy() {
    yozish << global_x                // 100
}
```

**Diqqat — global o'zgaruvchilardan ehtiyot bo'ling!** Ularni har joyda
o'zgartirish mumkin, va bu dasturni o'qish hamda tuzatishni qiyinlashtiradi.
Iloji bo'lsa, global o'zgaruvchilar o'rniga mahalliy ishlatib, ularni
funksiyalarga parametr sifatida bering.

### Shadowing (Soyalash)

Agar ichki blokda tashqi bilan bir xil nomli o'zgaruvchi e'lon qilinsa
— ichkari _tashqarisini yopadi_:

```cpp
butun asosiy() {
    butun x = 10
    {
        butun x = 20                  // tashqi x ni yopadi
        yozish << x                   // 20
    }
    yozish << x                       // 10 (tashqi x qaytdi)
}
```

Bu odatda **yomon amaliyot**. Ikki xil o'zgaruvchini bir xil nomlash
xatoga olib keladi.

---

## Qiymat berish va operatorlar

### Asosiy qiymat berish

```cpp
butun x = 5             // x ga 5 qiymatini ber
x = 10                  // x ni 10 ga o'zgartir
x = x + 1               // x ni o'zining qiymati + 1 ga o'zgartir
```

### Qisqacha qiymat berish

`x = x + 1` ni qisqacha yozish mumkin:

```cpp
x += 1                  // x = x + 1
x -= 2                  // x = x - 2
x *= 3                  // x = x * 3
x /= 4                  // x = x / 4
x %= 5                  // x = x % 5
```

Va eng qisqasi — **inkrement** va **dekrement**:

```cpp
x++                     // x = x + 1 (orqa)
++x                     // x = x + 1 (oldi)
x--                     // x = x - 1
--x                     // x = x - 1
```

**`++x` va `x++` o'rtasidagi farq:** ifoda ichida qiymat _qachon_
yangilanadi:

```cpp
butun a = 5
butun b = a++           // b = 5, keyin a = 6
                        // (oldin qiymat berdi, keyin oshirdi)

butun c = 5
butun d = ++c           // c = 6, keyin d = 6
                        // (oldin oshirdi, keyin qiymat berdi)
```

Yangi boshlovchilarga: agar ifoda ichida qiymat olmasangiz (faqat `x++`
yoki `++x` deb yozsangiz) — farqi yo'q.

---

## Amaliy misol: Talabaning ma'lumotnomasi

Endi biz o'rgangan barcha narsalarni bitta amaliy dasturda ko'rib
chiqamiz. Talabaning ma'lumotnomasini yarataylik.

```cpp
// talaba.uzpp — talaba ma'lumotnomasi

o'zgarmas butun JORIY_YIL = 2026

butun asosiy() {
    // Talaba ma'lumotlari
    matn ism = "Aziza"
    matn familiya = "Karimova"
    butun tugilgan_yili = 2003
    ikkilangan ortacha_bahosi = 4.7
    mantiq stipendiyada = rost

    // Hisoblash
    butun yoshi = JORIY_YIL - tugilgan_yili

    // Chiqarish
    yozish << "=== TALABA MA'LUMOTNOMASI ===" << qator_oxiri
    yozish << "Ism: " << ism << " " << familiya << qator_oxiri
    yozish << "Yoshi: " << yoshi << qator_oxiri
    yozish << "O'rtacha baho: " << ortacha_bahosi << qator_oxiri

    agar (stipendiyada) {
        yozish << "Stipendiyada: ha" << qator_oxiri
    }
    aks_holda {
        yozish << "Stipendiyada: yo'q" << qator_oxiri
    }

    qaytarish 0
}
```

**Natija:**

```
=== TALABA MA'LUMOTNOMASI ===
Ism: Aziza Karimova
Yoshi: 23
O'rtacha baho: 4.7
Stipendiyada: ha
```

### Bu dasturda nima bo'ldi?

1. **`JORIY_YIL`** — o'zgarmas konstanta. U hech qachon o'zgarmaydi
   (yoki yangi yilga ko'tarish kerak bo'lsa, **bitta joyda** — bu
   yerda).
2. **Turli turdagi o'zgaruvchilar** — matn (ism), butun (yosh), kasr
   (baho), mantiq (stipendiya). Har bir tur o'zining maqsadi uchun.
3. **Hisoblash** — `yoshi = JORIY_YIL - tugilgan_yili`. Ikkita butun
   sonning ayirmasi yana butun.
4. **Shart operatori** — `agar` orqali boolean qiymatga qarab boshqacha
   xabar chiqarish.

---

## Eng ko'p uchraydigan xatolar

### 1. Tip mosligini buzish

```cpp
butun yosh = 3.7              // ← XATO yoki ogohlantirish: 3 bo'lib qoladi
butun yosh = 3                // TO'G'RI
ikkilangan yosh = 3.7         // TO'G'RI
```

### 2. Butun bo'lish

```cpp
ikkilangan ortacha = 7 / 2     // ← XATO: 3.0 (3.5 emas)
ikkilangan ortacha = 7.0 / 2   // TO'G'RI: 3.5
```

### 3. Inicializatsiya qilmaslik

```cpp
butun a                        // a — noma'lum qiymat
yozish << a                    // ← XATO: kim biladi nima chiqadi!

butun a = 0                    // TO'G'RI
```

### 4. O'zgarmasni o'zgartirishga urinish

```cpp
o'zgarmas butun PI_TAXMINAN = 3
PI_TAXMINAN = 4                // ← XATO: o'zgarmas
```

### 5. Scope tashqarisidan o'zgaruvchini ishlatish

```cpp
butun asosiy() {
    {
        butun temp = 100
    }
    yozish << temp             // ← XATO: temp ko'rinmaydi
}
```

---

## Bob bo'yicha mashqlar

### Mashq 1: Barcha turlar (oson)

Har bir asosiy tur (`butun`, `kasr`, `mantiq`, `belgi`, `matn`) uchun
bittadan o'zgaruvchi yarating va konsolga chiqaring.

### Mashq 2: Tur o'zgartirish (o'rta)

Foydalanuvchidan `ikkilangan` turdagi son so'rang. Uni `butun` ga
aylantiring va ikkala qiymatni chiqaring. Kasr qismi qancha yo'qolganini
ham ko'rsating.

**Maslahat:** `kasr_qismi = asl - butun(asl)`

### Mashq 3: Yosh kalkulyatori (o'rta)

Foydalanuvchidan tug'ilgan yilini va joriy yilni so'rang. Yoshini
hisoblang. Lekin bu safar: shuningdek, foydalanuvchi qancha kun, soat
va daqiqa yashaganini ham hisoblang.

**Maslahat:**
- Kunlar: yosh × 365 (yoki aniqroq — 365.25)
- Soatlar: kunlar × 24
- Daqiqalar: soatlar × 60

### Mashq 4: O'zgarmaslarni qo'llash (o'rtacha)

Doira yuzasini hisoblovchi dastur yozing. Foydalanuvchidan radiusni
so'rang. Yuzasi = π × r². PI ni o'zgarmas qilib e'lon qiling.

### Mashq 5: Pul kalkulyatori (o'rtacha)

Foydalanuvchidan miqdorni (so'mlarda) va valyuta kursini (1 dollar
necha so'm) so'rang. Miqdorni dollarga aylantiring.

### Mashq 6: Caesar shifri (qiyinroq)

Foydalanuvchidan bitta belgi so'rang. Uni 3 ta oldinga siljitib
chiqaring (`A` → `D`, `B` → `E`, va h.k.).

### Mashq 7: O'zgaruvchini almashtirish (qiyin)

Ikkita o'zgaruvchining qiymatlarini almashtirib chiqing. Sizga uchinchi
o'zgaruvchi kerak — vaqtinchalik saqlash uchun.

```cpp
butun temp = a
a = b
b = temp
```

---

## Xulosa

Bu bobda biz:

- **O'zgaruvchi nimaligini** o'rgandik — xotirada nomlangan quti.
- **Xotira haqida** umumiy tushunchaga ega bo'ldik — har bir o'zgaruvchi
  baytlar miqdorini egallaydi.
- **Asosiy turlarni** ko'rib chiqdik: `butun`, `uzun`, `kasr`,
  `ikkilangan`, `mantiq`, `belgi`, `matn`.
- **O'zgaruvchi e'lon qilish** uch usulini bilib oldik — aniq tur,
  `o'zgaruvchan`, qiymatsiz.
- **O'zgarmaslar (`o'zgarmas`)** bilan tanishdik.
- **Tur o'zgartirishni** va undagi tuzoqlarni o'rgandik.
- **Ko'rinish sohasi (scope)** — mahalliy va global o'zgaruvchilar
  o'rtasidagi farq.
- **Qiymat berish operatorlari** — `=`, `+=`, `++`.
- **Eng ko'p uchraydigan xatolar** va ularni qanday oldini olish.

Keyingi bobda biz **operatorlar va ifodalar** haqida batafsil
gaplashamiz.
