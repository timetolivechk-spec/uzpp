# Kutilayotgan salbiy testlar | Pending negative tests

Bu papkadagi `.uzpp` fayllari **kompilyatorda mavjud kamchilik tufayli hozir
xatoga olib kelmaydi**. Ularni asosiy `tests/negative/` ga ko'chirish kerak,
qachonki tegishli kamchilik tuzatilsa.

These `.uzpp` files **don't currently fail to compile** because of a known gap
in the compiler. Move each one back up into `tests/negative/` once the
underlying bug is fixed.

## Hozirgi pending fayllar | Current pending

### `parse_keyword_as_var.uzpp`

Parser `butun agar = 5;` kabi e'lonni qabul qiladi — `agar` (if) kalit so'zini
o'zgaruvchi nomi sifatida ishlatishga ruxsat beradi.

The parser accepts `butun agar = 5;` — it allows reserved keywords like `agar`
(`if`) to be used as variable names. Reserved-word check is missing.

**Tuzatish | Fix:** [src/parser.cpp](../../../src/parser.cpp) — variable name
identifier check should reject any token whose value is in the keyword table.

### `parse_match_no_cases.uzpp`

Bo'sh `moslash (x) {}` (case'larsiz switch) qabul qilinadi — bu ehtimol C++
grammatikasi nuqtai nazaridan to'g'ri (chunki C++ ham bo'sh switch'ga ruxsat
beradi), lekin uz++ uchun foydali emas.

Empty `moslash (x) {}` (switch with no cases) is accepted. C++ technically
allows this too, so this may not be a "bug" so much as a quality-of-life
warning we don't yet emit.

**Tuzatish | Fix:** [src/type_checker.hpp](../../../src/type_checker.hpp) — emit
a warning (not error) for switches with no cases.

### `type_division_by_zero_const.uzpp`

Kompilyator `butun x = 10 / 0;` ni qabul qiladi. g++ bunda faqat ogohlantirish
beradi, xato emas, shuning uchun exit code 0.

The compiler accepts `butun x = 10 / 0;`. g++ only warns about it, doesn't
error, so the build succeeds with warnings.

**Tuzatish | Fix:** Constant-fold detection in [src/type_checker.hpp](../../../src/type_checker.hpp)
or [src/codegen.cpp](../../../src/codegen.cpp). Treat division by literal 0 as
a hard error.
