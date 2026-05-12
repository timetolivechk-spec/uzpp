#!/usr/bin/env bash
# Negative test runner — har bir tests/negative/*.uzpp fayli kompilyatsiyada
# xatoga olib kelishi kerak. Exit code 0 — regression deb hisoblanadi.
#
# Usage:
#   bash tests/negative/run.sh [path/to/uzpp]
#
# Default uzpp binary: build/uzpp.exe (Windows) or build/uzpp (POSIX).

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

UZPP="${1:-}"
if [ -z "${UZPP}" ]; then
    if [ -x "${REPO_ROOT}/build/uzpp.exe" ]; then
        UZPP="${REPO_ROOT}/build/uzpp.exe"
    elif [ -x "${REPO_ROOT}/build/uzpp" ]; then
        UZPP="${REPO_ROOT}/build/uzpp"
    else
        echo "XATO: uzpp binary topilmadi. Awwal: cmake --build build" >&2
        exit 2
    fi
fi

caught=0
missed=0
missed_files=""

# pending/ ichidagilar — bilingan kompilyator kamchiliklari, ular hozir
# topilmaydi. Faqat top-level fayllarni yugurtiramiz.
shopt -s nullglob
for f in "${SCRIPT_DIR}"/*.uzpp; do
    name="$(basename "$f")"
    "${UZPP}" qurish "$f" >/dev/null 2>&1
    rc=$?
    if [ $rc -eq 0 ]; then
        echo "  MISS  ${name}  (kompilyator xatoni topa olmadi — exit 0)"
        missed=$((missed + 1))
        missed_files="${missed_files} ${name}"
    else
        caught=$((caught + 1))
    fi
done

echo
echo "============================================="
echo " Salbiy testlar | Negative tests"
echo "  caught (xatolik aniqlandi):  ${caught}"
echo "  missed (regression!):        ${missed}"
echo "============================================="

if [ ${missed} -gt 0 ]; then
    echo "Topilmagan testlar:${missed_files}"
    exit 1
fi
exit 0
