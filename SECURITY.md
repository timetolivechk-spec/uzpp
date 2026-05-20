# Xavfsizlik siyosati

## Xavfsizlik xatoligi topdingizmi?

Xavfsizlik xatoligini **ommaviy GitHub Issue sifatida ochmang**. Buning o'rniga
quyidagi yopiq kanallardan biriga xabar bering, shunda biz uni tuzatib, javob
bermay turib chiqmaymiz.

**Murojaat:** [GitHub Security Advisory](https://github.com/timetolivechk-spec/uzpp/security/advisories/new)
([repo'dagi Security yorlig'i orqali](https://github.com/timetolivechk-spec/uzpp/security)).

Iltimos, xabaringizda quyidagilarni ko'rsating:

- Hujum sahnasi (qaysi buyruq, qaysi fayl).
- Reproduktsiya qadamlari (minimal `.uzpp` fayl yoki buyruq).
- Mumkin bo'lgan zarar (masofadan kod ijro etish? mahalliy fayl o'qish?).
- Sizningcha, qanday tuzatish kerak.

## Qo'llab-quvvatlanadigan versiyalar

| Versiya | Qo'llab-quvvatlanadi | Izoh |
|---------|---------------------|------|
| `2.x` (joriy) | ✅ | Faol rivojlanishda. |
| `1.x` | ❌ | Yangilanishlar yo'q — `2.x` ga o'ting. |

## Javob vaqti

| Bosqich | Maqsad |
|---------|--------|
| Tasdiqlash | 72 soat ichida |
| Birinchi tahlil | 7 kun ichida |
| Tuzatish (kritik) | 30 kun ichida |
| Tuzatish (o'rtacha) | 90 kun ichida |
| Ommaviy disclosure | Tuzatishdan keyin 14 kun |

Loyiha bitta dasturchi tomonidan boshqarilmoqda — chuqur sabr-toqat uchun
oldindan rahmat.

## Hozirgi ma'lum xavfsizlik chegaralari

uz++ **kompilyator va transpilyatsiya vositasi** sifatida ishlatiladi: u
`.uzpp` faylini C++ ga aylantiradi va keyin g++/clang++ ni chaqiradi. Hujum
yuzasi:

- ✅ **Path traversal in `ulash`** — yopiq (commit `a12a84a`,
  [src/parser.cpp::parseIncludeStatement](src/parser.cpp), negative tests
  [tests/negative/path_traversal_*.uzpp](tests/negative/)).
- ✅ **Shell injection in compiler/uzpm/run** — yumshatildi (commit
  `a12a84a`, `isShellSafePath`/`isValidPackageName`/`isValidPackageUrl` in
  [src/main.cpp](src/main.cpp)). To'liq `system()` o'rniga `posix_spawn` ga
  o'tish kelajakda rejalashtirilgan.
- ⚠️ **Untrusted `.uzpp` ni qurish** — sizning g++ ga shu kompilyatsiya
  qilinishini bildiradi. Notanish `.uzpp` fayllarini sandbox tashqarisida
  qurish — **doim** xavfli, hatto tilning o'zi xavfsiz bo'lsa ham. Shu sababli
  notanish `.uzpp` fayllarni faqat izolatsiya qilingan muhitda quring.
- ⚠️ **`uzpm install`** — paket fayllari `https://` orqali yuklab olinadi.
  Markaziy reestr `uzlang/uzpm-registry` SHA256 sumlarni tekshiradi.
  Boshqa manbalar uchun siz o'zingiz mas'ulsiz.

---

