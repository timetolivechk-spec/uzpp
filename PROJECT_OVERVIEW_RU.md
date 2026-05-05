# uz++ Project Overview — Executive Summary

**Дата:** 4 мая 2026  
**Проект:** uz++ Programming Language & Compiler  
**Статус:** Production Alpha (v1.0)  

---

## ЧТО ЭТО ТАКОЕ? 

**uz++** — это компилятор и язык программирования, позволяющий писать код на **узбекском языке** с полной типизацией и производительностью C++23.

```
Узбекский Код → Компилятор uz++ → C++23 → Бинарный файл (исполняемый)
```

---

## БЫСТРЫЕ ФАКТЫ

| Метрика | Значение |
|---------|----------|
| **Строк кода** | 15,614 |
| **Компонентов** | 8 (Lexer, Parser, TypeChecker, CodeGen, LSP, DAP, Formatter, DocGen) |
| **Модулей StdLib** | 30+ |
| **Поддерживаемых ключевых слов** | 160+ |
| **Типов AST узлов** | 40+ |
| **Статус** | Alpha (основной функционал работает) |
| **ОС** | Windows, Linux, macOS |
| **Язык реализации** | C++23 |
| **Система сборки** | CMake 3.20+ |

---

## АРХИТЕКТУРА КОМПИЛЯТОРА

```
.uzpp файл
    ↓
[LEXER] → Tokenize (160+ keywords)
    ↓
[PARSER] → Build AST (40+ node types)
    ↓
[TYPE CHECKER] → Validate types
    ↓
[CODE GENERATOR] → Generate C++23
    ↓
.generated.cpp
    ↓
[g++/clang++] → Compile
    ↓
Binary executable ✅
```

---

## СТРУКТУРА ПРОЕКТА

```
src/           (8,289 строк) — Компилятор
├── lexer.cpp/h       — Лексический анализ
├── parser.cpp/h      — Парсинг & AST
├── ast.h             — Определения узлов
├── type_checker.hpp  — Проверка типов
├── codegen.cpp/h     — Генерация C++
├── lsp_server.cpp/h  — IDE поддержка
├── dap_server.cpp/h  — Отладка
└── main.cpp          — CLI интерфейс

stdlib/        (4,498 строк) — Стандартная библиотека (30+ модулей)
├── matematika.hpp    — Математика
├── tarmoq.hpp        — Сетевое программирование
├── asinxron.hpp      — Async/await
├── json.hpp          — JSON
├── kripto.hpp        — Криптография
├── suniy_intellekt.hpp — AI/ML
└── ... (24 more)

tests/         (408 строк) — Тесты
misollar/      (419 строк) — Примеры программ
build/         (Generated) — Результаты сборки
```

---

## ЯЗЫК uz++

### Синтаксис (примеры)

```uzpp
// Переменные
butun x = 42;              // int x = 42;
matn name = "Ali";         // string name = "Ali";
haqiqiy pi = 3.14159;      // double pi = 3.14159;

// Функции
butun qo'shish(butun a, butun b) {
    qaytarish a + b;       // return a + b;
}

// Условия
agar (x > 10) {            // if (x > 10) {
    yozish << "Big";       // cout << "Big";
}

// Циклы
uchun (butun i = 0; i < 10; i++) {  // for (int i = 0; i < 10; i++) {
    yozish << i << " ";
}

// Классы
sinf Shaxs {               // class Person {
    omma:                  // public:
        matn ism;
        butun yosh;
};
```

### Типы данных

| uz++ | C++ | Пример |
|-----|-----|--------|
| `butun` | `int` | `butun x = 42;` |
| `uzun` | `long long` | `uzun y = 1000000;` |
| `haqiqiy` | `double` | `haqiqiy pi = 3.14;` |
| `matn` | `std::string` | `matn s = "hello";` |
| `mantiqiy` | `bool` | `mantiqiy ok = rost;` |
| `vektor<T>` | `std::vector<T>` | `vektor<butun> nums;` |
| `lug'at<K,V>` | `std::map<K,V>` | `lug'at<matn, butun> ages;` |

---

## КОМПОНЕНТЫ

### 1. Lexer (Лексер)
- **Файл:** `src/lexer.cpp/h`
- **Функция:** Преобразует текст в токены
- **160+ ключевых слов** узбекского языка
- **Поддерживает:** String escape sequences, comments, numbers

### 2. Parser (Парсер)
- **Файл:** `src/parser.cpp/h`
- **Функция:** Строит Abstract Syntax Tree (AST)
- **40+ типов узлов AST**
- **Метод:** Рекурсивный спуск
- **Приоритет операторов:** ✅ Правильный

