# Windows installer (Inno Setup)

Builds **`uzpp-setup.exe`** — the official Windows installer for uz++.

## What's bundled

| Item | Source | Notes |
|---|---|---|
| `uzpp.exe` | `build/uzpp.exe` | Compiler frontend (statically linked against MSYS2 UCRT64). |
| `stdlib/` | `stdlib/` | Header-only standard library (~30 modules). |
| `misollar/` | `misollar/` | 10 canonical example programs. |
| `compiler/` | WinLibs MinGW-w64 | **Bundled GCC 14.2 + UCRT runtime** (~520 MB on disk, ~150 MB in installer). |
| `LICENSE.txt` | `LICENSE` | MIT license, shown on the wizard's first page. |
| `README.txt` | generated | Plain-text quick-start, shown after install. |

**MinGW IS bundled** as of the v2.1.1 hot-fix. `uzpp.exe` looks for
`<install_dir>\compiler\bin\g++.exe` first, so users get a working
"download → install → build" flow without touching PATH or downloading
anything else. Bundle source: WinLibs
[GCC 14.2.0 + MinGW-w64 UCRT 12.0.0](https://github.com/brechtsanders/winlibs_mingw/releases/tag/14.2.0posix-19.1.1-12.0.0-ucrt-r2).

## Default install location

- `%LOCALAPPDATA%\Programs\uzpp\` (no admin required).
- User can re-target via the Browse button — Inno Setup will then prompt for
  elevation automatically if a system path like `C:\Program Files\` is
  chosen.

## What the installer does

1. Copies files to install dir.
2. (optional, default on) Adds install dir to **user PATH** (HKCU\Environment).
   System-wide PATH is intentionally not touched — it would require admin and
   surprise users.
3. (optional, default on) Registers `.uzpp` file association: double-click
   runs `uzpp ishga-tushirish <file>`; right-click → Edit opens VS Code.
4. (optional, default on) Adds Start Menu entries for the examples folder
   and a GitHub link.
5. Adds an entry under Settings → Apps for clean uninstallation.
6. Optional post-install: opens the `misollar\` folder so the user can try
   `01_salom_dunyo.uzpp` immediately.

## How to build

### Prerequisites

- Windows 10/11 host.
- [Inno Setup 6.x](https://jrsoftware.org/isdl.php) installed (provides
  `iscc.exe`).
- A built `uzpp.exe` in the repository's `build/` directory:

  ```pwsh
  cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B build
  cmake --build build
  ```

### Build the installer

From the repository root:

```pwsh
# 1. Stage installer payload into dist/
$dist = "installer\windows\..\..\dist"
Remove-Item -Recurse -Force $dist -ErrorAction SilentlyContinue
New-Item  -ItemType Directory -Path $dist | Out-Null
Copy-Item build\uzpp.exe   $dist\
Copy-Item -Recurse stdlib  $dist\stdlib
Copy-Item -Recurse misollar $dist\misollar
Copy-Item LICENSE          $dist\LICENSE.txt

# 2. Download + extract WinLibs MinGW into dist\compiler\
$mingwUrl = 'https://github.com/brechtsanders/winlibs_mingw/releases/download/14.2.0posix-19.1.1-12.0.0-ucrt-r2/winlibs-x86_64-posix-seh-gcc-14.2.0-mingw-w64ucrt-12.0.0-r2.zip'
Invoke-WebRequest -Uri $mingwUrl -OutFile $dist\winlibs.zip
Expand-Archive -Force -Path $dist\winlibs.zip -DestinationPath $dist\winlibs-extracted
Move-Item $dist\winlibs-extracted\mingw64 $dist\compiler
Remove-Item -Recurse -Force $dist\winlibs.zip, $dist\winlibs-extracted

@"
uz++ - O'zbek Dasturlash Tili
=============================

Tezda ishga tushirish | Quick start:
  uzpp ishga-tushirish misollar\01_salom_dunyo.uzpp

Bundled C++ compiler: compiler\bin\g++.exe (MinGW-w64 GCC 14.2 UCRT)
VS Code kengaytmasi: Marketplace -> uzpp.uzpp
GitHub: https://github.com/timetolivechk-spec/uzpp
"@ | Set-Content -Path $dist\README.txt -Encoding utf8

# 3. Compile the installer
&"C:\Program Files (x86)\Inno Setup 6\iscc.exe" `
   /DAppVersion=2.1.1 `
   installer\windows\installer.iss

# Output:
#   installer\windows\Output\uzpp-setup.exe   (~150 MB compressed)
```

To override the version: `iscc /DAppVersion=2.2.0 installer.iss`.

### Sign the installer (optional, recommended for releases)

```pwsh
signtool sign /a /tr http://timestamp.digicert.com /td sha256 /fd sha256 `
    installer\windows\Output\uzpp-setup.exe
```

## Verifying the installer

After installing on a clean VM:

```pwsh
# 1. PATH should contain the install dir
$env:Path -split ';' | Select-String "uzpp"

# 2. uzpp should be on PATH
uzpp --version

# 3. Build a sample
uzpp ishga-tushirish "$env:LOCALAPPDATA\Programs\uzpp\misollar\01_salom_dunyo.uzpp"

# 4. Double-click misollar\01_salom_dunyo.uzpp in Explorer — should run.
```

## Uninstalling

Settings → Apps → "uz++ 2.1.1" → Uninstall.

The uninstaller removes installed files, the registry keys for file
association, and (if the task was checked) the PATH entry. It does NOT
delete user projects or caches in `%USERPROFILE%\.uzpp\`.
