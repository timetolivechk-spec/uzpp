# uz++ Architecture Visualization

## Overall Pipeline

```
INPUT                PROCESSING                        OUTPUT
┌──────────────┐     ┌────────────┐
│ .uzpp Source │────→│   LEXER    │ Tokenization
└──────────────┘     │ (lexer.cpp)│ (160+ keywords)
                     └─────┬──────┘
                           │
                           ▼
                     ┌────────────┐
                     │   PARSER   │ AST Building
                     │(parser.cpp)│ (40+ node types)
                     └─────┬──────┘
                           │
                           ▼
              ┌────────────────────────┐
              │  SEMANTIC ANALYZER     │ Type Checking
              │ (type_checker.hpp)     │ Scope Analysis
              └────────────┬───────────┘
                           │
                           ▼
              ┌────────────────────────┐
              │   CODE GENERATOR       │ AST → C++23
              │  (codegen.cpp/h)       │ Translation
              └────────────┬───────────┘
                           │
                           ▼
              ┌────────────────────────┐
              │  C++23 Source Code     │
              │  (.generated.cpp)      │
              └────────────┬───────────┘
                           │
                           ▼
              ┌────────────────────────┐
              │ g++/clang++ Compiler   │ Binary
              │  (External, O2/O3)     │ Compilation
              └────────────┬───────────┘
                           │
                           ▼
              ┌────────────────────────┐ ◄────────────────┐
              │  Executable Binary     │                  │
              │   (.exe/.bin)          │                  │
              └────────────────────────┘         Can be executed
                                                 (if debugging)
                                                        │
                                                   ┌────▼──────────┐
                                                   │  DAP Server    │
                                                   │ (Debugger)     │
                                                   └────────────────┘
```

## Compiler Components (Frontend)

```
                    ┌─────────────────────────────────────┐
                    │        uz++ Compiler (frontend)      │
                    │      (uzpp_frontend CMake lib)       │
                    └─────────────────────────────────────┘
                                    │
                    ┌───────────────┼───────────────┐
                    │               │               │
         ┌──────────▼──────────┐   │        ┌──────▼──────────┐
         │   LEXICAL ANALYSIS  │   │        │ IDE INTEGRATION │
         ├─────────────────────┤   │        ├────────────────-┤
         │ • Tokenization      │   │        │ • LSP Server    │
         │ • Symbol matching   │   │        │ • DAP Server    │
         │ • String escape seq │   │        │ • Formatter     │
         │ • Comment handling  │   │        │ • DocGen        │
         └────────────────────-┘   │        └─────────────────┘
                    ▲              │
                    │              ▼
              [lexer.cpp/h]   ┌──────────────────┐
                    │         │  SEMANTIC LAYER  │
                    │         ├──────────────────┤
         ┌──────────▼──────────────┐ │ • Type Checker   │
         │  SYNTAX ANALYSIS        │ │ • AST Validator  │
         ├─────────────────────────┤ │ • Scope Analysis │
         │ • AST Construction      │ │                  │
         │ • Operator precedence   │ └──────────────────┘
         │ • Error recovery        │        ▲
         │ • Expression parsing    │        │
         │ • Statement parsing     │   [type_checker.hpp]
         │ • Declaration parsing   │        │
         └────────────┬────────────┘        │
                      │                     │
                [parser.cpp/h] ◄────────────┘
                      │
                      ▼
         ┌──────────────────────┐
         │  CODE GENERATION     │
         ├──────────────────────┤
         │ • AST traversal      │
         │ • Type translation   │
         │ • Symbol mangling    │
         │ • C++ emission       │
         └──────────┬───────────┘
                    │
              [codegen.cpp/h]
                    │
                    ▼
         [C++23 Output]
```

## Type System Mapping

```
uz++ Type           C++ Type                    Storage
─────────────────────────────────────────────────────────
butun          →    int                        32-bit
uzun           →    long long                  64-bit
haqiqiy        →    double                     64-bit
kasr           →    float                      32-bit
matn           →    std::string                Variable
mantiqiy       →    bool                       1 byte
belgi          →    char                       1 byte
bosh           →    void                       —

vektor<T>      →    std::vector<T>            Dynamic array
lug'at<K,V>    →    std::unordered_map<K,V>   Hash map
to'plam<T>     →    std::set<T>               Set
juftlik<A,B>   →    std::pair<A,B>            Pair
ixtiyoriy<T>   →    std::optional<T>          Optional

Natija<T>      →    std::expected<T, E>       Result
Tanlov<T>      →    std::optional<T>          Option
```

## Standard Library Organization

