# Changelog

## [2.3.4] — 2026-09-07

### Tuzatildi
- **"Komponentlar holati" C++ kompilyatorini har doim "topilmadi" deb
  ko'rsatardi** — hammasi ishlayotgan tizimda ham.
  `componentManager.checkComponents()` `systemCpp` ni qattiq `null` qilib
  qaytarardi va `mingwOk` ni umuman qaytarmasdi, extension.js esa aynan shu
  ikki maydonni o'qirdi. Endi haqiqiy qidiruv bor
  (`detectHostCpp`): rasmiy o'rnatuvchi qo'ygan MinGW, kengaytma papkasidagi
  MinGW, so'ng PATH dagi `g++` / `clang++`.
- uz++ o'zi bor, lekin C++ kompilyatori yo'q holat endi alohida aytiladi:
  `tekshirish` ishlaydi, `qurish` uchun C++ kompilyatori kerak.

### Talab qilinadigan kompilyator
- Kengaytma endi uz++ **v2.3.2** bilan ishlaydi (beshta til xatosi tuzatildi).
  `uz++: Kompilyatorni yangilash` buyrug'i so'nggi relizni yuklab oladi.

## [2.3.3] — 2026-09-07

Marketplace sahifasining o'zi tuzatildi.

### Tuzatildi
- **Kengaytmaning ikonkasi yo'q edi** — marketplace va Extensions ro'yxatida
  standart bo'sh kvadrat ko'rinardi. `package.json` da `icon` maydoni umuman
  belgilanmagandi, holbuki `docs/logo/uzpp-icon.svg` allaqachon bor edi.
  Endi 256x256 PNG (`icon.png`) o'sha SVG dan olingan, `galleryBanner` esa
  ikonka foniga (`#1f2c38`) moslashtirildi.
- **README dagi ikkala nishon (badge) o'lik edi** — shields.io
  `visual-studio-marketplace` yo'nalishini bekor qilgan va marketplace
  sahifasida ular "retired badge" deb ko'rinardi. `vsmarketplacebadges.dev`
  ga almashtirildi (tekshirildi: versiya va o'rnatishlar soni to'g'ri
  qaytadi).

## [2.3.2] — 2026-09-07

Kengaytma kompilyatorning hozirgi holatiga moslashtirildi — v2.3.1 dan beri
qo'shilgan buyruqlar va sintaksis kengaytmada aks etmagan edi.

### Tuzatildi
- **"Yangi loyiha" ishlamaydigan loyiha yaratardi.** Manifest `nom = ` deb
  yozilardi, kompilyator esa `nomi` ni o'qiydi (`package_manager.h`,
  `TomlParser::parseProjectSection`) — nom jim yo'qolardi. `asosiy_fayl`
  umuman yozilmasdi va kod loyiha ildizida (`asosiy.uzpp`) yaratilardi,
  kompilyator esa `src/asosiy.uzpp` ni kutadi. Natijada yangi loyihada
  `uzpp ishga-tushirish` darhol "Asosiy uz++ fayli topilmadi" deb to'xtardi.
  Endi skelet `uzpp init` orqali yaratiladi — sxema bo'yicha ikkinchi manba
  qolmadi; kompilyator topilmasa, `uzpp init` bilan bir xil zaxira shablon
  ishlatiladi.
- **Xush kelibsiz ekrani `uz++ 2.1` deb yozardi** — raqam qo'lda yozilgan edi
  va uch reliz orqada qolgandi. Endi `package.json` dan olinadi.
- **Xush kelibsiz ekrani mavjud bo'lmagan kalit so'zlarni reklama qilardi:**
  `ozgaruvchan` / `ozgarmas` (Phase 2.5 da olib tashlangan — parser ularni
  xatolik bilan rad etadi), `aks holda` (probel bilan; to'g'risi `aks_holda`),
  va `holda` — tilda umuman yo'q so'z.
- **README kalit so'zlar jadvali:** `aks` — rad etiladigan eskirgan sinonim,
  `aks_holda` ga almashtirildi; `o'qish` esa kalit so'z emas — `std::cin`
  uchun kanonik so'z `kiritish`.

