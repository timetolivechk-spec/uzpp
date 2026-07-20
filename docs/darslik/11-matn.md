# 11 · Matn va satrlar bilan ishlash {#11-matn}

## Matn — eng ko'p ishlatiladigan tur

Dunyodagi har qanday dasturning katta qismi **matn bilan ishlaydi**.
Foydalanuvchidan ism so'rash, sahifaning HTML kodini tahlil qilish,
fayldagi matnni o'qish, ma'lumotlar bazasidan ism olish, JSON yuborish
— hammasi matn.

uz++ da matn `matn` (C++ da `std::string`) turi bilan ifodalanadi.
Bu juda kuchli tur — uzunligi cheksiz, ko'p amallar bilan kelgan, va
xotirani avtomatik boshqaradi.

```cpp
matn ism = "Aziza";
matn salom = "Salom, " + ism + "!";
yozish << salom; // "Salom, Aziza!"
```

Bu darsda biz matn bilan ishlashning eng kerakli usullarini ko'rib
chiqamiz.

---

## Asosiy matn amallari

### Bo'sh matn

```cpp
matn s = "";
matn t; // ham bo'sh
```

### Uzunlikni olish

```cpp
matn s = "Salom";
yozish << s.length(); // 5
yozish << s.size(); // 5 (length bilan bir xil)
```

### Belgilarga kirish

```cpp
matn s = "Salom";
yozish << s[0]; // 'S'
yozish << s[4]; // 'm'
yozish << s.at(2); // 'l' (xavfsizroq)
```

### Birlashtirish (concatenation)

```cpp
matn a = "Salom, ";
matn b = "dunyo!";
matn c = a + b; // "Salom, dunyo!"

c += "?"; // "Salom, dunyo!?"
```

### Solishtirish

```cpp
matn a = "alma";
matn b = "anor";

agar (a == b) { ... }                 // tenglik
agar (a < b)  { ... }                 // alfavit bo'yicha (a < b rost)
agar (a != b) { ... }                 // teng emas
```

### Ostigi matn (substring)

```cpp
matn s = "Salom, dunyo!";
matn s1 = s.substr(0, 5); // "Salom"
matn s2 = s.substr(7); // "dunyo!"
matn s3 = s.substr(7, 5); // "dunyo"
```

`substr(boshlanish, uzunlik)` — boshlanish indeksidan uzunlik belgi.

### Qidirish

```cpp
matn s = "Salom, dunyo!";
butun joy = s.find("dunyo"); // 7
butun yoq = s.find("yer"); // matn::npos (topilmadi)

agar (joy != matn::npos) {
    yozish << "Topildi indeks: " << joy;
}
```

### Almashtirish

```cpp
matn s = "Salom, dunyo!";
s.replace(7, 5, "olam");
// "Salom, olam!"
```

`replace(boshlanish, uzunlik, yangi_matn)`.

---

## Matnni boshqa turlarga aylantirish

### Son → matn

```cpp
butun yosh = 25;
matn s = std::to_string(yosh); // "25"

haqiqiy baho = 3.14;
matn t = std::to_string(baho); // "3.140000"
```

### Matn → son

```cpp
matn s = "42";
butun n = std::stoi(s); // 42

matn t = "3.14";
haqiqiy d = std::stod(t); // 3.14
```

`std::stoi` (string to int), `std::stod` (string to double).

**Diqqat!** Agar matn raqam emas — istisno tashlanadi:

```cpp
butun n = std::stoi("abc"); // ← XATO!
```

Xavfsiz versiya:

```cpp
urinish {
    butun n = std::stoi(s);
}
ushlash (o'zgarmas std::invalid_argument& e) {
    yozish << "Bu raqam emas";
}
```

---

## Matnni solishtirish

### Aniq taqqoslash

```cpp
matn a = "Salom";
matn b = "salom";

a == b; // yolg'on (S va s farqli)
```

### Katta-kichik harfsiz taqqoslash

```cpp
ulash <algorithm>
ulash <cctype>

bosh kichiklash(matn& s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](belgi c) { qaytarish std::tolower(c) });
}

matn a = "Salom", b = "SALOM";
kichiklash(a);
kichiklash(b);
agar (a == b) {
    yozish << "Bir xil";
}
```

---

## Formatli matn (C++20/23 `std::format`)

`std::format` — bu Python ning `f-string` ga o'xshash kuchli vosita.

```cpp
ulash <format>

matn ism = "Aziza";
butun yosh = 22;

matn s = std::format("Salom, {}! Yoshingiz {}", ism, yosh);
// "Salom, Aziza! Yoshingiz 22"
```

### Format spetsifikatorlari

```cpp
std::format("{:>10}", "abc"); // "       abc" (o'ng tomon)
std::format("{:<10}", "abc"); // "abc       " (chap)
std::format("{:^10}", "abc"); // "   abc    " (markaz)

std::format("{:.2f}", 3.14159); // "3.14" (2 ta kasr)
std::format("{:6.2f}", 3.14); // "  3.14"
std::format("{:08.3f}", 3.14); // "0003.140"

std::format("{:x}", 255); // "ff" (heks)
std::format("{:o}", 8); // "10" (oktal)
std::format("{:b}", 5); // "101" (binar)
```

