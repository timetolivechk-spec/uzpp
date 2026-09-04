#!/usr/bin/env bash
# ============================================================================
#  uz++ Unix installer — Linux + macOS
# ----------------------------------------------------------------------------
#  Bir qatorli o'rnatish:
#
#      curl -fsSL https://github.com/timetolivechk-spec/uzpp/releases/latest/download/install.sh | bash
#
#  Nima qiladi:
#    1. Platformani aniqlaydi (Linux x64 / macOS arm64 / macOS x64).
#    2. C++ kompilyatori borligini tekshiradi (g++/clang++).
#    3. Mos tarballni oxirgi relizdan yuklab oladi.
#    4. uzpp -> <prefix>/bin/uzpp, stdlib -> <prefix>/lib/uzpp/ ga qo'yadi.
#    5. macOS da karantin belgisini olib tashlaydi va ad-hoc imzo qo'yadi
#       (Gatekeeper "ishlab chiquvchini tekshirib bo'lmadi" xatosi shundan).
#    6. Sinov: `uzpp --version` + haqiqiy kichik dastur qurish.
#
#  Sozlash:
#    UZPP_VERSION=v2.3.0  ./install.sh    # aniq reliz
#    UZPP_PREFIX=$HOME    ./install.sh    # ~/bin + ~/lib ga o'rnatish
# ============================================================================

set -euo pipefail

REPO="timetolivechk-spec/uzpp"
UZPP_VERSION="${UZPP_VERSION:-latest}"
UZPP_PREFIX="${UZPP_PREFIX:-/usr/local}"

bold()   { printf '\033[1m%s\033[0m\n' "$*"; }
green()  { printf '\033[32m%s\033[0m\n' "$*"; }
yellow() { printf '\033[33m%s\033[0m\n' "$*"; }
red()    { printf '\033[31m%s\033[0m\n' "$*" >&2; }

# ─── 1. Platformani aniqlash ────────────────────────────────────────────────
os="$(uname -s)"
arch="$(uname -m)"

case "$os" in
    Linux)
        case "$arch" in
            x86_64|amd64) asset="uzpp-linux-x64.tar.gz"; platform="Linux x64" ;;
            *) red "Linux $arch hozircha qo'llab-quvvatlanmaydi (faqat x86_64)."
               red "Manbadan qurish: https://github.com/$REPO#manbadan-qurish"
               exit 1 ;;
        esac
        ;;
    Darwin)
        case "$arch" in
            arm64)  asset="uzpp-macos-arm64.tar.gz"; platform="macOS Apple Silicon" ;;
            x86_64) asset="uzpp-macos-x64.tar.gz";   platform="macOS Intel" ;;
            *) red "macOS $arch hozircha qo'llab-quvvatlanmaydi."; exit 1 ;;
        esac
        ;;
    *) red "Tizim $os hozircha qo'llab-quvvatlanmaydi (Linux / macOS)."; exit 1 ;;
esac

bold "uz++ installer — $platform"

# ─── 2. C++ kompilyatorini tekshirish ───────────────────────────────────────
need_cpp=true
for cxx in g++-15 g++-14 g++ clang++ c++; do
    if command -v "$cxx" >/dev/null 2>&1; then
        ver="$("$cxx" --version 2>/dev/null | head -1)"
        green "✓ C++ kompilyatori topildi: $cxx ($ver)"
        need_cpp=false
        break
    fi
done

if $need_cpp; then
    yellow "⚠ C++ kompilyatori (g++/clang++) topilmadi."
    if [ "$os" = "Linux" ]; then
        if command -v apt-get >/dev/null 2>&1; then
            yellow "  Debian/Ubuntu:  sudo apt-get install -y build-essential"
        elif command -v dnf >/dev/null 2>&1; then
            yellow "  Fedora/RHEL:    sudo dnf install -y gcc-c++"
        elif command -v pacman >/dev/null 2>&1; then
            yellow "  Arch:           sudo pacman -S gcc"
        else
            yellow "  Tizimingiz uchun gcc/g++ paketini o'rnating."
        fi
    else
        yellow "  macOS:  xcode-select --install     (Apple Command Line Tools)"
        yellow "          brew install gcc           (to'liq C++23 uchun tavsiya etiladi)"
    fi
    if [ -t 0 ]; then
        read -r -p "Davom etishni xohlaysizmi? [y/N] " ans
        [[ "$ans" =~ ^[Yy]$ ]] || exit 1
    else
        yellow "  Avtomatik rejim — davom etilmoqda. C++ kompilyatorisiz uzpp ishlamaydi."
    fi
