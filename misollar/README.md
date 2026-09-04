# Misollar

Har bir fayl — mustaqil, ishlaydigan dastur. Nusxa oling, o'zgartiring,
ishga tushiring:

```bash
uzpp ishga-tushirish misollar/01_salom_dunyo.uzpp
```

Tartib oddiydan murakkabga qarab tuzilgan — birinchidan boshlab ketma-ket
o'qib chiqsangiz, tilning asosiy imkoniyatlarini ko'rasiz.

| Fayl | Nima ko'rsatadi |
|---|---|
| `01_salom_dunyo.uzpp` | Eng kichik dastur: `asosiy`, `yozish`, `qator_oxiri` |
| `02_fizzbuzz.uzpp` | Sikllar, shartlar, qoldiqli bo'lish |
| `03_rekursiya.uzpp` | Rekursiya: faktorial, Fibonachchi, Hanoy minoralari |
| `04_sinflar.uzpp` | Sinflar, konstruktor, meros, virtual metodlar |
| `05_kolleksiyalar.uzpp` | `vektor`, `lug'at`, `to'plam_noyob` va ular ustidagi amallar |
| `06_xatolik_boshqaruvi.uzpp` | `urinish`/`ushlash`, `Natija<T, E>`, `Tanlov<T>` |
| `07_matn_ishlash.uzpp` | `uzpp::Matn` — bo'lish, birlashtirish, qidirish, UTF-8 |
| `08_json_ishlash.uzpp` | `uzpp::Json` — o'qish, yozish, ichma-ich tuzilmalar |
| `09_kop_oqimlilik.uzpp` | `oqim`, `atomik`, `OqimHovuz` (oqim havzasi) |
| `10_ilgor_dasturlash.uzpp` | Shablonlar, lambda, `tushuncha` (concepts) |
| `11_ozgaruvchilar.uzpp` | Turlar, `o'zgaruvchan`, `o'zgarmas`, qamrov |
| `12_funksiyalar.uzpp` | Parametrlar, standart qiymatlar, ortiqcha yuklash |
| `13_natija.uzpp` | `Natija<T, E>` bilan xatolikni qiymat sifatida uzatish |
| `14_moslik.uzpp` | `moslash` — bir nechta yo'nalishdan tanlash |
| `15_oyna.uzpp` | GUI (CopperSpice) — **tashqi kutubxona kerak**, pastga qarang |

## 15_oyna.uzpp — GUI misoli

Bu yagona misol tashqi kutubxonasiz ishlamaydi: unga
[CopperSpice](https://www.copperspice.com/) (Qt ning ochiq forki) kerak.
Qolgan 14 tasi hech qanday qo'shimcha o'rnatishsiz ishlaydi.

O'rnatgandan keyin (Windows misoli):

```powershell
$env:PATH = "C:/msys64/mingw64/bin;C:/CopperSpice/bin;$env:PATH"
uzpp ishga-tushirish misollar/15_oyna.uzpp --bare `
    -I C:/CopperSpice/include `
    -I C:/CopperSpice/include/QtCore `
    -I C:/CopperSpice/include/QtGui `
    -l C:/CopperSpice/lib/libCsCore2.1.dll.a `
    -l C:/CopperSpice/lib/libCsGui2.1.dll.a
```

- `--bare` — uz++ ning standart runtime'ini o'chiradi (u konsol dasturlari
  uchun mo'ljallangan);
- `-I` va `-l` — kutubxona yo'llarini to'g'ridan-to'g'ri kompilyatorga uzatadi.

Batafsil: [darslikning 15-bobi](../docs/darslik/15-gui.md).

## Ko'proq kod

Til imkoniyatlarining to'liq ro'yxati va har biriga izoh —
[darslikda](../docs/darslik/00-mundarija.md). `tests/` papkasida ham 89 ta
kichik dastur bor: ular til imkoniyatlarini bittalab tekshiradi va namuna
sifatida o'qishga yaroqli.
