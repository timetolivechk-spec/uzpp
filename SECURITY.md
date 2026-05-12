# Xavfsizlik siyosati | Security Policy | Политика безопасности

## 🇺🇿 O'zbekcha

### Xavfsizlik xatoligi topdingizmi?

Xavfsizlik xatoligini **ommaviy GitHub Issue sifatida ochmang**. Buning o'rniga
quyidagi yopiq kanallardan biriga xabar bering, shunda biz uni tuzatib, javob
bermay turib chiqmaymiz.

**Murojaat | Contact:** [GitHub Security Advisory](https://github.com/timetolivechk-spec/uzpp/security/advisories/new)
([repo'dagi Security yorlig'i orqali](https://github.com/timetolivechk-spec/uzpp/security)).

Iltimos, xabaringizda quyidagilarni ko'rsating:

- Hujum sahnasi (qaysi buyruq, qaysi fayl).
- Reproduktsiya qadamlari (minimal `.uzpp` fayl yoki buyruq).
- Mumkin bo'lgan zarar (masofadan kod ijro etish? mahalliy fayl o'qish?).
- Sizningcha, qanday tuzatish kerak.

### Qo'llab-quvvatlanadigan versiyalar | Supported versions

| Versiya | Qo'llab-quvvatlanadi | Izoh |
|---------|---------------------|------|
| `2.x` (joriy) | ✅ | Faol rivojlanishda. |
| `1.x` | ❌ | Yangilanishlar yo'q — `2.x` ga o'ting. |

### Javob vaqti | Response timeline

| Bosqich | Maqsad |
|---------|--------|
| Tasdiqlash | 72 soat ichida |
| Birinchi tahlil | 7 kun ichida |
| Tuzatish (kritik) | 30 kun ichida |
| Tuzatish (o'rtacha) | 90 kun ichida |
| Ommaviy disclosure | Tuzatishdan keyin 14 kun |

Loyiha bitta dasturchi tomonidan boshqarilmoqda — chuqur sabr-toqat uchun
oldindan rahmat.

### Hozirgi ma'lum xavfsizlik chegaralari | Current known limitations

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

## 🇷🇺 Русский

### Нашли уязвимость?

**Не открывайте публичный GitHub Issue для уязвимостей**. Вместо этого
сообщите через приватный канал, чтобы мы успели починить до раскрытия.

**Канал связи:** [GitHub Security Advisory](https://github.com/timetolivechk-spec/uzpp/security/advisories/new)
(вкладка Security в репо).

В отчёте укажите:

- Сценарий атаки (какая команда, какой файл).
- Шаги воспроизведения (минимальный `.uzpp` файл или команда).
- Возможный ущерб (RCE? чтение локальных файлов? path traversal?).
- Предлагаемое исправление, если есть идея.

### Поддерживаемые версии

| Версия | Поддержка | Комментарий |
|---|---|---|
| `2.x` (текущая) | ✅ | В активной разработке. |
| `1.x` | ❌ | Без обновлений — переходите на `2.x`. |

### Сроки ответа

| Этап | Цель |
|---|---|
| Подтверждение получения | 72 часа |
| Первичный анализ | 7 дней |
| Фикс (critical) | 30 дней |
| Фикс (medium) | 90 дней |
| Публичное раскрытие | через 14 дней после фикса |

Проект ведёт один разработчик — заранее благодарю за терпение.

### Текущие известные границы безопасности

- ✅ **Path traversal в `ulash`** — закрыто (commit `a12a84a`).
- ✅ **Shell injection** — смягчено (commit `a12a84a`); полный переход на
  `posix_spawn` запланирован.
- ⚠️ **Сборка непроверенного `.uzpp`** — равноценна запуску чужого g++.
  Стройте только в sandbox.

---

## 🇬🇧 English

### Found a vulnerability?

**Do not open a public GitHub Issue for security bugs.** Use a private channel
so we can patch before disclosure.

**Reporting channel:** [GitHub Security Advisory](https://github.com/timetolivechk-spec/uzpp/security/advisories/new)
(Security tab on the repo).

Please include:

- Attack scenario (which command, which file).
- Reproduction steps (minimal `.uzpp` file or command line).
- Potential impact (RCE? local file read? path traversal? denial of service?).
- A suggested fix, if you have one.

### Supported versions

| Version | Supported | Notes |
|---|---|---|
| `2.x` (current) | ✅ | Actively developed. |
| `1.x` | ❌ | No updates — please upgrade to `2.x`. |

### Response timeline

| Stage | Target |
|---|---|
| Acknowledgement | within 72 hours |
| Initial triage | within 7 days |
| Fix (critical) | within 30 days |
| Fix (medium) | within 90 days |
| Public disclosure | 14 days after fix |

This is a solo-maintained project — thanks in advance for your patience.

### Currently known security boundaries

uz++ is a **compiler/transpiler**: it converts `.uzpp` to C++ and then invokes
g++/clang++. The attack surface is therefore:

- ✅ **Path traversal in `ulash`** (the include directive) — fixed in commit
  `a12a84a`. See [src/parser.cpp::parseIncludeStatement](src/parser.cpp) and
  the negative tests under [tests/negative/path_traversal_*.uzpp](tests/negative/).
- ✅ **Shell injection in compiler / uzpm / run paths** — mitigated in commit
  `a12a84a` via `isShellSafePath`, `isValidPackageName`, and
  `isValidPackageUrl` in [src/main.cpp](src/main.cpp). A future hardening pass
  will replace `system()`/`popen()` with `posix_spawn` / `CreateProcess`
  argument arrays so escaping is no longer a concern at all.
- ⚠️ **Building an untrusted `.uzpp` file** is equivalent to running someone
  else's `g++` against generated C++. Always build untrusted code in a sandbox
  (container, VM), no matter how safe the language itself looks.
- ⚠️ **`uzpm install`** — packages are downloaded over `https://`. The central
  `uzlang/uzpm-registry` registry checks SHA-256 sums. Other sources are at
  your own risk.

### What is NOT in scope

- The behaviour of arbitrary user code that you compile. uz++ programs run
  with the privileges of the user that runs them.
- Bugs in the underlying C++ compiler (g++, clang++). Report those upstream.
- VSCode extension reaching out to `https://api.github.com` to look for
  installer updates — that is by design and documented.

Thank you for helping keep uz++ users safe.