fi

# ─── 3. Yuklab olish manzili ────────────────────────────────────────────────
if [ "$UZPP_VERSION" = "latest" ]; then
    url="https://github.com/$REPO/releases/latest/download/$asset"
else
    url="https://github.com/$REPO/releases/download/$UZPP_VERSION/$asset"
fi

bold "Yuklab olinmoqda: $url"
tmpdir="$(mktemp -d)"
trap 'rm -rf "$tmpdir"' EXIT

if ! curl -fL --retry 3 -o "$tmpdir/$asset" "$url"; then
    red "Yuklab bo'lmadi: $url"
    if [ "$os" = "Darwin" ] && [ "$arch" = "x86_64" ]; then
        red "macOS Intel uchun binar bu relizda bo'lmasligi mumkin."
        red "Rosetta orqali arm64 versiyasini sinab ko'ring yoki manbadan quring."
    fi
    exit 1
fi
size=$(stat -c %s "$tmpdir/$asset" 2>/dev/null || stat -f %z "$tmpdir/$asset")
green "✓ $size bayt yuklandi."

# ─── 4. Ochish + o'rnatish ──────────────────────────────────────────────────
bold "Ochilmoqda..."
tar -xzf "$tmpdir/$asset" -C "$tmpdir"

# Arxiv tuzilmasi: uzpp binari + stdlib/ yuqori darajada.
if [ ! -f "$tmpdir/uzpp" ] || [ ! -d "$tmpdir/stdlib" ]; then
    red "Arxiv kutilgan tuzilmaga ega emas (uzpp + stdlib/ topilmadi)."
    exit 1
fi

bin_dir="$UZPP_PREFIX/bin"
lib_dir="$UZPP_PREFIX/lib/uzpp"

# /usr/local uchun sudo kerak; $HOME uchun yo'q.
if [ -w "$UZPP_PREFIX" ] || [ "$UZPP_PREFIX" != "/usr/local" ]; then
    sudo_cmd=""
else
    sudo_cmd="sudo"
    yellow "  → $UZPP_PREFIX yozish uchun sudo so'raladi"
fi

$sudo_cmd mkdir -p "$bin_dir" "$lib_dir"
$sudo_cmd install -m 0755 "$tmpdir/uzpp" "$bin_dir/uzpp"
$sudo_cmd cp -R "$tmpdir/stdlib/." "$lib_dir/"

green "✓ Kompilator: $bin_dir/uzpp"
green "✓ Stdlib:     $lib_dir/"

# ─── 5. macOS: Gatekeeper / karantin ────────────────────────────────────────
#
# macOS yuklab olingan fayllarga `com.apple.quarantine` kengaytirilgan
# atributini qo'yadi. Notarizatsiya qilinmagan binar shu atribut bilan
# ishga tushirilsa, Gatekeeper uni bloklaydi:
#
#     "uzpp" cannot be opened because the developer cannot be verified.
#     Apple could not verify "uzpp" is free of malware.
#
# Bundan tashqari Apple Silicon da HAR QANDAY binar imzolangan bo'lishi
# SHART — imzosiz jarayon `zsh: killed` bilan darhol o'ldiriladi.
# Ikkalasini ham quyidagi ikki buyruq hal qiladi.
if [ "$os" = "Darwin" ]; then
    bold "macOS xavfsizlik sozlamalari..."

    if command -v xattr >/dev/null 2>&1; then
        $sudo_cmd xattr -dr com.apple.quarantine "$bin_dir/uzpp" 2>/dev/null || true
        $sudo_cmd xattr -dr com.apple.quarantine "$lib_dir" 2>/dev/null || true
        green "✓ Karantin belgisi olib tashlandi (com.apple.quarantine)"
    else
        yellow '⚠ xattr topilmadi — karantin belgisini qo'"'"'lda olib tashlang:'
        yellow "    sudo xattr -dr com.apple.quarantine $bin_dir/uzpp"
    fi

    if command -v codesign >/dev/null 2>&1; then
        # Ad-hoc imzo (`-`) — sertifikat talab qilmaydi, lekin Apple Silicon
        # uchun yetarli. Binar allaqachon imzolangan bo'lsa qayta imzolaymiz.
        if $sudo_cmd codesign --force --sign - "$bin_dir/uzpp" >/dev/null 2>&1; then
            green "✓ Ad-hoc imzo qo'yildi (codesign --sign -)"
        else
            yellow "⚠ codesign ishlamadi. Agar uzpp ishga tushmasa:"
            yellow "    sudo codesign --force --sign - $bin_dir/uzpp"
        fi
    fi
