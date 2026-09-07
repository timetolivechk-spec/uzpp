# Changelog

## [2.3.2] — 2026-09-07

Real dastur yozish paytida topilgan beshta xato tuzatildi. Sinov usuli:
`kutubxona` loyihasi noldan, hujjatga qarab yozildi — xatolarning aksariyati
birinchi sinfdayoq uchradi.

### Tuzatildi

- **Konstruktor/metod parametri sinf a'zosi nomi bilan to'qnashardi.**
  `Kitob(matn nomi) : nomi_(nomi) {}` — agar sinfda `nomi()` metodi bo'lsa,
  "O'zgaruvchi 'nomi' ushbu qamrovda allaqachon e'lon qilingan" xatosi
  chiqardi. Sabab: metod qamroviga sinf maydonlari va metod nomlari oldindan
  joylanadi (`joriy->` siz murojaat uchun), parametrlar esa shu qamrovga
  e'lon qilinardi. C++ da parametr a'zoni soyalaydi — bu qonuniy va
  `getter` + bir xil nomli parametr eng keng tarqalgan uslub.
  Regressiya: `tests/test_azo_soyalash.uzpp`.

- **`shartnoma` metodlarida `ustidan_yozish` ishlamasdi.** Tur tekshiruvchi
  `InterfaceDeclaration` tugunini umuman ko'rmasdi, shuning uchun interfeys
  `classes_` da yo'q edi va "bazaviy sinfda mos metod topilmadi" xatosi
  chiqardi. Oddiy `sinf` bilan ishlardi — faqat interfeyslar bilan emas.
  Regressiya: `tests/test_shartnoma_ustidan_yozish.uzpp`.

- **`funksiya<...>` ichidagi turlar C++ ga tarjima qilinmasdi.**
  `funksiya<mantiqiy(butun)>` dan `std::function<mantiqiy(butun)>` hosil
  bo'lardi — hosil bo'lgan C++ umuman kompilyatsiya bo'lmasdi. Ikki sabab
  bor edi: imzo shakli (`Qaytish(Arg)`) typeMap da yo'q edi, va shablon
  argumentlarini ajratuvchi qavslarni sanamagani uchun
  `funksiya<butun(butun, butun)>` verguldan ikkiga bo'linib ketardi.
  Regressiya: `tests/test_funksiya_imzosi.uzpp`.

- **g++ bosqichidagi xatolar noto'g'ri qatorni ko'rsatardi.** Hosil bo'lgan
  C++ da butun funksiya uchun bitta `#line 1` direktivasi bor edi, bo'sh
  qatorlar esa ko'chirilmasdi — shuning uchun g++ ning qator hisobi asl
  fayldan uzoqlashardi. Bitta faylda semantik analiz 13-qatorni,
  g++ esa 4-qatorni ko'rsatardi. Endi har bir operator oldidan `#line`
  chiqariladi (`ASTNode::sourceLine()`).

- **C++ kutubxona xabarlari tarjimadan buzilardi.** Xato matnlari uz++ ga
  o'girilganda `std::__cxx11::basic_string<char>` → `basic_string<belgi>`,
  `class function` → `sinf function` bo'lardi — mavjud bo'lmagan turlar.
  Endi ikki tagchiziqli (implementatsiyaga ajratilgan) nomlar tegilmaydi,
  oddiy `std::string` esa avvalgidek `matn` ga tarjima qilinaveradi.

Barcha muhim o'zgarishlar shu yerda hujjatlashtiriladi.

## [v2.3.1] — 2026-09-04

Sifat va ishonchlilik relizi. Yangi imkoniyat qo'shilmagan — mavjudlari
haqiqatan ishlashi ta'minlangan. Barcha o'zgarishlar darslik misollarini
haqiqiy kompilyator bilan solishtirish natijasida topilgan.

### Tildagi tuzatishlar
- **`<<` / `>>` ustuvorligi C++ bilan moslashtirildi.** Ilgari ular qo'shish
  darajasida edi: `yozish << a + b` → `(std::cout << a) + b`.
  `a << 2 + 1` endi `a << 3` (avval `(a << 2) + 1` edi) — bu JIMGINA
  noto'g'ri natija berardi.
