# 09 · Shablonlar — Umumlashgan dasturlash {#09-shablonlar}

## Muammo: bir xil kod, har xil tur

Tasavvur qiling, sizga eng kattasini topadigan funksiya kerak:

```cpp
butun maks(butun a, butun b) {
    qaytarish (a > b) ? a : b;
}
```

Yaxshi! Lekin endi sizga `haqiqiy` lar uchun ham kerak bo'ldi.
Yana yozasiz:

```cpp
haqiqiy maks(haqiqiy a, haqiqiy b) {
    qaytarish (a > b) ? a : b;
}
```

Endi `matn` uchun ham — yana takrorlaysiz. `kasr` uchun ham — yana.
**Bir xil kod, faqat tur farqli**. Bu — yomon. Agar logikani o'zgartirish
kerak bo'lsa — barchasini o'zgartirish kerak.

**Yechim: shablonlar.** Bir marta yozing, kompilyator har bir tur
uchun avtomatik versiyani yaratadi.

```cpp
shablon<tur T>
T maks(T a, T b) {
    qaytarish (a > b) ? a : b;
}

butun x = maks(5, 3); // butun versiya
haqiqiy y = maks(2.5, 3.7); // haqiqiy versiya
matn z = maks("alma", "anor"); // matn versiya
```

Bitta funksiya — har xil turlarda ishlaydi!

### Bu nima — sehrmi?

Yo'q, sehr emas. **Kompilyator** har bir chaqirish uchun **maxsus
versiyani avtomatik yaratadi**. Yuqoridagi kod kompilyatsiyaga shunday
o'tadi:

```cpp
butun maks(butun a, butun b)         { qaytarish (a > b) ? a : b }
haqiqiy maks(haqiqiy a, haqiqiy b) { qaytarish (a > b) ? a : b }
matn maks(matn a, matn b)            { qaytarish (a > b) ? a : b }
```

Bu — **kompilyatsiya vaqtidagi kod generatsiyasi**. Demak, shablonlar
ish jarayonida sekinlashtirmaydi — har bir versiya **alohida tezkor**.

---

## Funksiya shablonlari

### Sintaksis

```cpp
shablon<tur T>
T funksiya_nomi(T parametr) {
    // ... T turida ishlash
}
```

`T` — bu **shablon parametri**. Bu istalgan tur o'rnida turishi
mumkin: `butun`, `matn`, `Talaba`, va h.k.

> **Nima uchun "T"?** Bu shunchaki traditsiya. "Type" so'zining birinchi
> harfi. Siz `U`, `V`, `Element`, `Tur` deb nomlashingiz mumkin.

### Bir nechta shablon parametri

```cpp
shablon<tur T, tur U>
bosh juftlikni_korsat(T a, U b) {
    yozish << a << " " << b << qator_oxiri;
}

juftlikni_korsat(5, "salom"); // T=butun, U=matn
juftlikni_korsat(3.14, rost); // T=haqiqiy, U=mantiqiy
```

### Aniq chaqirish

Ba'zan kompilyator turini aniqlay olmaydi — bu holda aniq aytishingiz
mumkin:

```cpp
shablon<tur T>
T defolt() {
    qaytarish T(); // T ning default qiymati
}

butun a = defolt<butun>(); // 0
haqiqiy b = defolt<haqiqiy>(); // 0.0
matn c = defolt<matn>(); // ""
```

### Misol: vektorning yig'indisi

```cpp
shablon<tur T>
T vektor_yigindi(o'zgarmas vektor<T>& v) {
    T natija = T(); // 0 yoki ""
    uchun (o'zgarmas T& element : v) {
        natija += element;
    }
    qaytarish natija;
}

vektor<butun> sonlar = {1, 2, 3, 4, 5}
yozish << vektor_yigindi(sonlar); // 15

vektor<matn> sozlar = {"a", "b", "c"}
yozish << vektor_yigindi(sozlar); // "abc"
```

Bitta funksiya, ikki xil ishlatish.

