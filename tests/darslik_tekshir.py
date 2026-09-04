#!/usr/bin/env python3
"""Hujjatlardagi har bir kod blokini haqiqiy uzpp kompilyatori bilan tekshiradi.

Misollar — foydalanuvchi ko'radigan birinchi kod. Ular ishlamasa, til
ishlamaydi. Skript `docs/darslik/*.md`, `README.md` va
`docs/getting-started.md` dagi har bir ```cpp blokini ajratib oladi va
uzpp orqali o'tkazadi.

Ishlatish:

    python tests/darslik_tekshir.py build/uzpp.exe              # transpilyatsiya
    python tests/darslik_tekshir.py build/uzpp --qurish         # to'liq qurish (g++)
    python tests/darslik_tekshir.py build/uzpp --bob 04         # bitta bob

Blok oldidagi HTML izohlar bilan boshqariladi:

    <!-- darslik:skip -->    bu blokni umuman tekshirmaslik (psevdokod, C++
                             solishtiruvi, tashqi kutubxona talab qiladigan kod)
    <!-- darslik:xato -->    ataylab noto'g'ri kod — kompilyatsiya KUTILMAYDI
    <!-- darslik:parcha -->  funksiya tanasi ichidagi parcha (asosiy ichiga o'raladi)
    <!-- darslik:global -->  global e'lonlar parchasi

Direktiva bo'lmasa, blok mazmuniga qarab avtomatik tasniflanadi.

Chiqish kodi: 0 — hamma narsa joyida; 1 — kutilmagan xato bor.
"""
import os
import re
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DOCS = os.path.join(ROOT, "docs", "darslik")

# Darslikdan tashqari — foydalanuvchi ko'radigan boshqa hujjatlar.
# Ularda ham kod misollari bor va ular ham ishlashi shart.
EXTRA_DOCS = [
    os.path.join(ROOT, "README.md"),
    os.path.join(ROOT, "docs", "getting-started.md"),
]

FENCE = re.compile(r"^```(\w*)\s*$")
DIRECTIVE = re.compile(r"<!--\s*darslik:(\w+)\s*-->")

# Ataylab noto'g'ri kodning odatiy belgilari (darslik "shunday qilmang" deydi).
BAD_MARKERS = [
    "XATO", "xato!", "crash!", "✗", "ISHLAMAYDI", "ishlamaydi!",
    "KOMPILYATSIYA XATOSI", "kompilyatsiya xatosi", "noto'g'ri!",
    "Xato:", "xatolik!", "sizib chiqish", "sig'maydi",
]
CPP_ONLY = ["int main(", "std::cout <<", "#include <", "return 0;"]
UZPP_HINTS = [
    "yozish", "qaytarish", "butun ", "o'zgaruvchan", "agar ", "sinf ",
    "funksiya ", "matn ", "toki ", "ulash ", "haqiqiy ", "mantiqiy ",
    "uchun (", "moslash", "tuzilma ", "bajar ", "shablon",
]


def _doc_paths(only=None):
    for fn in sorted(os.listdir(DOCS)):
        if not fn.endswith(".md"):
            continue
        if only and not fn.startswith(only):
            continue
        yield fn, os.path.join(DOCS, fn)
    if not only:
        for path in EXTRA_DOCS:
            if os.path.exists(path):
                yield os.path.basename(path), path


def blocks(only=None):
    for fn, path in _doc_paths(only):
        lines = open(path, encoding="utf-8").read().split("\n")
        i = 0
        while i < len(lines):
            m = FENCE.match(lines[i])
            if not m:
                i += 1
                continue
            lang, start = m.group(1), i + 1
            j = start
            while j < len(lines) and not FENCE.match(lines[j]):
                j += 1
            directive = None
            for k in range(max(0, i - 3), i):
                d = DIRECTIVE.search(lines[k])
                if d:
                    directive = d.group(1)
            yield fn, start + 1, lang, "\n".join(lines[start:j]), directive
            i = j + 1