- **`bajar { ... } toki (shart);`** (do/while) qo'shildi — ilgari umuman yo'q edi.
- **`<<=` va `>>=`** operatorlari qo'shildi.
- **Bo'sh gap `;`** — `uchun (butun i = 0; i < 10; i++);` endi ishlaydi.
- **`moslash` da guruhlangan yorliqlar**: `holat 1, 2, 3:` va ketma-ket
  bo'sh `holat` yorliqlari. Ilgari `holat 12: holat 1: holat 2: <tana>`
  kompilyatsiya bo'lardi, lekin faqat OXIRGI qiymat uchun ishlardi.
- **Massiv parametrlari**: `butun asosiy(butun argc, belgi* argv[])`.
- **`yangi Tur{a, b}`** — qavsli initsializatsiya (agregat tuzilmalar uchun).
- **`butun* o'zgarmas p`** — o'zgarmas ko'rsatkich (`int* const`).
- **`o'zgarmas PI = 3.14;`** — tursiz o'zgarmas (`const auto`). Ilgari
  `const PI =;` degan buzuq C++ chiqarardi.
- **Sinf maydonlarining standart qiymati** (`butun soni_ = 42;`) ilgari
  JIMGINA tashlab yuborilardi — maydon initsializatsiyalanmagan qolardi.
- Asosiy sinf nomi malakalangan va kirish darajali bo'lishi mumkin:
  `sinf X : std::runtime_error`, `sinf X : ochiq Ota`, `sinf X : Baza<butun>`.

### Standart kutubxona
- **`Natija<matn, matn>`** (T == E) kompilyatsiya bo'lmasdi — teg turlari
  bilan tuzatildi. 10-bob aynan shu turni o'rgatadi.
- **`uzpp sinov` (@sinov) umuman ishlamasdi** — codegen mavjud bo'lmagan
  sinf nomlarini chaqirardi.
- **`uzpp bench` (@bench) uchun stdlib da sinf yo'q edi** — `BenchTo'plami`
  qo'shildi (isitish chaqiruvi, ns/amal o'lchov).
- **`OqimHovuz::kutish()` abadiy osilib qolardi** — u ishchi oqimlarni
  `join` qilardi, ishchilar esa `toxtatish()` chaqirilmaguncha
  aylanaveradi. `misollar/09_kop_oqimlilik.uzpp` oxirigacha yetib
  bormasdi. Endi `kutish()` navbat bo'shashini kutadi va havzani
  to'xtatmaydi.

### Tur tekshiruvchi
- Shablon funksiyasi `-> T` qaytarsa, chaqiruv joyida soxta
  "'butun' qaytarishi kerak, lekin 'T' qaytarilmoqda" ogohlantirishi berardi.
- `vektor<T>` maydonli shablon sinf soxta ogohlantirish berardi.
- Shablon chaqiruvidan keyingi gaplar "erishib bo'lmaydi" deb belgilanardi.
- Katta harfli o'zgaruvchilar (`PI`, `MAX`) ishlatilsa ham
  "ishlatilmagan" deb ogohlantirilardi.
- Bir xil xabar ikki marta chiqmaydi.

### Xato xabarlari
- Funksiya TANASI ichidagi parser xatoliklari YUTILARDI — foydalanuvchi
  haqiqiy sabab o'rniga faylning oxiridagi "Noto'g'ri ifoda" xabarini
  ko'rardi.
- Joylashuv xabarda ikki marta takrorlanardi.
- `moslash` ichida `to'xtatish` uchun aniq, o'rgatuvchi xabar.
- Kalit so'zni o'zgaruvchi nomi qilganda yechim ham ko'rsatiladi.

### LSP (VS Code)
- **Sintaksis xatoliklari muharrirda UMUMAN ko'rinmasdi** (v2.3.0 dan beri).
- **Hujjat keshini JSON dekodlash buzuq edi**: `yozish << "salom"` bor
  har qanday fayl birinchi qo'shtirnoqda kesilardi — diagnostika, hover
  va semantik ranglar noto'g'ri ishlardi.

