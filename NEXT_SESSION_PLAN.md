# Next session plan — uz++

> Read `~/.claude/projects/C--Users-MSN-uz--/memory/project_uzpp.md` first.
> It has the full project context, hard-won gotchas, file map, and conventions.
> This document only lists what to work on **next**.

## Current state at a glance

- **64/64 tests pass locally.** ~90% C++23 coverage.
- Latest commit: `d0b48bb feat(tooling): semantic tokens + round-trip comments`.
- Build: `cmake --build build_wt`. Regression: see memory file.
- **Nothing is pushed by default.** User pushes manually.
- VSCode extension at v2.1.7 — do not bump.

## What to pick from (rough priority — verify with user before starting)

### 🟡 Tooling polish — inline comments
The formatter still loses comments that are inside expressions, statement
bodies, blank lines between sections. Lexer captures them already
(`Token.leadingComments`), but formatter only emits them at top-level
nodes. Big-ish work (~200 lines). Files: `src/formatter.cpp`,
specifically the `formatBlock` / `formatStatement` / `formatExpression`
paths.

### 🟡 LSP — inlay hints
`o'zgaruvchan x = qaytar_qiymat()` — IDE should show `: <inferred type>`
after the variable name. Requires re-running TypeChecker on the cached
document and emitting `textDocument/inlayHint` responses with positions.
~80 lines in `src/lsp_server.cpp`. Declare `inlayHintProvider:true` in
the initialize capabilities response.

### 🟡 LSP — code actions
Quick-fixes for the warnings TypeChecker emits. E.g. when "O'zgaruvchi 'x'
e'lon qilingan, lekin ishlatilmagan" fires, offer "Prefix with `_`" or
"Remove declaration". Needs new handler `textDocument/codeAction` plus
mapping from warning text → action. ~120 lines.

### 🟡 DAP — better variable rendering
`parseGdbVariables` produces flat strings; complex types (vector of pair,
std::optional, smart pointers) come out unreadable. ~100 lines in
`src/dap_server.cpp` to invoke gdb-MI's `-var-create` / `-var-list-children`
for rich tree expansion.

### 🟢 Error messages — better positional mapping
`translateErrors()` is pure substring sub. Real win would be parsing the
g++ output for `<file>:<line>:<col>:` and re-mapping to .uzpp positions via
the `#line` directives we already emit. ~60 lines, `src/main.cpp:~720`.

### 🔴 Type-checker honesty (hard, multi-session)
Template body type tracking, partial specialization, SFINAE / concept
dispatch, dead-branch in `if constexpr`. **Don't pick this without 3+
sessions of runway** — it changes architectural assumptions throughout.

### 🟢 Language gaps left (small wins individually)
- `std::source_location` alias (~5 lines, just identifierTranslations)
- Module partitions `export module foo:bar;` (~25 lines, parser)
- Trailing `requires` on **classes** (currently only on functions; ~15 lines)
- `static operator()` C++23 (deducing-this on call site; ~20 lines)

## Rules

1. **Surgical edits only.** Touch what you need; don't reformat-while-here.
2. **Tests with the implementation.** No feature is done without `tests/test_X.uzpp`.
3. **0 regressions, 64/64+ on every change.** Full loop is the contract.
4. **Don't push.** Don't publish. Don't bump versions. User does that.
5. **Don't bring back `sabit_*`.** It's intentionally removed; the canonical
   forms are `sobit_*` / `o'zgarmas_*`.
6. **If the disk fills (>95%) the linker fails silently** with cryptic
   "ld returned 1 exit status" and no real error. Check `df -h /c`; clean
   `dist/`, `build_release/`, `zip_staging/` if needed.
7. **Worktrees diverge from main.** Always work in `C:/Users/MSN/uz++`
   unless told otherwise.

## Suggested next batch (4 surgical tasks, ~120 lines)

I'd suggest LSP inlay hints + code actions + error-message line mapping
+ `std::source_location` alias as one session — they're independent,
all 4 fit comfortably in 200 lines total, and they all visibly improve
the editor experience.

But ask the user first — they may have other priorities.