---

## Sinf shablonlari

Sinflarni ham shablon qilish mumkin. Bu juda kuchli.

### Misol — qutilar

```cpp
shablon<tur T>
sinf Quti {
yopiq:
    T element;

ochiq:
    Quti(T qiymat) : element(qiymat) {}

    T olish() o'zgarmas {
        qaytarish element;
    }

    bosh qoyish(T yangi_qiymat) {
        element = yangi_qiymat;
    }
}

butun asosiy() {
    Quti<butun> quti1(42);
    yozish << quti1.olish(); // 42

    Quti<matn> quti2("Salom");
    yozish << quti2.olish(); // Salom

    Quti<haqiqiy> quti3(3.14);
    yozish << quti3.olish(); // 3.14
}
```

### Standart kutubxonadagi shablonlar

Aslida `vektor<T>`, `std::map<K, V>`, `std::pair<A, B>` — barchasi
shablonlar:

```cpp
vektor<butun>; // vektor<T> ning butun versiyasi
vektor<matn>; // vektor<T> ning matn versiyasi
std::map<matn, butun>; // std::map<K, V> ning matn-butun versiyasi
```

---

## Standart shablon kutubxonasi (STL)

C++ ning **Standart Shablon Kutubxonasi** (STL) — bu tayyor
shablonlarning katta to'plami:

### Konteynerlar

| Konteyner | Tavsif |
|-----------|--------|
| `vektor<T>` | Dinamik massiv |
| `std::array<T, N>` | Statik massiv |
| `std::list<T>` | Bog'langan ro'yxat |
| `std::deque<T>` | Ikki tomonlama navbat |
| `std::map<K, V>` | Tartiblangan xarita |
| `std::set<T>` | Tartiblangan to'plam |
| `std::unordered_map<K, V>` | Hash xarita (tezroq) |
| `std::unordered_set<T>` | Hash to'plam |

### Algoritmlar

```cpp
ulash <algorithm>

vektor<butun> v = {3, 1, 4, 1, 5, 9, 2, 6}

std::sort(v.begin(), v.end()); // tartiblash
std::reverse(v.begin(), v.end()); // teskari
butun yigindi = std::accumulate(v.begin(), v.end(), 0);
butun max_el = *std::max_element(v.begin(), v.end());
butun joy = std::count(v.begin(), v.end(), 1);
```

Hammasi shablon orqali, har xil tur bilan ishlay oladi.

---

## Concepts (C++20)

Zamonaviy C++ da shablon parametrlarini cheklash mumkin. Masalan,
"faqat sonli turlar" deb cheklash:

```cpp
ulash <concepts>

shablon<tur T> shart (std::integral<T>)   // T butun bo'lishi kerak
T kvadrat(T x) {
    qaytarish x * x;
}

butun asosiy() {
    yozish << kvadrat(5) << qator_oxiri;  // OK
    // kvadrat(3.14);                     // ← XATO: haqiqiy butun emas
    qaytarish 0;
}
```

Cheklov `shablon<...>` dan keyin, `shart (...)` shaklida yoziladi
(C++ dagi `requires`). Qavslar ichida — istalgan `concept` yoki
mantiqiy ifoda.

Concepts — qadimgi shablonlarning xatolik xabarlarini soddalashtiradi.

---

## Shablonlar va turlash

Shablonni ba'zan sekin yoki keraksiz ishlatish mumkin. Misol:

```cpp
shablon<tur T>
T qoshish(T a, T b) { qaytarish a + b; }
```

Bu yaxshi. Lekin agar siz `matn + butun` ni qo'shmoqchi bo'lsangiz:

```cpp
qoshish("Yosh: ", 25); // ← XATO! Matn va butun bir xil tur emas
```

Yechim — ikki shablon parametri:

```cpp
shablon<tur T, tur U>
avto qoshish(T a, U b) {
    qaytarish a + b;
}
```

Lekin bu kontekstga qarab — turli xil amallar bo'lishi mumkin.

