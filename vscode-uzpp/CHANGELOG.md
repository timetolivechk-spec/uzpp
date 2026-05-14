# Changelog

## [2.1.7] — 2026-05-13

### Til imkoniyatlari (transpilatsiya)
- **Compile-time modifikatorlari (Stage 1):** `sobit_ifoda` → `constexpr`,
  `sobit_baholash` → `consteval`, `sobit_boshlangich` → `constinit`.
- **Funksiya atributlari:** `@tashlab_yuborilmas` → `[[nodiscard]]`,
  `@eskirgan` → `[[deprecated]]`.
- **`agar sobit_ifoda (...)`** → `if constexpr (...)` — shablonlarda compile-time tarmoqlash.
- **`statik_tasdiqlash(cond, "msg")`** → `static_assert(cond, "msg")`.
- **`xato_tashlamaydi`** → `noexcept` — funksiya parametrlaridan keyin yoki konstruktor
  initializer-list dan oldin.
- **`birlashma`** → `union`, **bitfield** sintaksisi (`butun x : 4`), **C-uslubidagi massivlar**
  sinf maydonlarida (`butun data[10]`).
- **C++23 multidim subscript:** `arr[i, j, k]` ko'p o'lchovli indekslash.
- **Variadic shablonlar va fold-ifodalar:** `shablon <tur... Args>`, `(args + ...)`,
  pack expansion `args...`.
- **Deducing this (C++23):** `oz`/`bosh` birinchi parametr — explicit object parameter.
- **Funksiya overloading:** TypeChecker bir nomli funksiyalarni qayta e'lon qilishga ruxsat beradi.
- **Structured binding:** `o'zgaruvchan [a, b] = p;` ichidagi nomlar to'g'ri ro'yxatga olinadi.

### Sifat
- 36/36 testlar muvaffaqiyatli o'tadi. Phase 11 va Phase 12 birlashtirildi.

## [2.1.6] — 2026-05-12

### Marketplace ko'rinishi
- **README to'liq qayta yozildi** — endi to'g'ridan-to'g'ri Marketplace sahifasida
  `uzpp-setup.exe` linki ko'rinadi va katta sarlavha bilan ko'rsatiladi. Avval
  "Xush kelibsiz ekranida tugma bosing" deyilardi — bu chalkash edi, chunki
  ekrandan tashqari hech qanday tugma `Marketplace` sahifasida ko'rinmaydi.
  Endi ham toza Marketplace o'qish, ham VS Code ichidagi tugma bir xil
  yo'lni ko'rsatadi.

