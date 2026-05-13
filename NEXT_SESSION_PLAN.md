# План работы для одной сессии — uz++ (для следующего AI агента)

## Проект кратко

**uz++** — транспилятор языка с узбекскоязычным синтаксисом в C++23. Конвейер: `Lexer → Parser → AST → TypeChecker → CodeGen → g++`. Цель: дать носителям узбекского языка возможность программировать на родном языке с полным покрытием возможностей C++23.

**Текущее состояние:** **33/33 тестов проходят** в main repo (`C:/Users/MSN/uz++`). Реализованы все 6 этапов покрытия C++23 (constexpr/consteval/constinit, указатели/ссылки/move, касты, перегрузка операторов, шаблоны+концепты+variadic+fold, атрибуты, structured bindings, unions, bitfields, multidim subscript, function overloading, C-array fields, deducing-this).

**Сборка:** `cmake --build "c:/Users/MSN/uz++/build_wt"`
**Регресс:** `cd C:/Users/MSN/uz++ && for f in tests/*.uzpp; do ./build_wt/uzpp.exe qurish "$f"; done`

---

## Три хирургические задачи

### Задача 1: `agar sobit_ifoda (...)` → `if constexpr (...)` (~20 строк)

**Проблема:** В C++17/20 идиоматично использовать `if constexpr` для compile-time выбора веток в шаблонах. uz++ имеет `agar` (if) и `sobit_ifoda` (constexpr) по отдельности, но не комбинацию.

**Где:**
- [src/parser.cpp](src/parser.cpp) — функция `parseIfStatement()`. Найти `grep -n "parseIfStatement" src/parser.cpp`.
- [src/ast.h](src/ast.h) — класс `IfStatement`. Добавить поле `bool isConstExpr_`.
- [src/codegen.cpp](src/codegen.cpp) — функция `visitIfStatement()`.

**Решение:**
1. В `IfStatement` добавить поле `bool isConstExpr_ = false` + `getConstExpr()/setConstExpr()`.
2. В `parseIfStatement()`: после `advance()` для `agar`, проверить `checkKeyword("sobit_ifoda")` — если да, `advance()` и установить флаг.
3. В `visitIfStatement()`: если флаг установлен, эмиссировать `if constexpr` вместо `if`.

**Тест:** `tests/test_if_constexpr.uzpp` — шаблон `shablon <tur T>` с `agar sobit_ifoda (sizeof(T) > 4) { ... } yoki { ... }`.

---

### Задача 2: `statik_tasdiqlash(cond, "msg")` → `static_assert(cond, "msg")` (~15 строк)

**Проблема:** `static_assert` — фундамент compile-time контрактов. Без него шаблоны нельзя защитить от неправильных аргументов на этапе компиляции.

**Где:**
- [src/parser.cpp](src/parser.cpp) — функция `parseGlobalDeclaration()` и `parseStatement()`. Добавить ветку перед стандартным fallback на `parseExpression()`.
- Не нужен новый AST node — можно использовать существующий `TokenNode` с буквальным C++ кодом (как сделано для `tushuncha`/`makro`).

**Решение:**
1. Добавить `"statik_tasdiqlash"` в список `uzbekKeywords` в [src/parser.cpp](src/parser.cpp).
2. В `parseStatement()` (или `parseDeclarationOrExpressionStatement()`): если `checkKeyword("statik_tasdiqlash")`, считать `(`, выражение, `,`, строку, `)`, `;` и собрать в `TokenNode` с C++ строкой вида `static_assert(<cond>, <msg>);`.

**Тест:** `tests/test_static_assert.uzpp` — простой `statik_tasdiqlash(sizeof(butun) >= 4, "butun kamida 4 bayt bo'lishi kerak");`.

---

### Задача 3: `xato_tashlamaydi` → `noexcept` (~25 строк)

**Проблема:** В C++ `noexcept` — критичная аннотация для move-операций, деструкторов и контейнеров. Без неё нельзя писать профессиональные библиотеки.

**Где:**
- [src/ast.h](src/ast.h) — `FunctionDeclaration` уже имеет много флагов (`isConstExpr`, `isNoDiscard` и т.д.). Добавить `bool isNoExcept_ = false` + геттер/сеттер.
- [src/parser.cpp](src/parser.cpp) — `parseFunctionDeclaration()` и `parseClassDeclaration()` (метод). Проверять `xato_tashlamaydi` ПОСЛЕ `)` и до `{`.
- [src/codegen.cpp](src/codegen.cpp) — `visitFunctionDeclaration()`. Эмиссировать `noexcept` после `)` и до `{`.

**Решение:**
1. Добавить `"xato_tashlamaydi"` в `uzbekKeywords`.
2. В парсере функций: после закрывающей `)` параметров проверить `checkKeyword("xato_tashlamaydi")`, если да — `advance()` и установить флаг.
3. В кодогене: эмиссировать `noexcept` сразу после `)`.

**Тест:** `tests/test_noexcept.uzpp` — функция `bosh ozgartirish(butun& x) xato_tashlamaydi { x = 0; }`, плюс move-ctor.

---

## Правила работы

1. **Никаких удалений без обоснования.** Каждое удаление существующего кода — только если оно:
   - заменяется эквивалентной/лучшей логикой в том же коммите,
   - либо удаляется мёртвый код, доказанно недостижимый (с указанием почему).
   - **Все существующие ключевые слова (`o'zgarmas_*`, `sobit_*`) — намеренные алиасы, не трогать.**

2. **Хирургически точно:** изменения локальные, на чётко обозначенных строках. Не рефакторить попутно.

3. **Регресс после каждой задачи:**
   ```bash
   cd C:/Users/MSN/uz++
   for f in tests/*.uzpp; do ./build_wt/uzpp.exe qurish "$f" 2>&1 | grep -q "MUVAFFAQIYAT" || echo "FAIL: $f"; done
   ```
   Должно показывать 33+/33+. Если хоть один тест упал — откатить и разобраться.

4. **Обратная совместимость:** новые поля AST должны иметь дефолтные значения (`isNoExcept_ = false` и т.п.). Существующие конструкторы не менять — добавлять только setters.

5. **Тесты пишутся до коммита** — сначала тест-файл, потом изменения парсера/кодогена, потом проверка.

6. **При неоднозначности — спросить.** Если вариант реализации неочевиден, лучше задать вопрос пользователю, чем угадывать.

---

## Контекст и подсказки

- Главный файл AST: [src/ast.h](src/ast.h) (~870 строк)
- Главный парсер: [src/parser.cpp](src/parser.cpp) (~2200 строк)
- Кодоген: [src/codegen.cpp](src/codegen.cpp) (~1800 строк)
- Тип чекер (header-only): [src/type_checker.hpp](src/type_checker.hpp) (~900 строк)
- Lexer уже распознаёт `...` как мульти-символ — никаких изменений в лексере не требуется.
- Атрибуты `@tashlab_yuborilmas` / `@eskirgan` — рабочий образец для аннотаций функций (смотри как образец).
- Compile-time модификаторы `sobit_ifoda` / `sobit_baholash` / `sobit_boshlangich` — рабочий образец для пре-функциональных модификаторов.

---

## Ожидаемый итог сессии

- **36/36 тестов проходят** (33 текущих + 3 новых)
- **0 регрессий**
- **3 новые возможности C++17/23**: `if constexpr`, `static_assert`, `noexcept`
- **~60 строк кода** добавлено суммарно
- Обновлена память (`C:/Users/MSN/.claude/projects/C--Users-MSN-uz--/memory/project_uzpp.md`) с актуальным состоянием
