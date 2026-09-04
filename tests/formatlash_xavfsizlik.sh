#!/usr/bin/env bash
# Formatlagich hech qachon kodni buzmasligi kerak.
#
# `uzpp formatlash` faylni QAYTA YOZISHDAN OLDIN natijani tekshiradi:
# formatlangan matnni qayta leksiy qilib, token oqimini asl fayl bilan
# solishtiradi. Farq bo'lsa — fayl tegilmaydi va xato qaytariladi.
#
# Bu skript shu shartnomani pinlaydi: har bir tests/*.uzpp nusxasi
# formatlanadi; agar formatlagich "Formatlandi" desa, fayl HAMON
# qurilishi SHART. "Rad etildi" — bu xato emas, bu xavfsiz xatti-harakat.
#
# Ishlatish:  bash tests/formatlash_xavfsizlik.sh build/uzpp.exe

set -u

UZPP="${1:-build/uzpp.exe}"
if [ ! -x "$UZPP" ] && [ ! -f "$UZPP" ]; then
    echo "XATO: uzpp topilmadi -> $UZPP" >&2
    exit 1
fi
UZPP_ABS="$(cd "$(dirname "$UZPP")" && pwd)/$(basename "$UZPP")"
TESTS_DIR="$(cd "$(dirname "$0")" && pwd)"

work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT
cp "$TESTS_DIR"/*.uzpp "$work/" 2>/dev/null

formatted=0
refused=0
broken=0
broken_list=""

cd "$work"
for f in *.uzpp; do
    out="$("$UZPP_ABS" formatlash "$f" 2>&1)"
    if echo "$out" | grep -q "Formatlandi"; then
        if "$UZPP_ABS" qurish "$f" 2>&1 | grep -q "MUVAFFAQIYAT"; then
            formatted=$((formatted + 1))
        else
            broken=$((broken + 1))
            broken_list="$broken_list $f"
        fi
    else
        refused=$((refused + 1))
    fi
done

echo "============================================="
echo " Formatlagich xavfsizligi"
echo "  formatlandi va quriladi: $formatted"
echo "  rad etildi (xavfsiz):    $refused"
echo "  BUZILDI (regressiya!):   $broken"
echo "============================================="

if [ "$broken" -ne 0 ]; then
    echo "BUZILGAN FAYLLAR:$broken_list" >&2
    echo "Formatlagich kodni buzdi — xavfsizlik tekshiruvi ishlamayapti." >&2
    exit 1
fi
exit 0
