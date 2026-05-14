# План работы для одной сессии — uz++ (для следующего AI агента)

## Проект кратко

**uz++** — транспилятор языка с узбекскоязычным синтаксисом в C++23. Конвейер: `Lexer → Parser → AST → TypeChecker → CodeGen → g++`. Цель: дать носителям узбекского языка возможность программировать на родном языке с полным покрытием возможностей C++23.

**Текущее состояние:** **37/37 тестов проходят** локально в main repo (`C:/Users/MSN/uz++`). Реализованы:
- compile-time модификаторы (`sobit_ifoda` / `sobit_baholash` / `sobit_boshlangich`)
- указатели/ссылки, move-семантика, все 4 каста, auto
- перегрузка операторов и функций
- шаблоны (включая variadic, fold, концепты)
- атрибуты `[[nodiscard]]` / `[[deprecated]]`
- structured bindings, unions, bitfields, multidim subscript `arr[i, j]`
- deducing-this (`oz`/`bosh` explicit object parameter)
- `if constexpr`, `static_assert`, `noexcept`
- C-style массивы **в полях класса** (не в локальных переменных!)

**Сборка:** `cmake --build "c:/Users/MSN/uz++/build_wt"`
**Регресс (37/37 ожидается):**
```bash
cd C:/Users/MSN/uz++
pass=0; fail=0; for f in tests/*.uzpp; do
  ./build_wt/uzpp.exe qurish "$f" 2>&1 | grep -q "MUVAFFAQIYAT: Dastur tayyor" \
    && pass=$((pass+1)) || { fail=$((fail+1)); echo "FAIL: $f"; }
done; echo "$pass / $((pass+fail))"
```

---

## Три хирургические задачи (развитие языка)

### Задача 1: Локальные C-style массивы `butun arr[10];` (~25 строк)

**Проблема:** Сейчас `butun data[10];` работает только как поле класса (Phase 12 добавил `Member::arraySize`). В теле функции или глобально это даёт `Noto'g'ri ifoda`:
```
butun asosiy() {
    butun buf[5];          // ← parser падает
    buf[0] = 42;
    qaytarish 0;
}
```

Это уже всплывало в `tests/test_multidim_subscript.uzpp` — пришлось обходить через `vektor<butun>`.

**Где:**
- [src/ast.h](src/ast.h) — `VariableDeclaration`. Добавить `std::string arraySize_;` (по аналогии с `ClassDeclaration::Member::arraySize`).
- [src/parser.cpp](src/parser.cpp) — `parseVariableDeclaration(typeName, varName)`. После имени проверить `[`, прочитать integer literal, прочитать `]`.
- [src/codegen.cpp](src/codegen.cpp) — `visitVariableDeclaration`. Если `arraySize_` непуст, эмиссировать `Type name[N]` вместо `Type name`.

**Решение:**
1. В `ast.h::VariableDeclaration` добавить:
   ```cpp
   const std::string& getArraySize() const { return arraySize_; }
   void setArraySize(const std::string& s) { arraySize_ = s; }
   private:
       std::string arraySize_;  // empty if not array
   ```
2. В `parser.cpp::parseVariableDeclaration` сразу после `Token token = ...` блока добавить:
   ```cpp
   std::string arraySize;
   if (!isAtEnd() && peek().type == TokenType::Symbol && peek().value == "[") {
       advance();
       if (peek().type == TokenType::IntegerLiteral) {
           arraySize = advance().value;
       }
       if (peek().value != "]") throw ParseError("Kutilgan ']' massiv hajmidan keyin");
       advance();
   }
   ```
   В конце функции — `decl->setArraySize(arraySize)`.
3. В `codegen.cpp::visitVariableDeclaration` после `emitRawToken(safeIdent(stmt->getName()));` добавить:
   ```cpp
   if (!stmt->getArraySize().empty()) {
       emitRawToken("[");
       emitRawToken(stmt->getArraySize());
       emitRawToken("]");
   }
   ```

**Тест:** `tests/test_local_arrays.uzpp` — функция с локальным массивом, цикл инициализации, чтение, печать суммы.

---

