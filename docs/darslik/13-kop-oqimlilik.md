# 13 · Ko'p oqimlilik — Parallel dasturlash {#13-kop-oqimlilik}

## Nega ko'p oqimlilik kerak?

Zamonaviy kompyuterlarning protsessorlarida **4, 8, 16 yoki undan ko'p
yadro** bor. Lekin agar siz oddiy dastur yozsangiz — u **bitta yadroda**
ishlaydi. Boshqa yadrolar bo'sh turadi! Bu — resurs sarfi.

**Ko'p oqimlilik** (multithreading) — bu bir dasturning bir necha
**oqim**lari bir vaqtning o'zida ishlashi. Har bir oqim — alohida
yadroni ishlatishi mumkin.

### Real hayotdagi misol

Tasavvur qiling, restoranda 4 ta oshpaz bor. Oddiy yondashuv: bitta
oshpaz buyurtmalarni navbat bilan tayyorlasin, qolganlari kutsin. Aqlli
yondashuv: 4 ta oshpaz **parallel** ishlasin — har biri o'z buyurtmasini.

Dasturda ham xuddi shunday:

```
Oqim 1: faylni o'qish     ─────────────►
Oqim 2: hisoblash         ─────────────►
Oqim 3: foydalanuvchi     ─────────────►
Oqim 4: tarmoq            ─────────────►
                          parallel ishlashadi
```

Bu — to'rt baravar tezroq (yoki shunchaga yaqin).

### Qachon ko'p oqimlilik foydali?

