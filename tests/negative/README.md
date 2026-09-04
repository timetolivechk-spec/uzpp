# Salbiy testlar

Bu papkadagi har bir `.uzpp` fayli **kompilyatsiya jarayonida xatoga olib kelishi
kerak**. Agar `uzpp qurish` ulardan biri uchun muvaffaqiyatli yakunlanadi
(exit code = 0), bu — kompilyatorda xato yoki kerakli tekshiruv yo'qligini
bildiradi.

## Ishga tushirish

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

Tasniflash:
- `path_traversal` — `ulash` orqali xavfli yo'l (Phase 1 himoyasi)
- `parse` — parser xatosi (ochiq qavslar, noto'g'ri sintaksis)
- `lexer` — leksema darajasidagi xato (yopilmagan satr, va h.k.)
- `type` — tip tekshiruvchidan xato
- `arg_count` — funksiya argumentlari soni mos kelmasa
- `cpp_level` — uz++ qabul qiladi, ammo C++ kompilyatori rad etadi
- `unknown_var` — noma'lum o'zgaruvchi (hozir faqat ogohlantirish)

## Bilingan kamchiliklar

Hozircha "kutilayotgan" testlar yo'q — barcha salbiy testlar asosiy
runner tomonidan bajariladi va hammasi xatoni tutadi (59/59).

Agar `TypeChecker` biror xatoni faqat `ogohlantirish` deb chiqarsa va
exit code 0 qaytarsa, test hozircha o'tib ketadi. Bunday holatda testni
`tests/negative/` da qoldiring, lekin birinchi qatorda sababni yozib
qo'ying — runner qaysi fayl xatoni TUTMAGANINI "missed (regression!)"
sifatida ko'rsatadi va bu son 0 bo'lishi shart.
