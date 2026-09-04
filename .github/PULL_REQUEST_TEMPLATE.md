## O'zgarishlar tavsifi

*Bu PR nima qiladi?*

## O'zgarish turi

- [ ] Xato tuzatish (mavjud funksionallikni buzmaydigan o'zgartirish)
- [ ] Yangi xususiyat (mavjud funksionallikni buzmaydigan qo'shimcha)
- [ ] Buzuvchi o'zgarish (`breaking change` — mavjud funksionallikni o'zgartiradi)
- [ ] Hujjatlar (faqat README, hujjatlar)
- [ ] Ishlash optimallashtirish
- [ ] Refaktorlash (funksionallik o'zgarmaydi)

## Tegishli xato/xususiyat

Closes #___

## Test

Eng oson yo'l — hammasini bitta buyruq bilan tekshirish:
`.un_tests.ps1` (Windows). CI aynan shu to'plamni yugurtiradi.

- [ ] `./build/uzpp_frontend_tests` — chiqish kodi 0 (oxirgi qatorga emas,
      CHIQISH KODIGA qarang: `assert` birinchi xatoda to'xtaydi)
- [ ] Yangi integratsiya testi qo'shilgan: `tests/test_*.uzpp`
- [ ] Barcha mavjud `tests/*.uzpp` testlari o'tadi
- [ ] `bash tests/negative/run.sh` — `missed (regression!): 0`
- [ ] `bash tests/formatlash_xavfsizlik.sh` — `BUZILDI: 0`
- [ ] `python3 tests/darslik_tekshir.py <uzpp>` va `--qurish` — 0 yiqilgan
- [ ] `misollar/` papkasidagi misollar hali ham ishlaydi

## Tekshiruv ro'yxati

- [ ] Kod loyiha uslubiga mos
- [ ] O'z-o'zini tekshirish amalga oshirildi
- [ ] Murakkab joylarda izohlar qo'shildi
- [ ] Hujjatlar yangilandi (agar kerak bo'lsa)
- [ ] Yangi xato kiritilmadi

## Qo'shimcha izohlar

*Tekshiruvchi bilishi kerak bo'lgan narsalar.*
