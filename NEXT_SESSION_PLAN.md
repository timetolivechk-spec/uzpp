# Next session plan — uz++

> Read `~/.claude/projects/C--Users-MSN-uz--/memory/project_uzpp.md` first.
> It has the full project context, hard-won gotchas, file map, and conventions.
> This document only lists what to work on **next** and the rules of engagement.

## Current state at a glance

- **70/70 tests pass locally** (Windows MSYS2 g++ 15.2). Linux CI = 68/70
  (`test_deducing_this` + `test_coyield` in `.ci_skip_linux` — both gated on
  Ubuntu toolchain, not uz++ bugs).
- **Latest tag pushed: `v2.1.9`** (VSCode extension texts refresh).
- **Local commits ahead of origin** (not yet pushed — 5):
  - `848cc20` fix(lang+lsp): namespace alias actually emits + drop bogus class trailing-requires + typesEquivalent + matn formatlash alias
  - `0a0c6e2` docs(plan): mark (A)/(B)/(C) done
  - `f4ca448` feat(lang): namespace aliases (claim of trailing requires on classes — rolled back in 848cc20)
  - `9915302` feat(lsp): AST-aware definition / references / rename
  - `0f34978` feat(stdlib): port xatoliklar to uz++
- **stdlib partially self-hosted:** `stdlib/matn.hpp` (~626 LOC source)
  AND `stdlib/xatoliklar.hpp` (~97 LOC source) are now generated from
  `.uzpp` via header-mode transpilation. Source of truth is `.uzpp`.
- **Marketplace: uzpp.uzpp v2.1.9** is live.
- **GitHub Release v2.1.9** has Windows installer (151 MB), portable zips,
  Linux/macOS tarballs, install.sh, MinGW WinLibs zip (255 MB).
- Build: `cmake --build build_wt` in the active worktree, or
  `cmake --build build` in the main repo at `C:/Users/MSN/uz++`.

## ⛔ Rules of engagement — DON'T BREAK THE SYSTEM

1. **Don't push to origin by default.** Local commits only this session.
   Push only on explicit user request.
2. **Don't bump versions** (`vscode-uzpp/package.json`, tags) — user does.
3. **Don't publish .vsix to Marketplace** — user does.
4. **Don't delete hundreds of lines without reason.** If you must remove
   code, justify it in the commit message AND verify nothing references it
   first (`grep -rn` across `src/`, `stdlib/`, `tests/`, `vscode-uzpp/`).
5. **Don't bring back `sabit_*`** — deliberately removed; canonical forms
   are `sobit_*` / `o'zgarmas_*`.
