# uz++ — Session Handoff

> **Read this FIRST in a new session.** Then `~/.claude/projects/C--Users-MSN-uz--/memory/project_uzpp.md`
> for deeper context (file map, hard-won gotchas, architecture).
> `NEXT_SESSION_PLAN.md` has prioritised work suggestions.

Last updated: end of session ending 2026-05-20 (TypeChecker Phase 1 + Phase 2 complete; v2.2.0 cut).

---

## 1. Snapshot

| Metric | Value |
|---|---|
| **Positive tests** | **74/74** (Windows MSYS2 g++ 15.2) |
| **Negative tests** | **51/51** caught (0 pending) |
| **frontend_smoke** | **26 pins** (was 5 — TypeChecker rebuild added 21) |
| **Linux CI** | 72/74 (`test_deducing_this` + `test_coyield` in `.ci_skip_linux` — Ubuntu toolchain gates, not uz++ bugs) |
| **Stdlib ports** | 5 self-hosted (`matn`, `xatoliklar`, `vaqt`, `matematika`, `sinov`) |
| **TypeChecker** | tri-state `Type` + composites (Korsatkich/Havola/Shablon) + Polimorf for template func+class bodies + composite-template detection |
| **Latest tag** | `v2.2.0` (this session — Marketplace + Release live after push) |
| **Working tree** | clean |

### What landed this session — TypeChecker rebuild (Phase 1 + Phase 2)

Tri-state Type with composite kinds — diagnostics only fire on Aniq × Aniq;
Nomalum and Polimorf are silently accepted. This eliminated the entire class
of spurious "Funksiya 'butun' qaytarishi kerak, lekin 'T' qaytarilmoqda"
warnings inside template bodies.

```
Phase 1.1  fe7c889  Type {Aniq, Nomalum, Polimorf}
Phase 1.2  ed07e08  8 diagnostic call sites → .isAniq()
Phase 1.3  7e5ab4e  7 expression kinds covered (&x, *p, !b, ternary, ...)
           364ce63  o'zbeklashtirish: Known→Aniq, Unknown→Nomalum, etc.
Phase 1.4  0ab1d49  Polimorf for template-function bodies
           3825c79  parser: drop 'satr' from uzbekKeywords
Phase 2.1  b0dbeee  Type composite kinds (Korsatkich/Havola/Shablon) + stable intern
           2b3709a  lsp: semantic tokens highlight class/struct members
Phase 2.2  a542b0c  Type through LSP boundary (getInferredAutoType → const Type*)
Phase 2.3  61ea1bf  Polimorf for template-class bodies
Phase 2.4  caa68c1  Composite types containing template params → Polimorf
```

Earlier session work (matematika+sinov+hardening) preserved:

```
6792652  feat(stdlib+lang): matematika+sinov ports, compiler hardening, 4 pending → caught
```

After this session and the v2.2.0 cut: clean tree, both tags pushed.

---

## 2. Verify (run this first to confirm nothing rotted)

```bash
# Where to work
cd /c/Users/MSN/uz++                                   # main repo on `main` branch
# OR cd /c/Users/MSN/uz++/.claude/worktrees/musing-satoshi-e40317   # active worktree

# Build
cmake --build build_wt 2>&1 | tail -3

# Positive regression (expects 74/74)
pass=0; fail=0; failed=""
for f in tests/*.uzpp; do
  ./build_wt/uzpp.exe qurish "$f" 2>&1 | grep -q "MUVAFFAQIYAT: Dastur tayyor" && pass=$((pass+1)) || { fail=$((fail+1)); failed="$failed $(basename $f)"; }
done
echo "REGRESSION: $pass/$((pass+fail))"
[ -n "$failed" ] && echo "FAIL:$failed"

# Negative tests (expects 51 caught / 0 missed)
bash tests/negative/run.sh build_wt/uzpp.exe 2>&1 | tail -5

# Frontend smoke (assertion-based unit tests)
./build_wt/uzpp_frontend_tests.exe
```

If anything's not 74/74 + 51/51 + smoke green, STOP — investigate before
adding features. The local commits queue assumes this baseline.

---

## 3. Online-services cheat sheet

