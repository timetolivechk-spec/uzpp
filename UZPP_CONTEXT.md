# uz++ Project Context

> Контекстный документ для передачи состояния проекта между сессиями AI.
> Обновлять после каждой завершённой крупной задачи.
> Последнее обновление: май 2026

---

## Кто и что

**Автор:** соло-разработчик из Узбекистана + AI-агент (Claude Pro / Claude Code).

**Проект:** uz++ — язык программирования с узбекским синтаксисом, транспилирующийся в C++23.

**Репозиторий:** https://github.com/timetolivechk-spec/uzpp

**VSCode Marketplace:** `uzpp.uzpp` v2.1.1 — опубликовано.

---

## Видение и стратегия

**Целевая аудитория (приоритет):**
1. Студенты и школьники Узбекистана (главная).
2. Узбекоязычные разработчики в регионах.
3. Глобальная аудитория интересующихся языковым дизайном (вторичная).

**Стратегический путь:** гибрид — образовательный язык для Узбекистана + уникальные мировые фичи.

**Ключевое позиционирование:** «Современный C++ с узбекским синтаксисом и продвинутыми фичами. Лёгкий старт, глубина по мере роста.»

**Уникальные фичи в планах (мировой уровень):**
- Bilingual mode — uz++ ↔ C++ проекция (один файл, два представления).
- Hot reload через `dlopen`/`LoadLibrary`.
- Refinement types — минимальная версия с Z3.

---

## Архитектура

**Pipeline компиляции:**

```
.uzpp → Lexer → Parser → TypeChecker → CodeGen → C++23 → g++/clang++ → binary
```

**Модули компилятора (src/):**
- `lexer.cpp/h` — токенизация, узбекские идентификаторы, тройные строки, f-strings.
- `parser.cpp/h` — рекурсивный нисходящий парсер, error recovery через синхронизацию.
- `ast.h` — 40+ узлов с Visitor pattern. Есть legacy-иерархия для совместимости.
- `type_checker.hpp` — 2-pass семантический анализ, базовый type inference для примитивов.
- `codegen.cpp/h` — генерация C++23, вставка `#line` директив для отладки.
- `formatter.cpp/h` — авто-форматирование.
- `lsp_server.cpp/h` — LSP: completion, hover, diagnostics, definition.
- `dap_server.cpp/h` — DAP через GDB MI mode (экспериментальный).
- `docgen.hpp` — генератор документации из `///` комментариев.
- `main.cpp` — CLI на ~1200 строк с пакетным менеджером.

**CLI команды (узбекские):**
`ishga-tushirish`, `qurish`, `transpile`, `tekshirish`, `format`, `lsp`, `dap`, `--test`, `--bench`, `--init`, `--install`, `--update`, `--docs`, `--target host|windows|linux|wasm`.

**Stdlib (stdlib/):** header-only C++23, 30+ модулей. Часть полностью реализована, часть — заглушки `// to be implemented`. Требует аудита.

**Аудит stdlib (см. [docs/stdlib-status.md](docs/stdlib-status.md), 2026-05-07):**
- **REAL (15):** `uzpp_runtime`, `matematika`, `matn`, `json`, `fayl_tizimi`, `kripto` (SHA1+SHA256+Base64+HMAC+JWT+XOR — больше чем считалось!), `malumotlar_bazasi` (SQLite ORM), `jurnal`, `tarmoq` (TCP/UDP/HTTP/WebSocket), `oyna` (OpenGL 2D/3D), `vaqt`, `xavfsizlik`, `xotira`, `tizim`, `fazo`, `grafika/vidjetlar`.
- **PARTIAL (7):** `xatoliklar`, `asinxron`, `veb_ui`, `kesh`, `koinot`, `sinov`, `tarjima`.
- **EXPERIMENTAL (7, помечены):** `suniy_intellekt`, `apparat`, `platforma`, `tarmoq_tuzilmalar`, `grafika/hodisalar`, `grafika/reaktiv`, `grafika/tuzilmalar`.

**Пакетный менеджер uzpm:** реальный, с центральным реестром `uzpm-registry.uz`, SHA256 проверкой, локальным кешем `~/.uzpp/packages/`.

---

## Текущее состояние