### Задача 2: `[[likely]]` / `[[unlikely]]` на if-операторах (~30 строк)

**Проблема:** C++20 ввёл `[[likely]]` / `[[unlikely]]` как hint для оптимизатора:
```cpp
if (x > 0) [[likely]] { ... } else [[unlikely]] { ... }
```
В uz++ нет способа выразить эту подсказку. Полезна для hot-loops и performance-critical кода — и идеально вписывается в существующий слой атрибутов.

**Где:**
- [src/ast.h](src/ast.h) — `IfStatement`. Добавить `bool isLikely_ = false; bool isUnlikely_ = false;` + сеттеры.
- [src/parser.cpp](src/parser.cpp) — `parseIfStatement()`. После `(condition)` ПЕРЕД `{` принимать `@bashqarib` (likely) или `@kamdan_kam` (unlikely). Альтернативно в else-ветке.
- [src/codegen.cpp](src/codegen.cpp) — `visitIfStatement()`. Эмиссировать `[[likely]]` сразу после `)` и до `{`.

**Решение:**
1. AST: новые поля + геттеры/сеттеры в `IfStatement`.
2. Парсер: после `advance()` для `)` (closed paren) проверить `@`. Если `peek().value == "@" && peek(1).value == "bashqarib"` → `isLikely`, если `"kamdan_kam"` → `isUnlikely`. Поглощать 2 токена.
3. Та же логика для else-ветки.
4. Кодоген: после `)` и до then-блока эмиссировать соответствующий атрибут.

**Имена** (на узбекском): `@bashqarib` = "обычно/чаще всего" (likely), `@kamdan_kam` = "редко" (unlikely). Если эти названия не нравятся — спросить пользователя перед коммитом.

**Тест:** `tests/test_likely.uzpp` — цикл с `agar (x > 0) @bashqarib { ... } yoki @kamdan_kam { ... }`, проверить что компилируется и печатает ожидаемое.

---

### Задача 3: Spaceship operator `<=>` (~25 строк)

**Проблема:** C++20 ввёл оператор `<=>` (three-way comparison) и `= default` для него. Это **standard** способ дать структуре полное упорядочивание автоматически:
```cpp
struct Nuqta {
    int x, y;
    auto operator<=>(const Nuqta&) const = default;
};
```
В uz++ нельзя написать `operator<=>` — лексер не распознаёт `<=>` как один токен, и нет понимания того что это — оператор.

**Где:**
- [src/lexer.cpp](src/lexer.cpp:340) — массив `multiCharacterSymbols`. Добавить `"<=>"` в начало (перед `"<="`).
- [src/codegen.cpp](src/codegen.cpp) — `getOperatorSymbol()` (если есть mapping). `<=>` уже валидный C++ оператор — должен работать сам по себе.
- [src/parser.cpp](src/parser.cpp) — `parseRelationalExpression()`. Возможно нужно добавить `<=>` в `isRelationalOperator()`.

**Решение:**
1. **Lexer**: добавить `"<=>"` в `multiCharacterSymbols` в самое начало — longest-match гарантирует что лексер увидит `<=>` как одну подстроку и не разобьёт на `<=` + `>`.
2. **Parser**: убедиться что `isRelationalOperator(text)` возвращает true для `"<=>"`. Если нет — добавить. Возможно потребуется отдельный precedence уровень (в C++ `<=>` имеет precedence ВЫШЕ чем `<`/`>`, но НИЖЕ чем `<<`).
3. **Codegen**: `getOperatorSymbol` должен возвращать `"<=>"` как есть. Никакого Uzbek-mapping не нужно.
4. **Operator overloading**: проверить что `operator<=>(...)` парсится через существующий путь в parseGlobalDeclaration (там уже есть special-case для `operator+`, `operator==` и др. — возможно нужно расширить).

**Тест:** `tests/test_spaceship.uzpp` — struct с двумя полями, `auto operator<=>(const Nuqta&) sabit = default;`, плюс сравнения `<`, `>`, `==`. Запустить и проверить что упорядочивание работает.