6. **Don't reintroduce `_`-mangling for apostrophes** — `o'lcham` must
   stay `oʼlcham` (U+02BC) in generated C++. Test: `tests/test_apostrof.uzpp`.
7. **Don't re-add `teskari` → `std::reverse` alias** to identifierTranslations
   — it conflicts with `uzpp::Matn::teskari`.
8. **Don't touch `stdlib/matn.hpp` by hand.** It's a generated artefact.
   Edit `stdlib/matn.uzpp` and regenerate (see "Regenerating matn.hpp" below).
9. **Surgical edits only.** Touch what you need; don't reformat-while-here.
10. **Tests with the implementation.** No feature is done without
    `tests/test_X.uzpp` (or an assert added to `tests/frontend_smoke.cpp`).
11. **0 regressions on every change.** Full loop is the contract:

    ```bash
    pass=0; fail=0; failed=""
    for f in tests/*.uzpp; do
      if ./build_wt/uzpp.exe qurish "$f" 2>&1 | grep -q "MUVAFFAQIYAT: Dastur tayyor"; then
        pass=$((pass+1))
      else
        fail=$((fail+1)); failed="$failed $(basename $f)"
      fi
    done
    echo "$pass/$((pass+fail))"; [ -n "$failed" ] && echo "FAIL:$failed"
    ```
    Expect **68/68**.
12. **Worktree gotcha**: `C:/Users/MSN/uz++/.claude/worktrees/` has its own
    `stdlib/`. If you regenerate `stdlib/matn.hpp`, update **both** the
    worktree's `stdlib/` AND the main repo's `stdlib/` (or work in one
    consistently).
13. **Disk fills cause silent linker fails** (>95% on `C:`). If `ld returned
    1 exit status` with no real error, run `df -h /c` first — clean
    `dist/`, `build_release/`, `zip_staging/`.
14. **Parser holds `const std::vector<Token>&`.** Store tokens in a local
    variable before constructing Parser; passing `lexer.tokenize()` directly
    is use-after-free. (Bug already burnt me once.)

### Regenerating `stdlib/matn.hpp` from `matn.uzpp`

```bash
mv stdlib/matn.hpp /tmp/matn.hpp.bak
sed -i 's|^#include "matn.hpp"|// #include "matn.hpp"|' stdlib/uzpp_runtime.hpp
rm -rf build
./build_wt/uzpp.exe qurish tests/test_matn_include.uzpp 2>&1 | tail -3
cp build/matn.hpp stdlib/matn.hpp
sed -i 's|^// #include "matn.hpp"|#include "matn.hpp"|' stdlib/uzpp_runtime.hpp
```

## What to pick from (rough priority)

### 🟢 Stdlib dogfooding — port the next library to uz++

Two modules self-hosted so far (`matn`, `xatoliklar`). Remaining order:

1. ~~**`xatoliklar`** (133 LOC)~~ — done in 0f34978.
2. **`vaqt`** (117 LOC) — date/time. Some `<chrono>` interop, manageable.
   Next easiest pick.
3. **`matematika`** (312 LOC) — lots of thin `<cmath>` wrappers (`sin`,
   `cos`, `log`, ...) plus statistics. Mostly mechanical translation.
4. **`json`** (324 LOC) — ambitious. Variant + shared_ptr recursion;
   may surface gaps in TypeChecker for recursive types. **Don't start
   without runway** — likely 1–2 sessions.

Process (proven on `matn`):
- Write `stdlib/<name>.uzpp` mirroring the existing `.hpp` API.
- Use `namespace uzpp::<Name>` to match what other stdlib code references.
- Self-test via `butun asosiy() { ... }` at file end with `tasdiqlash`
  helpers — split asosiy out into `tests/test_<name>_module.uzpp` once
  it compiles cleanly.
- Add `tests/test_<name>_include.uzpp` to exercise the `ulash` import path.
- Regenerate `.hpp` (procedure above), commit both source `.uzpp` and
  generated `.hpp`.

### 🟡 LSP & tooling polish

- ~~**Definition / references / rename are text-based.**~~ Done in
  9915302 — now AST-aware (walks IdentifierExpression nodes).
- **Inline comments in formatter** still drop inside expressions / blank
  lines. `Lexer::Token::leadingComments` captures them; formatter only
  emits at top-level. ~200 LOC in `src/formatter.cpp::formatBlock` etc.
- **DAP variable rendering** for complex C++ types (vector of pair,
  `std::optional`, smart pointers) uses gdb-MI's flat output. Switch to
  `-var-create` / `-var-list-children` for tree expansion. ~100 LOC in
  `src/dap_server.cpp`.
- **Code actions for more warnings.** "unused variable" + "unreachable
  code" land in 9915302 / f4ca448. Still to do: "type mismatch" (suggest
  `static_cast`), "redeclaration" (rename), warning-driven import
  suggestions. ~80 LOC per fix.

### 🟡 Language gaps (small wins individually)

- ~~**Namespace aliases**~~ — working (fixed in 848cc20 — the original
  f4ca448 patch was a stub; emit path is now correct, test_namespace_alias
  covers it).
- ~~**Trailing `requires` on classes**~~ — **don't reintroduce.** Was rolled
  back in 848cc20: C++20 `class X requires C { };` is invalid. Use the
  template-wrapper form already supported: `shablon<tur T> shart(C) sinf Foo`.
- **Module partitions** (`export module foo:bar;`) — parser gap, ~25 LOC.
- **`static operator()`** C++23 (deducing-this on call site) — ~20 LOC.
- **Variadic templates with `std::format_string<Args...>`** — would let
  `Matn::formatlash` accept `Args...` directly (currently it forwards to
  `formatlash_indeksli` via a `vektor<matn>` wrapper). Bigger work,
  parser + codegen — ~80 LOC. Risk: may collide with existing variadic.

### 🟡 CI/release polish

- **`test_coyield` skipped on Linux** (`<generator>` missing from
  libstdc++-15-dev in PPA). Check periodically if Ubuntu toolchain ships
  it; if so, remove from `.ci_skip_linux`.
- **`test_deducing_this` skipped on Linux** (gcc-15 PPA rejects `this`
  syntax). Same — recheck when toolchain updates.
- **macOS Intel dropped** in v2.1.8 (macos-13 runner deprecated). If user
  reports demand, switch to `macos-14-large` (paid Intel xlarge) or add
  cross-build from arm64.
- **`publish-extension` always fails on re-tag** when version already
  published. Make idempotent: check `vsce show uzpp.uzpp` first, skip if
  current version already up. ~10 LOC in `release.yml`.
- **`install.sh` not smoke-tested in CI.** Add a tiny job: in Ubuntu
  container, `bash installer/unix/install.sh` against a fake release,
  verify `uzpp --version` succeeds.

### 🔴 Type-checker honesty (multi-session, architectural)

Doesn't track:
- Template body types (every type-param becomes `noma'lum`)
- Overload resolution (allows redeclaration but doesn't pick right one)
- SFINAE / concept-driven dispatch
- `if constexpr` dead-branch elimination
- Lifetime / borrow checking

**Don't pick this without 3+ sessions of runway** — changes architectural
assumptions throughout. Discuss approach with user before starting.

## Suggested next batch

Quick wins (G, H) landed in 848cc20. Remaining to choose from:

- **(D) Port `vaqt` to uz++** — third stdlib module. `<chrono>` interop
  the trickiest part; otherwise straightforward. ~120 LOC + tests.
  After `matn` + `xatoliklar` template, this is mechanical.
- **(E) Better LSP code actions** — type-mismatch quick-fix (`static_cast`
  suggestion), redeclaration → rename. Reuses the warnings TypeChecker
  already emits. ~80 LOC per fix.
- **(F) Push the merged work to origin + cut v2.2.0 tag** — requires
  user authorisation. Five local commits worth shipping. Bump
  vscode-uzpp to v2.2.0; CHANGELOG entry; tag; let release.yml fire.
- **(I) MinGW zip slim-down** — current artefact is 255 MB. Strip
  `share/locale`, `share/doc`, `share/man`, `lib/python`, `share/cmake`,
  `lib/gcc/.../include-fixed/X11`. Target ~150 MB. ~25 LOC change to
  the `package-mingw` job in release.yml.
- **(J) Module partitions** (`export module foo:bar;`) — parser gap,
  ~25 LOC. Small standalone win.
- **(K) Inline comments in formatter** — Lexer captures them as
  `Token.leadingComments` already; formatter only emits at top-level.
  ~200 LOC in `src/formatter.cpp::formatBlock` etc.

## Detailed plan for an AI agent (any of D / E / I / J)

### (D) Port `vaqt` to uz++

**Goal.** Make `stdlib/vaqt.uzpp` the source of truth; regenerate
`stdlib/vaqt.hpp` via header-mode transpilation. Same pattern as `matn`
and `xatoliklar`.

**Steps.**
1. Read `stdlib/vaqt.hpp` (117 LOC). Note the public API: classes
   (`Vaqt`, `Soat`...), helpers (`hozir()`, `sana_olish()` ...). Catalog
   every `inline` function, every `class` field, every `[[nodiscard]]`.
2. Write `stdlib/vaqt.uzpp`:
   - `nomlar_fazosi uzpp::Vaqt { ... }` to match what other stdlib
     references.
   - `ulash <chrono>` + `<ctime>` + `<thread>` near the top — header mode
     needs explicit includes because preamble is suppressed.
   - One uz++ function per C++ function. Avoid `formatlash`-style
     variadics; use `formatlash_indeksli` if needed.
3. Self-test in `butun asosiy()` at file end (`tasdiqlash` helpers).
   When it compiles green, split asosiy out into
   `tests/test_vaqt_module.uzpp` — keeps the library file clean.
4. Add `tests/test_vaqt_include.uzpp` (consumer): pure `ulash
   "uzpp_runtime.hpp"`, call `uzpp::Vaqt::*`. Drives the
   `ulash "*.uzpp"` import path.
5. **Regenerate `stdlib/vaqt.hpp`** using the documented procedure
   (don't edit by hand). Both `stdlib/matn.hpp` precedent and the
   "Regenerating matn.hpp" section apply identically.
6. Sanity-check `stdlib/uzpp_runtime.hpp` still `#include "vaqt.hpp"`.
   Don't change other modules.
7. Full regression must stay `≥ 71/71` (was 70/70; +test_vaqt_module
   + test_vaqt_include).

**Acceptance.**
- Both `tests/test_vaqt_module.uzpp` and `tests/test_vaqt_include.uzpp`
  compile and print `=== MUVAFFAQIYATLI ===`.
- `tests/test_yangi_imkoniyatlar.uzpp` (uses `uzpp::Vaqt::*`) still passes.
- `stdlib/vaqt.hpp` starts with `#pragma once / #ifndef UZPP_GEN_VAQT_HPP_`
  and contains `namespace uzpp::Vaqt { ... }`.

**Pitfalls to watch.**
- `<chrono>` types in templates are tricky — if you hit "Tur
  nomutanosibligi" warnings on `chrono::seconds`, treat them as opaque
  raw C++ types in uz++ (use `auto`-style declarations).
- Don't translate `std::chrono::...` into uz++ aliases unless you also
  wire them into `identifierTranslations` / `typeMap` in codegen.cpp.
- `vaqt.hpp` may have ABI exposed via `#if defined(ARDUINO)` blocks —
  keep those as raw C++ inside the .uzpp via inline string emit, or
  document explicitly that vaqt drops embedded-Arduino support.

### (E) Better LSP code actions

**Goal.** Surface two more quick-fixes alongside the existing
"unused variable" pair (prefix `_` / remove line) and the new
"unreachable code" remover added in 848cc20.

**Steps.**
1. In `src/lsp_server.cpp::computeCodeActions` — find the existing
   warning-text matching block (`kUnusedPrefix` / `kUnusedSuffix`).
   Add two new pattern matches:
   - `Tur nomutanosibligi: '<from>' kutilgan, lekin '<to>' berildi` →
     offer "Wrap with `statik_otkazish<<from>>`" quick-fix. Parse
     `<from>`, `<to>` out of the message.
   - `Metod '<name>' qayta e'lon qilindi` (or whichever redeclaration
     wording TypeChecker uses; verify by grep) → offer "Rename to `<name>_2`"
     and "Remove redeclaration" quick-fixes.
2. Each quick-fix needs a `WorkspaceEdit` JSON payload. Existing
   patterns in `computeCodeActions` show the shape.
3. Add corresponding unit assertions in
   `tests/frontend_smoke.cpp` — feed a snippet with the relevant
   warning, call `LspServer::computeCodeActions`, assert the action
   titles appear.
4. No regression budget: 70/70 must still pass.

**Pitfalls.**
- TypeChecker warning text is in Uzbek; match exact substrings, not
  English equivalents.
- `statik_otkazish` wrapping must respect operator precedence — wrap
  the bare expression in parens before the cast.

### (I) MinGW zip slim-down

**Goal.** Shrink `mingw-w64-windows-x64.zip` from 255 MB to ~150 MB by
removing parts of WinLibs that uzpp never invokes.

**Steps.**
1. In `.github/workflows/release.yml::package-mingw`, after the
   download + extract step, before the `mv` to canonical name:
   ```bash
   cd mingw64
   rm -rf share/locale share/doc share/man share/info share/cmake \
          share/gettext share/aclocal share/gtk-doc lib/python* \
          include/X11 include/GL include/freetype2 include/gtk* \
          include/cairo*
   # Optional, larger savings: drop debug libs, but uzpp users may want them
   # rm -rf lib/debug
   cd ..
   ```
2. Re-zip. Verify the archive is `< 175 MB` and still contains
   `bin/g++.exe` + `bin/gcc.exe` + `lib/gcc/x86_64-w64-mingw32/*/`.
3. Run `uzpp-setup.exe` (locally if possible, or in a VM) to confirm
   the bundled compiler still builds `misollar/01_salom_dunyo.uzpp`.

**Acceptance.**
- Release artifact size drops to ~150 MB.
- VSCode `componentManager.js` install still works (download +
  unpack + `g++ --version`).
- Smoke test: build any non-trivial example end-to-end.

### (J) Module partitions

**Goal.** Parse `eksport modul foo:bar;` and emit `export module foo:bar;`.

**Steps.**
1. In `src/parser.cpp`, find the existing `eksport modul` (or
   `export module`) handler — search for `parseExportModule` /
   `ExportModuleStatement`.
2. After consuming `modul X`, peek for `:`. If present, consume
   `: <Identifier>` and append `:Identifier` to the module name string.
3. AST node `ExportModuleStatement` stores name as a string; partition
   suffix lives inside that string. Codegen emits as-is.
4. Add `tests/test_module_partition.uzpp` — `eksport modul foo:bar;`
   followed by a trivial function. Should compile (no `qurish` runtime
   needed — module bootstrapping in GCC is finicky; just verify the
   transpile output matches).

**Acceptance.**
- `eksport modul foo:bar;` parses (no "Noto'g'ri ifoda" error).
- Generated C++ contains literally `export module foo:bar;`.

## Things still off the list (don't pick without runway)

- **🔴 Type-checker honesty** — multi-session, architectural. See main
  memory file.
- **Variadic-template `std::format_string<Args...>`** — parser change
  is small but interacts with how existing variadic templates emit
  template parameters; tested poorly. Defer.
- **DAP `-var-create` / `-var-list-children`** — needs a GDB-MI-aware
  reader on top of the current line-buffer parser. ~100 LOC, but
  requires a Windows test setup to validate.

## Found in the last session (consider for future polish)

- `mingw-w64-windows-x64.zip` is 255 MB — could be slimmed to ~150 MB by
  stripping `share/locale`, `share/doc`, `share/man`, `lib/python`,
  `share/cmake` etc. inside `package-mingw` job. Saves bandwidth for
  componentManager users.
- `install.sh` assumes `/usr/local` is writable or `sudo` is available.
  Could detect rootless containers (CI, codespaces) and silently fall
  back to `$HOME/.local/`. ~15 LOC change.
- VSCode extension `runOfficialInstaller` on Linux/macOS pipes
  `curl ... | bash` directly — security-conscious users might want
  download-and-inspect-first option. Add a "Show install script" link
  in the welcome screen pointing at the install.sh URL. ~10 LOC.
- TypeChecker emits "Tur nomutanosibligi: 'mantiq' kutilgan, lekin
  'mantiqiy' berildi" warnings — `mantiq` and `mantiqiy` are aliases for
  the same C++ `bool`, so this warning is noise. Fix in
  `src/type_checker.hpp` by treating them as equivalent. ~5 LOC.