### Formatlagich
- **Kodni buzardi.** 87 test faylidan 26 tasi formatlashdan keyin
  kompilyatsiya bo'lmasdi. Endi fayl qayta yozilishdan OLDIN natija
  tekshiriladi; farq bo'lsa fayl tegilmaydi.
- Chiqish sifati: ustuvorlikka qarab qavs, K&R uslub, kirish darajalari
  saqlanadi, ichki `__uzpp_*` nomlari manba faylga chiqmaydi. Idempotent.

### macOS
- **`uzpp qurish` macOS da HAR DOIM yiqilardi**: `-fmodules-ts` (Apple
  Clang bilmaydi) va `-Wl,--gc-sections` (Mach-O linkerida yo'q).
  Yangi `src/host_compiler.h` kompilyatorni aniqlab, bayroqlarni moslaydi.
- `/proc/self/exe` Darwin da yo'q — `_NSGetExecutablePath` ishlatiladi.
- `install.sh`: karantin belgisi olib tashlanadi (`xattr -dr`), ad-hoc
  imzo qo'yiladi (`codesign --sign -`) — Gatekeeper to'sig'i shundan.
- macOS Intel (x86_64) qo'llab-quvvatlanadi; relizda universal binar.

### CI
- **YANGI macOS ishi** — ilgari macOS umuman tekshirilmasdi.
- Uchala platformada: salbiy testlar, `uzpp sinov`/`uzpp bench`,
  formatlagich xavfsizligi va HUJJATLARDAGI HAMMA KOD MISOLLARI.

### Hujjatlar
- Darslik misollari kompilyator bilan solishtirildi: 4-bob `moslash`
  bo'limi (C `switch` semantikasini o'rgatardi) va 14-bob `@sinov`
  bo'limi (mavjud bo'lmagan sintaksis) qayta yozildi; ikki bobda
  yo'qolgan ``` tiklandi; eskirgan sinonimlar, yetishmagan `;` va
  noto'g'ri API chaqiruvlari tuzatildi.
- `getting-started.md` 12 misolidan 7 tasi kompilyatsiya bo'lmasdi.
- Darslik PDF qayta qurildi; uchta nusxa o'rniga bitta; README dan havola.
- `uzpp --version` "v4.0.0" deb yolg'on aytardi (endi CMake dan keladi).
- `uzpp --yordam` / `-h` qo'shildi; `formatlah` → `formatlash`.

## [v2.3.0] — 2026-05-22

### Phase 2.5 — Sinonim tozalash ✅
- `mantiq` → faqat `mantiqiy` (bool)
- `ikkilangan` → faqat `haqiqiy` (double)
- `ozgaruvchan` → faqat `o'zgaruvchan` (auto)
- `ozgarmas` → faqat `o'zgarmas` (const)
- Prinsip: **bitta C++ tushunchasi = bitta uz++ so'zi**

### Phase 3 — Overload Resolution ✅
- `functionReturns_`/`functionParams_`/`functionMinArgs_` (map<string,X>)
  o'rniga `functionOverloads_: map<string, vector<FunctionOverload>>`
- `resolveOverload()`: aniq moslik (0), promotion (1), konversiya (2),
  Nomalum jarimasi (5)
- Diagnostika: "chaqiruv noaniq" va "argument soni mos emas"
- Backward-compatible `getFunctionReturns()`/`getFunctionParams()` API

### Stage 1 — LSP yaxshilanishlari ✅
- **Hover**: `butun y = 5` kabi aniq turlar uchun ham ishlaydi
  (avval faqat `o'zgaruvchan x = ...` uchun ishlardi)
- **Semantic tokens**: meros qilib olingan klass a'zolari ham ajratib
  ko'rsatiladi (`collectClassMembers` → `collectClassWithInheritance`)

