# 10 · Xatolik bilan ishlash {#10-xatolik}

## Xatolik — bu hayotning bir qismi

Real dasturlar **doim** xatoliklarga duch keladi. Fayl topilmaydi,
internet uziladi, foydalanuvchi 0 ga bo'lishni so'raydi, disk to'lib
ketadi, parol noto'g'ri kiritiladi. Yaxshi dastur — bu xatolikni
**bo'lmasa kerak**, deb da'vo qiladigan dastur emas, balki xatolikni
**oqilona qabul qilib**, foydalanuvchiga aniq xabar beradigan dastur.

Tasavvur qiling, bankomatda pul olmoqchisiz, lekin balans yetmaydi.
Bankomat sizga **xato xabari** beradi: "Mablag' yetarli emas". Aksincha
bo'lsa nima bo'lardi? Bankomat indamasdan o'chib qolardi yoki noto'g'ri
miqdor berardi. **Xatoliklarni** to'g'ri ushlash — yaxshi dasturning
asosi.

uz++ (va C++) da xatoliklarni boshqarishning bir nechta usuli bor:

1. **Qaytarish qiymati** — funksiya muvaffaqiyat yoki xatolikni
   qaytaradi.
2. **Istisno** (exception) — `urinish`/`ushlash` bloki bilan ushlash.
3. **`Natija<T>` / `std::expected`** — zamonaviy yondashuv (C++23).
4. **`Tanlov<T>` / `std::optional`** — qiymat bo'lishi/bo'lmasligi.
5. **`tasdiqlash`** (assert) — dasturchi xatolarini ushlash.

---

## Usul 1: Qaytarish qiymati

Eng oddiy va eski usul. Funksiya muvaffaqiyat bo'lsa qiymatni, xato
bo'lsa maxsus belgi qaytaradi.

```cpp
ulash "uzpp_runtime.hpp"

butun bolish(butun a, butun b) {
    agar (b == 0) {
        qaytarish -1; // xato belgisi
    }
    qaytarish a / b;
}

butun asosiy() {
    butun n = bolish(10, 0);
    agar (n == -1) {
        yozish << "Xato: 0 ga bo'lib bo'lmaydi" << qator_oxiri;
    }
    qaytarish 0;
}
```

**Muammosi:** `-1` — bu odatiy qiymat bo'lishi mumkin. `bolish(-5, 5)`
ham `-1` qaytaradi! Qanday xato va qanday natijani farqlash qiyin.

---

## Usul 2: Istisnolar (Exceptions)

Modern C++ ning asosiy yondashuvi: funksiya xato hosil bo'lganda
**istisno tashlaydi** (throw), chaqiruvchi uni **ushlaydi** (catch).

### `urinish` / `ushlash` / `irgitish` bloki

```cpp
urinish {
    // xato bo'lishi mumkin kod
}
ushlash (xato_turi xato_nomi) {
    // xato bilan ishlash
}
```

### Misol

```cpp
butun bolish(butun a, butun b) {
    agar (b == 0) {
        irgitish std::runtime_error("0 ga bo'lib bo'lmaydi");
    }
    qaytarish a / b;
}

butun asosiy() {
    urinish {
        butun n = bolish(10, 0);
        yozish << n << qator_oxiri;
    }
    ushlash (o'zgarmas std::runtime_error& e) {
        yozish << "Xato yuz berdi: " << e.what() << qator_oxiri;
    }

    qaytarish 0;
}
```

**Natija:**

```
Xato yuz berdi: 0 ga bo'lib bo'lmaydi
```

### Qanday ishlaydi?

1. `urinish` bloki ichidagi kod bajariladi.
2. Agar kod ichida `irgitish` bo'lsa — istisno _ko'tarib chiqariladi_.
3. Mos `ushlash` bloki topiladi — uning kodi bajariladi.
4. Agar mos `ushlash` topilmasa — dastur ishdan chiqadi (terminate).

### Bir nechta `ushlash` blok

```cpp
urinish {
    // ...
}
ushlash (o'zgarmas std::out_of_range& e) {
    yozish << "Chegaradan tashqari: " << e.what();
}
ushlash (o'zgarmas std::runtime_error& e) {
    yozish << "Runtime xato: " << e.what();
}
ushlash (...) {                       // hamma boshqa xatolar
    yozish << "Noma'lum xato";
}
```

### Standart istisnolar

| Istisno | Tavsif |
|---------|--------|
| `std::runtime_error` | Umumiy runtime xato |
| `std::logic_error` | Mantiqiy xato |
| `std::out_of_range` | Chegaradan tashqari |
| `std::invalid_argument` | Noto'g'ri argument |
| `std::bad_alloc` | Xotira ajratilmadi |