fi

# ─── 6. PATH tekshiruvi ─────────────────────────────────────────────────────
case ":$PATH:" in
    *":$bin_dir:"*) ;;
    *)
        yellow "⚠ $bin_dir PATH da yo'q. Qo'shish uchun:"
        if [ -n "${ZSH_VERSION:-}" ] || [ "$(basename "${SHELL:-}")" = "zsh" ]; then
            yellow "    echo 'export PATH=\"$bin_dir:\$PATH\"' >> ~/.zshrc && source ~/.zshrc"
        else
            yellow "    echo 'export PATH=\"$bin_dir:\$PATH\"' >> ~/.bashrc && source ~/.bashrc"
        fi
        ;;
esac

# ─── 7. Sinov ───────────────────────────────────────────────────────────────
bold "Sinov..."
if ! "$bin_dir/uzpp" --version >/dev/null 2>&1; then
    red "Sinov muvaffaqiyatsiz: $bin_dir/uzpp --version ishlamadi."
    if [ "$os" = "Darwin" ]; then
        red "macOS da bu odatda Gatekeeper sababli. Qo'lda bajaring:"
        red "    sudo xattr -dr com.apple.quarantine $bin_dir/uzpp"
        red "    sudo codesign --force --sign - $bin_dir/uzpp"
    fi
    exit 1
fi
"$bin_dir/uzpp" --version | head -1

# Haqiqiy uchdan-uchgacha sinov: kichik dastur qurish va ishga tushirish.
# `--version` faqat binarning ishga tushishini tekshiradi; bu esa C++
# kompilyatori va stdlib ham joyida ekanini tasdiqlaydi.
smoke_dir="$tmpdir/smoke"
mkdir -p "$smoke_dir"
cat > "$smoke_dir/salom.uzpp" <<'UZPP'
ulash "uzpp_runtime.hpp"

butun asosiy() {
    yozish << "Salom, dunyo!" << qator_oxiri;
    qaytarish 0;
}
UZPP

if (cd "$smoke_dir" && "$bin_dir/uzpp" ishga-tushirish salom.uzpp >/dev/null 2>&1); then
    green "✓ Sinov dasturi qurildi va ishga tushdi."
else
    yellow "⚠ uzpp ishga tushdi, lekin sinov dasturini qura olmadi."
    yellow "  Odatda bu C++ kompilyatori yo'qligini bildiradi. Tekshirish:"
    yellow "    cd $smoke_dir && $bin_dir/uzpp qurish salom.uzpp"
fi

green ""
green "✓ uz++ muvaffaqiyatli o'rnatildi."
green ""
green "  Tezda boshlash:"
green "    mkdir uzpp-test && cd uzpp-test"
green "    printf 'ulash \"uzpp_runtime.hpp\"\\n\\nbutun asosiy() {\\n    yozish << \"Salom!\" << qator_oxiri;\\n    qaytarish 0;\\n}\\n' > salom.uzpp"
green "    uzpp ishga-tushirish salom.uzpp"
green ""
green "  VS Code kengaytmasi: code --install-extension uzpp.uzpp"
green "  Darslik:             https://github.com/$REPO/blob/main/docs/darslik/00-mundarija.md"
