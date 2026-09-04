# 03 · Operatorlar va ifodalar {#03-operatorlar}

## Ifoda nima?

**Ifoda** (expression) — bu bir yoki bir nechta qiymat va ular ustida
amallar bajaradigan belgilar (operatorlar) majmuasi, qaysi natijada
**bitta qiymat** beradi.

Tasavvur qiling, siz oshxonada turli ingredientlardan yaxshi taom
tayyorlaysiz. Kartoshka + sabzi + suv + olov = osh. Bu — _ifoda_:
bir nechta narsa (operandlar) va amal (qo'shish/qaynatish — operator)
bir yangi narsa (taom — qiymat) beradi.

Kodda:

```cpp
5 + 3; // ifoda, qiymati 8
yosh > 18; // ifoda, qiymati rost yoki yolg'on
(a + b) * c; // murakkab ifoda
yozish << "Salom"; // bu ham ifoda!
```

Har bir ifoda **bitta qiymat** beradi. Bu qiymatni siz o'zgaruvchiga
saqlashingiz, taqqoslashingiz yoki boshqa ifodaga qo'shishingiz mumkin.

```cpp
butun a = 5 + 3; // 8 qiymati a ga saqlandi
mantiqiy katta = (a > 5); // rost qiymati katta ga saqlandi
butun b = (a + 3) * 2; // 22 qiymati b ga saqlandi
```

> **Operator va operand:** Bu yerda kichik atamalar bor.
> - **Operator** — bu amal belgisi. Masalan, `+`, `-`, `*`, `==`.
> - **Operand** — bu operator ishlaydigan qiymat. `5 + 3` da `5` va `3`
>   — operandlar, `+` — operator.

Endi keling, har bir operator turini batafsil ko'rib chiqamiz.

---

## Arifmetik operatorlar

Bular eng asosiy operatorlar — siz maktabda matematikadan o'rgangan
amallar. Beshtasi bor:

| Operator | Ma'nosi | Misol | Natija |
|----------|---------|-------|--------|
| `+` | Qo'shish | `5 + 3` | `8` |
| `-` | Ayirish | `10 - 4` | `6` |
| `*` | Ko'paytirish | `6 * 7` | `42` |
| `/` | Bo'lish | `15 / 3` | `5` |
| `%` | Qoldiq (modulo) | `17 % 5` | `2` |

`+`, `-`, `*` belgilarini siz biladingiz. `/` (bo'lish) ham aniq.
Lekin `%` (modulo) sizga yangi bo'lishi mumkin.

### Modulo (`%`) — bo'lishdan qolgan qism

`a % b` — `a` ni `b` ga bo'lganda **qolgan qism**.

Misollar:
- `17 % 5` = `2` (17 = 5×3 + 2)
- `10 % 3` = `1` (10 = 3×3 + 1)
- `20 % 4` = `0` (aniq bo'linadi)
- `7 % 2` = `1` (7 toq)
- `8 % 2` = `0` (8 juft)

**Modulo nima uchun foydali?**

#### 1. Juft yoki toqligini aniqlash

```cpp
agar (son % 2 == 0) {
    yozish << "Juft son" << qator_oxiri;
}
aks_holda {
    yozish << "Toq son" << qator_oxiri;
}
```

#### 2. Aylanish (circular indexing)

```cpp
butun kun = 10;
matn hafta_kunlari[7] = {"Dush", "Sesh", "Chor", "Pay", "Jum", "Shan", "Yak"}
matn bugun = hafta_kunlari[kun % 7]; // har 7 kunda aylanadi
```

#### 3. Bir necha raqamga ajratish

```cpp
butun son = 12345;
butun birlar = son % 10; // 5
butun onliklar = (son / 10) % 10; // 4
butun yuzliklar = (son / 100) % 10; // 3
```

> **Hayotiy misol:** Soat 24 soatlik ko'rsatkichdan 12 soatlikga
> o'tkazish kerak: `12_soatlik = 24_soatlik % 12`. Soat 14 → 2,
> soat 23 → 11.

### Bo'lish va butun bo'lish — eslang!

Avvalgi bobda ko'rganimizdek, `butun / butun = butun`. Kasr qismi
yo'qoladi:

```cpp
butun a = 7 / 2; // 3 (3.5 emas!)
haqiqiy b = 7.0 / 2; // 3.5
haqiqiy c = 7 / 2.0; // 3.5
haqiqiy d = 7.0 / 2.0; // 3.5
```

**Eng muhim qoida:** Agar siz kasr natija olishni xohlasangiz, **kamida
bittasi operand `haqiqiy` (yoki `kasr`) bo'lishi kerak**.

### Bir operandli minus (`-`)

`-` ikki holatda ishlatiladi:

```cpp
butun a = 5 - 3; // ikki operandli ayirish
butun b = -7; // bir operandli — manfiy belgi
butun c = -a; // -5 ga teng
```

---

## Taqqoslash operatorlari

Bular ikkita qiymatni solishtiradi. Natija har doim **`mantiqiy`** turida
(`rost` yoki `yolg'on`).

| Operator | Ma'nosi | Misol | Natija |
|----------|---------|-------|--------|
| `==` | Teng | `5 == 5` | `rost` |
| `!=` | Teng emas | `5 != 3` | `rost` |
| `<` | Kichik | `3 < 5` | `rost` |
| `>` | Katta | `7 > 2` | `rost` |
| `<=` | Kichik yoki teng | `5 <= 5` | `rost` |
| `>=` | Katta yoki teng | `5 >= 3` | `rost` |

### `=` va `==` — eng yomon sintaktik xato

Bu ikkisini farqlash juda muhim:

- `=` — **qiymat berish** (saqlash). `a = 5` — `a` ga 5 ni saqla.
- `==` — **tenglikni tekshirish**. `a == 5` — `a` 5 ga tengmi?

```cpp
butun yosh = 18;

agar (yosh = 21) {           // ← XATO: yosh ga 21 ni saqlamoqda
    // Bu blok HAR DOIM bajariladi!
}

agar (yosh == 21) {          // TO'G'RI: solishtirmoqda
    yozish << "Bu yoshda" << qator_oxiri;
}
```

uz++ kompilyatori bunday xatoni ushlashga harakat qiladi (ogohlantirish
beradi), lekin siz e'tiborli bo'ling.

### Matnlarni taqqoslash

`matn` turi bilan ham `==` va `!=` ishlaydi:

```cpp
matn ism1 = "Aziza";
matn ism2 = "Aziza";
matn ism3 = "Bobur";

agar (ism1 == ism2) {
    yozish << "Bir xil" << qator_oxiri; // ← chiqadi
}

agar (ism1 != ism3) {
    yozish << "Farqli" << qator_oxiri; // ← chiqadi
}
```

`<` va `>` ham ishlaydi — alfavit bo'yicha tartiblaydi:

```cpp
agar ("Aziza" < "Bobur") {
    yozish << "A B dan oldin" << qator_oxiri;
}
```

---

## Mantiqiy operatorlar

Bularning yordamida bir nechta shartni birlashtirish mumkin. Faqat
**`mantiqiy`** turidagi qiymatlar bilan ishlaydi.

| Operator | Ma'nosi | Misol |
|----------|---------|-------|
| `&&` | VA (AND) | `rost && rost` = `rost` |
| `\|\|` | YOKI (OR) | `rost \|\| yolg'on` = `rost` |
| `!` | EMAS (NOT) | `!rost` = `yolg'on` |

### Haqiqat jadvali

| `a` | `b` | `a && b` | `a \|\| b` |
|-----|-----|----------|-----------|
| `rost` | `rost` | `rost` | `rost` |
| `rost` | `yolg'on` | `yolg'on` | `rost` |
| `yolg'on` | `rost` | `yolg'on` | `rost` |
| `yolg'on` | `yolg'on` | `yolg'on` | `yolg'on` |

### Misollar

```cpp
butun yosh = 25;
mantiqiy talaba = rost;
mantiqiy stipendiyada = yolg'on;

agar (yosh >= 18 && yosh <= 25) {
    yozish << "Yosh — talaba" << qator_oxiri;
}

agar (talaba || stipendiyada) {
    yozish << "Imtiyozli" << qator_oxiri;
}

agar (!stipendiyada) {
    yozish << "Stipendiya yo'q" << qator_oxiri;
}
```

### Real hayotdagi misol

Ovoz berishga ruxsat berish sharti:
- Yoshi 18 dan katta YA'NI
- O'zbekiston fuqarosi YA'NI
- Royhatdan o'tgan

```cpp
mantiqiy ruxsat = (yosh >= 18) && (fuqaro == "O'zbekiston") && royxatda;
```

### Qisqa baholash (short-circuit evaluation)

Bu — **juda muhim** xususiyat. uz++ (C++ kabi) mantiqiy ifodani
**chapdan o'ngga** baholaydi va to'xtaydi natija ma'lum bo'lishi
bilanoq.

```cpp
// && bilan: agar birinchi yolg'on — ikkinchini umuman tekshirmaydi
agar (yolg'on && murakkab_amal()) {       // murakkab_amal() chaqirilmaydi!
    // ...
}

// || bilan: agar birinchi rost — ikkinchini umuman tekshirmaydi
agar (rost || murakkab_amal()) {           // murakkab_amal() chaqirilmaydi!
    // ...
}
```

**Bu qachon foydali?** Xavfsiz tekshirish uchun:

```cpp
// Avval tekshiramiz x noldan farqli, keyin bo'lamiz
agar (x != 0 && y / x > 5) {
    // Agar x = 0 bo'lsa, y / x amalga oshmaydi
    // chunki birinchi shart yolg'on, qisqa baholash to'xtaydi
}
```

Buni "qisqa baholash" yoki "lazy evaluation" deyiladi. C++ ning klassik
xususiyatlaridan biri.

---

## Bit operatorlari

Bu operatorlar **bitlar darajasida** ishlaydi. Yangi boshlovchilar
uchun bu ko'pincha murakkab tuyuladi, lekin amaliyotda juda foydali.

Eslating: har bir butun son xotirada bitlarda saqlanadi. Masalan,
`5` = `00000101` (8 bit), `3` = `00000011`.

| Operator | Ma'nosi |
|----------|---------|
| `&` | Bit AND |
| `\|` | Bit OR |
| `^` | Bit XOR |
| `~` | Bit NOT (inkor) |
| `<<` | Chapga siljitish |
| `>>` | O'ngga siljitish |

### Bit AND (`&`)

Har bir bit pozitsiyasini alohida AND amal:

```
  00000101  (5)
& 00000011  (3)
-----------
  00000001  (1)
```

```cpp
butun a = 5 & 3; // 1
```

### Bit OR (`|`)

```
  00000101  (5)
| 00000011  (3)
-----------
  00000111  (7)
```

```cpp
butun a = 5 | 3; // 7
```

### Bit XOR (`^`)

XOR (eksklyuziv yoki) — agar bitlar **farqli** bo'lsa 1, aks holda 0:

```
  00000101  (5)
^ 00000011  (3)
-----------
  00000110  (6)
```

### Siljitish (`<<` va `>>`)

`a << n` — `a` ning bitlari `n` ta chap tomonga siljiydi. Bu **2 ga
ko'paytirish bilan teng**:

```cpp
butun a = 5 << 1; // 10 (5 × 2)
butun b = 5 << 2; // 20 (5 × 4)
butun c = 5 << 3; // 40 (5 × 8)
```

`a >> n` — `a` ning bitlari `n` ta o'ng tomonga siljiydi. Bu **2 ga
bo'lish bilan teng**:

```cpp
butun a = 20 >> 1; // 10 (20 / 2)
butun b = 20 >> 2; // 5
```

> **Diqqat!** `<<` va `>>` bit operatorlari, lekin biz ularni `yozish
> << matn` da ham ishlatamiz. Bu boshqacha — `yozish` da bu **chiqarish
> operatori**, son emas. Kompilyator kontekstga qarab to'g'ri tushunadi.

### Bit operatorlari amaliyotda

#### 1. Sifatlar bayrog'i (flags)

Tasavvur qiling, foydalanuvchi sifatlari mavjud: o'qiy oladi, yoza
oladi, administrator. Har birini bitta bit bilan ifodalaymiz:

```cpp
o'zgarmas butun OQISH = 1; // 001
o'zgarmas butun YOZISH = 2; // 010
o'zgarmas butun ADMIN = 4; // 100

// Foydalanuvchi o'qiy oladi VA yoza oladi
butun sifatlar = OQISH | YOZISH; // 011 = 3

// O'qiy oladimi?
agar (sifatlar & OQISH) {
    yozish << "O'qiy oladi" << qator_oxiri;
}

// Yoza oladimi?
agar (sifatlar & YOZISH) {
    yozish << "Yoza oladi" << qator_oxiri;
}
```

Bu juda foydali — siz **bitta butun sonda 32 ta turli xil bayroqni**
saqlashingiz mumkin (32-bit butun uchun)!

#### 2. Tezkor 2 ga ko'paytirish/bo'lish

Eski mashinalarda `<<` va `>>` `*` va `/` dan ancha tez edi.
Zamonaviy protsessorlarda farq sezilarli emas, lekin bit operatorlari
hali ham koddni qisqaroq qiladi.

---

## Qiymat berish operatorlari

| Operator | Ma'nosi | Ekvivalenti |
|----------|---------|-------------|
| `=` | Sodda qiymat berish | — |
| `+=` | Qo'shib berish | `a = a + b` |
| `-=` | Ayirib berish | `a = a - b` |
| `*=` | Ko'paytirib berish | `a = a * b` |
| `/=` | Bo'lib berish | `a = a / b` |
| `%=` | Qoldiqni berish | `a = a % b` |
| `&=` | Bit AND berish | `a = a & b` |
| `\|=` | Bit OR berish | `a = a \| b` |
| `<<=` | Chap siljitib berish | `a = a << b` |

```cpp
butun a = 10;
a += 5; // a = 15
a *= 2; // a = 30
a %= 7; // a = 2 (30 % 7)
a <<= 3; // a = 16 (2 × 8)
```

---

## Boshqa operatorlar

### Inkrement (`++`) va dekrement (`--`)

```cpp
butun a = 5;
a++; // a = 6 (post-increment)
++a; // a = 7 (pre-increment)
a--; // a = 6
--a; // a = 5
```

`++a` va `a++` ifoda ichida farqli ishlaydi:

```cpp
butun a = 5;
butun x = a++; // x = 5 (oldin oldi, keyin oshirdi)
yozish << a; // 6

butun b = 5;
butun y = ++b; // y = 6 (oldin oshirdi, keyin oldi)
yozish << b; // 6
```

### Uchlik operator (`?:`)

Bu — qisqartirilgan `agar/aks_holda`:

```cpp
butun yosh = 20;
matn natija = (yosh >= 18) ? "Voyaga yetgan" : "Voyaga yetmagan";
```

Ya'ni:
```cpp
matn natija;
agar (yosh >= 18) {
    natija = "Voyaga yetgan";
}
aks_holda {
    natija = "Voyaga yetmagan";
}
```

Sintaksis: `shart ? agar_rost_qiymat : agar_yolg'on_qiymat`

**Qachon ishlatish kerak?**
- Oddiy ikki tomonlama tanlov uchun (kod qisqaroq)
- Lekin ko'p shartlar bo'lsa — `agar/aks_holda` o'qish osonroq

### `sizeof` operatori

Bu turning xotirada qancha bayt egallashini qaytaradi:

```cpp
yozish << sizeof(butun) << qator_oxiri; // 4
yozish << sizeof(haqiqiy) << qator_oxiri; // 8
yozish << sizeof(belgi) << qator_oxiri; // 1
```

---

## Operatorlar ustunligi (Precedence)

Agar ifoda ichida bir nechta operator bo'lsa, ular qaysi tartibda
bajariladi?

Matematikadagi kabi: birinchi `*` va `/`, keyin `+` va `-`.

```cpp
butun a = 2 + 3 * 4; // 14, not 20 (3*4 oldin)
butun b = (2 + 3) * 4; // 20
```

### To'liq jadval

| Daraja | Operatorlar | Yo'nalish |
|--------|-------------|-----------|
| 1 (eng yuqori) | `()`, `[]`, `.`, `->` | chapdan o'ngga |
| 2 | `!`, `~`, `++`, `--`, `-` (unar) | o'ngdan chapga |
| 3 | `*`, `/`, `%` | chapdan o'ngga |
| 4 | `+`, `-` | chapdan o'ngga |
| 5 | `<<`, `>>` | chapdan o'ngga |
| 6 | `<`, `<=`, `>`, `>=` | chapdan o'ngga |
| 7 | `==`, `!=` | chapdan o'ngga |
| 8 | `&` (bit) | chapdan o'ngga |
| 9 | `^` | chapdan o'ngga |
| 10 | `\|` (bit) | chapdan o'ngga |
| 11 | `&&` | chapdan o'ngga |
| 12 | `\|\|` | chapdan o'ngga |
| 13 | `?:` | o'ngdan chapga |
| 14 | `=`, `+=`, `-=`, ... | o'ngdan chapga |

**Eslab qolish kerak emas!** Ikkita maslahat:

1. **Murakkab ifodalarda doimo qavslar qo'ying.** Hatto kerak bo'lmasa ham.

   ```cpp
   // Yomon — ustunlikka tayanish
   butun natija = a + b * c << 2;

   // Yaxshi — aniq ifoda
   butun natija = (a + (b * c)) << 2;
   ```

2. **Kod o'qish kerakliroq ekanligini eslang.** Sizning kodingizni
   keyin boshqalar (yoki kelajakdagi siz) o'qiydi. Ortiqcha qavslar
   yomon emas, ular yordam beradi.

---

## Amaliy misol: Kvadrat tenglama yechish

`ax² + bx + c = 0` ko'rinishidagi tenglamani yechish. Diskriminant:
`D = b² - 4ac`.
```cpp
// kvadrat_tenglama.uzpp

butun asosiy() {
    // Koeffitsiyentlarni kiritish
    haqiqiy a, b, c;
    yozish << "a koeffitsiyent: ";
    kiritish >> a;
    yozish << "b koeffitsiyent: ";
    kiritish >> b;
    yozish << "c koeffitsiyent: ";
    kiritish >> c;

    // Diskriminant
    haqiqiy D = b * b - 4 * a * c;
    yozish << "Diskriminant D = " << D << qator_oxiri;

    // Yechimlarni hisoblash
    agar (D > 0) {
        haqiqiy x1 = (-b + std::sqrt(D)) / (2 * a);
        haqiqiy x2 = (-b - std::sqrt(D)) / (2 * a);
        yozish << "Ikkita haqiqiy ildiz:" << qator_oxiri;
        yozish << "  x_1 = " << x1 << qator_oxiri;
        yozish << "  x_2 = " << x2 << qator_oxiri;
    }
    aks_holda agar (D == 0) {
        haqiqiy x = -b / (2 * a);
        yozish << "Bitta haqiqiy ildiz: x = " << x << qator_oxiri;
    }
    aks_holda {
        yozish << "Haqiqiy ildizlar mavjud emas." << qator_oxiri;
    }

    qaytarish 0;
}
```

**Natija** (kiritilgan: `a=1, b=-5, c=6`):
```
Diskriminant D = 1
Ikkita haqiqiy ildiz:
  x_1 = 3
  x_2 = 2
```

### Bu dasturda nima kechdi?

1. **`haqiqiy` turi** — koeffitsiyentlar va natijalar kasr
   bo'lishi mumkin.
2. **`std::sqrt(D)`** — kvadrat ildiz funksiyasi. 8-bobda funksiyalar
   haqida o'rganganimizda batafsil ko'ramiz.
3. **Operatorlar ustunligi** — `b * b - 4 * a * c` da `*` `-` dan
   oldin ishlaydi: aslida `(b*b) - (4*a*c)`. Lekin tushunmoqchi bo'lsangiz,
   qavs qo'yish yaxshi.
4. **`aks_holda agar`** — bir necha shartlarni ketma-ket tekshirish.

---

## Eng ko'p uchraydigan xatolar

### 1. `=` va `==` ni adashtirish
```cpp
agar (yosh = 18) { ... }     // ← XATO: 18 ni saqlamoqda
agar (yosh == 18) { ... }    // TO'G'RI
```

### 2. Butun bo'lish kutilmagan natija beradi
```cpp
haqiqiy notogri = (1 + 2 + 3) / 3;   // 2.0 — chunki 6 / 3 BUTUN bo'linish
haqiqiy togri   = (1 + 2 + 3) / 3.0; // 2.0 — bu safar haqiqiy bo'linish

// Farq 7 / 2 da yaqqol ko'rinadi:
haqiqiy yarim_notogri = 7 / 2;   // 3.0  (butun bo'linish, kasr qismi yo'qoladi)
haqiqiy yarim_togri   = 7 / 2.0; // 3.5
```

### 3. Operatorlar ustunligini noto'g'ri tushunish
```cpp
agar (a == 5 || b == 5 && c == 5);
// Bu aslida: a == 5 || (b == 5 && c == 5)
// Sizning niyatingiz boshqa edimi? Qavs qo'ying!
agar ((a == 5 || b == 5) && c == 5);
```

### 4. Sonni 0 ga bo'lish
```cpp
butun a = 10;
butun b = 0;
butun c = a / b; // ← XATO: dastur to'xtaydi (crash)
```

Tekshiring:
```cpp
agar (b != 0) {
    butun c = a / b;
}
aks_holda {
    yozish << "0 ga bo'lib bo'lmaydi" << qator_oxiri;
}
```

### 5. `&&` va `&` ni adashtirish
```cpp
agar (a > 5 & b < 10); // bit AND — yomon, lekin ishlaydi
agar (a > 5 && b < 10); // mantiqiy AND — TO'G'RI
```

Mantiqiy operatorlar uchun har doim **ikki belgi** (`&&`, `||`).

---

## Bob bo'yicha mashqlar

### Mashq 1: Sodda kalkulyator (oson)

Foydalanuvchidan ikkita son va amal (`+`, `-`, `*`, `/`) so'rang.
Natijani chiqaring.

### Mashq 2: Juft yoki toq (oson)

Foydalanuvchidan son so'rang. `%` operatori orqali u juft yoki toq
ekanligini aniqlang.

### Mashq 3: Yil — kabisa yili (o'rta)

Kabisa yili — bu:
- 4 ga bo'linadi, lekin
- 100 ga bo'linmaydi yoki
- 400 ga bo'linadi

Foydalanuvchidan yil so'rang va kabisami yoki yo'qligini aniqlang.

**Maslahat:**
```cpp
mantiqiy kabisa = (yil % 4 == 0 && yil % 100 != 0) || (yil % 400 == 0);
```

### Mashq 4: Sonning raqamlari yig'indisi (o'rta)

3 xonali son uchun raqamlarini ajrating va yig'indisini hisoblang.
Masalan, 234 → 2 + 3 + 4 = 9.

**Maslahat:** `%` va `/` ishlatib ajratish mumkin.

### Mashq 5: Soat hisoblovchi (o'rtacha)

Sekundlar kiritilsa, ularni soat:daqiqa:sekund ko'rinishida chiqaring.
Masalan, 3725 sekund → 1:02:05.

**Maslahat:**
```cpp
butun soat = sekundlar / 3600;
butun daqiqa = (sekundlar % 3600) / 60;
butun sekund = sekundlar % 60;
```

### Mashq 6: Bayroqlar (qiyinroq)

3 ta bayroq belgilang: O'QISH, YOZISH, BAJARISH (har biri bitta bit).
Foydalanuvchining sifatlarini kiriting (`butun` qiymat sifatida) va
har bir sifat uchun "ha/yo'q" deb chiqaring.

### Mashq 7: Eng katta uchidan (qiyin)

Foydalanuvchidan 3 ta son so'rang. Eng kattasini toping. Faqat
`agar/aks_holda` va taqqoslash operatorlaridan foydalaning.

---

## Xulosa

Bu bobda biz:

- **Ifoda nimaligini** tushunib oldik — operandlar va operatorlardan
  iborat, qiymat beruvchi kod bo'lagi.
- **Arifmetik operatorlar** (`+`, `-`, `*`, `/`, `%`) bilan ishladik
  va **modulo** ning amaliy ishlatilishini ko'rdik.
- **Taqqoslash operatorlari** bilan tanishdik va `=` va `==` ni
  farqlashni o'rgandik.
- **Mantiqiy operatorlar** (`&&`, `||`, `!`) va ularning **qisqa
  baholash** xususiyatini bilib oldik.
- **Bit operatorlari** (`&`, `|`, `^`, `<<`, `>>`) bilan tanishdik va
  ularning bayroqlar ko'rinishidagi amaliy qo'llanishini ko'rdik.
- **Qiymat berish operatorlari** (`=`, `+=`, `-=`, va h.k.) ni
  o'rgandik.
- **Uchlik operator** (`?:`) va `sizeof` bilan tanishdik.
- **Operatorlar ustunligi** ni ko'rdik va qavslar muhimligini tushundik.
- **Amaliyotda** kvadrat tenglamani yechib chiqdik.

Keyingi bobda biz **boshqaruv tuzilmalari** ni o'rganamiz — `agar`,
`uchun`, `davr` kabi konstruksiyalar yordamida dasturning oqimini
boshqarishni o'rganasiz. Bularsiz dasturlar — bu shunchaki bir necha
qator ko'rsatmalar; ular bilan — bu mantiqiy mexanizmlar.
