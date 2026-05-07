# Salbiy testlar | Negative tests

Bu papkadagi har bir `.uzpp` fayli **kompilyatsiya jarayonida xatoga olib kelishi
kerak**. Agar `uzpp qurish` ulardan biri uchun muvaffaqiyatli yakunlanadi
(exit code = 0), bu — kompilyatorda xato yoki kerakli tekshiruv yo'qligini
bildiradi.

Each `.uzpp` file in this directory **must fail to compile**. If `uzpp qurish`
returns exit code 0 for any of them, that means the compiler missed an error
check — a regression.

## Ishga tushirish | Running

```bash
# POSIX shell
bash tests/negative/run.sh

# PowerShell (Windows)
pwsh tests/negative/run.ps1
```

Skript har bir `.uzpp` faylga `uzpp qurish` ni ishga tushiradi va exit kodi
0 bo'lsa fail (regression) deb belgilaydi.

## Format

Har bir test fayli birinchi qatorda izoh shaklida kutilgan xato kategoriyasini
ko'rsatadi:

```uzpp
// XATOLIK_KUTILMOQDA: <category>: <short description>
```

Tasniflash | Categories:
- `path_traversal` — `ulash` orqali xavfli yo'l (Phase 1 himoyasi)
- `parse` — parser xatosi (ochiq qavslar, noto'g'ri sintaksis)
- `lexer` — leksema darajasidagi xato (yopilmagan satr, va h.k.)
- `type` — tip tekshiruvchidan xato
- `arg_count` — funksiya argumentlari soni mos kelmasa
- `cpp_level` — uz++ qabul qiladi, ammo C++ kompilyatori rad etadi
- `unknown_var` — noma'lum o'zgaruvchi (hozir faqat ogohlantirish)

## Bilingan kamchiliklar | Known compiler gaps

Hozirgi `TypeChecker` ba'zi tip xatolarini faqat `ogohlantirish` (warning) deb
chiqaradi va exit code 0 qaytaradi. Bunday testlar
[`tests/negative/pending/`](pending/) papkasida saqlanadi va asosiy runner
ulardan o'tib ketadi. Tegishli kamchilik tuzatilganidan keyin, fayl yuqori
darajaga ko'chiriladi.

Tests for known compiler gaps live in [`pending/`](pending/) and are skipped
by the main runner. When the underlying bug is fixed, move the file back up
into the top-level `tests/negative/` directory.
