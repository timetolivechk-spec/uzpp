# uz++ standart kutubxona — modullar holati | stdlib status

> Bu hujjat har bir `stdlib/*.hpp` modulining haqiqiy holatini aks ettiradi.
> Maqsad: foydalanuvchilarni qaysi modullarga ishonish mumkinligi haqida
> halol xabardor qilish.
>
> This document reports the actual state of every `stdlib/*.hpp` module so
> users know which modules are production-ready vs experimental.

## Holat belgilari | Status legend

- **REAL** — ishlaydigan to'liq implementatsiya, production uchun yaroqli.
  Working full implementation, suitable for production.
- **PARTIAL** — asosiy qismi ishlaydi, ammo ba'zi funksiyalar to'ldirilmagan
  yoki cheklangan. Core works, but some functions are incomplete or limited.
- **EXPERIMENTAL** — qisqa, bo'sh yoki to'liq joy egallash uchun.
  Kelajakda ishlash uchun rejalashtirilgan. Tiny / mostly empty / placeholder.
  Subject to change or removal.

## Holatlar jadvali | Status table

| Module | Status | Lines | Description |
|---|---|---|---|
| `stdlib/uzpp_runtime.hpp` | REAL | 368 | Result/Option types, thread pool, coroutines, type aliases. |
| `stdlib/matematika.hpp` | REAL | 312 | 200+ math functions: trig, vectors 2D/3D, matrices, random, stats. |
| `stdlib/matn.hpp` | REAL | 276 | String split/join/search/replace/case/trim/pad; regex support. |
| `stdlib/json.hpp` | REAL | 325 | Full JSON parser/serializer with proper type handling. |
| `stdlib/fayl_tizimi.hpp` | REAL | 303 | File I/O, directory ops, simple JSON read, binary serialization. |
| `stdlib/kripto.hpp` | REAL | 277 | SHA1 + SHA256 (full impl), Base64, Base64URL, HMAC-SHA256, JWT, XOR cipher. |
| `stdlib/malumotlar_bazasi.hpp` | REAL | 552 | SQLite ORM with coroutines; MySQL conditional support; query builder. |
| `stdlib/jurnal.hpp` | REAL | 94 | Logging (DEBUG/INFO/WARN/ERROR) with profiler; embedded support. |
| `stdlib/tarmoq.hpp` | REAL | 573 | TCP/UDP sockets, HTTP server, WebSocket, Router, cURL client. |
| `stdlib/oyna.hpp` | REAL | 675 | OpenGL 2D/3D rendering, input handling, .obj loader, textures (Windows/Linux). |
| `stdlib/vaqt.hpp` | REAL | 118 | Full time/date APIs with timezone support; C++20 chrono and fallback. |
| `stdlib/xavfsizlik.hpp` | REAL | 62 | std::expected/optional wrappers with UZPP_TRY macro. |
| `stdlib/xotira.hpp` | REAL | 51 | shared_ptr aliases + arena allocator with proper alignment. |
| `stdlib/tizim.hpp` | REAL | 89 | Platform detection, CLI argument parsing, process execution. |
| `stdlib/fazo.hpp` | REAL | 91 | Matrix class (2D add/mul/transpose) + statistics (mean/median). |
| `stdlib/grafika/vidjetlar.hpp` | REAL | 154 | Button, text, input, vertical layout widgets; hover/focus logic; animations. |
| `stdlib/xatoliklar.hpp` | PARTIAL | 113 | Rust-style error types; signal handlers are stubbed. |
| `stdlib/asinxron.hpp` | PARTIAL | 85 | Coroutine types for C++20+; legacy fallback only; embedded stubs. |
| `stdlib/veb_ui.hpp` | PARTIAL | 128 | HTML/HTMX generation with XSS escaping; Redux boilerplate incomplete. |
| `stdlib/kesh.hpp` | PARTIAL | 45 | String memory cache; embedded platform stubs. |
| `stdlib/koinot.hpp` | PARTIAL | 37 | Web framework skeleton; coroutine return incomplete. |
| `stdlib/sinov.hpp` | PARTIAL | 103 | Unit test framework; benchmarking works but micro-optimized. |
| `stdlib/tarjima.hpp` | PARTIAL | 42 | i18n basics with fallback; JSON load unimplemented for malformed files. |
| `stdlib/suniy_intellekt.hpp` | EXPERIMENTAL | 48 | Conditional curl wrapper for Ollama LLM; no fallback if curl missing. |
| `stdlib/apparat.hpp` | EXPERIMENTAL | 64 | Arduino pin/serial mocks; disabled on PC. |
| `stdlib/platforma.hpp` | EXPERIMENTAL | 89 | Platform detection macros only; no functionality. |
| `stdlib/tarmoq_tuzilmalar.hpp` | EXPERIMENTAL | 38 | Request/response structs only; no actual network logic. |
| `stdlib/grafika/hodisalar.hpp` | EXPERIMENTAL | 16 | Input state structs only. |
| `stdlib/grafika/reaktiv.hpp` | EXPERIMENTAL | 29 | Simple observer pattern; no rendering integration. |
| `stdlib/grafika/tuzilmalar.hpp` | EXPERIMENTAL | 19 | Data structures (points, colors, 3D model); no logic. |

## Foydalanish bo'yicha tavsiyalar | Usage recommendations

- **REAL** modullari production-ready: ishonch bilan ishlatishingiz mumkin.
- **PARTIAL** modullarini ishlatish mumkin, ammo qaysi qismlari to'liq emasligini
  hujjat va manba kodidan tekshiring (yuqoridagi tavsifga qarang).
- **EXPERIMENTAL** modullariga ishonmang. Ular har qanday vaqtda o'zgarishi yoki
  olib tashlanishi mumkin. Faqat tajriba uchun ishlating.

## Status maydoni | Status field

EXPERIMENTAL modullar ushbu izoh bilan boshlanadi:

```cpp
// status: EXPERIMENTAL — bu modul to'liq ishlamaydi va o'zgarishi mumkin.
// This module is incomplete and subject to change.
```

Auditor: 2026-05-07 (Phase 2 — quality and honesty pass).