def looks_like_code(body):
    """ASCII-diagramma, dastur chiqishi yoki oddiy matn — kod emas."""
    stripped = [l for l in body.split("\n") if l.strip()]
    if not stripped:
        return False
    boxy = sum(1 for l in stripped if any(c in l for c in "─│┌┐└┘├┤▼←→↑↓╔╗╚╝═║^"))
    if boxy >= max(1, len(stripped) // 3):
        return False
    codey = sum(1 for l in stripped if (";" in l or "{" in l or "}" in l))
    return codey >= max(1, len(stripped) // 4)


def classify(body, directive):
    if directive in ("skip", "xato", "parcha", "global"):
        return directive
    if any(mk in body for mk in BAD_MARKERS):
        return "xato"
    has_uzpp = any(mk in body for mk in UZPP_HINTS)
    has_cpp = any(mk in body for mk in CPP_ONLY)
    if not looks_like_code(body):
        return "matn"
    if has_cpp and not has_uzpp:
        return "cpp"
    if not has_uzpp:
        return "matn"
    return "uzpp"


def wrap(body, kind):
    head = "" if 'ulash "uzpp_runtime.hpp"' in body else 'ulash "uzpp_runtime.hpp"\n'
    if re.search(r"\basosiy\s*\(", body):
        return [head + body]
    if kind == "parcha":
        return [head + "butun asosiy() {\n" + body + "\nqaytarish 0;\n}\n"]
    if kind == "global":
        return [head + body + "\n\nbutun asosiy() { qaytarish 0; }\n"]
    return [
        head + body + "\n\nbutun asosiy() { qaytarish 0; }\n",
        head + "butun asosiy() {\n" + body + "\nqaytarish 0;\n}\n",
    ]


def main():
    args = [a for a in sys.argv[1:]]
    mode = "qurish" if "--qurish" in args else "transpile"
    args = [a for a in args if a != "--qurish"]
    only = None
    if "--bob" in args:
        idx = args.index("--bob")
        only = args[idx + 1]
        del args[idx:idx + 2]
    uzpp = args[0] if args else os.path.join(ROOT, "build", "uzpp.exe")
    if not os.path.exists(uzpp):
        print("XATO: uzpp topilmadi -> %s" % uzpp)
        return 1
    # MUTLAQ yo'lga aylantiramiz: bloklar vaqtinchalik papkada (cwd=work)
    # quriladi, nisbiy `build/uzpp` esa POSIX da BOLA jarayonning cwd siga
    # nisbatan qidiriladi va topilmaydi.
    uzpp = os.path.abspath(uzpp)

    work = tempfile.mkdtemp(prefix="darslik_")
    tested = failed = skipped = 0
    problems = []

    for fn, line, lang, body, directive in blocks(only):
        if lang not in ("cpp", "uzpp", ""):
            continue
        kind = classify(body, directive)
        if kind in ("skip", "matn", "cpp", "xato"):
            skipped += 1
            continue
        if mode == "qurish" and not re.search(r"\basosiy\s*\(", body):
            skipped += 1
            continue

        tested += 1
        ok, out = False, ""
        for k, variant in enumerate(wrap(body, kind)):
            path = os.path.join(work, "b%s_%d_v%d.uzpp" % (fn[:2], line, k))
            with open(path, "w", encoding="utf-8") as f:
                f.write(variant)
            try:
                r = subprocess.run([uzpp, mode, path], capture_output=True, text=True,
                                   timeout=300, encoding="utf-8", errors="replace",
                                   cwd=work)
                if r.returncode == 0:
                    ok = True
                    break
                out = (r.stdout or "") + (r.stderr or "")
            except subprocess.TimeoutExpired:
                out = "TIMEOUT"
        if not ok:
            failed += 1
            problems.append((fn, line, out[-1500:]))

    print("=" * 60)
    print(" Darslik kod bloklari — %s" % mode)
    print("   tekshirildi: %d" % tested)
    print("   o'tdi:       %d" % (tested - failed))
    print("   yiqildi:     %d" % failed)
    print("   o'tkazildi:  %d  (psevdokod / ataylab xato / matn)" % skipped)
    print("=" * 60)

    for fn, line, out in problems:
        print()
        print("--- %s:%d ---" % (fn, line))
        print(out)

    if failed:
        print()
        print("Darslikdagi kod kompilyatordan o'tmadi. Yo kodni tuzating,")
        print("yo blok oldiga <!-- darslik:skip --> / <!-- darslik:xato --> qo'ying.")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
