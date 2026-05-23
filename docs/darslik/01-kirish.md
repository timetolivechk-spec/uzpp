# 01 · Kirish — Dasturlash dunyosiga xush kelibsiz {#01-kirish}

## Dasturlash nima?

Tasavvur qiling, siz oshpazsiz va do'stingizga osh tayyorlashni o'rgatmoqchisiz.
Siz unga "osh qil" desangiz, hech narsa kelib chiqmaydi. Lekin agar siz aniq
ko'rsatmalar bersangiz — "guruchni yuvib qo'y, sabzini chiziqcha qilib to'g'ra,
go'shtni qovur, suv qo'shib qaynat, guruchni sol, 40 daqiqa damla" —
do'stingiz ovqatni tayyorlay oladi.

**Dasturlash — bu aynan shunday narsa, faqat kompyuter uchun.** Kompyuter
nimanidir o'zicha qilolmaydi. Unga aniq, ketma-ket, hech qanday talqin
qilinmaydigan ko'rsatmalar berishingiz kerak. Bu ko'rsatmalar majmuasi —
_dastur_, ularni yozish jarayoni esa — _dasturlash_ deyiladi.

Bir muammo bor: kompyuter o'zbek tilini ham, ingliz tilini ham bevosita
tushunmaydi. U faqat ikkita raqamni biladi — 0 va 1. Bizning vazifamiz —
odam tushunadigan tilda yozilgan dasturni mashina tushunadigan ikkilik
(_binary_) kodiga aylantirish. Buni **dasturlash tillari** va
**kompilyatorlar** bajaradi.

> **Hayotiy misol:** Bir teatrda rejissor aktyorlarga gapirayotgan paytida
> "siz bu yerda yig'lab yuborasiz" deydi. Aktyor buni o'z miyasida shunday
> talqin qiladi: yuz mushaklarini bo'shashtirish, ko'z yoshlarini hosil
> qilish, ovozni titratish. Rejissor "neyron impulslari" yuborolmaydi —
> u **odam tilida** so'zlaydi. Dasturlash tili xuddi shu vositachi: siz
> niyatingizni gapirasiz, til esa uni mashinaga tushunarli signallarga
> aylantiradi.

### Past va yuqori darajadagi tillar

Dasturlash tillarini ikki katta guruhga bo'lish mumkin:

- **Past darajadagi tillar** (Assembler, machine code) — mashina
  arxitekturasiga juda yaqin. Har bir buyruq aniq bir elektron amal
  (registr yuklash, qiymatlar qo'shish, xotira manziliga yozish). Odam
  uchun yozish ham, o'qish ham juda qiyin. Bitta dastur — minglab
  satrlar.
- **Yuqori darajadagi tillar** (C, C++, Python, Java, **uz++**) —
  abstraktsiya beradi. Bitta qator kod 10-100 ta mashina buyrug'iga
  aylanishi mumkin. Odam uchun yozish va o'qish oson, lekin orqada
  murakkab tarjima jarayoni yashiringan.

Yuqori darajadagi tillar yana ikki guruhga bo'linadi:

- **Kompilyatsiya qilinadigan tillar** (C, C++, Rust, Go) — kod _oldindan
  to'liq_ mashina kodiga aylantiriladi, so'ng ishga tushiriladi. Natijada
  dastur juda tez.
- **Interpretatsiya qilinadigan tillar** (Python, JavaScript, Ruby) —
  kod _ishga tushirish vaqtida_ qator-qator o'qiladi va bajariladi.
  Yozish oson, lekin ishlash sekinroq.

**uz++** — kompilyatsiya qilinadigan tillar oilasidan. Lekin uning yo'li
biroz boshqacha: u oldin _transpilyatsiya_ qiladi. Bu nima ekanini
keyinroq batafsil ko'ramiz.

---

## uz++ — bu qanday til?

uz++ — bu o'zbek kalit so'zlari asosida ishlaydigan, **C++23 standarti**
ustida qurilgan zamonaviy dasturlash tili. U C++ ning butun quvvatini
saqlab qoladi, lekin sintaksisni o'zbek tiliga yaqinlashtiradi.

### Nima uchun aynan uz++ ni o'rganish kerak?

Dunyoda yuzlab dasturlash tillari bor. Nega aynan uz++? Sababi to'rt
asosiy:

#### 1. Ona tilingizda

Tibbiyot, huquq, fizika kabi sohalarda kitoblar o'zbek tiliga
tarjima qilinadi. Nima uchun dasturlash bunday qilinmasin? uz++ shu
savolga amaliy javob. Barcha kalit so'zlar — `agar`, `funksiya`,
`qaytarish`, `sinf`, `aks_holda` — o'zbek tilida. Sizning miyangiz
endi parallel ravishda ikkita ishni qilishi shart emas: "if degani
nima edi?" va "bu yerda mantiq nima?" — siz to'g'ridan-to'g'ri
mantiqni o'ylashingiz mumkin.

#### 2. Haqiqiy quvvat

uz++ — o'yinchoq emas. U C++23 ning barcha imkoniyatlarini taqdim
etadi:
- **Tezlik:** Python'dan 10-100 marta tezroq, JavaScript'dan 20 marta
  tezroq.
- **To'g'ridan-to'g'ri xotira boshqaruvi:** har bir baytni siz nazorat
  qilasiz.
- **Shablonlar (templates):** bitta funksiya istalgan tur bilan ishlay
  oladi.
- **C++23 yangiliklari:** `std::expected` (xatoliklarni qaytarish),
  `std::format` (formatlash), ranges (zamonaviy iteratsiya),
  coroutines (asinxron amallar).

uz++ da yozilgan dastur _xuddi shu C++ kodi kabi_ ishlaydi — chunki u
oxir-oqibat C++ ga aylantirilib, so'ng kompilyatsiya qilinadi.

#### 3. Sodda sintaksis

uz++ Python tilidan ilhomlangan: kod oson o'qiladi, blocklar
chekinish (indentation) orqali ko'rinadi, nuqtali vergullar ko'p
hollarda ixtiyoriy:

```cpp
butun asosiy() {
    o'zgaruvchan ism = "Aziz"
    yozish << "Salom, " << ism << qator_oxiri
    qaytarish 0
}
```

Solishtiring, bir xil dastur C++ da:

```cpp
#include <iostream>
#include <string>

int main() {
    std::string ism = "Aziz";
    std::cout << "Salom, " << ism << std::endl;
    return 0;
}
```

uz++ versiyasi qisqaroq, soddaroq va — eng muhimi — _o'zbekcha_.

#### 4. Zamonaviy asboblar

uz++ bilan birga keladi:
- **VS Code kengaytmasi** — kod yozayotganda real vaqtda xatolarni
  ko'rsatadi, avtoto'ldirish beradi, har bir o'zgaruvchining turini
  hover'da ko'rsatadi.
- **LSP server** — barcha zamonaviy IDE'lar bilan ishlaydi.
- **DAP server** — disk to'xtatish (debugging) breakpoint'lar bilan.
- **Formatter** — kodingizni avtomatik tartibga keltiradi.

> **Yangi boshlovchilar uchun:** Agar siz birinchi marta dasturlash
> bilan tanishayotgan bo'lsangiz, uz++ siz uchun mukammal til. Sintaksis
> sodda, lekin orqada _haqiqiy_ til turadi. Bu yerda o'rgangan
> ko'nikmalar to'g'ridan-to'g'ri C++ ga o'tadi — kelajakda agar
> "katta dunyo"ga chiqmoqchi bo'lsangiz, hech qanday qayta o'rganish
> kerak emas.

---

## uz++ ichida nima sodir bo'ladi?

Bu bo'lim ehtimol kitobning eng muhim bo'limi. Agar siz dasturlash
qanday ishlashini tushunsangiz, hamma narsa keyin ravshan bo'ladi.

### Transpilyator nima?

Oddiy kompilyator (masalan, g++) sizning kodingizni _to'g'ridan-to'g'ri_
mashina kodiga aylantiradi. Lekin uz++ avval bir narsa qiladi: u
sizning **`.uzpp`** faylingizni **C++23 kodiga** aylantiradi. Keyin
C++ kompilyatori (g++) bu kodni mashina kodiga aylantiradi.

Quyidagi diagrammaga e'tibor bering — bu sizning dasturingizning
yo'li:

```
   sizning_dasturingiz.uzpp           ← siz yozgan fayl
              │
              ▼
   ┌─────────────────────┐
   │  uzpp.exe           │            ← transpilyator
   │  (tarjima qiladi)   │
   └─────────────────────┘
              │
              ▼
   .tmp_xxx.cpp                       ← oraliq C++ fayli
              │
              ▼
   ┌─────────────────────┐
   │  g++                │            ← haqiqiy C++ kompilyator
   │  (kompilyatsiya)    │
   └─────────────────────┘
              │
              ▼
   sizning_dasturingiz.exe            ← yakuniy ikkilik fayl
              │
              ▼
   [ ishga tushiriladi ]              ← OS dasturni yuklaydi
```

Har bir bosqichni batafsil ko'rib chiqamiz.

### 1-bosqich: Leksik tahlil (Lexer)

Transpilyator avval sizning kodingizni o'qiydi va uni alohida
"so'zlar"ga ajratadi. Bu so'zlar **token** deb ataladi.

Misol uchun, agar siz quyidagi kodni yozsangiz:

```cpp
o'zgaruvchan x = 10
```

Lexer uni quyidagi tokenlarga ajratadi:

| Token | Turi |
|-------|------|
| `o'zgaruvchan` | kalit so'z (`KEYWORD`) |
| `x` | identifikator (`IDENT`) |
| `=` | operator (`ASSIGN`) |
| `10` | son literal (`NUMBER`) |

Bu — xuddi siz gapni alohida so'zlarga ajratganingizdek.

### 2-bosqich: Sintaksik tahlil (Parser)

Endi tokenlardan _ma'no_ olish kerak. Parser tokenlarni o'qiydi va
ulardan **AST** (Abstract Syntax Tree — abstrakt sintaksis daraxti)
quradi.

Yuqoridagi misol uchun AST shunday ko'rinadi:

```
        VariableDeclaration
       /        |          \
   Type     Identifier    Initializer
   (auto)      (x)            (10)
```

Bu daraxt aytadi: "bu yerda o'zgaruvchi e'lon qilinmoqda, nomi `x`,
boshlang'ich qiymat 10".

### 3-bosqich: Turini tekshirish (Type Checker)

Parser AST'ni qurganidan keyin, **type checker** uni tekshiradi. U
quyidagi savollarga javob beradi:

- O'zgaruvchining turi to'g'ri ishlatilganmi?
- Funksiya chaqirilganda to'g'ri turdagi argumentlar berilganmi?
- Sinflar va metodlarni to'g'ri qo'llanilganmi?

Misol uchun:

```cpp
butun yosh = "yigirma"   // ← xato! butun (int) ga matn berilmoqda
```

Type checker buni ushlaydi va sizga xato xabarini chiqaradi —
_dastur ishga tushirilmasidan oldin_. Bu qadar foydali xususiyat:
ko'p xatolarni siz tezda ushlay olasiz.

### 4-bosqich: Kod generatsiyasi (CodeGen)

Endi tekshirilgan AST C++23 kodiga aylantiriladi. Bu erda
"sehr" sodir bo'ladi:

uz++:
```cpp
o'zgaruvchan x = 10
yozish << x << qator_oxiri
```

Generated C++ (sizga ko'rinmaydi, lekin shunday yaratiladi):
```cpp
auto x = 10;
std::cout << x << std::endl;
```

E'tibor bering — har bir uz++ tushunchasi tegishli C++ tushunchasiga
aylangan: `o'zgaruvchan` → `auto`, `yozish` → `std::cout`, `qator_oxiri`
→ `std::endl`.

### 5-bosqich: Kompilyatsiya (g++)

Hosil bo'lgan C++ kodi g++ kompilyatoriga uzatiladi. g++ uni mashina
kodiga aylantiradi va sizga `.exe` (Linux'da kengaytmasiz) fayl beradi.

### 6-bosqich: Bajarish (OS)

Siz `.exe` faylni ishga tushirsangiz, operatsion tizim uni xotiraga
yuklaydi va protsessor unga buyruqlarni yetkazib bera boshlaydi.
Dastur ishlamoqda!

> **Ishonsangiz:** Bu 6 bosqichli jarayon millisoniyada o'tadi. Siz
> "uzpp ishga-tushirish dastur.uzpp" deganingizdan keyin 1-2 soniya
> ichida natija ko'rinadi. Sahnaning orqasida bo'layotgan barcha
> ishlar — sizning ko'zingiz uchun ko'rinmas.

---

## O'rnatish

Dasturlashga boshlash uchun ikkita narsa kerak:
1. **uz++ transpilyatori** (`uzpp.exe`)
2. **C++ kompilyatori** (g++, MinGW yoki Clang)

Yaxshi xabar: Windows'da har ikkalasini bir o'rnatuvchi orqali olishingiz
mumkin.

### Windows

1. **uz++ o'rnatuvchisini yuklab oling:**
   [GitHub Releases](https://github.com/timetolivechk-spec/uzpp/releases)
   sahifasidan eng so'nggi `uzpp-setup.exe` faylini olib oling.

2. **O'rnatuvchini ishga tushiring.** U avtomatik ravishda quyidagilarni
   o'rnatadi:
   - `uzpp.exe` — transpilyatorning o'zi
   - **MinGW-w64** — Windows uchun g++ kompilyatori
   - **VS Code kengaytmasi** (agar VS Code o'rnatilgan bo'lsa)

3. **Tekshiring.** Terminalni qayta oching va quyidagi buyruqni
   kiriting:

   ```bash
   uzpp --yordam
   ```

   Agar yordam matni chiqsa — hammasi joyida! Agar "uzpp: command not
   found" deb yozilsa, terminalni yopib qayta oching (yo'l o'zgaradi).

### Linux / macOS

Terminalda quyidagi buyruqni ishga tushiring:

```bash
curl -fsSL https://raw.githubusercontent.com/timetolivechk-spec/uzpp/main/install.sh | bash
```

Bu skript g++ ning o'rnatilganligini tekshiradi (agar yo'q bo'lsa,
sistema paket menejeri orqali o'rnatadi) va uz++ ni yuklab oladi.

### VS Code kengaytmasi

Agar siz VS Code'dan foydalansangiz (tavsiya qilamiz), Marketplace'dan
**`uzpp.uzpp`** kengaytmasini o'rnating. Kengaytma sizga quyidagilarni
beradi:

- **Sintaksis yoritilishi:** kalit so'zlar rangli ko'rinadi
- **Avtoto'ldirish (IntelliSense):** sizning kodingizni tushunadi
- **Real vaqtli xatoliklar:** yozayotganingizda xatolarni darhol
  ko'rsatadi
- **F5 orqali ishga tushirish:** kodni bir tugmacha bilan yuritish

> **Birinchi marta o'rnatish bilan muammo bormi?** Eng ko'p
> uchraydigan muammolar:
> - **PATH'ga qo'shilmadi:** terminalni yopib qayta oching, yoki
>   kompyuterni qayta yuklang.
> - **MinGW topilmadi:** o'rnatuvchini administrator nomidan ishga
>   tushiring.
> - **VS Code kengaytmasi ko'rinmaydi:** VS Code'ni qayta ishga
>   tushiring va Extensions panelida "uzpp" deb qidiring.

---

## Birinchi dastur: "Salom, dunyo!"

Har qanday dasturlash tilini o'rganish an'anaviy ravishda **"Salom,
dunyo!"** dasturi bilan boshlanadi. Bu — 1972 yilda Brian Kernighan
tomonidan C tilining birinchi darsligida ishlatilgan dastur. O'sha
paytdan beri har bir dasturchi shu dastur bilan boshlaydi — bu kasbiy
an'ana.

### Kod

Quyidagi kodni `salom.uzpp` nomli faylga saqlang:

```cpp
// salom.uzpp — birinchi uz++ dasturi

butun asosiy() {
    yozish << "Salom, dunyo!" << qator_oxiri
    qaytarish 0
}
```

### Ishga tushirish

Terminalni faylingiz joylashgan papkada oching va kiriting:

```bash
uzpp ishga-tushirish salom.uzpp
```

Yoki agar VS Code'da bo'lsangiz, oddiy `F5` tugmasini bosing.

**Natija:**

```
Salom, dunyo!
```

Tabriklayman! Siz hozirgina birinchi uz++ dasturingizni yozdingiz va
ishga tushirdingiz.

### Endi nimani ko'rdik — qator-qator tahlil

Hozir dasturimizning har bir qatorini batafsil ko'rib chiqamiz. Agar
ba'zi tushunchalar sizga noma'lum bo'lsa — xavotir olmang, biz ularning
har biriga keyingi boblar bag'ishlaymiz.

#### 1-qator: Izoh

```cpp
// salom.uzpp — birinchi uz++ dasturi
```

`//` belgisi izoh (kommentariy) boshlanadi. Izohlar dastur uchun
ahamiyatsiz — kompyuter ularni o'qimaydi. Lekin **odamlar** uchun
muhim: izohlar sizga keyingi safar kelganingizda kodingiz nima
qilayotganini eslatadi.

> **Yaxshi tavsiya:** Izohlar — bu o'tmishdagi siz kelajakdagi sizga
> qoldirgan xat. Yarim yildan keyin o'z kodingizga qaytsangiz, "Bu
> nima ekan?" demaslik uchun, izohlar yozing.

#### 2-qator: Funksiya e'loni

```cpp
butun asosiy() {
```

Bu qator juda muhim. Uni qismlarga ajrataylik:

| Qism | Ma'nosi |
|------|---------|
| `butun` | Funksiya qaytaradigan qiymat turi — _butun son_ |
| `asosiy` | Funksiya nomi — `asosiy` (bu maxsus nom!) |
| `()` | Funksiya parametrlari — bu yerda parametr yo'q (bo'sh qavslar) |
| `{` | Funksiya tanasining boshlanishi |

**`asosiy` — bu maxsus nom.** Har bir uz++ dasturida bitta `asosiy`
funksiya bo'lishi shart. Bu — _dasturning kirish nuqtasi_. Operatsion
tizim sizning dasturingizni ishga tushirganida, u _aynan shu_ funksiyani
chaqiradi.

> **Real hayotdagi misol:** Tasavvur qiling, sizning uyingizda ko'p
> xona bor — yotoqxona, oshxona, hammom. Lekin sizga mehmon kelganda,
> u har doim **eshik orqali kiradi**, to'g'ridan-to'g'ri yotoqxonaga
> emas. `asosiy` funksiyasi — bu sizning dasturingizning eshigi.

#### 3-qator: Konsolga yozish

```cpp
    yozish << "Salom, dunyo!" << qator_oxiri
```

E'tibor bering — qator boshida 4 ta bo'sh joy bor. Bu **chekinish**
(_indentation_). uz++ da chekinish bloklarni ko'rsatadi: bu qator
`asosiy` funksiyaning _ichida_ ekanligini aytadi.

Endi qatorning o'zini tahlil qilaylik:

| Qism | Ma'nosi |
|------|---------|
| `yozish` | Konsolga (terminalga) yozish vositasi |
| `<<` | "Bu ma'lumotni `yozish`ga yubor" operatori |
| `"Salom, dunyo!"` | Matn satri (_string_) |
| `<<` | Yana yuborish operatori |
| `qator_oxiri` | Yangi qator belgisi (_newline_) |

Bu qator shuni aytadi: "konsolga `Salom, dunyo!` matnini chiqaring,
keyin yangi qatorga o'ting".

`<<` operatorini "kanal" deb tasavvur qiling: ma'lumotlar oqim bo'lib
bir tomonga oqadi.

```
"Salom, dunyo!"  ──┐
                   ├──→  [yozish (konsol)]
qator_oxiri      ──┘
```

#### 4-qator: Qiymat qaytarish

```cpp
    qaytarish 0
```

`qaytarish 0` — funksiyadan **0** raqamini qaytarish degani.

Nima uchun 0? Chunki **`asosiy`** funksiyaning qaytarish qiymati —
bu dasturning _tugash holati_. Konventsiya bo'yicha:
- **0** — "hammasi yaxshi tugadi" degan ma'noda
- **0 dan boshqa raqam** — "xatolik yuz berdi" degan ma'noda

Operatsion tizim bu qiymatni qabul qiladi va u orqali sizning
dasturingizning natijasi haqida xulosa qiladi.

> **Real hayotdagi misol:** Siz pochta xizmatiga buyurtma berdingiz.
> Buyurtma yetkazilganidan keyin sizdan ovoz beriladi: "qaytib keldim,
> yetkazib berdim" (0), yoki "muvaffaqiyatsizlik bo'ldi, qaytib
> kelmoqdaman" (1, 2, va h.k.). `qaytarish 0` shu birinchi holat.

#### 5-qator: Funksiya tanasining yopilishi

```cpp
}
```

`{` ochilgan edi, `}` uni yopadi. `asosiy` funksiya tugadi.

### Endi, butun rasm

Qachon biz `uzpp ishga-tushirish salom.uzpp` desak:

1. **Transpilyator** sizning kodingizni o'qiydi va C++ ga aylantiradi:
   ```cpp
   #include "uzpp_runtime.hpp"
   int main() {
       std::cout << "Salom, dunyo!" << std::endl;
       return 0;
   }
   ```

2. **g++** bu C++ kodini mashina kodiga aylantiradi va `salom.exe`
   yaratadi.

3. **Operatsion tizim** `salom.exe` ni ishga tushiradi.

4. Dastur `main` (yoki uz++ da `asosiy`) funksiyasidan boshlanadi.

5. `std::cout` orqali "Salom, dunyo!" konsolga chiqariladi.

6. `return 0` orqali dastur tugaydi va OS ga "men muvaffaqiyatli
   tugadim" deydi.

Hech qachon bu kabi oddiy 4 qator kod orqasida bunchalik ko'p ish
borligini o'ylab ko'rganmidingiz?

---

## Izohlar — kelajakdagi o'zingizga xat

Izohlar — dastur uchun ko'rinmas, lekin _odamlar_ uchun ko'rinadigan
matn. uz++ da izohlarning ikki turi bor:

### Bir qatorli izoh

```cpp
// bu — bir qatorli izoh
butun yosh = 25   // o'zgaruvchidan keyin ham yozish mumkin
```

`//` belgisidan keyin satr oxirigacha hamma narsa — izoh.

### Ko'p qatorli izoh

```cpp
/*
   Bu — ko'p qatorli izoh.
   U bir necha qatorni egallashi mumkin.
   Foydali bo'limni tushuntirish uchun kerak bo'lganda.
*/
```

`/*` bilan boshlanib `*/` bilan tugaydi.

### Yaxshi izoh va yomon izoh

Yomon izoh — kodda yozilgan narsani takrorlaydigan izoh:

```cpp
butun yosh = 25   // yoshni 25 ga teng qil   ← bu izoh keraksiz!
```

Yaxshi izoh — kod _nima qilayotgani_ emas, **nega** shunday qilayotganini
tushuntiradi:

```cpp
// Yoshi 18 dan kichik foydalanuvchilarga ovoz berish ruxsat etilmaydi.
agar (yosh < 18) {
    yozish << "Kechirasiz, ovoz bera olmaysiz" << qator_oxiri
}
```

> **Mashhur dasturchi Brian Kernighan'ning maslahati:** "Eng yaxshi
> izoh — bu kerakmas izoh. Agar kodingiz tushunarli bo'lsa, izoh
> kerak emas. Agar tushunarli emas bo'lsa — kodni qayta yozing,
> izoh qo'shmang."

---

## Qatorlar va ko'rsatmalar

uz++ ning bir o'ziga xosligi — **nuqtali vergullar ixtiyoriy**. C++ da
har bir ko'rsatma `;` bilan tugashi shart, lekin uz++ siz ularsiz
yozishingiz mumkin:

```cpp
butun a = 5
butun b = 10
butun c = a + b
yozish << c << qator_oxiri
```

Lekin agar xohlasangiz, vergullarni qo'yishingiz mumkin — xato bo'lmaydi:

```cpp
butun a = 5;
butun b = 10;
butun c = a + b;
yozish << c << qator_oxiri;
```

**Tavsiya:** Boshlanishida vergullarsiz yozing — kod tozaroq ko'rinadi.

### Bir qator ichida bir nechta ko'rsatma

Agar siz bir qatorda bir nechta ko'rsatma yozmoqchi bo'lsangiz,
ularni `;` bilan ajrating:

```cpp
butun x = 5; butun y = 10; butun z = x + y
```

Lekin bu uslubdan qochish kerak — kod o'qish qiyin bo'ladi.

### Bloklar — chekinish orqali

uz++ da kod bloklari **chekinish** (indentation) orqali ifodalanadi:

```cpp
butun asosiy() {
    agar (rost) {
        yozish << "ichkari blok" << qator_oxiri
    }
    yozish << "tashqari blok" << qator_oxiri
}
```

E'tibor bering: ichkari `yozish` 8 ta bo'sh joy bilan chekinmoqda
(`agar` ichida + `asosiy` ichida). Tashqarisi — 4 ta bo'sh joy.

Tavsiya: har bir daraja uchun **4 ta bo'sh joy** ishlating. Tab
tugmasidan foydalanmang — turli muharrirlarda u turli kenglikda
ko'rinishi mumkin.

---

## Birinchi interaktiv dastur

Endi biz dasturni biroz "tirik" qilaylik — foydalanuvchidan ma'lumot
so'raylik va unga javob beraylik.

### Kod

```cpp
// muloqot.uzpp — foydalanuvchi bilan dastlabki muloqot

butun asosiy() {
    matn ism
    butun yosh

    yozish << "Ismingiz nima? "
    kiritish >> ism

    yozish << "Yoshingiz necha? "
    kiritish >> yosh

    yozish << qator_oxiri
    yozish << "Salom, " << ism << "!" << qator_oxiri
    yozish << "Siz " << yosh << " yoshdasiz." << qator_oxiri

    agar (yosh < 18) {
        yozish << "Siz hali yoshsiz — kelajak sizniki!" << qator_oxiri
    }
    aks_holda {
        yozish << "Tajribangizdan foydalanish vaqti keldi." << qator_oxiri
    }

    qaytarish 0
}
```

### Ishga tushirish

```bash
uzpp ishga-tushirish muloqot.uzpp
```

**Natija (misol):**

```
Ismingiz nima? Aziza
Yoshingiz necha? 22

Salom, Aziza!
Siz 22 yoshdasiz.
Tajribangizdan foydalanish vaqti keldi.
```

### Yangi tushunchalar

Bu dasturda biz bir necha yangi narsalarni ko'rdik:

#### 1. O'zgaruvchi e'loni

```cpp
matn ism
butun yosh
```

**O'zgaruvchi** — bu xotirada saqlash uchun nomlangan joy. Bu yerda:
- `ism` — `matn` turidagi (string) o'zgaruvchi
- `yosh` — `butun` turidagi (int) o'zgaruvchi

Tasavvur qiling, sizda ikkita quti bor: birida "ism" yorlig'i yopishtirilgan
(faqat matn saqlash uchun), ikkinchisida "yosh" yorlig'i (faqat butun
sonlar uchun).

#### 2. Kiritish (Input)

```cpp
kiritish >> ism
```

`kiritish` — bu klaviaturadan ma'lumot olish vositasi. `>>` operatori
"klaviaturadan o'qib `ism` o'zgaruvchisiga sol" degan ma'nodi.

E'tibor bering: chiqarish uchun `<<`, kiritish uchun `>>`. Strelkalarning
yo'nalishi yordam beradi:
- `<<` — "bu yerga oqsin" (chiqarish)
- `>>` — "u yerdan keladi" (kiritish)

#### 3. Shart operator

```cpp
agar (yosh < 18) {
    yozish << "Siz hali yoshsiz — kelajak sizniki!" << qator_oxiri
}
aks_holda {
    yozish << "Tajribangizdan foydalanish vaqti keldi." << qator_oxiri
}
```

`agar` (if) shart bajarilsa, birinchi blok ishlaydi. `aks_holda` (else)
— aks holda, ikkinchi blok ishlaydi.

`<` belgisi — "kichik" (less than) ma'nosida. Boshqa taqqoslash
operatorlari:

| Operator | Ma'nosi |
|----------|---------|
| `<` | kichik |
| `>` | katta |
| `<=` | kichik yoki teng |
| `>=` | katta yoki teng |
| `==` | teng (e'tibor bering, **ikkita** belgi!) |
| `!=` | teng emas |

> **Eng ko'p uchraydigan xatolik:** `=` va `==` ni adashtirib qo'yish.
> `=` — qiymat berish (`a = 5` — `a` ga 5 ni saqla). `==` — taqqoslash
> (`a == 5` — `a` 5 ga tengmi?). Bu xato birinchi haftalarda ko'p
> sodir bo'ladi.

---

## Tilning asosiy elementlari

Kelinglar, hozir uz++ ning eng asosiy 3 ta narsasini umumiy ko'rib
chiqaylik. Keyingi boblarda har birini batafsil o'rganamiz.

### Kalit so'zlar (Keywords)

Quyida eng muhim kalit so'zlar — siz ularni keyingi boblarda barchasini
o'rganasiz:

| uz++ | C++ ekvivalenti | Ma'nosi |
|------|-----------------|---------|
| `agar` | `if` | Agar shart bajarilsa |
| `aks_holda` | `else` | Aks holda |
| `uchun` | `for` | Takrorlash sikli |
| `davr` | `while` | Shartli takrorlash |
| `tanlash` | `switch` | Tanlash operatori |
| `sinf` | `class` | Klass e'lon qilish |
| `tuzilma` | `struct` | Tuzilma e'lon qilish |
| `qaytarish` | `return` | Qiymat qaytarish |
| `rost` | `true` | Mantiqiy rost qiymat |
| `yolg'on` | `false` | Mantiqiy yolg'on qiymat |
| `to'xtatish` | `break` | Sikldan chiqish |
| `davom_ettirish` | `continue` | Keyingi takrorlashga o'tish |

> **Eslatma:** Kalit so'zlar — bu **bron qilingan** so'zlar. Siz ularni
> o'zgaruvchi nomi sifatida ishlatolmaysiz. Masalan, `butun agar = 5`
> deb yozolmasiz — chunki `agar` allaqachon kalit so'z.

### Ma'lumot turlari

Kompyuter har xil turdagi ma'lumotlarni boshqacha saqlaydi. Asosiy
turlar:

| Tur | C++ ekvivalenti | Ma'nosi | Misol |
|-----|-----------------|---------|-------|
| `butun` | `int` | Butun son (32 bit) | `5`, `-100`, `42` |
| `kasr` | `float` | Haqiqiy son (32 bit) | `3.14`, `0.5` |
| `ikkilangan` | `double` | Aniq haqiqiy son (64 bit) | `3.14159265` |
| `mantiq` | `bool` | Rost/yolg'on | `rost`, `yolg'on` |
| `belgi` | `char` | Bitta belgi | `'A'`, `'b'`, `'!'` |
| `matn` | `std::string` | Matn satri | `"Salom"`, `"Aziza"` |
| `bosh` | `void` | Bo'sh (qiymatsiz) | — |

> **Nima uchun turlar kerak?** Kompyuter `butun` va `kasr`ni boshqa
> joyda saqlaydi. `42` — bu 4 baytlik xotirada saqlanadi (`int`).
> `3.14` — bu 4 baytlik, lekin _boshqacha tashkil etilgan_ xotirada
> (`float`). Agar siz turini noto'g'ri ko'rsatsangiz, kompyuter
> ma'lumotni noto'g'ri talqin qiladi va dasturingiz xato beradi.

### Operatorlar

Operatorlar — bu qiymatlar ustida amallar bajaradigan belgilar:

```cpp
// Arifmetik amallar
butun yigindi = 5 + 3      // 8
butun farq = 10 - 4         // 6
butun kopaytma = 6 * 7      // 42
butun bolinma = 15 / 3      // 5
butun qoldiq = 17 % 5       // 2 (17 / 5 dan qolgani)

// Taqqoslash — natija mantiq turida (rost/yolg'on)
mantiq teng = (5 == 5)      // rost
mantiq kichik = (3 < 7)     // rost
mantiq farqli = (5 != 5)    // yolg'on

// Mantiqiy amallar
mantiq va = rost && yolg'on    // yolg'on (ikkalasi ham rost bo'lishi kerak)
mantiq yo = rost || yolg'on    // rost (kamida bittasi rost)
mantiq inkor = !rost            // yolg'on
```

Operatorlarni keyingi bobda batafsil ko'ramiz.

---

## Eng ko'p uchraydigan birinchi xatolar

Yangi boshlovchilar tez-tez bir xil xatolarga duch keladi. Mana ulardan
eng ko'p tarqalganlari:

### 1. Qo'shtirnoqlarni unutish

```cpp
yozish << Salom, dunyo! << qator_oxiri      // ← XATO
yozish << "Salom, dunyo!" << qator_oxiri    // ← TO'G'RI
```

Matn satrlari har doim qo'shtirnoq (`"..."`) ichida bo'lishi kerak.

### 2. Operatorni noto'g'ri yo'naltirish

```cpp
yozish >> "Salom"     // ← XATO (yozish chiqarish uchun, >> kiritish uchun)
yozish << "Salom"     // ← TO'G'RI
```

### 3. Funksiya tanasini yopishni unutish

```cpp
butun asosiy() {
    yozish << "Salom" << qator_oxiri
    qaytarish 0
                       // ← XATO: } yo'q!
```

Har bir `{` uchun mos `}` bo'lishi kerak.

### 4. Tip mosligini buzish

```cpp
butun yosh = "yigirma"     // ← XATO: matn `butun` ga sig'maydi
butun yosh = 20            // ← TO'G'RI
```

### 5. `=` va `==` ni adashtirish

```cpp
agar (yosh = 18) { ... }       // ← XATO: `yosh` ga 18 ni saqlamoqda!
agar (yosh == 18) { ... }      // ← TO'G'RI: 18 ga tengmi?
```

Bu juda nozik xato. Birinchi qator dasturingizda buzilmaydi, lekin u
har doim "rost" deb hisoblanadi. Buni topish qiyin bo'ladi!

> **Yaxshi xabar:** uz++ ning type checker'i bu xatolarning ko'pini
> kod yozayotgan paytda darhol ushlaydi. VS Code'da xato joylari qizil
> chiziq bilan ostiga chiziladi. Foydalaning!

---

## Asboblar — dasturchi sherigi

Yaxshi dasturchi — bu nafaqat tilni biluvchi odam, balki o'z asboblarini
yaxshi bilgan odam. Quyidagi 4 ta narsa dasturchining asosiy
"asboblarini" tashkil qiladi:

### 1. Matn muharriri (Editor)

Kod yozish uchun. Tavsiya qilamiz: **VS Code** — bepul, kuchli, va
uz++ kengaytmasi mavjud.

### 2. Terminal

Buyruqlarni ishga tushirish uchun. Windows'da: PowerShell yoki Windows
Terminal. Linux/macOS'da: tabiiy terminal.

### 3. Kompilyator / Transpilyator

Sizning kodingizni mashina kodiga aylantiradi. Bizning holatda — `uzpp`
va u o'z navbatida `g++` ni chaqiradi.

### 4. Debugger

Dasturingiz qanday ishlayotganini ko'rish uchun. uz++ DAP server orqali
breakpoint'larni qo'llab-quvvatlaydi.

### Rivojlanish sikli

Dasturchining odatdagi ish kuni shunday ko'rinadi:

```
   1. Kod yozish (muharrirda)
        │
        ▼
   2. Ishga tushirish (terminalda)
        │
        ▼
   3. Natija ko'rish
        │
        ▼
   4. Xato bormi?
       ├── Ha → 1 ga qayt (kodni tuzating)
       └── Yo'q → keyingi vazifaga o't
```

Bu **REPL** sikli (Read-Eval-Print Loop) — har bir dasturchining
kundalik mehnati. U qancha tezroq aylanib o'tsa, siz shuncha samaraliroq
ishlaysiz.

---

## Bob bo'yicha mashqlar

Quyidagi mashqlarni o'zingiz hal qiling. Har birining yechimi sizning
keyingi kuningizgacha bo'lgan asosiy ko'nikmangizdir.

### Mashq 1: Salomlashish (oson)

"Salom, dunyo!" dasturini yozing va ishga tushiring. Keyin matnni
o'zgartiring — masalan, "Salom, do'stlarim!" deb yozing.

**Maslahat:** Hech qanday maslahat kerak emas — bu darsda ko'rgan
dastur.

### Mashq 2: Ism va familiya (oson)

Foydalanuvchidan **ikkita** narsani so'rang — ism va familiya. So'ng
"Salom, [ism] [familiya]!" deb javob bering.

**Maslahat:** Sizga ikkita `matn` o'zgaruvchi kerak bo'ladi va ikkita
`kiritish` qatori.

### Mashq 3: Sodda kalkulyator (o'rtacha)

Foydalanuvchidan ikkita butun son so'rang. Keyin ularning yig'indisi,
ayirmasi, ko'paytmasi va bo'linmasini chiqaring.

**Maslahat:**
```cpp
butun a, b
kiritish >> a >> b      // ikkita sonni ketma-ket o'qish mumkin
```

**Hushyor bo'ling!** Agar `b` 0 bo'lsa, `a / b` bo'lganda dastur xato
beradi. Buni keyingi boblarda ko'ramiz.

### Mashq 4: Tug'ilgan yili (o'rtacha)

Foydalanuvchidan tug'ilgan yilini so'rang. Joriy yil deb 2026 ni oling.
Foydalanuvchining yoshini hisoblang va chiqaring.

**Maslahat:** Yosh = joriy_yil - tug'ilgan_yil.

### Mashq 5: O'rtacha baho (o'rtacha)

Foydalanuvchidan 3 ta fan bahosini so'rang (har biri 0 dan 100 gacha).
O'rtacha bahoni hisoblab chiqaring.

**Maslahat:** O'rtacha = (a + b + c) / 3. Lekin natija `ikkilangan`
turida bo'lishi kerak — agar siz `butun` ishlatsangiz, kasr qismi
yo'qoladi! `o'zgaruvchan ortacha = (a + b + c) / 3.0` deb yozsangiz,
3.0 dueto avtomatik kasrli hisoblanadi.

### Mashq 6: Quyon va toshbaqa (qiyinroq)

Quyon soatiga 50 km, toshbaqa soatiga 5 km yuradi. Foydalanuvchidan
ikki hayvon orasidagi masofani (km) so'rang. Toshbaqa boshlang'ich
masofani oladi (favorit). Necha soatdan keyin quyon toshbaqani quvib
yetadi?

**Maslahat:** Tenglama: `50 * t = 5 * t + masofa`. Yechimi: `t = masofa
/ 45`.

### Mashq 7: O'z dasturingiz (cheksiz)

Endi sizning o'z dasturingiz — biror narsa o'ylab toping va yozing.
Bu turing hamfikrlarga ko'rsating!

---

## Xulosa — biz bu bobda nimani o'rgandik

Bu bobda biz quyidagi narsalarni ko'rdik:

- **Dasturlash nima** — kompyuterga aniq ko'rsatmalar yozish jarayoni.
- **Tillar ierarxiyasi** — past darajadagi va yuqori darajadagi tillar,
  kompilyatsiya va interpretatsiya farqi.
- **uz++ ning falsafasi** — ona tilida dasturlash, lekin C++23 ning
  to'liq quvvati bilan.
- **Transpilyatsiya jarayoni** — kodingizdan binary fayligacha bo'lgan
  6 bosqichli yo'l.
- **Birinchi dastur** — "Salom, dunyo!" ni yozdik va har bir qatorini
  tushuntirdik.
- **Asosiy sintaksis** — izohlar, chekinish, bloklar, ko'rsatmalar.
- **Birinchi interaktiv dastur** — foydalanuvchi bilan muloqot
  qilishni o'rgandik.
- **Asosiy tushunchalar** — kalit so'zlar, ma'lumot turlari,
  operatorlar.
- **Eng ko'p uchraydigan xatolar** — qo'shtirnoqlar, operatorlar,
  tiplar mosligi.
- **Asboblar** — muharrir, terminal, kompilyator, debugger.

Keyingi bobda biz **o'zgaruvchilar va ma'lumot turlari** haqida
batafsil gaplashamiz. Sizning xotirangizda nima va qanday saqlanishini,
har bir tur qancha joy egallashini, va kompyuter ularni qanday
ishlatishini o'rganasiz.

> **Eslatma — dasturlash yo'lida birinchi qadam:** Eng muhimi —
> _yozib boring_. Kitobni o'qish kifoya emas. Har bir mashqni o'zingiz
> qo'lingiz bilan terib chiqing, ishga tushiring, xatolarni tuzating.
> Dasturlash — bu fizika emas, falsafa emas. Bu — _amaliy mahorat_.
> Faqat amaliyot orqali siz dasturchi bo'lasiz.