### Foydalanuvchi tajribasi (parallel kompilyator fixi)
- `uzpp-setup.exe` ham yangilandi: ichidagi `uzpp.exe` endi
  `#include "uzpp_runtime.hpp"` ni har qanday CWD'dan to'g'ri topadi
  (`stdlib/` katalogi `uzpp.exe` yonida yoki bittasi yuqorida bo'lsa). Avval
  `cd <repo-root>` qilmasdan ishga tushirib bo'lmasdi.

## [2.1.5] — 2026-05-12

Marketplace v2.1.4 slotini muvaffaqiyatsiz upload bloklab qo'ydi
(MS API "version exists and cannot be modified" deydi). 2.1.4 ning
to'liq tarkibi 2.1.5 da chiqariladi — quyidagi 2.1.4 yozuvini ko'ring.

## [2.1.4] — 2026-05-12

### Yangi imkoniyatlar

- **«O'rnatish» tugmasi har doim ko'rinadi.** Welcome ekranida endi tugma
  hech qachon yashirilmaydi: agar uz++ topilmagan bo'lsa — «Hammasini
  o'rnatish», topilgan bo'lsa — «Qayta o'rnatish / yangilash».
- **Status bar'da doimiy `$(cloud-download) uz++ install` tugmasi** —
  istalgan paytda welcome ekranini bir bosish bilan ochadi.
- **Welcome'da holat blok** — uz++ versiyasi va aniq yo'lini ko'rsatadi.
- **Welcome'dagi tugma rasmiy `uzpp-setup.exe` ni yuklab oladi va ishga
  tushiradi** — ichida MinGW GCC 14.2 bor, qo'shimcha hech narsa kerak emas.

### Tuzatishlar (kritik)

- **HTTP 404 hatosi olib tashlandi.** Eski welcome `releases/download/
  14.2.0posix-18.1.8...` (WinLibs o'chirgan release) ga murojaat qilardi —
  foydalanuvchi xato xabarini ko'rardi va o'rnatish ishlamasdi. Endi
  rasmiy releasedan `uzpp-setup.exe` ishlatiladi.
- **Welcome endi PATH'dagi uz++ ni hisobga oladi** — `uzpp-setup.exe` orqali
  o'rnatgan foydalanuvchi «✓ Tayyor» holatini ko'radi va ortiqcha 150 MB
  qayta yuklamaydi.
- **Status bar va `requireComponents` (F5 oldi tekshiruvi)** ham PATH'ni
  hisobga oladi.

## [2.1.3] — 2026-05-12

### Yangi imkoniyatlar
- **Tizim PATH dan kompilyatorni topish** — `uzpp-setup.exe` (Windows) yoki
  Homebrew/apt orqali o'rnatilgan `uzpp` endi avtomatik aniqlanadi.
  Globalstorage'ga ikkinchi nusxa yuklab olish shart emas.
- **Kompilyator topilmaganda yo'l-yo'riq xabari** — kengaytma faollashganda
  `uzpp` topilmasa, foydalanuvchiga uchta variant ko'rsatiladi:
  «Avtomatik o'rnatish» (Windows uchun `uzpp-setup.exe` ni yuklab oladi),
  «Komponentlar (eski usul)» (eski globalstorage flow) yoki
  «Yo'lni ko'rsatish» (mavjud `uzpp.exe` ga qo'lda yo'naltirish).

### Tuzatishlar
- `findCompilerPath` zanjiriga `where`/`which` orqali PATH qidiruvi qo'shildi
  (eski tartib: globalStorage > embedded > settings > workspace > PATH;
  yangi tartib: globalStorage > embedded > settings > **PATH** > workspace).

## [2.1.2] — 2026-05-07

- Marketplace'da chop etildi (oraliq versiya, hujjatsiz).

## [2.1.1] — 2026-05-06

### O'zgarishlar
- MinGW WinLibs GCC 14.2.0 dan to'g'ridan-to'g'ri yuklanadi (release artifact sifatida emas)
- GitHub Release yaratildi: `uzpp-windows-x64.zip` (`uzpp.exe` + `stdlib/`)

## [2.1.0] — 2026-05-06

### Yangi imkoniyatlar
- **F5 / Ctrl+F5** — klaviatura yorliqlari orqali ishga tushirish va qurish
- **uz++ yangi loyiha** — papka va shablon fayl (`asosiy.uzpp` + `uzpp.toml`) yaratish
- **uz++ C++ kodini ko'rish** — transpile qilingan `.generated.cpp` ni yonida ochish
- **uz++ kompilyatorni yangilash** — GitHub Releases'dan avtomatik yuklash
- **uz++ faylni formatlash** — `uzpp format` buyrug'i orqali formatlash
- **Status bar** — kompilyator versiyasi va tezkor ishga tushirish tugmasi
- **Xush kelibsiz ekrani** — birinchi o'rnatishda qo'llanma va misollar
- **Editor title run button** — `.uzpp` faylida yuqori o'ngdagi ▷ tugma

### O'zgarishlar
- `uzpp.serverPath` sozlamasi `uzpp.compilerPath` ga o'zgartirildi
- Kompilyator qidirish tartibi takomillashtirildi (bin/ > settings > workspace > PATH)

---

## [2.0.0] — 2025-11-01

### Yangi imkoniyatlar
- LSP (Language Server Protocol) integratsiyasi
- Real-vaqt diagnostika (xatolar va ogohlantirishlar)
- Sintaksis yorqinligi yaxshilandi
- Avtoto'ldirish qo'shildi

---

## [1.0.0] — 2025-06-01

### Birinchi chiqarilish
- Asosiy sintaksis yorqinligi
- `.uzpp` fayl kengaytmasi qo'llab-quvvatlash
