# uz++ — Session Handoff

> **Read this FIRST in a new session.** Then `~/.claude/projects/C--Users-MSN-uz--/memory/project_uzpp.md`
> for deeper context (file map, hard-won gotchas, architecture).
> `NEXT_SESSION_PLAN.md` has prioritised work suggestions.

Last updated: end of session ending 2026-05-19 (matematika+sinov ports + hardening).

---

## 1. Snapshot

| Metric | Value |
|---|---|
| **Positive tests** | **74/74** (Windows MSYS2 g++ 15.2) |
| **Negative tests** | **51/51** caught (0 pending — all four ex-pending tests now caught) |
| **frontend_smoke** | green |
| **Linux CI** | 72/74 (`test_deducing_this` + `test_coyield` in `.ci_skip_linux` — Ubuntu toolchain gates, not uz++ bugs) |
| **Stdlib ports** | 5 self-hosted (`matn`, `xatoliklar`, `vaqt`, `matematika`, `sinov`) — remaining stdlib still C++ |
| **Latest pushed tag** | `v2.1.9` (Marketplace + GitHub Release live) |
| **Local commits ahead of `origin/main`** | **18** (none pushed this session) |

### Local commits queue (in order, oldest → newest)

```
5db4656  docs: refresh NEXT_SESSION_PLAN with current state (post v2.1.9)
0f34978  feat(stdlib): port xatoliklar to uz++
9915302  feat(lsp): AST-aware definition / references / rename
f4ca448  feat(lang): namespace aliases + trailing requires on classes (stub — fixed in 848cc20)
0a0c6e2  docs(plan): mark (A)/(B)/(C) done
848cc20  fix(lang+lsp): namespace alias actually emits + drop bogus class trailing-requires
ecfbe02  docs(plan): record (G)/(H)/namespace-alias-fix done
0f41b92  feat(stdlib): port vaqt to uz++
2eeb3aa  feat(lang+lsp): module partitions + type-mismatch code actions
55bc932  feat(formatter): preserve inline comments inside statements
7d903f3  ci(release): slim MinGW zip from 255 MB to ~150 MB
51beb10  fix(cli): catch missing asosiy() with a helpful message, not 'ld returned 5'
b765e16  docs(plan): record stress-test findings + 71/71 + 29/29 negative
6e1a881  feat(lexer): accept Unicode (UTF-8) identifiers — Cyrillic, CJK, Arabic
8cf7f45  test(negative): +18 regression-pins (lexer/parser/type/cpp_level)
01eb21e  docs(plan): mark hardening progress (72/72 + 47/47)
43789ac  docs: HANDOFF.md — self-handoff brief for the next session
6792652  feat(stdlib+lang): matematika+sinov ports, compiler hardening, 4 pending → caught
```

### Working tree

Clean — no uncommitted changes.

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

The full memory file has 16 hard-won gotchas. The ones most likely to bite
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
7. **Worktree has its own stdlib/.** `C:/Users/MSN/uz++/stdlib/` AND
   `.../worktrees/musing-satoshi-e40317/stdlib/`. Sync when regenerating.
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
    allowlist `{asosiy, main, yangi, bosh, bekor}`. `yangi`/`bosh`/`bekor`
    are alias keywords that the parser/codegen handle via lookahead or
    `localScopes_`. If you extend `isUzbekKeyword`, check whether any new
    entry should be added to the allowlist (`tests/frontend_smoke.cpp:493`
    pins `butun yangi = 5` as valid).

---

## 5. Open work the user might pick

In rough priority order. The user picks; don't preempt.

### 🟢 Tiny wins (each: half-session or less)

- **`asosiy()` return-type check** — done in 6792652 (parsing-time error).
- **Compile-time div-by-zero** — done in 6792652 (`10 / 0` flagged in TypeChecker).
- **Empty `moslash { }`** — done in 6792652 (parse error).
- **Keyword-as-varname guard** — done in 6792652 with allowlist (gotcha #13).
- **`mantiq`/`mantiqiy` warning cleanup** — already done in 848cc20.
- **Module partitions** — already done in 2eeb3aa.
- **LSP hover with inferred types** — done in 6792652.

### 🟡 Stdlib ports — repeat the matn/xatoliklar/vaqt pattern

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
- ~~**Hover with inferred types** for `o'zgaruvchan` vars.~~ Done in 6792652.
- **Semantic-tokens for class members** (currently only top-level names).

### 🔴 Multi-session work (don't pick without runway)

- **TypeChecker honesty** — templates body types, partial spec, SFINAE,
  `if constexpr` dead-branch. Architectural. 3+ sessions.
- **Variadic templates with `std::format_string<Args...>`** — small parser
  change, big test surface. Defer until someone needs it.

### Push & release (only on user request)

- 18 local commits accumulated. A push + tag `v2.2.0` would ship:
  - 5 self-hosted stdlib modules (`matn`, `xatoliklar`, `vaqt`, `matematika`,
    `sinov`), AST-aware LSP, namespace aliases (working), module partitions,
    inline-comments formatter, Unicode identifiers, code actions expansion,
    asosiy() check, keyword-as-varname guard, compile-time div-by-zero,
    LSP hover with inferred types, MinGW slim-down.
- See section 3b for the exact procedure.

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