All actions below are **explicit user authorisation required** — never run
them without "go ahead" in the new session. The handoff documents the
HOW, the user controls the WHEN.

### 3a. Push to GitHub (`origin/main`)

```bash
cd /c/Users/MSN/uz++
git push origin main
```

What happens:
- Triggers `.github/workflows/ci.yml` (build + tests on Windows MSYS2 + Linux gcc-15).
- Does NOT trigger `release.yml` (that's tag-only).

Watch:
```bash
gh run list --limit 3
gh run watch <run-id> --exit-status
```

### 3b. Cut a new release (`vX.Y.Z` tag → release workflow)

```bash
# 1. Bump VSCode extension version + CHANGELOG (manual edit + commit):
#    vscode-uzpp/package.json: "version": "X.Y.Z"
#    vscode-uzpp/CHANGELOG.md: prepend new entry
#    CHANGELOG.md (repo root): prepend new entry

git add vscode-uzpp/package.json vscode-uzpp/CHANGELOG.md CHANGELOG.md
git commit -m "chore: bump v X.Y.Z + CHANGELOG"
git push origin main

# 2. Tag + push tag
git tag -a vX.Y.Z -m "uz++ vX.Y.Z — short release blurb

See CHANGELOG.md for full notes."
git push origin vX.Y.Z
```

`release.yml` then runs 7 jobs in parallel (~10–20 min total):
- `build-windows` → `uzpp-windows-x64.zip`
- `build-linux` → `uzpp-linux-x64.tar.gz` (Ubuntu 24.04 + gcc-15 PPA + libstdc++-15-dev + libx11/libgl)
- `build-macos-arm64` → `uzpp-macos-arm64.tar.gz` (macos-14 runner)
- `package-mingw` → `mingw-w64-windows-x64.zip` (~150 MB after slim-down)
- `build-windows-installer` → `uzpp-setup.exe` (Inno Setup, ~150 MB)
- `release` → creates GitHub Release with all artifacts + install.sh
- `publish-extension` → publishes to VSCode Marketplace via VSCE_PAT secret

**`publish-extension` fails if the version already exists in Marketplace** —
this is expected when re-tagging the same version. The release itself still
gets created with all artifacts.

### 3c. Build and publish .vsix locally (skip the workflow)

```bash
cd /c/Users/MSN/uz++/vscode-uzpp

# Build .vsix
npx --yes @vscode/vsce package --allow-missing-repository
# → produces uzpp-X.Y.Z.vsix

# Publish (need the PAT, stored in GitHub secret VSCE_PAT)
# Token belongs to the 'uzpp' publisher on Visual Studio Marketplace.
npx --yes @vscode/vsce publish --packagePath uzpp-X.Y.Z.vsix -p "$VSCE_PAT"
```

**VSCE_PAT** — kept in GitHub Actions secret `VSCE_PAT` (set 2026-05-18).
If you need it locally, ask user. **Never hard-code it.** A literal token
appeared in earlier turns of the prior session — that disclosure was
intentional (user-provided), but the token is the user's, not for storage.

### 3d. Re-tag a version (if first attempt's release failed)

```bash
gh release delete vX.Y.Z --yes --cleanup-tag
git tag -d vX.Y.Z
git push --delete origin vX.Y.Z  # if remote tag was already pushed
git tag -a vX.Y.Z -m "..."        # recreate on the new commit
git push origin vX.Y.Z
```

`gh release delete --cleanup-tag` removes both the GH Release object AND
the remote tag, so you don't need the separate `--delete` push.

### 3e. Cancel a stuck run

```bash
gh run cancel <run-id>
```

Common reasons we cancelled previously:
- `macos-13` runner deprecated → `build-macos-x64` stayed queued forever (dropped that job in `3f22ba1`).
- Release failed early on Linux job → cancel rather than wait for the rest.

### 3f. Regenerate a stdlib `.hpp` from its `.uzpp`

Critical procedure — `stdlib/<name>.hpp` for ports that went self-hosted
(`matn`, `xatoliklar`, `vaqt`) is a **generated artefact**. Never edit by
hand. To regenerate:

```bash
cd /c/Users/MSN/uz++  # or the active worktree — sync both later

# 1. Temporarily disable the include so transpile doesn't double-define
mv stdlib/<name>.hpp /tmp/<name>.hpp.bak
sed -i 's|^#include "<name>.hpp"|// #include "<name>.hpp"|' stdlib/uzpp_runtime.hpp

# 2. Compile any test that ulash-imports the .uzpp — the buildUzppHeaderDependencies
#    plumbing writes build/<name>.hpp as a side effect.
rm -rf build
./build_wt/uzpp.exe qurish tests/test_<name>_include.uzpp 2>&1 | tail -3

# 3. Install the freshly-generated header
cp build/<name>.hpp stdlib/<name>.hpp

# 4. Restore the include
sed -i 's|^// #include "<name>.hpp"|#include "<name>.hpp"|' stdlib/uzpp_runtime.hpp

# 5. ALWAYS sync both — main repo and active worktree have separate stdlib/
cp stdlib/<name>.hpp /c/Users/MSN/uz++/.claude/worktrees/musing-satoshi-e40317/stdlib/<name>.hpp
# (or in the other direction; check md5sum after to confirm)
```

---

## 4. Don't break these (rules for next agent)

The full memory file has 18 hard-won gotchas. The ones most likely to bite
this session:

1. **Don't push or bump versions without explicit user OK.** User has said
   so 5+ times. Local commits only by default.
2. **Don't edit `stdlib/{matn,xatoliklar,vaqt,matematika,sinov}.hpp` by hand.**
   Generated artefacts. Edit the `.uzpp` source and regenerate (3f).
3. **Don't reintroduce `_`-mangling for apostrophes.** `o'lcham` must stay
   `oʼlcham` (U+02BC). `tests/test_apostrof.uzpp` pins this.
4. **Don't reintroduce class-trailing-`requires`.** `class X requires C {}`
   is invalid C++. Use the template-wrapper form
   (`shablon<T> shart(C) sinf X`).
5. **Don't re-add `teskari` → `std::reverse` alias.** It conflicts with
   `uzpp::Matn::teskari`. Real `std::reverse` callers must write it.
6. **Parser holds `const std::vector<Token>&` — keep tokens alive.**
   `Parser parser(lexer.tokenize())` is use-after-free. Always
   `const auto tokens = lexer.tokenize(); Parser parser(tokens);`.
7. **Worktree has its own stdlib/.** `C:/Users/MSN/uz++/stdlib/` AND each
   `.claude/worktrees/<name>/stdlib/`. Sync when regenerating.
8. **Disk-full → silent linker fail** (>95% on `C:`). `ld returned 1 exit
   status` with no real error text? Check `df -h /c` first.
9. **Include guards in generated .hpp**: use `UZPP_GEN_<NAME>_HPP_`, NOT
   `_UZPP_GEN_<NAME>_HPP_`. GCC drops leading-underscore-uppercase macros.
10. **`yoki` after `}` = `else` (only there).** Everywhere else it's `||`.
    Don't reuse that parser branch for anything else.
11. **`mantiq`/`mantiqiy` are equivalent (both → `bool`).** Same for
    `haqiqiy`/`ikkilangan` (both → `double`). `typesEquivalent()` in
    `type_checker.hpp` handles this — don't add direct `!=` comparisons
    that bypass it.
12. **Apostrophe (U+0027) → U+02BC mapping** in `CodeGen::safeIdent`. UTF-8
    high bytes (>= 0x80) pass through verbatim as identifier characters
    (Unicode identifier support — `tests/test_unicode_identifiers.uzpp`).
13. **`parseVariableDeclaration` rejects keyword-as-varname** but with an
    allowlist `{asosiy, main, yangi, bosh, bekor}` (+ `satr` is dropped from
    `isUzbekKeyword` in 3825c79). `yangi`/`bosh`/`bekor` are alias keywords
    that the parser/codegen handle via lookahead or `localScopes_`. If you
    extend `isUzbekKeyword`, check whether any new entry should be added
    to the allowlist (`tests/frontend_smoke.cpp` pins `butun yangi = 5`).
14. **Type's tri-state contract: never emit diagnostics on Nomalum / Polimorf
    operands.** This is the whole point of Phase 1. If you add a new
    diagnostic that compares types, gate it on `.isAniq()` for BOTH sides.
    A regression here is invisible (false-silent or false-noisy) until a
    user complains — frontend_smoke has pins to catch the obvious cases.
15. **Type::intern uses `unordered_map<string, Type>`** (NOT `std::vector`).
    The original draft used a vector and was use-after-free across
    reallocations. Don't "optimise" back to vector — the structural key
    (`<kind-char>:<aniqNomi()>`) keeps the map size proportional to unique
    type count, not call count.
16. **`looksLikeTemplateParam` + `typeMentionsTemplateParam` heuristics
    define what counts as a template type parameter.** They look at: starts
    with uppercase, all alnum/underscore, not in `classes_`/`typeAliases_`/
    `wellKnownTypes`. If you add a new stdlib type that starts with a single
    uppercase letter (e.g. `T`, `K`), add it to `wellKnownTypes` in
    `looksLikeTemplateParam` to prevent false-positive Polimorf.
17. **`inferredAutoTypes_` stores `Type` (not `std::string`).** Phase 2.2
    changed the storage type and the public API. If you write code that
    expects `getInferredAutoType(var)` to return `const std::string*`,
    you're on the old code path — update to `const Type*` and call
    `.aniqNomi()` (bare name) or `.tasvirla()` (annotated for hover).
18. **VSCode extension version policy** — only bump when user explicitly says.
    Marketplace v2.2.0 (this session) supersedes v2.1.9. Don't republish
    .vsix manually unless the workflow `publish-extension` failed.

---

## 5. Open work the user might pick

In rough priority order. The user picks; don't preempt.

### 🟢 Done this session — won't bite again

- **Phase 1.1–1.4** — tri-state Type, diagnostic gates, expression coverage,
  Polimorf for template functions (5 commits).
- **Phase 2.1–2.4** — composite Type with stable intern, Type at LSP
  boundary, Polimorf for template classes, composite-template detection
  (4 commits).
- **LSP semantic tokens for class members** — 2b3709a.
- **Parser: drop `satr` from uzbekKeywords** — 3825c79.

### 🟡 Stdlib ports — repeat the matn/xatoliklar/vaqt/matematika/sinov pattern

Order of difficulty (LOC of original .hpp):

- **`jurnal`** (93) — logging. Trivial.
- **`apparat`** (100) — hardware utilities.
- **`platforma`** (99) — platform detection.
- **`veb_ui`** (136) — UI primitives.
- **`kripto`** (276) — hash/encrypt. Needs careful UTF-8 handling.
- **`json`** (324) — variant + shared_ptr recursion. **AMBITIOUS** — may
  surface TypeChecker gaps for recursive types. Allocate 2 sessions.
- **`fayl_tizimi`** (302) — `<filesystem>` interop.
- **`malumotlar_bazasi`** (551) — DB connector. Networking + lifetime stuff.
- **`tarmoq`** (572) — HTTP server + client. Big.
- **`oyna`** (674) — OpenGL/X11 graphics. Big + platform-specific.

(Done so far: `matn`, `xatoliklar`, `vaqt`, `matematika`, `sinov`.)

### 🟡 LSP polish

- **Type-mismatch quick-fix smarter** (currently in 2eeb3aa it offers
  `statik_otkazish<T>(...)` — make it actually parse the expression and
  emit a clean wrap).
- **Semantic-tokens for inherited members** — currently only direct class
  members (own fields + methods); ancestor classes' members aren't
  highlighted yet.
- **Hover for non-auto variables** — currently only `o'zgaruvchan x = ...`
  triggers inferred-type hover. Hovering over a parameter or a regular
  declared variable doesn't show the structural Type yet.