**Что работает:**
- Полный pipeline `.uzpp` → бинарник.
- Все 10 примеров из `misollar/` компилируются и запускаются.
- 23 теста `.test.uzpp` проходят (формат `// OUT: "..."`).
- 25 frontend unit-тестов в [tests/frontend_smoke.cpp](tests/frontend_smoke.cpp).
- 27 negative-тестов в [tests/negative/](tests/negative/) с runner'ом.
- VSCode расширение опубликовано: подсветка, LSP, DAP-клиент, status bar, F5 запуск.
- VSCode v2.1.2: PATH-детекция компилятора через `where`/`which` + улучшенный fallback flow при отсутствии. Опубликован код в репо; **Marketplace publish** требует нового PAT с правильным scope (см. ниже).
- Inno Setup скрипт для Windows: [installer/windows/installer.iss](installer/windows/installer.iss) — собирает `uzpp-setup.exe` (2.9 MB), HKCU PATH + .uzpp ассоциация без админ-прав.
- One-click install через welcome screen работает на Windows.
- **GitHub Release v2.1.1** опубликован: https://github.com/timetolivechk-spec/uzpp/releases/tag/v2.1.1 — `uzpp-setup.exe` (2.9 MB) + `uzpp-windows-x64.zip` (1.2 MB) + trilingual release notes.
- GitHub About panel заполнен: description (uz/en), website → Marketplace, 13 topics.
- MinGW скачивается с WinLibs автоматически.
- SPEC.md существует (572 строки, версия 0.1).
- SECURITY.md trilingual с координатами Security Advisory channel.

**Что не работает:**
- GitHub Actions CI — billing issue, не разрешено.
- Linux/macOS бинарники — отложены до починки CI.
- Полный type inference для сложных выражений.
- DAP — нестабильный маппинг строк.
- Часть stdlib — заглушки.

---

## Известные проблемы (приоритезировано)

### Критично (блокирует production)
1. **Path traversal в `ulash`** — нет защиты от `../../../etc/passwd`. Срочный фикс.
2. **Shell injection при вызове компилятора** — имена файлов не экранируются. Срочный фикс.

### Высокий приоритет
3. **Мусор в корне репозитория** — 20 файлов `test_*.uzpp`, `setup.bat`, `demo_tizim.uzpp`. Перенести в `tests/legacy/` или удалить.
4. **Сломанные ссылки в README** — placeholder `YOUR_USERNAME/uz-plus-plus` вместо `timetolivechk-spec/uzpp`.
5. **About panel пустая на GitHub** — нет description, topics, website.
6. **Отсутствие CONTRIBUTING.md, CODE_OF_CONDUCT.md, CHANGELOG.md в корне** — нужно создать.
7. **Лишние markdown отчёты в корне** — 7 файлов, перенести в `docs/`.
8. **Лишние папки** — `phase12_demo/`, `test_project_demo/`, `test_project_uzpm/` — экспериментальные остатки.

### Средний приоритет
9. **TypeChecker слабый** — `inferType` возвращает `noma'lum` для большинства сложных выражений. Критично для образования.
10. **Сообщения об ошибках не на уровне Rust** — нет подсветки позиции, нет suggestions, нет multilingual.
11. **Stdlib заглушки** — нужно либо доделать, либо честно пометить `EXPERIMENTAL`.
12. **Нет negative тестов** — что компилятор корректно отвергает плохой код.

### Низкий приоритет (после стабилизации)
13. main.cpp 1200 строк монолитом — рефакторинг.
14. Нет инкрементального парсинга в LSP.
15. Производительность лексера/codegen.

---

## План разработки (приоритезированный)

### Фаза 1: Очистка и безопасность (срочно, 1-2 недели)
- [x] Очистка корня репозитория от мусорных файлов (commit `6626b5d`).
- [x] Перенос markdown отчётов в `docs/` (отчётов в корне нет).
- [x] Исправление ссылок `YOUR_USERNAME` → `timetolivechk-spec`.
- [x] Заполнение About panel на GitHub (description, topics, website) — сделано через `gh repo edit` 2026-05-12.
- [x] Создание CONTRIBUTING.md, CODE_OF_CONDUCT.md, CHANGELOG.md, SECURITY.md (commit `317d643`).
- [x] Фикс path traversal в `ulash` (commit `a12a84a`, negative tests).
- [x] Фикс shell injection в вызове компилятора (commit `a12a84a`).

