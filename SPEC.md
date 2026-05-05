# uz++ Til Spetsifikatsiyasi / Language Specification

**Versiya:** 0.1  
**Sana:** 2026-05-04

---

## 1. Umumiy ko'rinish

uz++ — O'zbek tilida yoziladigan, C++23 ga transpilatsiya qilinadigan statik tipli dasturlash tili. Transpilatsiya pipeline quyidagicha:

```
.uzpp fayl → Lexer → Parser (AST) → TypeChecker → CodeGen → .cpp → g++/clang++ → binary
```

---

## 2. Leksik tuzilma (Lexical Structure)

### 2.1 Tokenlar

| Token turi          | Tavsif                              | Misollar                        |
|---------------------|-------------------------------------|---------------------------------|
| `Identifier`        | Identifikator                       | `son`, `asosiy`, `Shahs`        |
| `Integer`           | Butun son literali                  | `42`, `0`, `-7`                 |
| `Float`             | Kasr son literali                   | `3.14`, `0.0`, `2.718`          |
| `String`            | Matn literali (qo'sh tirnoq)        | `"Salom"`, `"uz++"`             |
| `Keyword`           | Til kalit so'zlari                  | `agar`, `uchun`, `qaytarish`    |
| `Symbol`            | Belgilar va operatorlar             | `+`, `-`, `{`, `}`, `::`        |
| `Comment`           | Izohlar (transpilatsiya yo'q)       | `// Bu izoh`                    |

### 2.2 Izohlar (Comments)

Faqat bir qatorli izohlar qo'llab-quvvatlanadi:

```uzpp
// Bu bir qatorli izoh
butun son = 42; // Qatorda izoh
```

### 2.3 String literallari

Standard C/C++ string escape sequence'lari qo'llab-quvvatlanadi:

```uzpp
matn s1 = "Salom, Dunyo!";
matn s2 = "Yangi qator:\n tab:\t";
matn s3 = "Qo'shtirnoq: \"ichida\"";
```

---

## 3. Tip tizimi (Type System)

### 3.1 Primitiv tiplar

| uz++ kalit so'z | C++ tipi        | Tavsif                    |
|-----------------|-----------------|---------------------------|
| `butun`         | `int`           | 32-bit butun son          |
| `uzun`          | `long long`     | 64-bit butun son          |
| `haqiqiy`       | `double`        | 64-bit kasr son           |
| `kasr`          | `float`         | 32-bit kasr son           |
| `matn`          | `std::string`   | Unicode matn              |
| `mantiqiy`      | `bool`          | Mantiqiy qiymat           |
| `belgi`         | `char`          | Bitta belgi               |
| `bosh`          | `void`          | Hech narsa                |

### 3.2 Kolleksiya tiplari

| uz++                          | C++                              | Tavsif           |
|-------------------------------|----------------------------------|------------------|
| `vektor<T>`                   | `std::vector<T>`                 | Dinamik massiv   |
| `lug'at<K, V>`                | `std::unordered_map<K, V>`       | Hash map         |
| `to'plam<T>`                  | `std::vector<T>`                 | To'plam          |
| `juftlik<A, B>`               | `std::pair<A, B>`                | Juftlik          |
| `ixtiyoriy<T>`                | `std::optional<T>`               | Ixtiyoriy qiymat |

### 3.3 Avtomatik tip aniqlash

```uzpp
ozgaruvchan son    = 42;         // auto → int
ozgaruvchan kasr   = 3.14;       // auto → double
ozgaruvchan matn1  = "salom";   // auto → string
ozgarmas PI        = 3.14159;    // const auto
```

### 3.4 Havola tiplari

```uzpp
butun& x = son;          // lvalue reference
butun&& y = 42;          // rvalue reference (move semantics)
ozgaruvchan& iter = v;   // auto reference (range-for)
```

### 3.5 Natija va Tanlov tiplari

```uzpp
// Natija<T> — xatolik boshqaruvi uchun (C++ std::expected analogiyasi)
uzpp::Natija<butun> xavfsiz_ildiz(butun n) {
    agar (n < 0) qaytarish uzpp::Natija<butun>::xato("Manfiy son!");
    qaytarish uzpp::Natija<butun>::muvaffaqiyat(n);
}

// Tanlov<T> — ixtiyoriy qiymat (std::optional analogiyasi)
uzpp::Tanlov<matn> topish(vektor<matn>& v, matn qidirish) {
    uchun (matn s : v) {
        agar (s == qidirish) qaytarish uzpp::Tanlov<matn>(s);
    }
    qaytarish uzpp::Tanlov<matn>();
}
```

---

## 4. O'zgaruvchilar va Konstantalar

### 4.1 O'zgaruvchi e'lon qilish

```uzpp
// Tip bilan aniq e'lon
butun son = 42;
haqiqiy pi = 3.14159;
matn ism = "Ali";

// Avtomatik tip (ozgaruvchan)
ozgaruvchan son2 = 100;
ozgaruvchan pi2  = 3.14;

// Konstanta
ozgarmas maksimal = 1000;

// C++ darajali const
const butun limit = 255;
```

### 4.2 Identifikator qoidalari

- Harflar, raqamlar, pastki chiziq (`_`) ishlatiladi
- Apostroflar (`'`) identifikatorda ishlatilmaydi (matn literalida mumkin)
- Katta-kichik harfga sezgir: `Son` va `son` turli identifikatorlar
- Til kalit so'zlari identifikator sifatida ishlatilmaydi

---

## 5. Operatorlar (Operators)

### 5.1 Arifmetik operatorlar

| Operator | Tavsif          |
|----------|-----------------|
| `+`      | Qo'shish        |
| `-`      | Ayirish         |
| `*`      | Ko'paytirish    |
| `/`      | Bo'lish         |
| `%`      | Qoldiq          |
| `++`     | Oshirish (post/pre) |
| `--`     | Kamaytirish     |

### 5.2 Solishtirish operatorlari

| Operator | Tavsif                |
|----------|-----------------------|
| `==`     | Tenglik               |
| `!=`     | Tengsizlik            |
| `<`      | Kichik                |
| `>`      | Katta                 |
| `<=`     | Kichik yoki teng      |
| `>=`     | Katta yoki teng       |

### 5.3 Mantiqiy operatorlar

| Operator | C++ | Tavsif  |
|----------|-----|---------|
| `&&`     | `&&` | VA      |
| `\|\|`   | `\|\|` | YOKI  |
| `!`      | `!`  | EMAS    |

### 5.4 O'zlashtirish operatorlari

```uzpp
son = 5;
son += 3;    // son = son + 3
son -= 1;    // son = son - 1
son *= 2;    // son = son * 2
son /= 4;    // son = son / 4
son %= 3;    // son = son % 3
```

### 5.5 Ko'rsatkich operatorlari

```uzpp
butun* ptr = &son;      // manzil olish
butun val = *ptr;       // qiymatga murojaat
ptr->metod();           // ko'rsatkich orqali metod
```

---

## 6. Boshqaruv oqimi (Control Flow)

### 6.1 Shartli ifodalar

```uzpp
// Oddiy if
agar (shart) {
    // ...
}

// if-else
agar (shart) {
    // ...
} aks_holda {
    // ...
}

// if-else if-else
agar (a > b) {
    // ...
} aks_holda agar (a == b) {
    // ...
} aks_holda {
    // ...
}

// Ternary operator
ozgaruvchan natija = (son > 0) ? "musbat" : "manfiy";
```

### 6.2 Tsikllar

```uzpp
// for tsikli
uchun (butun i = 0; i < 10; i++) {
    yozish << i << qator_oxiri;
}

// Range-for (foreach)
vektor<butun> v = [1, 2, 3, 4, 5];
uchun (butun n : v) {
    yozish << n << " ";
}

// while tsikli
holda (son > 0) {
    son--;
}

// Tsikldan chiqish
uchun (butun i = 0; i < 100; i++) {
    agar (i == 42) to'xtatish;     // break
    agar (i % 2 == 0) davom_etish; // continue
}
```

### 6.3 switch (tanlov)

```uzpp
// C++ switch orqali ham ishlatish mumkin
// uz++ versiyasi hali ishlab chiqilmoqda
```

---

## 7. Funksiyalar (Functions)

### 7.1 Funksiya e'lon qilish

```uzpp
// Qaytarish tipi bilan
butun kvadrat(butun x) {
    qaytarish x * x;
}

// void funksiya (bosh)
bosh chop_et(matn s) {
    yozish << s << qator_oxiri;
}

// Bir nechta parametr
haqiqiy uchburchak_yuzi(haqiqiy a, haqiqiy b, haqiqiy c) {
    haqiqiy s = (a + b + c) / 2.0;
    qaytarish uzpp::Matematika::ildiz(s * (s-a) * (s-b) * (s-c));
}
```

### 7.2 Default parametrlar va overloading

```uzpp
// C++ sintaksisida
butun yigindi(butun a, butun b = 0) {
    qaytarish a + b;
}
```

### 7.3 Lambda funksiyalar

```uzpp
// Oddiy lambda
ozgaruvchan ikki_baravar = [](butun x) { qaytarish x * 2; };

// Capture bilan
butun koeffitsient = 3;
ozgaruvchan kup = [&](butun x) { qaytarish x * koeffitsient; };

// Parametrsiz
ozgaruvchan salomlash = []() {
    yozish << "Salom!" << qator_oxiri;
};
salomlash();
```

### 7.4 Shablon funksiyalar (Template)

```uzpp
shablon <tur T>
bosh chop_vektor(vektor<T>& v) {
    uchun (ozgaruvchan x : v) yozish << x << " ";
    yozish << qator_oxiri;
}

// Chaqirish
vektor<butun> sonlar = [1, 2, 3];
chop_vektor(sonlar);  // T = butun avtomatik aniqlanadi
```

---

## 8. Sinflar (Classes)

### 8.1 Sinf e'lon qilish

```uzpp
sinf Shahs {
yopiq:
    matn ism_;
    butun yosh_;

ochiq:
    // Konstruktor
    Shahs(matn ism, butun yosh) : ism_(ism), yosh_(yosh) {}

    // Metodlar
    matn ism() const { qaytarish ism_; }
    butun yosh() const { qaytarish yosh_; }

    bosh tanishtir() {
        yozish << ism_ << " (" << yosh_ << " yosh)" << qator_oxiri;
    }
};
```

### 8.2 Meros (Inheritance)

```uzpp
sinf Talaba : ochiq Shahs {
ochiq:
    matn guruh;
    haqiqiy gpa;

    Talaba(matn ism, butun yosh, matn g, haqiqiy b)
        : Shahs(ism, yosh), guruh(g), gpa(b) {}

    bosh tanishtir() {
        Shahs::tanishtir();
        yozish << "Guruh: " << guruh << ", GPA: " << gpa << qator_oxiri;
    }
};
```

### 8.3 Strukturalar

```uzpp
sinf_nomi Nuqta {
    haqiqiy x;
    haqiqiy y;

    haqiqiy masofa(Nuqta boshqa) {
        haqiqiy dx = x - boshqa.x;
        haqiqiy dy = y - boshqa.y;
        qaytarish uzpp::Matematika::ildiz(dx*dx + dy*dy);
    }
};
```

---

## 9. Standart kutubxona (Standard Library)

### 9.1 `uzpp_runtime.hpp` — Asosiy modul

**`uzpp::Natija<T>`** — Xatolikni qaytara oladigan natija turi:

```uzpp
// Yaratish
uzpp::Natija<butun>::muvaffaqiyat(42)   // Muvaffaqiyatli natija
uzpp::Natija<butun>::xato("Xato!")      // Xato natijasi

// Tekshirish
natija.yaroqliMi()    // bool: muvaffaqiyatlimi?
natija.qiymat()       // T: qiymat (agar yaroqli bo'lsa)
natija.xatoMazmun()  // string: xato mazmuni
```

**`uzpp::Tanlov<T>`** — Ixtiyoriy qiymat (`std::optional` analogiyasi):

```uzpp
uzpp::Tanlov<butun> t1(42);       // Qiymat bor
uzpp::Tanlov<butun> t2();         // Bo'sh
t1.borMi()                        // true
t1.qiymat()                       // 42
```

**`uzpp::OqimPool`** — Ko'p oqimli ishlov uchun:

```uzpp
uzpp::OqimPool havza(4);   // 4 ta worker thread
havza.vazifaQosh([]() {
    // parallel bajariladigan kod
});
havza.kutish();             // Barcha vazifalar tugashini kutish
```

### 9.2 `matematika.hpp` — Matematik funksiyalar

```uzpp
uzpp::Matematika::PI              // 3.14159...
uzpp::Matematika::E               // 2.71828...
uzpp::Matematika::ildiz(x)        // sqrt(x)
uzpp::Matematika::daraja(x, n)    // pow(x, n)
uzpp::Matematika::mutloq(x)       // abs(x)
uzpp::Matematika::sinus(x)        // sin(x)
uzpp::Matematika::kosinus(x)      // cos(x)
uzpp::Matematika::logarifm(x)     // log(x)
uzpp::Matematika::faktorial(n)    // n!
uzpp::Matematika::ortacha(v)      // mean of vector
uzpp::Matematika::eng_katta_el(v) // max element
uzpp::Matematika::eng_kichik_el(v)// min element
```

### 9.3 `matn.hpp` — Matn funksiyalari

```uzpp
uzpp::Matn::qirqish(s)                    // trim whitespace
uzpp::Matn::kichik_harfga(s)              // to lowercase
uzpp::Matn::katta_harfga(s)               // to uppercase
uzpp::Matn::birinchi_harfni_katta(s)      // capitalize first
uzpp::Matn::ajratish(s, delimitr)         // split → vector<string>
uzpp::Matn::birlashtirish(v, biriktiruvchi) // join vector with separator
uzpp::Matn::almashtirish(s, eski, yangi)  // replace all occurrences
uzpp::Matn::boshlanganda(s, prefiks)      // starts_with
uzpp::Matn::tugaganda(s, sufiks)          // ends_with
uzpp::Matn::teskari(s)                    // reverse string
uzpp::Matn::butun_songa(s)               // parse int (throws on error)
uzpp::Matn::kasr_songa(s)                // parse double (returns optional)
uzpp::Matn::songa_aylantirish(n)         // to_string
```

### 9.4 `json.hpp` — JSON moduli

```uzpp
// Yaratish
ozgaruvchan obj = uzpp::Json::yaratObyekt();
obj->set("ism", uzpp::Json::yaratMatn("Ali"));
obj->set("yosh", uzpp::Json::yaratSon(25));
obj->set("faol", uzpp::Json::yaratMantiq(rost));

// Chiqarish
matn json_str = uzpp::Json::chiqarish(*obj, 2);  // 2-space indent

// Tahlil
ozgaruvchan j = uzpp::Json::tahlil(json_str);
agar (j->bor("ism")) {
    matn ism = (*j)["ism"]->matn();
}
```

---

## 10. Modullar va ulash (Modules / Includes)

```uzpp
// Standart kutubxona modullari
ulash "uzpp_runtime.hpp"   // Har doim birinchi bo'lishi kerak
ulash "matematika.hpp"
ulash "matn.hpp"
ulash "json.hpp"
```

Modullar `ulash` kalit so'zi bilan ulanadi. Standart modullar `stdlib/` papkasida joylashgan.

---

## 11. Yig'ish va ishlatish (Build & Run)

### 11.1 CLI buyruqlari

```bash
uzpp ishga-tushirish fayl.uzpp   # Transpile + compile + run
uzpp transpile fayl.uzpp          # Transpile to C++ only
uzpp tekshirish fayl.uzpp         # Lint/type-check without compiling
uzpp format fayl.uzpp             # Auto-format source file
uzpp lsp                          # Start LSP server
uzpp --version                    # Show version
uzpp --yordam                     # Show help
```

### 11.2 Transpilatsiya pipeline

```
fayl.uzpp
  │
  ▼ Lexer (tokenize)
  │  Kalit so'zlar, identifikatorlar, sonlar, matnlar
  │
  ▼ Parser (AST yaratish)
  │  FunctionDecl, ClassDecl, IfStatement, ...
  │
  ▼ TypeChecker (semantik tekshirish)
  │  O'zgaruvchilar, funksiyalar, tiplar
  │
  ▼ CodeGen (C++23 generatsiya)
  │  uz++ → C++ kalit so'zlar, tip tarjimalari
  │
  ▼ build/fayl.generated.cpp
  │
  ▼ g++ -std=c++23 (tashqi kompilyator)
  │
  ▼ build/fayl.exe  (yoki fayl Linux-da)
```

---

## 12. Xatoliklar va diagnostika

### 12.1 Xato formati

```
fayl.uzpp:12:5: xato: Noma'lum o'zgaruvchi 'son'
  12 |     yozish << son << qator_oxiri;
     |     ^
```

### 12.2 Xato turlari

| Tur                  | Tavsif                                     |
|----------------------|--------------------------------------------|
| Sintaksis xatosi     | Parser tomonidan aniqlanadi                |
| Tip xatosi           | TypeChecker tomonidan aniqlanadi           |
| Ogohlantirish        | Potensial muammo, lekin kompilyatsiya to'xtamaydi |
| C++ xatosi           | g++/clang++ tomonidan aniqlanadi           |

---

## 13. Cheklovlar (Known Limitations)

- Ko'p qatorli string literallari (`"..." "..."`) hali qo'llab-quvvatlanmaydi
- Nested `>>` template closing (masalan `vektor<vektor<butun>>`) parser bug
- Identifikatorda apostrof (`'`) ishlatib bo'lmaydi (C++ cheklovi)
- `switch/case` hali uz++ sintaksisida yo'q
- Modul tizimi (import/export) hali to'liq ishlamaydi

---

## 14. Versiyalar tarixi

| Versiya | Sana       | Asosiy o'zgarishlar                           |
|---------|------------|-----------------------------------------------|
| 0.1     | 2026-05-04 | Birinchi ommaviy reliz: 80+ kalit so'z, stdlib |

---

*uz++ 0.1 — O'zbek dasturlash tili*
