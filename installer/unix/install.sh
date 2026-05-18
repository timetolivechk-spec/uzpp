#!/usr/bin/env bash
# ============================================================================
#  uz++ Unix installer — Linux + macOS
# ----------------------------------------------------------------------------
#  One-line install:
#
#      curl -fsSL https://github.com/timetolivechk-spec/uzpp/releases/latest/download/install.sh | bash
#
#  What it does:
#    1. Detects platform (Linux x64 / macOS arm64).
#    2. Checks for a C++ compiler (g++/clang++); on Linux it offers to
#       `apt install g++-15` if missing.
#    3. Downloads the matching uzpp tarball from the latest release.
#    4. Installs uzpp -> /usr/local/bin/uzpp, stdlib -> /usr/local/lib/uzpp/.
#    5. Runs a smoke test (uzpp --version).
#
#  Override targets:
#    UZPP_VERSION=v2.1.8  ./install.sh    # specific release
#    UZPP_PREFIX=$HOME    ./install.sh    # install under ~/bin + ~/lib
# ============================================================================

set -euo pipefail

REPO="timetolivechk-spec/uzpp"
UZPP_VERSION="${UZPP_VERSION:-latest}"
UZPP_PREFIX="${UZPP_PREFIX:-/usr/local}"

bold()   { printf '\033[1m%s\033[0m\n' "$*"; }
green()  { printf '\033[32m%s\033[0m\n' "$*"; }
yellow() { printf '\033[33m%s\033[0m\n' "$*"; }
red()    { printf '\033[31m%s\033[0m\n' "$*" >&2; }

# ─── 1. Detect platform ─────────────────────────────────────────────────────
os="$(uname -s)"
arch="$(uname -m)"

case "$os" in
    Linux)
        case "$arch" in
            x86_64|amd64) asset="uzpp-linux-x64.tar.gz"; platform="Linux x64" ;;
            *) red "Linux $arch hozircha qo'llab-quvvatlanmaydi (faqat x86_64)."; exit 1 ;;
        esac
        ;;
    Darwin)
        case "$arch" in
            arm64) asset="uzpp-macos-arm64.tar.gz"; platform="macOS Apple Silicon" ;;
            x86_64)
                red "macOS Intel uchun rasmiy binar yo'q. Apple Silicon"
                red "Mac sotib oling yoki manbadan kompilatsiya qiling:"
                red "  https://github.com/$REPO#building-from-source"
                exit 1
                ;;
            *) red "macOS $arch hozircha qo'llab-quvvatlanmaydi."; exit 1 ;;
        esac
        ;;
    *) red "Tizim $os hozircha qo'llab-quvvatlanmaydi (Linux / macOS)."; exit 1 ;;
esac

bold "uz++ installer — $platform"

# ─── 2. Check for C++ compiler ──────────────────────────────────────────────
need_cpp=true
for cxx in g++ clang++; do
    if command -v "$cxx" >/dev/null 2>&1; then
        ver="$("$cxx" --version | head -1)"
        green "✓ C++ kompilyatori topildi: $cxx ($ver)"
        need_cpp=false
        break
    fi
done

if $need_cpp; then
    yellow "⚠ C++ kompilyatori (g++/clang++) topilmadi."
    if [ "$os" = "Linux" ]; then
        if command -v apt-get >/dev/null 2>&1; then
            yellow "  Debian/Ubuntu uchun:"
            yellow "    sudo apt-get update && sudo apt-get install -y g++ libstdc++-15-dev"
            yellow "  yoki tezroq (ko'pchilik holatlar uchun):"
            yellow "    sudo apt-get install -y build-essential"
        elif command -v dnf >/dev/null 2>&1; then
            yellow "  Fedora/RHEL: sudo dnf install -y gcc-c++"
        elif command -v pacman >/dev/null 2>&1; then
            yellow "  Arch:  sudo pacman -S gcc"
        else
            yellow "  Tizimingiz uchun gcc/g++ paketini o'rnating."
        fi
    else
        yellow "  macOS: xcode-select --install"
        yellow "  yoki: brew install gcc"
    fi
    if [ -t 0 ]; then
        read -r -p "Davom etishni xohlaysizmi? [y/N] " ans
        [[ "$ans" =~ ^[Yy]$ ]] || exit 1
    else
        yellow "  Avtomatik rejim — davom etilmoqda. C++ kompilyatorisiz uzpp ishlamaydi."
    fi
fi

# ─── 3. Resolve download URL ────────────────────────────────────────────────
if [ "$UZPP_VERSION" = "latest" ]; then
    url="https://github.com/$REPO/releases/latest/download/$asset"
else
    url="https://github.com/$REPO/releases/download/$UZPP_VERSION/$asset"
fi

bold "Yuklab olinmoqda: $url"
tmpdir="$(mktemp -d)"
trap 'rm -rf "$tmpdir"' EXIT

curl -fL --retry 3 -o "$tmpdir/$asset" "$url"
size=$(stat -c %s "$tmpdir/$asset" 2>/dev/null || stat -f %z "$tmpdir/$asset")
green "✓ $size bayt yuklandi."

# ─── 4. Extract + install ───────────────────────────────────────────────────
bold "Ochilmoqda..."
tar -xzf "$tmpdir/$asset" -C "$tmpdir"

# Tarball layout: uzpp binary + stdlib/ at top level.
if [ ! -f "$tmpdir/uzpp" ] || [ ! -d "$tmpdir/stdlib" ]; then
    red "Arxiv kutilgan tuzilmaga ega emas (uzpp + stdlib/ topilmadi)."
    exit 1
fi

bin_dir="$UZPP_PREFIX/bin"
lib_dir="$UZPP_PREFIX/lib/uzpp"

# Need sudo for /usr/local; not for $HOME.
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

# ─── 5. Smoke test ──────────────────────────────────────────────────────────
bold "Sinov..."
if "$bin_dir/uzpp" --version 2>&1 | head -1; then
    green ""
    green "✓ uz++ muvaffaqiyatli o'rnatildi."
    green ""
    green "  Tezda boshlash:"
    green "    echo 'butun asosiy() { yozish << \"Salom!\" << qator_oxiri; qaytarish 0; }' > salom.uzpp"
    green "    uzpp ishga-tushirish salom.uzpp"
    green ""
    green "  VS Code kengaytmasi: code --install-extension uzpp.uzpp"
else
    red "Sinov muvaffaqiyatsiz — qo'lda tekshiring: $bin_dir/uzpp --version"
    exit 1
fi