### Phase 4 — Lazy Template Instantiation ✅
- Shablon funksiyalari chaqirilganda, konkret argument turlari bilan
  qayta tekshiriladi (Polimorf rejim o'chiriladi)
- `currentTemplateSubsts_` — shablon parametrlarini konkret turlarga
  almashtirish xaritasi
- Variadic shablonlar (`Args...`) to'g'ri ishlaydi
- `instantiatedTemplates_` — takroriy tekshirishni oldini oladi

### Phase 5 — Constant Evaluation ✅
- `ConstValue` va `evaluateConstExpr()` — kompilyatsiya vaqtidagi
  ifodalarni hisoblash (integer, float, boolean, string, unary, binary)
- `agar sobit_ifoda` — shartni hisoblab, o'lik tarmoqni o'chiradi
- `statik_tasdiqlash` — compile-time shart tekshiruvi (baholay olmasa,
  g++ ga o'tkazib yuboradi)

## [2.2.1] — 2026-05-20

### Loyiha tuzilmasi va hujjatlar tartibi

- **`examples/` papkasi olib tashlandi** — `misollar/` (o'zbekcha) bilan
  bir xil maqsadda edi. Unikal misollar `misollar/11..15` ga ko'chirildi.
  Endi yagona `misollar/` (15 ta misol). Tushuncha: bitta C++ uchun bitta
  uz++ qoidasi nafaqat til, balki loyiha tuzilmasi uchun ham.
- **`loyihalar/` olib tashlandi** — bo'sh "playground" papkasi edi.
- **`HANDOFF.md` / `NEXT_SESSION_PLAN.md` / `UZPP_CONTEXT.md` olib
  tashlandi** — ichki AI ishchi hujjatlari edi, ommaviy repo'da kerak emas.
- **Hujjatlar yagona tilga ko'chirildi (o'zbek)** — README, getting-started,
  stdlib-status, CONTRIBUTING, SECURITY, CODE_OF_CONDUCT, issue/PR
  shablonlari. Texnik atamalar uchun ingliz so'zlari quvirda
  (`"templates"`, `"closures"` va h.k.).
- **Installer matnlari soddalashtirildi** — `examples` → `misollar`.

## [2.2.0] — 2026-05-20

### Til (uz++ kompilyatori) — hardening pass
- **`asosiy()` qaytaruvchi turi `butun` bo'lishi shart.** Boshqacha yozilsa
  (masalan `matn asosiy()`) parsing bosqichida xatolik beriladi. Avval g++
  jim drop qilardi.
- **Kalit so'zlar o'zgaruvchi nomi sifatida taqiqlandi.** `butun agar = 5`
  endi xatolik. Allowlist (`asosiy`, `main`, `yangi`, `bosh`, `bekor`, `satr`) —
  bular alias keywords bo'lib parser lookahead yoki codegen `localScopes_`
  orqali soyalanishi mumkin.
- **Bo'sh `moslash { }`** (case'siz match) — parsing xatolik.
- **`10 / 0`** kompilyatsiya vaqtida aniqlanadi.

### Type checker — to'liq qayta qurilish

Phase 1: uch-holatli tur tasviri (Aniq / Nomalum / Polimorf).
- Diagnostika faqat `Aniq × Aniq` solishtirishda chiqariladi — `Nomalum × _`
  va `Polimorf × _` jim qabul qilinadi. Bu shablon tanasidagi noto'g'ri
  ogohlantirishlarni butunlay yo'q qildi.
- Shablon funksiyalari (`shablon<tur T> funksiya f(T x) -> butun { qaytarish x; }`)
  endi noto'g'ri return-type ogohlantirishini bermaydi.
- Ifoda turlari to'liq qoplandi: `&x`, `*p`, `!b`, `-x`, `++y`, ternar (`?:`),
  assignment-as-expression, await/throw/lambda/pipeline.

Phase 2: strukturali Type — `Korsatkich(base)`, `Havola(base, konst)`,
`Shablon(nomi, args)` endi to'g'ridan-to'g'ri tasvirlanadi (avval string edi).
- Rekursiv `isAniq()`: `Korsatkich(Polimorf("T"))` → yolg'on, ya'ni
  `butun**` yoki `vektor<T>*` ham diagnostikada to'g'ri jim qoladi.
- Strukturali interning: bir xil tur har gal bir xil ko'rsatkichga ega bo'ladi.
- LSP `getInferredAutoType` endi strukturali `Type*` qaytaradi. Hover
  natijasida tip turi (`butun* (ko'rsatkich)`, `T (shablon parametri)`)
  qo'shilgan.
- Shablon sinflari ham qamrab olindi (Phase 2.3) — metodlardagi T-li
  ifodalar Polimorf sifatida xulosalanadi.
- Kompozit shablon argumentlari (`vektor<T>`, `Foo<T>*`) ham to'g'ri
  Polimorf belgilanadi (Phase 2.4).

### Stdlib — yangi self-hosted modullar
- **`stdlib/matematika.uzpp`** (~500 LOC) — trigonometriya, statistika,
  vektor (2D/3D), matritsa amallari, tasodifiy sonlar.
- **`stdlib/sinov.uzpp`** (~130 LOC) — test freymvorki: `TestTo'plami`,
  `tasdiqlash()`, `tasdiqlash_teng()`.
- Endi 5 ta self-hosted modul: `matn`, `xatoliklar`, `vaqt`, `matematika`,
  `sinov`.

### LSP yaxshilanishi
- **Sinf a'zolari uchun semantic tokens** — sinf maydonlari va metodlari
  endi editorda alohida rangda ajratiladi.
- **Hover endi xulosalangan turni ko'rsatadi** `o'zgaruvchan x = ...` uchun,
  shu jumladan kompozit turlar (`butun*`, `vektor<butun>`).

### Imlo / standartlashtirish
- `OqimPool` → `OqimHovuz`, `OqimPooliPrivate` → `OqimHovuzIchki`.
- `bolish` → `bo'lish`, `fibonacci` → `fibonachchi`, `togri` → `to'g'ri`.

### Testlar
- Ijobiy: 72 → **74** (matematika + sinov include testlari).
- Salbiy: 47 → **51** (4 ta pending → caught).
- Frontend smoke: 5 → **26** pinlari (Type kind, kompozit, Polimorf, ...).

## [2.1.8] — 2026-05-18

### Til (uz++ kompilyatori)
- **Apostrof identifikatorlarda haqiqatan ishlaydi.** `o'lcham`, `G'oya`,
  `to'lov` kabi haqiqiy o'zbek so'zlari endi `_` ga aylantirilmaydi — C++23
  da U+02BC MODIFIER LETTER APOSTROPHE sifatida saqlanadi (`oʼlcham`).
- **Keyword shadowing.** `yangi`, `bor`, `kasr`, `uzun`, `bosh` kabi
  alias-keywordlarni endi lokal o'zgaruvchi nomi sifatida ishlatish mumkin —
  codegen scope-tracking shadow qilingan nomlarni alias jadvalidan o'tkazmaydi.
- **`} yoki { ... }`** — `yoki` (logical OR) endi `if`-`then`dan keyin
  `else` sifatida ham qabul qilinadi.
- **`funksiya X() o'zgarmas -> T { ... }`** — sinf metodlarida `funksiya`
  uslubida `o'zgarmas` modifier va trailing return type birga ishlaydi.
- **Bitwise operatorlar `& | ^`** — parser ularni binary sifatida tan oladi
  (C-precedence: `&&` va `==` orasida). Unary `&` (address-of) o'zgarmagan.
- **`yangi`/`o'chirish` lookahead** — endi `new`/`delete` sifatida faqat
  type/expression davom etganda olinadi. `butun yangi = 5;` ishlaydi.

### Stdlib
- **`stdlib/matn.uzpp`** — `matn` kutubxonasi to'liq uz++ tilida qayta
  yozildi (626 LOC). Eski C++ `matn.hpp` artefakt sifatida shu fayldan
  generatsiya qilinadi (header-mode transpilatsiyasi).
- **41 funksiya** `matn` da: barcha 27 ta eski + 14 ta yangi:
  `qadar_qirqish`, `qator_son`, `belgi_almashtirish`, `bosh_belgi` /
  `oxir_belgi`, `katta_yokib` / `kichik_yokib`, `formatlash_indeksli`,
  `oxir_topish`, `bir_marta_ajratish`.
- **UTF-8 yordamchilari** (`belgilar_soni`, `belgi_pozitsiyasi`,
  `chapdan_belgi_bo'yicha`, `qism_matn_belgi_bo'yicha`, `belgi_olish`,
  `belgi_uzunligi`) — kirill/o'zbek matnlari uchun kritik.

### Til — `ulash "*.uzpp"`
- Uz++ fayllarni boshqa uz++ fayldan to'g'ridan-to'g'ri `ulash` qilish
  mumkin. Transpilator rekursiv ravishda `.uzpp` ni `.hpp` ga aylantiradi
  (header-mode) va build papkasiga joylashtiradi.
- Header-mode: `#pragma once` + `#ifndef UZPP_GEN_<NAME>_HPP_` guard.
- `manba_joyi` — `std::source_location` ning o'zbekcha aliasi.

### LSP
- **Inlay hints**: `o'zgaruvchan x = ...` dan keyin xulosa qilingan tur ko'rsatiladi.
- **Code actions**: `unused variable` warning uchun ikkita quick-fix:
  `_` prefiks qo'shish yoki e'lonni o'chirish.

### Codegen tuzatishlar
- Char literal default qiymatlar (`' '`) endi mangle qilinmaydi.
- `(pos = s.find(...)) != npos` formulasi to'g'ri qavslar bilan
  emit qilinadi — avval cheksiz tsikl yaratardi.
- Error remap: g++ xato xabarlaridagi `.cpp:N:M` joylar `.uzpp` ga
  qaytariladi (`#line` direktivalari orqali).

### Sinov natijasi
- Local regression: **68/68** (avval 64/64).


## [2.1.1] — 2026-05-06

### VS Code Extension
- MinGW WinLibs GCC 14.2.0 UCRT64 dan to'g'ridan-to'g'ri yuklanadi
- GitHub Release `uzpp-windows-x64.zip` chop etildi (`uzpp.exe` + `stdlib/`)
- Extension v2.1.1 Marketplace'da

## [2.1.0] — 2026-05-06

### VS Code Extension — Zero-friction o'rnatish
- **One-Click Install**: Welcome ekranida bir tugma — barcha komponentlar yuklanadi
- 7 ta yangi buyruq: `F5` ishga tushirish, `Ctrl+F5` qurish, yangi loyiha, C++ kodi ko'rish va boshqalar
- Status bar: kompilyator versiyasi va holati
- Offline o'rnatish: `uz++ fayldan o'rnatish` buyrug'i
- `componentManager.js`: komponentlarni yuklash, o'rnatish, tekshirish
- Wrapper script (`uzpp-run.bat`): MinGW PATH va stdlib CWD ni avtomatik sozlaydi

### Kompilyator / Transpayler
- `StatementList` AST node: vergul bilan ajratilgan o'zgaruvchi e'lonlari
- TypeChecker: `ushlash` blokida o'zgaruvchi ro'yxatdan o'tkaziladi
- `va`/`yoki` mantiqiy operatorlar to'g'ri tahlil qilinadi

### Misollar
- `examples/` papkasida 10 ta to'liq ishlaydigan misol
- CI/CD: `examples/` integratsion testlari qo'shildi

## [2.0.0] — 2025-11-01

### VS Code Extension
- LSP (Language Server Protocol) integratsiyasi
- Real-vaqt diagnostika — yozayotganda xatolar ko'rsatiladi
- Avtoto'ldirish qo'shildi

### Kompilyator
- C++23 transpilatsiya mexanizmi
- Type checker semantik tahlil bilan
- DAP (Debug Adapter Protocol) serveri
- `uzpp lsp`, `uzpp dap` serverlari
- Paket menejeri (`uzpp ornatish`)

## [1.0.0] — 2025-06-01

### Birinchi chiqarilish
- Asosiy sintaksis: o'zgaruvchilar, funksiyalar, sinflar, tsikllar
- Leksik va sintaktik tahlilchi
- Kod generatori (C++ chiqarish)
- VS Code sintaksis yorqinligi
