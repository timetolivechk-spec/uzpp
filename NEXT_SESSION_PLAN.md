# Next session plan — uz++

> Read `HANDOFF.md` first (it has the up-to-date snapshot, verification
> commands, online-services cheat sheet, and 18 gotchas).
> Memory file: `~/.claude/projects/C--Users-MSN-uz--/memory/project_uzpp.md`.

## Current state at a glance

- **74/74 positive** + **51/51 negative** + **26 frontend_smoke pins** —
  all green (Windows MSYS2 g++ 15.2). Linux CI: 72/74
  (`test_deducing_this` + `test_coyield` skipped on Ubuntu — toolchain
  gates, not uz++ bugs).
- **Latest tag: `v2.2.0`** — pushed, Marketplace + GitHub Release live.
- **stdlib self-hosted: 5 modules** — `matn`, `xatoliklar`, `vaqt`,
  `matematika`, `sinov`.
- **TypeChecker rebuilt** — Phase 1 (tri-state Type) + Phase 2 (composite
  kinds + LSP boundary + template func+class bodies + composite template
  detection). All 9 phase commits + cleanup in `git log`.
- Build: `cmake --build build_wt` (or `build` in main repo).

## ⛔ Rules of engagement — DON'T BREAK THE SYSTEM

1. **Don't push to origin by default.** Local commits only — user
   approves before pushes.
2. **Don't bump versions** (`vscode-uzpp/package.json`, tags) unless
   user explicitly asks for a release cycle.
3. **Don't publish .vsix to Marketplace manually.** Workflow does it via
   `VSCE_PAT` secret. If workflow fails, surface to user.
4. **Don't delete hundreds of lines without reason.** If you must remove
   code, explain why in the commit body.
5. **Type's tri-state contract** — diagnostics fire ONLY on `Aniq × Aniq`.
   Adding a new diagnostic that bypasses this gate is a regression even
   if all tests pass (because tests don't yet exercise the silent path).
6. Read gotchas 1–18 in `HANDOFF.md` §4 before touching anything in
   `src/parser.cpp`, `src/type_checker.hpp`, or `stdlib/{matn,xatoliklar,
   vaqt,matematika,sinov}.hpp`.

## Verify baseline before doing anything

```bash
cd /c/Users/MSN/uz++
cmake --build build_wt 2>&1 | tail -3

# 74/74 expected
pass=0; fail=0
for f in tests/*.uzpp; do
  ./build_wt/uzpp.exe qurish "$f" 2>&1 | grep -q "MUVAFFAQIYAT: Dastur tayyor" \
    && pass=$((pass+1)) || fail=$((fail+1))
done
echo "POS: $pass/$((pass+fail))"

# 51 caught / 0 missed expected
bash tests/negative/run.sh build_wt/uzpp.exe 2>&1 | tail -5

# 26 pins
./build_wt/uzpp_frontend_tests.exe
```

If any of these is not green — STOP. Bisect with `git log --oneline | head`,
revert the suspect commit, surface to user.

## Open work, in rough priority

### 🟢 Half-session wins

- **Hover for non-auto variables.** Currently only `o'zgaruvchan x = ...`
  triggers hover; parameters and regular declarations don't. Extend
  `getInferredTypeAtPosition` to look up scope-declared types too.
- **Semantic-tokens for inherited members.** `collectClassMembers` in
  `src/lsp_server.cpp` only walks the class's own members — extend to
  follow `getBaseClass()` recursively.

### 🟡 Stdlib ports (mechanical — follow the matn/sinov template)

Order of difficulty (LOC of original .hpp):

- **`jurnal`** (93) — logging. Trivial.
- **`apparat`** (100) — hardware utilities.
- **`platforma`** (99) — platform detection.
- **`veb_ui`** (136) — UI primitives.
- **`kripto`** (276) — hash/encrypt. Needs careful UTF-8 handling.
- **`json`** (324) — variant + shared_ptr recursion. **AMBITIOUS** — may
  surface TypeChecker gaps for recursive types. Allocate 2 sessions.
- **`fayl_tizimi`** (302) — `<filesystem>` interop.
- **`malumotlar_bazasi`** (551) — DB connector. Networking + lifetime.
- **`tarmoq`** (572) — HTTP server + client. Big.
- **`oyna`** (674) — OpenGL/X11 graphics. Big + platform-specific.

### 🔴 Multi-session architectural work (the next big levers)

- **Phase 3 — Overload resolution.** `functionReturns_` is `map<string,
  string>` (last declaration wins). Convert to `map<string,
  vector<Overload>>`, add ranking, diagnose ambiguous / no viable.
  2–3 sessions. **Highest visible user-facing impact** after Phase 2.
- **Phase 4 — Lazy template instantiation.** Store template bodies as
  AST; at first call site with concrete args, substitute and re-type-check.
  Honest template body checking without SFINAE machinery. 3 sessions.
- **Phase 5 — Constant evaluation.** Constexpr expression evaluator;
  `agar sobit_ifoda` actually eliminates dead branches; honest
  `statik_tasdiqlash`. 2–3 sessions.

### 🟡 LSP polish

- **Type-mismatch quick-fix smarter.** Currently offers `statik_otkazish<T>(...)`
  as a literal text replacement. Make it actually parse the expression
  and emit a clean wrap with proper parens.

### Tooling / DAP

- **Better complex-type rendering in DAP variables panel.** `parseGdbFrames`
  / `parseGdbVariables` give shallow output; vectors of pairs, optionals,
  variants render as raw GDB text.

## Notes from the v2.2.0 release

- 30 commits between v2.1.9 and v2.2.0 — that's a lot for one release.
  Future releases should probably be smaller-grained (every 5–10 commits)
  to keep CHANGELOG digestible.
- Phase 2 (Type composite + LSP boundary) was the biggest architectural
  shift in v2.2.0. It's mostly invisible to users but unlocks Phase 3/4/5.
- frontend_smoke pin count grew from 5 → 26 — those pins are now the
  contract; treat them as production tests, not casual asserts.
