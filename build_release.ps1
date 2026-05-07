# build_release.ps1 - Yagona uz++ paketi (kompilyator bilan birga) yaratish

$ErrorActionPreference = "Stop"

$MingwUrl = "https://github.com/brechtsanders/winlibs_mingw/releases/download/14.2.0posix-18.1.8-12.0.0-ucrt-r2/winlibs-x86_64-posix-seh-gcc-14.2.0-mingw-w64ucrt-12.0.0-r2.zip"
$ReleaseDir = "release-pkg\uzpp-windows-x64"
$ZipName = "uzpp-windows-x64.zip"

Write-Host "1. Eski jildlarni tozalash..." -ForegroundColor Cyan
if (Test-Path "release-pkg") { Remove-Item -Recurse -Force "release-pkg" }
New-Item -ItemType Directory -Path $ReleaseDir | Out-Null
New-Item -ItemType Directory -Path "release-pkg\tmp" | Out-Null

Write-Host "2. uz++ kompilyatorini (uzpp.exe) qurish..." -ForegroundColor Cyan
if (!(Test-Path "build")) { New-Item -ItemType Directory -Path "build" | Out-Null }
cd build
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
mingw32-make -j4
cd ..

Write-Host "3. Fayllarni nusxalash..." -ForegroundColor Cyan
Copy-Item "build\uzpp.exe" -Destination $ReleaseDir
Copy-Item -Recurse "stdlib" -Destination "$ReleaseDir\stdlib"

Write-Host "4. C++ kompilyatorini (MinGW) yuklab olish..." -ForegroundColor Cyan
$MingwZip = "release-pkg\tmp\mingw.zip"
Invoke-WebRequest -Uri $MingwUrl -OutFile $MingwZip

Write-Host "5. MinGW ni ochish va to'g'ri joylashtirish..." -ForegroundColor Cyan
Expand-Archive -Path $MingwZip -DestinationPath "release-pkg\tmp\mingw-extract" -Force

# MinGW ning ichidagi haqiqiy jildni topamiz (bin papkasi bor jild)
$MingwRoot = (Get-ChildItem -Path "release-pkg\tmp\mingw-extract" -Directory -Recurse | Where-Object { (Test-Path "$($_.FullName)\bin\g++.exe") }).FullName[0]

# Uni release papkasidagi "compiler" degan joyga ko'chiramiz
Rename-Item -Path $MingwRoot -NewName "compiler"
Move-Item -Path (Split-Path $MingwRoot -Parent) -Destination $ReleaseDir -Force
# Hozir u $ReleaseDir\mingw64\ bo'lishi mumkin, biz to'g'ridan to'g'ri ko'chirishimiz kerak
$ActualCompilerDir = (Get-ChildItem -Path $ReleaseDir -Directory -Recurse | Where-Object { $_.Name -match "compiler" }).FullName[0]
if ($ActualCompilerDir -ne "$((Get-Item $ReleaseDir).FullName)\compiler") {
    Move-Item -Path $ActualCompilerDir -Destination "$((Get-Item $ReleaseDir).FullName)\compiler" -Force
}

Write-Host "6. Keraksiz mingw fayllarini tozalash (hajmini kamaytirish uchun)..." -ForegroundColor Cyan
# Hajmni tejash uchun keraksiz papkalarni o'chirishimiz mumkin, lekin xavfsizlik uchun qoldiramiz
# Remove-Item -Recurse -Force "$ReleaseDir\compiler\share" -ErrorAction SilentlyContinue

Write-Host "7. ZIP arxiv yaratish..." -ForegroundColor Cyan
Compress-Archive -Path "$ReleaseDir\*" -DestinationPath "release-pkg\$ZipName" -Force

Write-Host "8. Tozalash..." -ForegroundColor Cyan
Remove-Item -Recurse -Force "release-pkg\tmp"

Write-Host "MUVAFFAQIYAT! To'liq paket yaratildi: release-pkg\$ZipName" -ForegroundColor Green
Write-Host "Buni GitHub Releases ga yuklashingiz mumkin." -ForegroundColor Yellow