### 🔴 Multi-session work (the next big architectural levers)

- **Phase 3 — Overload resolution.** `functionReturns_` is currently a
  flat `map<string, string>` (last declaration wins). Convert to
  `map<string, vector<Overload>>`, add ranking (exact > promotion >
  standard conversion > user-defined > variadic), diagnose ambiguous
  and no-viable. 2–3 sessions. Most visible user-facing improvement
  after Phase 2.
- **Phase 4 — Lazy template instantiation.** Store template bodies as
  AST; at the first call site with concrete args, substitute and
  type-check the instantiated body. This is what gives real
  template-body checking without unification/SFINAE machinery. 3
  sessions.
- **Phase 5 — Constant evaluation.** Constexpr expression evaluator;
  `agar sobit_ifoda` actually eliminates dead branches; honest
  `statik_tasdiqlash`. 2–3 sessions.
- **Variadic templates with `std::format_string<Args...>`** — small parser
  change, big test surface. Defer until someone needs it.

### Push & release

- Already shipped this session: tag `v2.2.0`, Marketplace `uzpp.uzpp@2.2.0`,
  GitHub Release with installer + MinGW bundle + Linux/macOS tarballs.
- Next release procedure unchanged — see section 3b.

---

## 6. Files to skim if you have context budget

- `~/.claude/projects/C--Users-MSN-uz--/memory/project_uzpp.md` — full
  project context (must-read).