```
┌────────────────────────────────────────────────────────────┐
│              uz++ Standard Library (stdlib/)                │
│                        (30+ modules)                        │
└────────────────────────────────────────────────────────────┘
         │
    ┌────┼────┬────────┬──────────┬────────────┐
    │    │    │        │          │            │
    ▼    ▼    ▼        ▼          ▼            ▼
┌────┐┌────┐┌─────┐┌──────┐┌──────────┐┌────────┐
│    ││    ││     ││      ││          ││        │
│SYS ││I/O ││MATH ││STRING││COLLECTION││ASYNC   │
│    ││    ││     ││      ││          ││        │
└────┘└────┘└─────┘└──────┘└──────────┘└────────┘
 │     │     │      │       │          │
 │     │     │      │       │          │
 ▼     ▼     ▼      ▼       ▼          ▼
tizim fayl  mate   matn  tarmoq      asinxron
platfor vaqt matika        tuzilmalar uzpp_runtime
apparat json         koinot
oyna   tizimi        kesh
pana

    ┌────────────────┬───────────────────┐
    │                │                   │
    ▼                ▼                   ▼
┌─────────┐    ┌─────────────┐    ┌──────────────┐
│SECURITY │    │ ADVANCED    │    │  GRAPHICS    │
│          │    │             │    │   & GUI      │
├─────────┤    ├─────────────┤    ├──────────────┤
│kripto   │    │suniy_int    │    │grafika/      │
│xavfsizlik   │tarjima       │    │veb_ui        │
│          │    │sinov        │    │oyna          │
└─────────┘    └─────────────┘    └──────────────┘
```

## Project File Structure

```
uz++/
├── src/                           (Compiler)
│   ├── main.cpp ................. CLI entry point, command processing
│   ├── lexer.cpp/h .............. Tokenization (160+ keywords)
│   ├── parser.cpp/h ............. AST construction via recursive descent
│   ├── ast.h .................... AST node definitions (40+ types)
│   ├── type_checker.hpp ......... Type checking & semantic analysis
│   ├── codegen.cpp/h ............ C++23 code generation
│   ├── formatter.cpp/h .......... Code formatting
│   ├── lsp_server.cpp/h ......... Language Server Protocol
│   ├── dap_server.cpp/h ......... Debug Adapter Protocol
│   ├── docgen.hpp ............... Documentation generator
│   └── package_manager.h ........ Package management
│
├── stdlib/                        (Standard Library)
│   ├── tizim.hpp ................ System API
│   ├── matematika.hpp ........... Math functions (sqrt, sin, cos)
│   ├── matn.hpp ................. String operations
│   ├── vaqt.hpp ................. Time & date
│   ├── fayl_tizimi.hpp .......... File I/O
│   ├── json.hpp ................. JSON processing
│   ├── tarmoq.hpp ............... HTTP server, networking
│   ├── asinxron.hpp ............. Async/await
│   ├── uzpp_runtime.hpp ......... Thread pool, runtime
│   ├── kripto.hpp ............... SHA256, AES, hashing
│   ├── suniy_intellekt.hpp ...... Neural networks, ML
│   ├── xavfsizlik.hpp ........... Security, validation
│   ├── tarmoq_tuzilmalar.hpp .... Data structures
│   ├── malumotlar_bazasi.hpp .... Database (SQLite)
│   ├── veb_ui.hpp ............... Web UI generation
│   ├── oyna.hpp ................. GUI windows
│   ├── grafika/ ................. Graphics, rendering
│   └── ... (18 more modules)
│
├── tests/                         (Test Suite)
│   ├── frontend_smoke.cpp ........ Compiler tests
│   ├── test_*.uzpp .............. Language feature tests
│   └── ...
│
├── misollar/                      (Example Programs)
│   ├── 01_salom_dunyo.uzpp ....... Hello World
│   ├── 02_fizzbuzz.uzpp .......... FizzBuzz
│   ├── 03_rekursiya.uzpp ......... Recursion
│   ├── 05_kolleksiyalar.uzpp .... Collections
│   └── ...
│
├── build/                         (Generated, CMake Output)
│   ├── *.generated.cpp ........... Transpiled C++ files
│   ├── compile_commands.json ..... Compilation database
│   ├── CMakeCache.txt ........... CMake cache
│   └── ...
│
├── CMakeLists.txt ................ Build configuration
├── README.md ..................... Project description
├── SPEC.md ....................... Language specification
└── ANALYSIS_REPORT.md ............ Technical analysis
```

## Data Flow (Simple Example)