---

## Amaliy misol: Generic stack

Stack — bu LIFO (Last In, First Out) ma'lumotlar tuzilmasi. Mantar
qutisi kabi: oxirgi qo'yilgan birinchi olinadi.

```cpp
shablon<tur T>
sinf Stack {
yopiq:
    vektor<T> ma'lumotlar;

ochiq:
    bosh push(T qiymat) {
        ma'lumotlar.push_back(qiymat);
    }

    T pop() {
        agar (ma'lumotlar.empty()) {
            qaytarish T();
        }
        T natija = ma'lumotlar.back();
        ma'lumotlar.pop_back();
        qaytarish natija;
    }

    T top() o'zgarmas {
        qaytarish ma'lumotlar.back();
    }

    butun olchami() o'zgarmas {
        qaytarish ma'lumotlar.size();
    }

    mantiqiy bo'sh() o'zgarmas {
        qaytarish ma'lumotlar.empty();
    }
}

butun asosiy() {
    Stack<butun> s;
    s.push(1);
    s.push(2);
    s.push(3);
    yozish << s.pop() << qator_oxiri; // 3
    yozish << s.pop() << qator_oxiri; // 2
    yozish << s.pop() << qator_oxiri; // 1

    Stack<matn> matnlar;
    matnlar.push("a");
    matnlar.push("b");
    yozish << matnlar.pop(); // "b"

    qaytarish 0;
}
```

Bitta `Stack` sinfi — har qanday tur bilan ishlay oladi.

---

## Eng ko'p uchraydigan xatolar

### 1. Shablonni `header` da ko'rsatmaslik

Funksiya shablonlari kompilyatsiya paytida yangilanadi. Shu sababli,
ular **header**da to'liq yozilishi kerak — `.cpp` faylida emas.

### 2. Turlarni adashtirish

<!-- darslik:skip -->
```cpp
shablon<tur T> T maks(T a, T b)

maks(5, 3.14); // ← XATO: 5 — butun, 3.14 — haqiqiy
```

To'g'risi: `maks<haqiqiy>(5, 3.14)` yoki aniq tur belgilash.

### 3. Cheksiz instantsiyalash

Murakkab shablonlar kompilyatsiya vaqtini juda ko'paytiradi. Sodda
saqlang.

---

## Bob bo'yicha mashqlar

### Mashq 1: min funksiyasi (oson)

`shablon<turdash T> T min(T a, T b)` — kichikrog'ini qaytaradi.

### Mashq 2: Vektorda qidirish (o'rta)

`shablon<turdash T> butun topish(vektor<T>& v, T maqsad)` — birinchi
indeksini qaytaring, yoki -1 (topilmasa).

### Mashq 3: Generic juftlik (o'rta)

`shablon<turdash A, turdash B> sinf Juftlik` — `first` va `second`
maydonlari bilan.

### Mashq 4: Navbat (o'rtacha)

`shablon<turdash T> sinf Navbat` (FIFO — birinchi kelgan birinchi
ketadi).

### Mashq 5: Binary daraxt (qiyin)

`shablon<turdash T> sinf BinaryDaraxt` — qo'shish, qidirish,
inorder yurish funksiyalari bilan.

---

## Xulosa

- **Shablon** — bitta kod, ko'p tur uchun.
- **Funksiya shablonlari** — `shablon<turdash T>`.
- **Sinf shablonlari** — sinfning butunlay shablon versiyasi.
- **STL** — tayyor shablonlar kutubxonasi (`vektor`, `map`, `set`,
  algoritmlar).
- **Concepts** — shablon parametrini cheklash (C++20).
- **Afzalligi** — kod qayta ishlatiladi, tezlik yo'qolmaydi.
- **Kamchiligi** — kompilyatsiya vaqti uzayadi, xato xabarlari
  murakkab.

Keyingi bobda biz **xatolik bilan ishlash** ni o'rganamiz —
dasturlaringizni mustahkam qilishning eng muhim yo'li.
