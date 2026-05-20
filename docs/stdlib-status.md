# uz++ standart kutubxona — modullar holati

> Bu hujjat har bir `stdlib/*.hpp` modulining haqiqiy holatini aks ettiradi.
> Maqsad: qaysi modullarga ishonish mumkinligi haqida halol xabardor qilish.

## Holat belgilari

- **TO'LIQ** — ishlaydigan to'liq implementatsiya, `production` uchun yaroqli.
- **QISMAN** — asosiy qismi ishlaydi, ammo ba'zi funksiyalar to'ldirilmagan
  yoki cheklangan.
- **TAJRIBA** — qisqa, bo'sh yoki joy egallash uchun. Kelajakda ishlash uchun
  rejalashtirilgan. Har qanday vaqtda o'zgarishi yoki olib tashlanishi mumkin.

## Holatlar jadvali

| Modul | Holat | Qator | Tavsifi |
|---|---|---|---|
| `stdlib/uzpp_runtime.hpp` | TO'LIQ | 368 | `Natija`/`Tanlov` turlari, `OqimHovuz`, korutinalar, tur aliaslari |
| `stdlib/matematika.hpp` | TO'LIQ | 312 | 200+ matematik funksiya: trig, 2D/3D vektor, matritsa, tasodifiy, statistika |
| `stdlib/matn.hpp` | TO'LIQ | 276 | Matn `split`/`join`/qidiruv/almashtirish/holat/`trim`/`pad`; `regex` qo'llab-quvvatlash |
| `stdlib/json.hpp` | TO'LIQ | 325 | To'liq JSON `parser`/`serializer` tur ishlovi bilan |
| `stdlib/fayl_tizimi.hpp` | TO'LIQ | 303 | Fayl I/O, papka amallari, oddiy JSON o'qish, binar serializatsiya |
| `stdlib/kripto.hpp` | TO'LIQ | 277 | SHA1 + SHA256 (to'liq), Base64, Base64URL, HMAC-SHA256, JWT, XOR shifr |
| `stdlib/malumotlar_bazasi.hpp` | TO'LIQ | 552 | SQLite ORM korutinalar bilan; MySQL shartli; `query builder` |
| `stdlib/jurnal.hpp` | TO'LIQ | 94 | Logging (DEBUG/INFO/WARN/ERROR) `profiler` bilan; embedded qo'llab-quvvatlash |
| `stdlib/tarmoq.hpp` | TO'LIQ | 573 | TCP/UDP `sockets`, HTTP server, WebSocket, `Router`, cURL `client` |
| `stdlib/oyna.hpp` | TO'LIQ | 675 | OpenGL 2D/3D rendering, `input`, `.obj loader`, teksturlar (Windows/Linux) |
| `stdlib/vaqt.hpp` | TO'LIQ | 118 | To'liq vaqt/sana API'lari, vaqt zonasi; C++20 `chrono` va fallback |
| `stdlib/xavfsizlik.hpp` | TO'LIQ | 62 | `std::expected`/`optional` o'rovi `UZPP_TRY` makrosi bilan |
| `stdlib/xotira.hpp` | TO'LIQ | 51 | `shared_ptr` aliaslari + arena allokator to'g'ri tekislash bilan |
| `stdlib/tizim.hpp` | TO'LIQ | 89 | Platforma aniqlash, CLI argument tahlili, jarayonni ishga tushirish |
| `stdlib/fazo.hpp` | TO'LIQ | 91 | Matritsa sinfi (2D add/mul/transpose) + statistika (mean/median) |
| `stdlib/grafika/vidjetlar.hpp` | TO'LIQ | 154 | Tugma, matn, kiritish, vertikal layout vidjetlari; hover/focus; animatsiyalar |
| `stdlib/xatoliklar.hpp` | QISMAN | 113 | Rust uslubidagi xatolik turlari; signal handler'lar `stub` |
| `stdlib/asinxron.hpp` | QISMAN | 85 | C++20+ korutina turlari; faqat legacy fallback; embedded stub'lar |
| `stdlib/veb_ui.hpp` | QISMAN | 128 | HTML/HTMX generatsiyasi XSS escape bilan; Redux boilerplate to'liq emas |
| `stdlib/kesh.hpp` | QISMAN | 45 | Matn xotira keshi; embedded platforma `stub` |
| `stdlib/koinot.hpp` | QISMAN | 37 | Web `framework` skeleti; korutina qaytishi to'liq emas |
| `stdlib/sinov.hpp` | QISMAN | 103 | `Unit test` `framework`; benchmarking ishlaydi lekin mikro-optimallashtirilgan |
| `stdlib/tarjima.hpp` | QISMAN | 42 | i18n asoslari fallback bilan; JSON yuklash xato fayllar uchun amalga oshmagan |
| `stdlib/suniy_intellekt.hpp` | TAJRIBA | 48 | Ollama LLM uchun shartli curl o'rovi; curl yo'q bo'lsa fallback yo'q |
| `stdlib/apparat.hpp` | TAJRIBA | 64 | Arduino pin/serial mock'lari; PC'da o'chirilgan |
| `stdlib/platforma.hpp` | TAJRIBA | 89 | Faqat platforma aniqlash makrolari; funksiya yo'q |
| `stdlib/tarmoq_tuzilmalar.hpp` | TAJRIBA | 38 | Faqat `Request`/`Response` struct'lari; tarmoq mantig'i yo'q |
| `stdlib/grafika/hodisalar.hpp` | TAJRIBA | 16 | Faqat `input state` struct'lari |
| `stdlib/grafika/reaktiv.hpp` | TAJRIBA | 29 | Oddiy `observer pattern`; rendering integratsiyasi yo'q |
| `stdlib/grafika/tuzilmalar.hpp` | TAJRIBA | 19 | Ma'lumot tuzilmalari (nuqtalar, ranglar, 3D model); mantiq yo'q |

## Self-hosted modullar

Quyidagi modullar to'g'ridan-to'g'ri uz++ tilida yozilgan (`stdlib/<nomi>.uzpp`)
va `header-mode transpile` orqali `.hpp` ga aylantirilgan:

- `matn`, `xatoliklar`, `vaqt`, `matematika`, `sinov`, `jurnal`

`.uzpp` manba — yagona haqiqat manbai. `.hpp` esa generatsiya artefakti —
qo'lda tahrir qilinmasligi shart.

## Foydalanish bo'yicha tavsiyalar

- **TO'LIQ** modullar `production` uchun: ishonch bilan ishlatishingiz mumkin.
- **QISMAN** modullarini ishlatish mumkin, ammo qaysi qismlari to'liq
  emasligini hujjat va manba kodidan tekshiring.
- **TAJRIBA** modullariga ishonmang. Har qanday vaqtda o'zgarishi yoki olib
  tashlanishi mumkin. Faqat tajriba uchun.

## TAJRIBA modulning manba kodida

TAJRIBA modullari ushbu izoh bilan boshlanadi:

```cpp
// holati: TAJRIBA — bu modul to'liq ishlamaydi va o'zgarishi mumkin.
```

So'nggi audit: 2026-05-20 (v2.2.0 release).
