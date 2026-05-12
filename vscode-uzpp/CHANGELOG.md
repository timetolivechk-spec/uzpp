# Changelog

## [2.1.2] — 2026-05-12

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