### O'zingizning istisno turingiz

```cpp
sinf MeningXatom : std::runtime_error {
ochiq:
    MeningXatom(matn xabar) : std::runtime_error(xabar) {}
}

bosh xato_yarat() {
    irgitish MeningXatom("Maxsus xato");
}
```

### Istisnolarni qachon ishlatish kerak?

**Yaxshi holatlar:**
- Funksiya ish bajara olmadi (fayl ochilmadi, tarmoq yo'q)
- Mantiqiy xato (noto'g'ri parametr)
- Xotira tugadi

**Yomon holatlar:**
- Oddiy oqim boshqaruvi uchun (siklldan chiqish kabi)
- Tez-tez yuz beradigan vaziyatlar uchun (sekin!)

> **Diqqat — `istisno` sekin amal.** Istisno tashlash va ushlash —
> bu sekin operatsiya. Agar siz har soniyada 1000 marta istisno
> tashlasangiz — dastur juda sekinlashadi. Faqat **istisnoiy
> vaziyatlar** uchun.

---

## Usul 3: `Natija<T>` / `std::expected` (C++23)

Bu — eng zamonaviy va xavfsiz yondashuv. Funksiya **qiymat** yoki
**xato** qaytaradi:

```cpp
Natija<butun, matn> bolish(butun a, butun b) {
    agar (b == 0) {
        qaytarish Natija<butun, matn>::xato("0 ga bo'lib bo'lmaydi");
    }
    qaytarish Natija<butun, matn>::muvaffaqiyat(a / b);
}

butun asosiy() {
    o'zgaruvchan natija = bolish(10, 0);

    agar (natija) {                   // muvaffaqiyatli?
        yozish << "Natija: " << natija.qiymat();
    }
    aks_holda {
        yozish << "Xato: " << natija.xatoMazmun();
    }
    qaytarish 0;
}
```

`Natija<T, E>` — bu **T (qiymat) yoki E (xato)** qaytaradigan tur.

**Afzalliklari:**
- Tez (istisno yo'q)
- Aniq (siz xatoni unutib bo'lmaysiz — qaytarish qiymati bilan ish)
- Tur bo'yicha xavfsiz

---

## Usul 4: `Tanlov<T>` / `std::optional`

Agar qiymat **bo'lishi yoki bo'lmasligi** mumkin bo'lsa, lekin xato
sababi muhim emas:

```cpp
Tanlov<butun> qidiruv(vektor<butun>& v, butun maqsad) {
    uchun (butun i = 0; i < v.size(); i++) {
        agar (v[i] == maqsad) {
            qaytarish Tanlov<butun>::bor(i);
        }
    }
    qaytarish Tanlov<butun>::yoq();
}

butun asosiy() {
    vektor<butun> v = {1, 2, 3, 4, 5};
    o'zgaruvchan natija = qidiruv(v, 3);

    agar (natija) {
        yozish << "Topildi, indeks: " << natija.qiymat();
    }
    aks_holda {
        yozish << "Topilmadi";
    }
    qaytarish 0;
}
```

`Tanlov` — `Natija` ning soddalashtirilgan versiyasi.

---

## Usul 5: `tasdiqlash` (assert)

`tasdiqlash` (C++ da `assert`) — bu **dasturchi xatolarini** ushlash
uchun. Foydalanuvchi xatolari uchun emas!

```cpp
ulash <cassert>

butun yashir_olish(o'zgarmas vektor<butun>& v, butun i) {
    tasdiqlash(i >= 0 && i < v.size()); // sharti tekshirish
    qaytarish v[i];
}
```

Agar `tasdiqlash` ichidagi shart **yolg'on** bo'lsa — dastur darhol
to'xtaydi va xato chiqaradi. Bu — siz hech qachon kutmagan vaziyatni
ushlash uchun.

### Release rejimida

`tasdiqlash` faqat **debug** rejimida ishlaydi. Release rejimida
(optimizatsiya bilan) avtomatik o'chiriladi. Shu sababli:

- Foydalanuvchi xatolari uchun ishlatmang
- Faqat dasturchi shartlarini tekshirish uchun
- Asosiy mantiq uchun emas

---

## Xatolik strategiyasini tanlash

| Holat | Tavsiya |
|-------|---------|
| Foydalanuvchi xatosi (noto'g'ri parol) | Qaytarish qiymati yoki `Tanlov` |
| Fayl yo'q, tarmoq xato | `Natija<T, E>` yoki istisno |
| Tubdan kutilmagan holat | `tasdiqlash` |
| Funksiya ba'zan qiymat qaytarmaydi | `Tanlov<T>` |
| Kutilgan, lekin past darajadagi xato | `Natija<T, E>` |
| Yangi C++ kodi | `Natija`, `Tanlov` afzal |
| Eski kod bilan integratsiya | Istisnolar |

---

## Amaliy misol: Konfiguratsiya faylini o'qish

```cpp
ulash "uzpp_runtime.hpp"
ulash <fstream>
ulash <string>

uzpp::Natija<matn, matn> faylni_oqish(matn yol) {
    std::ifstream f(yol);
    agar (!f.is_open()) {
        qaytarish uzpp::Natija<matn, matn>::xato("Fayl ochilmadi: " + yol);
    }

    matn mazmun((std::istreambuf_iterator<char>(f)),
                std::istreambuf_iterator<char>());
    qaytarish uzpp::Natija<matn, matn>::muvaffaqiyat(mazmun);
}

butun asosiy() {
    o'zgaruvchan natija = faylni_oqish("config.txt");

    agar (natija.yaroqliMi()) {
        yozish << "Fayl mazmuni:" << qator_oxiri << natija.qiymat();
    }
    aks_holda {
        yozish << "Xato: " << natija.xatoMazmun() << qator_oxiri;
        qaytarish 1;
    }

    qaytarish 0;
}
```

**Faqat fayl bor bo'lsa**: mazmunini chiqaradi.
**Yo'q bo'lsa**: xato xabarini chiqaradi va `1` kod bilan tugaydi.

---

## Eng ko'p uchraydigan xatolar

### 1. Istisno ushlashning unutilishi

```cpp
// urinish blokisiz, istisno dasturning yuqori darajalariga qadar
// ko'tariladi va terminate qiladi
butun n = bolish(10, 0); // crash!
```

### 2. Hamma istisnolarni `catch (...)` bilan yutib qo'yish

```cpp
ushlash (...) {
    // hech narsa qilmaslik — xatolarni yashiradi
}
```

Hech bo'lmaganda log qiling yoki qayta tashlang.

### 3. Istisno xabarisiz

```cpp
irgitish std::runtime_error(""); // foydasiz
irgitish std::runtime_error("Fayl ochilmadi: " + yo'l); // foydali
```

### 4. Destruktorda istisno tashlash

```cpp
sinf X {
    ~X() {
        irgitish std::runtime_error("..."); // ← XATO! crash
    }
}
```

Destruktorda hech qachon istisno tashlamang.

### 5. Resource leak

```cpp
fayl_ochish();
urinish {
    // xato yuz berdi
}
fayl_yopish(); // bajarilmaydi!
```

To'g'risi — RAII (Resource Acquisition Is Initialization): destruktor
fayl yopadi.

---

## Bob bo'yicha mashqlar

### Mashq 1: Xavfsiz bo'lish (oson)

`Natija<haqiqiy, matn>` qaytaruvchi `xavfsiz_bolish` funksiyasi.

### Mashq 2: Vektorda qidirish (oson)

`Tanlov<butun> topish(vektor<butun>&, butun)` — topilmasa nullopt.

### Mashq 3: Matnni songa aylantirish (o'rta)

`Natija<butun, matn> matn_son(matn s)` — noto'g'ri formatda xato.

### Mashq 4: Kalkulyator istisno bilan (o'rta)

Sodda kalkulyator. 0 ga bo'lish — istisno tashlasin.

### Mashq 5: Fayl operatsiyalari (qiyinroq)

Fayl ochish, o'qish, yozish — har bir bosqichda xatolarni boshqarish.

---

## Xulosa

- **Xatolik** — real dasturlarning oddiy hodisasi.
- **Qaytarish qiymati** — eng oddiy, lekin chalkash usul.
- **Istisnolar** (`urinish`/`ushlash`/`irgitish`) — kuchli, lekin
  sekin va murakkab.
- **`Natija<T, E>`** (`std::expected`, C++23) — zamonaviy, tez,
  aniq.
- **`Tanlov<T>`** (`std::optional`) — qiymat yoki yo'q.
- **`tasdiqlash`** — faqat dasturchi xatolarini ushlash uchun.

Keyingi bobda biz **matn va satrlar** bilan ishlashni o'rganamiz —
matn dasturning eng ko'p ishlatiladigan turi. Aniq matn manipulyatsiyasi
sizga juda foydali bo'ladi.