### Qo'shildi
- **`@` atributlari endi bo'yaladi:** `@sinov`, `@bench`,
  `@tashlab_yuborilmas`, `@eskirgan`, `@tekislash(N)`, `@taxmin(...)`,
  `@noyob_manzil`, `@bashqarib`, `@kamdan_kam`. Grammatikada atributlar uchun
  qoida umuman yo'q edi. Noma'lum atribut `invalid` sifatida belgilanadi —
  parser ham uni "Noma'lum annotatsiya" deb rad etadi.
- **Kompilyatorda bor, lekin kengaytmada yo'q bo'lgan buyruqlar:**
  - `uz++: Tekshirish (lint)` — `uzpp tekshirish`, qurmasdan tip tekshirish.
  - `uz++: Testlarni yugurtirish (@sinov)` — `uzpp sinov`.
  - `uz++: Benchmark (@bench)` — `uzpp bench`.
  - `uz++: Hujjat yaratish (API qo'llanma)` — `uzpp hujjat`, natijani ochadi.

### Eslatma
- `uzpp dap` (DAP serveri) kompilyatorda bor, lekin kengaytmada debugger
  hali ulanmagan: server gdb ustida ishlaydi va uzilish nuqtalarini
  generatsiya qilingan C++ ga qo'yadi, `.uzpp` qatorlariga emas. Tekshirilgan
  manba-xarita bo'lgunicha ulash to'g'ri emas.

## [2.3.1] — 2026-09-04

### Tuzatildi
- Kengaytma `uzpp formatlah` deb chaqirardi (imlo xatosi) — endi
  `uzpp formatlash`.
- Formatlagich faylni o'zgartirmasa, endi SABAB ko'rsatiladi. Kompilyator
  natijani asl kod bilan solishtiradi va farq bo'lsa faylni tegmaydi;
  ilgari kengaytma faqat "Command failed" deb yozardi.
- **Sintaksis bo'yash (`syntax highlighting`) jiddiy yangilandi.**
  Grammatikada mavjud bo'lmagan so'zlar bor edi (`holda`, `tuxta`,
  `siklni_tuxtat`, `keyingi`, `virtual`, `shaxsiy`), eng asosiylari esa
  yo'q edi — jumladan `toki` (while) va `bajar` (do). `ozgaruvchan` /
  `ozgarmas` — Phase 2.5 da olib tashlangan apostrofsiz shakllar —
  kanonik `o'zgaruvchan` / `o'zgarmas` ga almashtirildi. Kalit so'zlar
  va turlar ro'yxati endi kompilyator manbasidan olingan.

### Kompilyator tomonidagi muhim tuzatishlar (v2.3.1)
- **Sintaksis xatoliklari muharrirda yana ko'rinadi.** v2.3.0 dan beri
  parser istisno tashlash o'rniga xatoliklarni yig'ardi, LSP esa hamon
  `catch` ga tayanardi — natijada sintaksis xatosi UMUMAN
  ko'rsatilmasdi.
- **Hujjat keshi buzilmaydi.** LSP `"text":"` dan keyingi BIRINCHI
  qo'shtirnoqqacha qirqib olardi — ya'ni `yozish << "salom"` bor har
  qanday fayl keshda yarim yo'lda kesilardi va diagnostika, hover,
  semantik ranglar noto'g'ri ishlardi.
- macOS da kompilyatsiya endi ishlaydi (Apple Clang uchun bayroqlar).

## [2.3.0] — 2026-05-22

- Kompilyator v2.3.0: overload resolution, kechiktirilgan shablonlar,
  konstanta hisoblash, ko'p xatolikli parser, modullar.
- Sinonim tozalash (Phase 2.5): har bir C++ tushunchasi uchun bitta
  uz++ so'z.

## [2.2.1] — 2026-05-20

### Loyiha tuzilmasi va hujjatlar