### 3. Type Checker (Проверка типов)
- **Файл:** `src/type_checker.hpp`
- **Функция:** Проверяет типы, область видимости
- **Статус:** ⚠️ Неполная реализация (есть баги)

### 4. Code Generator (Генератор кода)
- **Файл:** `src/codegen.cpp/h`
- **Функция:** AST → C++23 код
- **Выход:** `.generated.cpp` файлы
- **Статус:** ⚠️ Есть баги в lambda синтаксисе

### 5. IDE Integration
- **LSP Server** (`lsp_server.cpp/h`) — Автодополнение, подсказки
- **DAP Server** (`dap_server.cpp/h`) — Отладка с точками останова
- **Formatter** (`formatter.cpp/h`) — Форматирование кода

### 6. Standard Library (30+ модулей)
- **matematika.hpp** — sqrt, sin, cos, pow
- **tarmoq.hpp** — HTTP server, networking
- **asinxron.hpp** — Async/await
- **json.hpp** — JSON processing
- **kripto.hpp** — SHA256, AES
- **suniy_intellekt.hpp** — Neural networks
- ... и 24+ других

---

## СТАТУС РАЗРАБОТКИ

### ✅ Завершено (Phase 11)

- ✅ Core compiler pipeline (Lexer → Parser → TypeChecker → CodeGen)
- ✅ AST с 40+ типов узлов
- ✅ Правильный приоритет операторов
- ✅ IDE интеграция (LSP + DAP)
- ✅ 30+ модулей стандартной библиотеки
- ✅ Примеры программ (10+)

### ⚠️ Известные проблемы

| Проблема | Файл | Строки | Критичность |
|----------|------|--------|-------------|
| Lambda синтаксис (4x) | parser.cpp | 923, 1163, 1392, 1548 | 🔴 CRITICAL |
| Lambda синтаксис (1x) | codegen.cpp | 446 | 🔴 CRITICAL |
| Trigraph warnings | various | 3 места | 🟡 HIGH |
| Undeclared members | type_checker.hpp | ~17 мест | 🔴 CRITICAL |
| For-loop parsing bug | parser.cpp | - | 🔴 CRITICAL |

### 🟡 In Progress (Phase 12-13)

- 🔧 Fix lambda syntax errors
- 🔧 Fix for-loop parsing
- 🔧 Complete TypeChecker
- 🔧 Performance optimizations

---

## КАК ЭТО РАБОТАЕТ? (Пример)

### Шаг 1: Пишем программу на uz++

```uzpp
// hello.uzpp
ulash "uzpp_runtime.hpp"

butun asosiy() {
    yozish << "Salom, Dunyo!" << qator_oxiri;
    qaytarish 0;
}
```

### Шаг 2: Компилируем командой

```bash
uzpp --qurish hello.uzpp
```

### Шаг 3: Компилятор транспилирует в C++23

```cpp
// hello.generated.cpp
#include "uzpp_runtime.hpp"

int main() {
    std::cout << "Salom, Dunyo!" << std::endl;
    return 0;
}
```

### Шаг 4: C++ компилятор создаёт исполняемый файл

```bash
g++ -O2 hello.generated.cpp -o hello.exe
```

### Шаг 5: Запускаем программу

```bash
./hello.exe
# Output: Salom, Dunyo!
```

---

## ПРИМЕРЫ ПРОГРАММ

### 1. Hello World
```uzpp
butun asosiy() {
    yozish << "Salom, Dunyo!" << qator_oxiri;
    qaytarish 0;
}
```

### 2. FizzBuzz
```uzpp
uchun (butun i = 1; i <= 100; i++) {
    agar (i % 15 == 0) {
        yozish << "FizzBuzz" << qator_oxiri;
    } aks_holda agar (i % 3 == 0) {
        yozish << "Fizz" << qator_oxiri;
    } aks_holda agar (i % 5 == 0) {
        yozish << "Buzz" << qator_oxiri;
    } aks_holda {
        yozish << i << qator_oxiri;
    }
}
```

### 3. Вычисление факториала
```uzpp
butun factorial(butun n) {
    agar (n <= 1) qaytarish 1;
    qaytarish n * factorial(n - 1);
}

butun asosiy() {
    yozish << "5! = " << factorial(5) << qator_oxiri;
    qaytarish 0;
}
```

### 4. Работа с коллекциями
```uzpp
vektor<butun> numbers = [1, 2, 3, 4, 5];

uchun (butun n : numbers) {
    yozish << n << " ";
}

lug'at<matn, butun> ages;
ages["Ali"] = 25;
ages["Vali"] = 30;
```

---

## СБОРКА ПРОЕКТА

### Требования

- **C++23 компилятор** (GCC 13+ или Clang 17+)
- **CMake** 3.20+
- **Windows, Linux или macOS**