### Фаза 2: Качество и правдивость (3-4 недели)
- [x] Аудит stdlib: классификация в [docs/stdlib-status.md](docs/stdlib-status.md), 7 STUB-модулей помечены `// status: EXPERIMENTAL`.
- [x] ~~Удаление или переименование `kripto.hpp` → `xesh.hpp`~~ — НЕ нужно: фактически содержит SHA1+SHA256+Base64+HMAC+JWT, аудит показал, что это REAL модуль.
- [x] Удаление лишних папок (phase12_demo и др.) — сделано в `6626b5d`.
- [x] Negative tests — 27 рабочих + 3 в [tests/negative/pending/](tests/negative/pending/) (трек-лист для багов компилятора). Runner: `bash tests/negative/run.sh` или `pwsh tests/negative/run.ps1`.
- [x] Unit тесты для lexer, parser, type_checker, codegen — 25 в [tests/frontend_smoke.cpp](tests/frontend_smoke.cpp): 6 лексер (тройные строки, escape, числа, комменты, compound операторы, пустой ввод), 5 парсер (классы, match, try/catch, lambda, enum), 5 type_checker (arg count, undef ident, unused var, line/col), 5 codegen (main, #line, range-for, virtual, static), плюс 7 исходных smoke-тестов. Запуск: `build/uzpp_frontend_tests.exe`.

### Фаза 3: Образовательная ценность (1-2 месяца)
- [ ] TypeChecker improvements — полный inferType.
- [ ] Сообщения об ошибках в Rust-стиле на узбекском.
- [ ] Каталог кодов ошибок (`uzpp explain E0042`).
- [ ] Multilingual support для ошибок (uz/ru/en).
- [ ] Стабилизация DAP.

### Фаза 4: Видимость и пользователи (параллельно с Фазой 3)
- [ ] Demo видео на YouTube.
- [ ] Анонс в узбекских IT-сообществах (Telegram, LinkedIn).
- [ ] Контакт с одним университетом для пилотного курса (TUIT, INHA).
- [ ] Web playground (Go playground стиль).
- [ ] Сбор feedback от первых 5-10 пользователей.

### Фаза 5: Уникальные фичи (после стабилизации, 3-6 месяцев)
- [ ] Bilingual mode прототип — VSCode плагин с переключением uz++ ↔ C++.
- [ ] Hot reload через dlopen — минимальная версия.
- [ ] Refinement types — для целочисленных типов через Z3.

---

## Технические соглашения

**Целевая платформа сборки:**
- Windows: GCC (MSYS2/MinGW) GCC 13+
- Linux: GCC 14+ или Clang 18+
- macOS: AppleClang Xcode 16+
- CMake 3.20+, Ninja recommended

**Стандарт C++:** C++23 (`CMAKE_CXX_STANDARD 23`).

**Стиль кода:**
- camelCase для функций.
- snake_case для переменных.
- Visitor pattern для AST.
- Header-only для stdlib.

**Тесты:**
- Формат `.test.uzpp` с `// OUT: "..."`.
- Запуск через `uzpp --test`.

**Релизы:**
- Сейчас вручную через GitHub UI.
- Когда CI заработает — авто через теги `v*`.

---

## Анти-паттерны (что НЕ делать)

- **Не добавлять новые stdlib модули** пока существующие не доделаны или не помечены.
- **Не расширять синтаксис** без стабилизации текущих фич.
- **Не позиционировать как «замена C++»** — это вызовет правильный скептицизм. Позиционировать как «образовательный + уникальный».
- **Не конкурировать с Carbon/Cppfront напрямую** — нет ресурсов. Найти свою нишу (узбекский + bilingual + образование).
- **Не делать GUI приоритетом** — SDL2 обёртка достаточна, не пытаться построить полноценный GUI фреймворк.
- **Не обещать в README то что не работает** — правдивость важнее маркетинга.
- **Не публиковать в Marketplace новые версии** без тестирования на чистой машине.

---

## Референсы для изучения

- Carbon Lang (Google) — как делают «преемник C++» большой командой.
- Cppfront (Herb Sutter) — синтаксические эксперименты над C++.
- Rust diagnostic system — образец сообщений об ошибок.
- Erlang/OTP — образец hot reload.
- Smalltalk / Pharo — образец живого программирования для образования.
- LiquidHaskell, F* — refinement types в практике.
- Unison — content-addressable code, projection model.

---

## Контакты и ссылки

- Репо: https://github.com/timetolivechk-spec/uzpp
- Marketplace: https://marketplace.visualstudio.com/items?itemName=uzpp.uzpp
- SPEC: SPEC.md в репо
- Лицензия: MIT