- **Hisoblashga boy** vazifalar (matritsa ko'paytirish, AI, video kodlash)
- **I/O bilan kutuvchi** vazifalar (fayl, tarmoq) — bittasi kutsa,
  boshqa oqim ishlasin
- **Interaktiv dasturlar** — UI muzlamasligi uchun

### Qachon kerak emas?

- Sodda, tezkor amallar (oqim ochish overhead'i amalga zarar yetkazadi)
- Ketma-ket mantiqiy ishlar (bir-biridan bog'liq)

---

## `std::thread` — birinchi oqim

```cpp
ulash <thread>


bosh salom() {
    yozish << "Salom oqimdan!" << qator_oxiri;
}

butun asosiy() {
    oqim t(salom); // yangi oqim yaratish va ishga tushirish
    t.kutish(); // u tugashini kutish

    yozish << "Asosiyda" << qator_oxiri;
    qaytarish 0;
}
```

**Tushuntirish:**
- `std::thread t(salom)` — yangi oqim ochiladi va `salom()` funksiyasi
  unda boshlanadi.
- `t.join()` — asosiy oqim t tugashini kutadi.

### Parametrli funksiya bilan

```cpp
bosh takrorlab_yoz(matn xabar, butun marta) {
    uchun (butun i = 0; i < marta; i++) {
        yozish << xabar << qator_oxiri;
    }
}

oqim t(takrorlab_yoz, "Salom", 3);
t.kutish();
```

### Lambda bilan

```cpp
oqim t([](butun n) {
    uchun (butun i = 0; i < n; i++) {
        yozish << i << qator_oxiri;
    }
}, 5);
t.kutish();
```

### Bir necha oqim

```cpp
butun asosiy() {
    oqim t1([]() { yozish << "Oqim 1" << qator_oxiri });
    oqim t2([]() { yozish << "Oqim 2" << qator_oxiri });
    oqim t3([]() { yozish << "Oqim 3" << qator_oxiri });

    t1.kutish();
    t2.kutish();
    t3.kutish();
    qaytarish 0;
}
```

Natija — **tartibsiz** chiqishi mumkin:
```
Oqim 2
Oqim 1
Oqim 3
```

Yoki:
```
Oqim 1Oqim 2

Oqim 3
```

Chunki oqimlar parallel ishlashadi va `yozish` ga bir vaqtda
yozishlari mumkin. Bu — **birinchi qiyinchilik**.

---

## Sinxronizatsiya — `std::mutex`

Yuqoridagi muammoning yechimi — **mutex** (mutual exclusion).
"Hech kim bir vaqtda kira olmaydi" qoidasi.

```cpp
ulash <mutex>

qulf qulf;

bosh xavfsiz_yozish(matn xabar) {
    qulf.lock();
    yozish << xabar << qator_oxiri;
    qulf.unlock();
}
```

Endi:

```cpp
oqim t1([]() { xavfsiz_yozish("Oqim 1") });
oqim t2([]() { xavfsiz_yozish("Oqim 2") });
t1.kutish();
t2.kutish();
```

Natija — har doim toza:
```
Oqim 1
Oqim 2
```

(yoki teskari, lekin bo'lib chiqmaydi).

### `lock_guard` — xavfsizroq usul

`lock()` va `unlock()` ni qo'lda chaqirish — xavfli. Agar oraliqda
istisno yuz bersa, `unlock()` chaqirilmaydi.

`std::lock_guard` — RAII (avtomatik) qulflash:

```cpp
bosh xavfsiz_yozish(matn xabar) {
    std::lock_guard<qulf> lg(qulf);
    yozish << xabar << qator_oxiri;
    // lg destruktori avtomatik unlock qiladi
}
```

### Resurs poyga (Race condition)

Quyidagi kodda muammo bor:

```cpp
butun hisoblagich = 0;

bosh oshirish() {
    uchun (butun i = 0; i < 100000; i++) {
        hisoblagich++;
    }
}

butun asosiy() {
    oqim t1(oshirish);
    oqim t2(oshirish);
    t1.kutish();
    t2.kutish();
    yozish << hisoblagich; // 200000 deb kutiladi, lekin...
    qaytarish 0;
}
```

Natija **noaniq** — 100000 dan 200000 gacha biror son. Nima uchun?

`hisoblagich++` aslida 3 ta amal:
1. `hisoblagich` ni o'qish
2. 1 qo'shish
3. Qayta saqlash

Ikki oqim bir vaqtda 1 ni o'qib, 1 qo'shib, 2 ni saqlasa — ikkalasi
ham 2 ga oshiradi, lekin aslida 3 bo'lishi kerak edi.

**Yechim:**

```cpp
qulf m;
butun hisoblagich = 0;

bosh oshirish() {
    uchun (butun i = 0; i < 100000; i++) {
        std::lock_guard<qulf> lg(m);
        hisoblagich++;
    }
}
```

Endi har bir `hisoblagich++` atomar — boshqa oqim aralasholmaydi.

---

## `std::atomic` — engil sinxronizatsiya

Mutex sekin. Ba'zan `std::atomic` tezroq:

```cpp
ulash <atomic>

atomik<butun> hisoblagich = 0;

bosh oshirish() {
    uchun (butun i = 0; i < 100000; i++) {
        hisoblagich++; // atomar amal
    }
}
```

`atomic` oddiy turlar uchun (butun, ko'rsatkich, mantiqiy) — protsessor
darajasidagi sinxronizatsiya. Mutex'dan tez.

---

## `std::async` — asinxron bajarish

Oqim ochib, uning natijasini olish:

```cpp
ulash <future>

butun katta_hisob() {
    butun yigindi = 0;
    uchun (butun i = 0; i < 1000000; i++) {
        yigindi += i;
    }
    qaytarish yigindi;
}

butun asosiy() {
    kelajak<butun> f = oqim_boshla(std::launch::async, katta_hisob);

    // Boshqa ish qilish ...
    yozish << "Hisobni kutyapmiz" << qator_oxiri;

    butun natija = f.get(); // natijani olamiz (kutamiz agar tayyor bo'lmasa)
    yozish << "Natija: " << natija;
    qaytarish 0;
}
```

`std::async` — yangi oqimda funksiyani ishga tushiradi va `future`
qaytaradi. `future.get()` orqali natijani olamiz.

---

## `std::promise` va `std::future`

Oqimlar o'rtasida ma'lumot uzatish:

```cpp
std::promise<butun> p;
kelajak<butun> f = p.get_future();

oqim t([&p]() {
    butun natija = 42;
    p.set_value(natija);
});

butun n = f.get(); // 42
t.kutish();
```

`promise` — ma'lumot yuborgich.
`future` — ma'lumot qabul qiluvchi.

---

## Oqimlar havzasi (Thread pool)

Har gal yangi oqim ochish — qimmat. **Thread pool** — oldindan
yaratilgan oqimlar to'plami. Vazifalar navbatga qo'yiladi.

uz++ da `uzpp::OqimHavzasi` mavjud (yoki o'zingiz yaratishingiz mumkin).

```cpp
ulash <vector>
ulash <thread>

sinf OqimHavzasi {
yopiq:
    vektor<oqim> oqimlar;

ochiq:
    OqimHavzasi(butun n) {
        uchun (butun i = 0; i < n; i++) {
            oqimlar.emplace_back([]() {
                // har bir oqim navbatdan vazifa olib bajaradi
            });
        }
    }
    // ...
}
```

Bu murakkab mavzu — bu yerda faqat tushuncha berdik.

---

## Sinxronizatsiya vositalari

| Vosita | Maqsadi |
|--------|---------|
| `std::mutex` | Yagona kirish (mutual exclusion) |
| `std::lock_guard` | RAII mutex (avtomatik unlock) |
| `std::unique_lock` | Yanada moslashuvchan mutex |
| `std::atomic` | Atomar o'zgaruvchilar |
| `std::condition_variable` | Oqimni kutish/signalga uyg'otish |
| `std::semaphore` (C++20) | Resurs hisoblagich |
| `std::barrier` (C++20) | Bir necha oqimni sinxronlash |

---

## Amaliy misol: Parallel massiv qayta ishlash

```cpp
ulash <vector>
ulash <thread>
ulash <numeric>

butun yigindi_qism(o'zgarmas vektor<butun>& v, butun bosh, butun oxir) {
    qaytarish std::accumulate(v.begin() + bosh, v.begin() + oxir, 0);
}

butun asosiy() {
    vektor<butun> v(1000000, 1); // million ta 1

    // Bitta oqimda
    butun y1 = std::accumulate(v.begin(), v.end(), 0);
    yozish << "Bitta oqim: " << y1 << qator_oxiri;

    // To'rtta oqimda
    butun n = v.size();
    butun qism = n / 4;
    kelajak<butun> f1 = oqim_boshla(yigindi_qism, std::ref(v), 0,       qism);
    kelajak<butun> f2 = oqim_boshla(yigindi_qism, std::ref(v), qism,    qism*2);
    kelajak<butun> f3 = oqim_boshla(yigindi_qism, std::ref(v), qism*2,  qism*3);
    kelajak<butun> f4 = oqim_boshla(yigindi_qism, std::ref(v), qism*3,  n);

    butun y2 = f1.get() + f2.get() + f3.get() + f4.get();
    yozish << "To'rt oqim: " << y2 << qator_oxiri;
    qaytarish 0;
}
```

Katta ma'lumotlar uchun — to'rtta oqim ~4 marta tezroq.

---

## Eng ko'p uchraydigan xatolar

### 1. `join()` qilishni unutish

```cpp
oqim t(salom);
// t.kutish() yo'q — dastur tugaganda crash
```

### 2. Race condition

Mutex'siz birgalikda o'zgartirilgan o'zgaruvchilar.

### 3. Deadlock

Ikki oqim bir-birini kutadi va hech qachon ozod bo'lmaydi:

```cpp
qulf m1, m2;

void f1() {
    m1.lock();
    m2.lock();
    // ...
}

void f2() {
    m2.lock(); // teskari tartib!
    m1.lock();
    // ...
}
```

`f1` `m1` ni oldi, `m2` ni kutadi. `f2` `m2` ni oldi, `m1` ni kutadi.
**Hech qachon tugamaydigan kutish**.

**Yechim:** Mutex'larni doim **bir xil tartibda** olish.

### 4. Detached oqim resurslari

```cpp
{
    std::vector<int> v = {1, 2, 3}
    oqim t([&v]() {
        // ...v ni ishlatish...
    });
    t.detach();
}                                      // v yo'qoldi, lekin oqim hali ishlaydi!
```

### 5. Thread overhead

```cpp
uchun (butun i = 0; i < 1000000; i++) {
    oqim t([]() { /* kichik ish */ });
    t.kutish();
}
```

Million ta oqim ochish — bu juda sekin va resurs sarfi. Thread pool
ishlating.

---

## Bob bo'yicha mashqlar

### Mashq 1: Birinchi oqim (oson)

Yangi oqimda "Salom, oqim!" deb chiqaring.

### Mashq 2: Bir nechta oqim (oson)

5 ta oqim yarating, har biri o'z raqamini chiqarsin.

### Mashq 3: Hisoblagich bilan mutex (o'rta)

Ikki oqim hisoblagichni 100000 marta oshirsin — natija aniq
bo'lsin.

### Mashq 4: Parallel yig'indi (o'rtacha)

Katta vektorni 4 oqimda yig'indini hisoblang.

### Mashq 5: Producer-Consumer (qiyin)

`Producer` oqim navbatga element qo'yadi. `Consumer` oqim oladi. Mutex
va `condition_variable` ishlating.

---

## Xulosa

- **Ko'p oqimlilik** — bir vaqtda bir necha ishni bajarish.
- **`std::thread`** — yangi oqim ochish.
- **`std::mutex`** — yagona kirish nazorati.
- **`std::lock_guard`** — RAII mutex.
- **`std::atomic`** — engil sinxronizatsiya.
- **`std::async`/`std::future`** — asinxron ishlatish va natija olish.
- **Race condition** va **deadlock** — eng ko'p tarqalgan muammolar.

Keyingi bobda biz **standart kutubxona** ni umumiy ko'rib chiqamiz —
uz++ bilan birga keladigan barcha foydali modullar haqida.