- `examples/` papkasi `misollar/` ga birlashtirildi (15 ta misol).
- Hujjatlar yagona tilga (o'zbek) keltirildi: README, getting-started,
  CONTRIBUTING, SECURITY, va boshqalar.
- Ichki AI ishchi hujjatlari (HANDOFF, NEXT_SESSION_PLAN, UZPP_CONTEXT)
  ommaviy repo'dan olib tashlandi.

## [2.2.0] — 2026-05-20

### Til imkoniyatlari (uz++ kompilyatori)
- **Yangi self-hosted stdlib modullari:** `matematika` (~500 LOC,
  trigonometriya, statistika, vektor, matritsa) va `sinov` (~130 LOC,
  test freymvorki). Endi 5 ta self-hosted modul: `matn`, `xatoliklar`,
  `vaqt`, `matematika`, `sinov`.
- **Asosiy hardening:**
  - `asosiy()` qaytaruvchi turi `butun` bo'lishi shart — `matn asosiy()` parsing
    bosqichida xatolik beradi (avval g++ jim drop qilardi).
  - Kalit so'z o'zgaruvchi nomi sifatida ishlatish taqiqlandi (`butun agar = 5`
    xatolik). Allowlist: `asosiy`, `main`, `yangi`, `bosh`, `bekor`, `satr` —
    bular alias keywords, lookahead/codegen orqali soyalanadi.
  - Bo'sh `moslash { }` (match without cases) parsing xatolik.
  - Kompilyatsiya vaqtida nolga bo'lish aniqlanadi (`10 / 0`).

### Type checker — to'liq qayta qurilish (Phase 1 + Phase 2)
- **Uch-holatli tur tasviri** — strukturali Type:
  - `Aniq(nomi)` — aniq belgilangan tur
  - `Nomalum` — biz xulosa qila olmadik (diagnostikada jim qabul qilinadi)
  - `Polimorf(param)` — shablon parametri (T, U), instansiyalashda aniq bo'ladi
- **Kompozit turlar** (Phase 2.1): `Korsatkich`, `Havola`, `Shablon` — endi
  `butun*`, `const matn&`, `vektor<T>` to'g'ridan-to'g'ri tasvirlanadi (avval
  shunchaki string edi).
- **Honestlik:** diagnostika faqat `Aniq × Aniq` solishtirishda chiqariladi.
  Nomalum yoki Polimorf operandlar — jim qabul qilinadi. Bu shablon tanasidagi
  noto'g'ri ogohlantirishlarni butunlay yo'q qildi.
- **Shablon tanasi to'g'ri tekshiriladi:**
  - Funksiyalar (Phase 1.4): `shablon<tur T> funksiya f(T x) -> butun { qaytarish x; }`
    endi noto'g'ri "Funksiya 'butun' qaytarishi kerak, lekin 'T' qaytarilmoqda"
    ogohlantirishini bermaydi.
  - Sinflar (Phase 2.3): bir xil narsa shablon sinflarning metodlari uchun.
  - Kompozit shablon argumentlari (Phase 2.4): `vektor<T>`, `Foo<T>*` — to'g'ri
    Polimorf sifatida belgilanadi.
- **Kompozit Type LSP orqali ko'rinadi** (Phase 2.2): `getInferredAutoType`
  endi strukturali `Type*` qaytaradi. Hover ifoda turi bilan birga uning
  tabiatini ham ko'rsatadi: `butun* (ko'rsatkich)`, `T (shablon parametri)`.
- **Ifoda turlari kengaytirildi** (Phase 1.3): `&x`, `*p`, `!b`, `-x`, `++y`,
  ternar (`?:`) promotion bilan, assignment-as-expression, await/throw/lambda/
  pipeline endi mos ravishda xulosalanadi yoki halol Nomalum sifatida
  belgilanadi.

### LSP yaxshilanishi
- **Sinf a'zolari uchun semantic tokens** — sinf maydonlari va metodlari endi
  editorda alohida rangda ajratiladi (avval oddiy matn edi).
- **Hover endi xulosalangan turni ko'rsatadi** `o'zgaruvchan x = ...` uchun.
- **F5 PowerShell launchda** — har doim `&` prefiks bilan komanda chaqirildi.

### Standartlashtirish va imlo
- `OqimPool` → `OqimHovuz` (5 ta `.uzpp` test + runtime + type_checker).
- `OqimPooliPrivate` → `OqimHovuzIchki`.
- `bolish` → `bo'lish`, `fibonacci` → `fibonachchi`, `togri` → `to'g'ri`,
  `ozgaruvchan` → `o'zgaruvchan`.