### Команды

```bash
# 1. Создаём директорию сборки
cmake -B build

# 2. Компилируем
cmake --build build --config Release

# 3. Запускаем тесты
ctest --test-dir build

# 4. Готовый компилятор находится в:
# build/uzpp  (Linux/macOS)
# build/uzpp.exe  (Windows)
```

---

## CLI КОМАНДЫ

```bash
# Компилировать программу
uzpp --qurish program.uzpp

# Запустить программу
uzpp --ishga-tushirish program.uzpp

# Транспилировать в C++
uzpp --tarjima program.uzpp -o program.cpp

# Форматировать код
uzpp --format program.uzpp

# Запустить LSP сервер (IDE)
uzpp --lsp

# Запустить DAP сервер (debugger)
uzpp --dap

# Установить пакет
uzpp --paket-o'rnatish package_name
```

---

## IDE ПОДДЕРЖКА

### VS Code

**Возможности:**
- ✅ Syntax highlighting (подсветка синтаксиса)
- ✅ IntelliSense (автодополнение)
- ✅ Go to Definition
- ✅ Find References
- ✅ Hover documentation
- ✅ Real-time diagnostics
- ✅ Code formatting
- ✅ Integrated debugging

**Запуск:**
1. Установить VS Code расширение uz++
2. Открыть `.uzpp` файл
3. IDE автоматически запустит LSP сервер

### Debugging

- ✅ Breakpoints (line, conditional)
- ✅ Step (into, over, out)
- ✅ Variables inspection
- ✅ Stack trace
- ✅ Exception handling

---

## СТАНДАРТНАЯ БИБЛИОТЕКА (Краткий Обзор)

### Математика (`matematika.hpp`)
```uzpp
haqiqiy x = uzpp::matematika::sqrt(16.0);  // 4.0
haqiqiy y = uzpp::matematika::sin(0);      // 0
haqiqiy z = uzpp::matematika::pow(2, 3);   // 8
```

### Работа с Файлами (`fayl_tizimi.hpp`)
```uzpp
matn content = uzpp::fayl_tizimi::o'qish("file.txt");
uzpp::fayl_tizimi::yozish("file.txt", "hello");
```

### JSON (`json.hpp`)
```uzpp
uzpp::json::JsonQiymat obj;
obj["ism"] = "Ali";
obj["yosh"] = 25;
matn json = uzpp::json::jsonStringKilish(obj);
```

### Сетевое Программирование (`tarmoq.hpp`)
```uzpp
uzpp::tarmoq::HttpServer server(8080);
server.GET("/hello", [](auto req) {
    qaytarish "Hello World!";
});
server.juration();  // run forever
```

### Async/Await (`asinxron.hpp`)
```uzpp
asinxron bosh slow_operation() {
    kutish uzpp::vaqt::usixish(1000);
    yozish << "Done!" << qator_oxiri;
}
```

---

## ЗАКЛЮЧЕНИЕ

### За Что Отвечает Каждый Компонент?

| Компонент | Размер | Функция | Статус |
|-----------|--------|---------|--------|
| **Lexer** | ~600 строк | Tokenize | ✅ OK |
| **Parser** | ~1,100 строк | Build AST | ⚠️ Bugs |
| **TypeChecker** | ~400 строк | Validate | ⚠️ Incomplete |
| **CodeGen** | ~1,200 строк | Generate C++ | ⚠️ Bugs |
| **Frontend** | ~2,500 строк | IDE/LSP/DAP | ✅ OK |
| **StdLib** | ~4,500 строк | Standard Library | ✅ Mostly OK |
| **Tests/Examples** | ~827 строк | Validation | ⚠️ Some fail |

### Почему Это Важно?

✅ **Образование** — Возможность обучаться программированию на родном языке  
✅ **Локализация** — Демонстрирует принципиальность подхода  
✅ **Производительность** — Выходной код = C++23 (высокая производительность)  
✅ **Интеграция** — Возможность использовать C++ библиотеки  
✅ **Современность** — Поддерживает async/await, ООП, функциональное программирование

---

## Дополнительные Материалы

📄 **Полный технический отчёт:** `TECHNICAL_REPORT_RU.md`  
📄 **Краткая справка:** `TECHNICAL_SUMMARY.md`  
📊 **Диаграммы архитектуры:** `ARCHITECTURE_DIAGRAMS.md`  
📖 **Спецификация языка:** `SPEC.md`  
🔍 **Анализ проблем:** `ANALYSIS_REPORT.md`  

---

**Дата создания:** 4 мая 2026  
**Версия:** 1.0  
**Статус:** Production Alpha  
**Язык программирования:** uz++ v0.1  