- `NEXT_SESSION_PLAN.md` — this session's plan + fragile-points list.
- `tests/negative/README.md` — negative-test conventions.
- `installer/windows/installer.iss` — Inno Setup config (only edit if
  changing what `uzpp-setup.exe` bundles).
- `installer/unix/install.sh` — Linux/macOS one-liner installer.
- `.github/workflows/release.yml` — 7-job release pipeline.
- `.github/workflows/ci.yml` — push-triggered CI.

---

## 7. If something breaks

- **Regression drops** → bisect: `git log --oneline | head -16`; revert
  suspect commit; rerun regression; surface to user.
- **CI fails on Linux** → check `tests/.ci_skip_linux` first (known
  toolchain gates). If new failure, look for libstdc++/PPA issues.
- **`vsce publish` fails "already exists"** → expected when re-tagging
  same version. Don't bump version unless user agrees.
- **`mingw-w64-windows-x64.zip` < 100 MB in release** → `package-mingw`
  download silently failed (curl needs `-f`). Already fixed in 8d3e427
  + sanity check.
- **Stress test surfaces a new fragile point** → add to
  `tests/negative/` (pin) AND to `NEXT_SESSION_PLAN.md` (document) AND
  to memory gotchas (so future agents don't re-fall into it).

---

## 8. Quick orientation in repo layout

```
src/
  lexer.cpp/.h          tokenizer
  parser.cpp/.h         recursive-descent → AST
  ast.h                 AST nodes
  type_checker.hpp      semantic analysis (incomplete, see 🔴)
  codegen.cpp/.h        AST → C++23
  formatter.cpp/.h      uz++ → uz++ pretty-print
  lsp_server.cpp/.h     LSP server (AST-aware as of 9915302)
  dap_server.cpp/.h     DAP server (limited variable rendering — future work)
  main.cpp              CLI driver (transpile/compile/run/test/bench/lsp/dap/...)
  error_remap.h         g++ output line-mapping back to .uzpp
  package_manager.h     uzpp.toml + dependency handling
stdlib/
  uzpp_runtime.hpp      umbrella header (#includes everything)
  matn.hpp              generated from matn.uzpp
  matn.uzpp             SOURCE for matn (uz++)
  xatoliklar.hpp        generated from xatoliklar.uzpp
  xatoliklar.uzpp       SOURCE
  vaqt.hpp              generated from vaqt.uzpp
  vaqt.uzpp             SOURCE
  *.hpp                 (everything else: hand-written C++)
tests/
  *.uzpp                72 positive regression tests
  frontend_smoke.cpp    C++ unit tests (TypeChecker / CodeGen helpers)
  negative/             47 must-fail tests + run.sh runner
    pending/            4 known gaps (file there means "still accepts
                        what it shouldn't" — fix moves back up)
vscode-uzpp/
  package.json          extension manifest (v2.1.9 in Marketplace)
  extension.js          welcome screen + commands
  componentManager.js   download/install uzpp from GitHub releases
  CHANGELOG.md          extension-specific changelog
  README.md             Marketplace listing
installer/
  windows/installer.iss Inno Setup config for uzpp-setup.exe
  unix/install.sh       Linux/macOS one-liner installer
.github/workflows/
  ci.yml                push-triggered build + test (Win + Linux)
  release.yml           tag-triggered: 7-job release + Marketplace
```