### Testlar
| Avval | Hozir |
|---|---|
| 72 ijobiy, 47 salbiy, 4 pending | **74 ijobiy, 51 salbiy, 0 pending** |
| 3 self-hosted stdlib | **5 self-hosted** (`matn`, `xatoliklar`, `vaqt`, `matematika`, `sinov`) |
| 5 frontend smoke pinlari | **26 frontend smoke pinlari** |

## [2.1.9] — 2026-05-18

### Welcome ekrani / o'rnatish
- **Linux/macOS uchun `install.sh` integratsiyasi** — Welcome ekranidagi
  "Hammasini o'rnatish" tugmasi endi Linux/macOS da terminalda
  `curl -fsSL .../install.sh | bash` ni ishga tushiradi (avval "rasmiy
  o'rnatuvchi hali tayyor emas" deyilardi).
- README to'liq qayta yozildi: `uzpp-setup.exe` (~152 MB, MinGW-w64
  GCC 15.2 UCRT) va Linux/macOS one-liner ko'rsatildi.
- Welcome ekranidagi installer hajmi yangilandi (~115 MB → ~152 MB,
  GCC 14.2 → 15.2).

## [2.1.8] — 2026-05-18

### Til imkoniyatlari (uz++ kompilyatori)
- **Apostrof identifikatorlarda haqiqatan ishlaydi.** `o'lcham`, `G'oya`,
  `to'lov` kabi haqiqiy o'zbek so'zlari endi `_` ga aylantirilmaydi —
  C++23 da U+02BC MODIFIER LETTER APOSTROPHE sifatida saqlanadi (`oʼlcham`).
- **Keyword shadowing.** `yangi`, `bor`, `kasr`, `uzun`, `bosh` kabi
  alias-keywordlarni endi lokal o'zgaruvchi nomi sifatida ishlatish mumkin.
- **`} yoki { ... }`** — `yoki` endi `if`-`then`dan keyin `else` sifatida
  qabul qilinadi.
- **`funksiya X() o'zgarmas -> T { ... }`** — sinf metodlarida `funksiya`
  uslubida `o'zgarmas` modifier va trailing return type birga ishlaydi.
- **Bitwise operatorlar `& | ^`** — parser ularni binary sifatida tan
  oladi (unary `&` o'zgarmagan).
- **`yangi`/`o'chirish` lookahead** — endi `new`/`delete` sifatida faqat
  type/expression davom etganda olinadi.

### Stdlib
- **`stdlib/matn.uzpp`** — `matn` kutubxonasi to'liq uz++ tilida qayta
  yozildi (626 LOC). Eski C++ `matn.hpp` artefakt sifatida shu fayldan
  generatsiya qilinadi.
- **41 funksiya** `matn` da: barcha 27 ta eski + 14 ta yangi
  (`qadar_qirqish`, `qator_son`, `formatlash_indeksli`, `bir_marta_ajratish`...)
  + 7 ta UTF-8 yordamchi (`belgilar_soni`, `chapdan_belgi_bo'yicha`,
  `qism_matn_belgi_bo'yicha` ...) — kirill/o'zbek matnlari uchun kritik.

### Til — `ulash "*.uzpp"`
- Uz++ fayllarni boshqa uz++ fayldan to'g'ridan-to'g'ri `ulash` qilish
  mumkin. Transpilator rekursiv ravishda `.uzpp` ni `.hpp` ga aylantiradi
  (header-mode).
- `manba_joyi` — `std::source_location` ning o'zbekcha aliasi.

### LSP
- **Inlay hints**: `o'zgaruvchan x = ...` dan keyin xulosa qilingan tur
  ko'rsatiladi.
- **Code actions**: `unused variable` warning uchun quick-fix.

### Release infrastruktura
- **`uzpp-setup.exe` avtomatik quriladi** (avval qo'lda upload qilinardi).
  Inno Setup + WinLibs GCC 15.2 UCRT bundle.
- **`install.sh`** — Linux/macOS bir-buyruqli installer.
- macOS x64 (Intel) qo'llab-quvvatlash to'xtatildi (macos-13 runner
  GitHub Actions tomonidan deprecated).

### Sinov natijasi
- Local regression: **68/68** (avval 64/64).

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