```
INPUT: 02_fizzbuzz.uzpp

┌─────────────────────────────┐
│ uchun (i = 1; i <= 100; i++) │
│   agar (i % 15 == 0) { ... }  │
└──────────────┬──────────────┘
               │
               ▼
        [LEXER tokenizes]
               │
         ┌─────┴──────┬──────────┬──────┬────┐
         │            │          │      │    │
     "uchun"      "("  "i"   "="  "1" ";"...
     KEYWORD     LPAREN IDENT ASSIGN INT SEMI
               │
               ▼
        [PARSER builds AST]
               │
      ┌────────┴────────┐
      │                 │
  ForStatement      BinaryOp
      │              i <= 100
      │
  BlockStatement
      │
  IfStatement
      │
  BinaryOp
   i % 15 == 0
               │
               ▼
      [CODEGEN translates]
               │
    ┌──────────┴──────────┐
    │                     │
 for (int i = 1;      if (i % 15 == 0)
      i <= 100; i++)   {
  {                      std::cout << "FizzBuzz"
                         << std::endl;
                       }
               │
               ▼
      [C++ COMPILER]
               │
               ▼
      fizzbuzz.exe
```

## Development Timeline (Phases)

```
Phase 1-10        Phase 11          Phase 12+
(Foundation)      (Current)         (Future)
   ✅              ✅ COMPLETE         🟡 IN PROGRESS
───────────────────────────────────────────────────

Core             Semantic AST       Advanced Features
Infrastructure   Expansion          ├─ Generics
├─ Lexer         ├─ 30+ AST nodes   ├─ Macros
├─ Parser        ├─ Expression      ├─ Templates
├─ CodeGen         Precedence       ├─ Reflection
├─ Type System   ├─ Statement       ├─ Pattern
├─ StdLib          Parsing          │  Matching
├─ CLI            ├─ Full Type      └─ Compile-time
├─ LSP/DAP         Checking            Evaluation
└─ IDE            └─ StdLib Expansion

        Current: Phase 11 Completion ✅
        Next: Phase 13 Bug Fixes 🔧
```

## Compilation Status Board

```
┌─────────────────────────────────────────────────┐
│         COMPILATION STATUS REPORT               │
├─────────────────────────────────────────────────┤
│ Component          Status    Issues   Priority  │
├─────────────────────────────────────────────────┤
│ Lexer              ✅ OK      0       —         │
│ Parser             ⚠️  BUGS   4       🔴 HIGH   │
│ TypeChecker        ⚠️  INCOMPLETE 2  🔴 HIGH   │
│ CodeGen            ⚠️  BUGS   1       🔴 HIGH   │
│ Formatter          ✅ OK      0       —         │
│ LSP Server         ✅ OK      0       —         │
│ DAP Server         ✅ OK      0       —         │
│ StdLib             ✅ MOSTLY OK 1-2  🟡 MEDIUM │
│ Examples           ⚠️  SOME FAIL 1   🔴 HIGH   │
├─────────────────────────────────────────────────┤
│ Overall: 5 Critical Bugs Blocking Build         │
│ Demo Projects: SOME COMPILE & RUN ✅            │
└─────────────────────────────────────────────────┘
```

## Performance Characteristics

```
Compilation Time (est.):
┌────────────────────────────┐
│ Simple program  (~100 LOC): │ ~50-100ms
│ Medium program (~1K LOC):   │ ~200-500ms
│ Large program (~10K LOC):   │ ~1-2 sec
│ Full rebuild:               │ ~3-5 sec
└────────────────────────────┘

Runtime Performance:
- Matches C++23 performance (code is C++23)
- No runtime interpreter overhead
- Full compiler optimizations (-O2/-O3)
- Same binary size as equivalent C++

Memory Usage (Compiler):
- Parser: ~10-20 MB (depends on source size)
- CodeGen: ~5-10 MB
- Total frontend: ~50-100 MB
```

## Summary Statistics

```
┌────────────────────────────────────────┐
│        PROJECT STATISTICS              │
├────────────────────────────────────────┤
│ Total Lines of Code       15,614       │
│ Compiler Source           8,289 (53%)  │
│ Standard Library          4,498 (29%)  │
│ Tests & Examples          1,827 (12%)  │
│ Build Files               Generated    │
│                                        │
│ Source Files              68 total     │
│ C++ Implementation        17 files     │
│ Header Files              25 files     │
│ uz++ Programs             26 files     │
│                                        │
│ Compiler Modules          8            │
│ Stdlib Modules            30+          │
│ AST Node Types            40+          │
│ Language Keywords         160+         │
│ Built-in Functions        50+          │
│                                        │
│ Build System              CMake 3.20+  │
│ Language Spec             C++23        │
│ Min GCC Version           13           │
│ Min Clang Version         17           │
│                                        │
│ Status                    Alpha v1.0   │
│ Production Ready          Partial ⚠️  │
│ Demo Projects             Working ✅   │
└────────────────────────────────────────┘
```