**Подсказка:** `operator<=>` парсится составным именем "operator<=>". В parser.cpp:1614-1620 уже есть код:
```cpp
if (name == "operator" && !isAtEnd() && peek().type == TokenType::Symbol) {
    name += advance().value;
    if (name == "operator(" && ...) name += advance().value;
    else if (name == "operator[" && ...) name += advance().value;
}
```
Нужно расширить — если первый присоединённый символ это `<=>`, имя уже верное (`operator<=>`).

---

## Правила работы

1. **Никаких удалений без обоснования.** Каждое удаление существующего кода — только если оно:
   - заменяется эквивалентной/лучшей логикой в том же коммите,
   - либо удаляется мёртвый код, доказанно недостижимый (с указанием почему).
   - **Все существующие ключевые слова (`o'zgarmas_*`, `sobit_*`) — намеренные алиасы, не трогать.** Слово `sabit` намеренно удалено из проекта — не возвращать.

2. **Хирургически точно.** Изменения локальные. Не рефакторить попутно. Не «улучшать» соседний код.

3. **Регресс после каждой задачи** (команда выше). Цель — **37/37 → 38-40/40** (37 текущих + 1-3 новых теста), 0 регрессий. Если хоть один существующий тест упал — откатить и разобраться.

4. **Обратная совместимость:** новые поля AST с дефолтными значениями. Существующие конструкторы не менять — только setters добавлять.

5. **Тесты пишутся до коммита** — сначала тест-файл, потом изменения парсера/кодогена, потом проверка.

6. **При неоднозначности — спросить.** Если имя ключевого слова или вариант реализации неочевиден (особенно: имя для `[[likely]]/[[unlikely]]`), лучше задать вопрос пользователю чем угадывать.

7. **Не пушить, не публиковать релиз, не трогать CI.** Только локальные коммиты в `main`. Пользователь сам решит когда пушить.

---

## Контекст и подсказки

- Главный файл AST: [src/ast.h](src/ast.h) (~880 строк)
- Главный парсер: [src/parser.cpp](src/parser.cpp) (~2250 строк)
- Кодоген: [src/codegen.cpp](src/codegen.cpp) (~1820 строк)
- Лексер: [src/lexer.cpp](src/lexer.cpp) (~370 строк)
- Тип-чекер (header-only): [src/type_checker.hpp](src/type_checker.hpp) (~900 строк)
- **Класс-поля C-array уже работают** ([src/parser.cpp:2153-2180](src/parser.cpp) парсит `[10]` после имени поля). Реюзать тот же паттерн для локальных переменных.
- **Атрибуты функций** ([src/parser.cpp:1480-1500](src/parser.cpp) — `@tashlab_yuborilmas` / `@eskirgan`) — рабочий образец для парсинга `@likely`/`@unlikely`.
- **Перегрузка операторов** уже работает: `tests/test_operator_overload.uzpp` показывает `operator+`, `operator==` и др. Используется парсинг составного имени в parseGlobalDeclaration.
- **Lexer multi-char symbols**: при добавлении `<=>` поставить **в начало списка** (порядок матчинга — first wins).

---

## Ожидаемый итог сессии

- **37/37 → 40/40 тестов проходят** локально (37 текущих + 3 новых)
- **0 регрессий**
- **3 новые возможности C++20**: локальные C-arrays, `[[likely]]/[[unlikely]]`, spaceship `<=>`
- **~80 строк кода** добавлено суммарно
- Обновлена память (`C:/Users/MSN/.claude/projects/C--Users-MSN-uz--/memory/project_uzpp.md`) с актуальным состоянием
- **НЕ пушить и не публиковать.** Локальные коммиты на `main`. Пользователь сам решит дальнейшие действия.

---

## Что НЕ делать в этой сессии

- **Не трогать CI/CD.** Workflow и skip-list оставить как есть.
- **Не публиковать VSCode extension и GitHub Release.** Версия остаётся 2.1.7.
- **Не пушить в GitHub.** Все коммиты только локально.
- **Не возвращать удалённый `sabit_*`.** Слово целенаправленно убрано.
- **Не переделывать `translateErrors()`.** Прошлый агент уже расширил его.
- **Не трогать type-checker'ные `sizeof`/lambda-исключения.** Они уже работают.
