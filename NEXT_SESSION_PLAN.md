# Next session plan — uz++

> Read `~/.claude/projects/C--Users-MSN-uz--/memory/project_uzpp.md` first.
> It has the full project context, hard-won gotchas, file map, and conventions.
> This document only lists what to work on **next** and the rules of engagement.

## Current state at a glance

- **68/68 tests pass locally** (Windows MSYS2 g++ 15.2). Linux CI = 66/68
  (`test_deducing_this` + `test_coyield` in `.ci_skip_linux` — both gated on
  Ubuntu toolchain, not uz++ bugs).
- **Latest tag pushed: `v2.1.9`** (VSCode extension texts refresh on top of
  v2.1.8 — apostrophe identifiers, ulash *.uzpp, matn rewritten in uz++,
  scope shadowing, uzpp-setup.exe + install.sh in release).
- **stdlib partially self-hosted:** `stdlib/matn.hpp` is generated from
  `stdlib/matn.uzpp` via header-mode transpilation. Other stdlib modules
  are still hand-written C++.
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

Now that `matn` works (and the `ulash "*.uzpp"` plumbing is solid),
repeat the pattern on the next module. Order of difficulty:

1. **`xatoliklar`** (133 LOC) — easiest. Error helpers, pure logic.
2. **`vaqt`** (117 LOC) — date/time. Some `<chrono>` interop, manageable.
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

- **Definition / references / rename are text-based.** Make them AST-aware
  — TypeChecker already has scope info, just plumb it through.
  Files: `src/lsp_server.cpp::findDefinition`, ~150 LOC.
- **Inline comments in formatter** still drop inside expressions / blank
  lines. `Lexer::Token::leadingComments` captures them; formatter only
  emits at top-level. ~200 LOC in `src/formatter.cpp::formatBlock` etc.
- **DAP variable rendering** for complex C++ types (vector of pair,
  `std::optional`, smart pointers) uses gdb-MI's flat output. Switch to
  `-var-create` / `-var-list-children` for tree expansion. ~100 LOC in
  `src/dap_server.cpp`.
- **Code actions for more warnings.** Today only "unused variable" gets
  quick-fixes. Add: "type mismatch" (suggest `static_cast`), "redeclaration"
  (rename), unreachable code (remove). ~80 LOC per fix.

### 🟡 Language gaps (small wins individually)

- **Namespace aliases** (`nomlar_fazosi A = B::C;`) — parser gap, ~20 LOC.
- **Module partitions** (`export module foo:bar;`) — parser gap, ~25 LOC.
- **Trailing `requires` on classes** (currently only on functions) — ~15 LOC.
- **`static operator()`** C++23 (deducing-this on call site) — ~20 LOC.
- **Variadic templates with `std::format_string<Args...>`** — would unblock
  `Matn::formatlash` (currently only `formatlash_indeksli`). Bigger work,
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

Pick ONE of these as a single-session goal (verify with user first):

- **(A) Port `xatoliklar` to uz++** — small, mechanical, gets a second
  stdlib module self-hosted. ~150 LOC + tests. 1 session.
- **(B) AST-aware LSP definition/references** — fix the long-standing
  text-based hack. Biggest UX win for non-trivial codebases. ~200 LOC.
  1 session.
- **(C) Two parser gaps + code action expansion** — namespace aliases,
  trailing `requires` on classes, one more code-action variant.
  ~80 LOC + tests. Half-session.

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