### Real misol

```cpp
haqiqiy narx = 15999.50;
matn s = std::format("Mahsulot narxi: {:,.2f} so'm", narx);
// "Mahsulot narxi: 15,999.50 so'm"
```

`std::cout` ning `<<` operatori bilan taqqoslang — `std::format` ancha
kuchli va o'qish oson.

---

## Son va matn orasidagi konvertatsiya

### `std::stringstream`

Murakkab amallar uchun:

```cpp
ulash <sstream>

std::stringstream ss;
ss << "Yosh: " << 22 << ", baho: " << 4.7;
matn s = ss.str(); // "Yosh: 22, baho: 4.7"
```

### Matnlardan qiymat o'qish

```cpp
matn satr = "42 3.14 salom";
std::stringstream ss(satr);

butun n;
haqiqiy d;
matn s;

ss >> n >> d >> s;
// n = 42, d = 3.14, s = "salom"
```

---

## UTF-8 va o'zbek alifbosi

uz++ Unicode (UTF-8) ni qo'llab-quvvatlaydi. Lekin ehtiyot bo'ling:

```cpp
matn s = "O'zbekiston";
yozish << s.length(); // 12 (BAYT soni, harf emas!)
```

`length()` — bu **bayt** soni, harf soni emas. O'zbek harflari "O'",
"Sh" bir necha bayt egallaydi.

### Harflar bilan to'g'ri ishlash

Murakkab tahlillar uchun maxsus UTF-8 kutubxonalari ishlatiladi. Sodda
hollarda — `std::wstring` (keng matn):

```cpp
ulash <string>
std::wstring s = L"O'zbekiston"; // L — wide
```

---

## `std::string_view` — matn ko'rinishi

`std::string_view` — matnga **ko'rinish** beradi, nusxa olmaydi:

```cpp
bosh chiqarish(std::string_view sv) {
    yozish << sv;
}

chiqarish("Salom"); // nusxa olmaydi — tez!
chiqarish(matn("Salom, dunyo!")); // nusxa olmaydi
```

Funksiya parametrlari uchun — afzal usul (nusxa olmaslik).

---

## Amaliy misol: Matn tahlilchisi

Foydalanuvchi yozgan jumlada so'zlar, harflar, gaplarini sanaydigan
dastur:

```cpp
matn jumla = "Bugun ob-havo ajoyib. Quyosh charaqlab turadi. Salom!";

butun harflar = 0;
butun sozlar = 1;
butun gaplar = 0;

uchun (belgi c : jumla) {
    agar (std::isalpha(c)) {
        harflar++;
    }
    agar (c == ' ') {
        sozlar++;
    }
    agar (c == '.' || c == '!' || c == '?') {
        gaplar++;
    }
}

yozish << "Harflar: " << harflar << qator_oxiri;
yozish << "So'zlar: " << sozlar << qator_oxiri;
yozish << "Gaplar: " << gaplar << qator_oxiri;
```

**Natija:**

```
Harflar: 38
So'zlar: 8
Gaplar: 3
```

---

## Eng ko'p uchraydigan xatolar

### 1. Chegaradan tashqari

```cpp
matn s = "Salom";
yozish << s[10]; // ← XATO yoki kutilmagan natija
```

### 2. UTF-8 da indeks bilan ishlash

```cpp
matn s = "O'zbekiston";
yozish << s[1]; // ← XATO: harfning bir qismi
```

### 3. `std::stoi` xato

```cpp
butun n = std::stoi("abc"); // ← XATO: istisno
```

### 4. `nullptr` ni matnga aylantirish

```cpp
o'zgarmas belgi* p = nullptr;
matn s = p; // ← XATO: crash
```

### 5. `c_str()` ni saqlash

```cpp
o'zgarmas belgi* p = matn("Salom").c_str();
yozish << p; // ← XATO: matn yo'qolgan, p invalid
```

---

## Bob bo'yicha mashqlar

### Mashq 1: Palindrom (o'rta)

So'z palindrommi (orqadan o'qisangiz ham bir xil — "ada", "kanal").

### Mashq 2: So'z sanash (o'rta)

Jumlada so'zlar sonini sanang.

### Mashq 3: Bosh harf bilan boshlash (o'rta)

Har bir so'zning birinchi harfini bosh harfga aylantiring.

### Mashq 4: Caesar shifri (qiyinroq)

Matnni N pozitsiyaga siljitib shifrlang.

### Mashq 5: HTML belgilarni almashtirish (qiyinroq)

`<`, `>`, `&` — `&lt;`, `&gt;`, `&amp;` ga almashtiring.

---

## Xulosa

- **`matn` (std::string)** — asosiy matn turi.
- **Asosiy amallar** — `length`, `substr`, `find`, `replace`,
  `+`.
- **Konvertatsiya** — `std::to_string`, `std::stoi`, `std::stod`.
- **Formatlash** — `std::format` (C++20+) eng kuchli vosita.
- **UTF-8** — bayt va harf farqi muhim.
- **`string_view`** — nusxa olmaydigan ko'rinish.

Keyingi bobda biz **fayllar bilan ishlash** ni o'rganamiz —
ma'lumotlarni diskka saqlash va o'qish.
