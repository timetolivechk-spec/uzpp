# ============================================================================
#  uz++ — barcha mahalliy tekshiruvlarni bir buyruq bilan yugurtirish.
#
#  CI (.github/workflows/ci.yml) aynan shu to'plamni uchala platformada
#  bajaradi. Push qilishdan oldin shuni yugurtiring.
#
#      .\run_tests.ps1                       # build/uzpp.exe bilan
#      .\run_tests.ps1 -Uzpp build_wt\uzpp.exe
#      .\run_tests.ps1 -Tez                  # sekin bosqichlarni o'tkazib yuborish
# ============================================================================
param(
    [string]$Uzpp = "build\uzpp.exe",
    [switch]$Tez
)

$ErrorActionPreference = "Continue"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $Root

if (-not (Test-Path $Uzpp)) {
    Write-Host "XATO: $Uzpp topilmadi. Avval quring:" -ForegroundColor Red
    Write-Host "  cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B build"
    Write-Host "  cmake --build build"
    exit 1
}

$UzppAbs = (Resolve-Path $Uzpp).Path
$bosqichlar = @()
$yiqilgan = 0

function Bosqich([string]$nom, [scriptblock]$ish) {
    Write-Host ""
    Write-Host "=== $nom ===" -ForegroundColor Cyan
    $ok = & $ish
    if ($ok) {
        Write-Host "  OK" -ForegroundColor Green
    } else {
        Write-Host "  YIQILDI" -ForegroundColor Red
        $script:yiqilgan++
    }
    $script:bosqichlar += [pscustomobject]@{ Nom = $nom; Holat = if ($ok) { "OK" } else { "YIQILDI" } }
}

# ─── 1. C++ birlik testlari ────────────────────────────────────────────────
Bosqich "Frontend birlik testlari" {
    $smoke = Join-Path (Split-Path $UzppAbs) "uzpp_frontend_tests.exe"
    if (-not (Test-Path $smoke)) { $smoke = Join-Path (Split-Path $UzppAbs) "uzpp_frontend_tests" }
    if (-not (Test-Path $smoke)) { Write-Host "  o'tkazib yuborildi (topilmadi)"; return $true }
    & $smoke
    return ($LASTEXITCODE -eq 0)
}

# ─── 2. tests/*.uzpp regressiyasi ──────────────────────────────────────────
Bosqich "tests/ regressiyasi" {
    $otdi = 0; $quladi = 0; $qulaganlar = @()
    foreach ($f in Get-ChildItem -Path tests -Filter *.uzpp) {
        $out = (& $UzppAbs qurish $f.FullName 2>&1) -join "`n"
        if ($out -match "MUVAFFAQIYAT: Dastur tayyor") {
            $otdi++
        } else {
            $quladi++; $qulaganlar += $f.Name
            Write-Host "  FAIL: $($f.Name)" -ForegroundColor Red
        }
    }
    Write-Host "  $otdi / $($otdi + $quladi)"
    if ($qulaganlar.Count -gt 0) { Write-Host "  Qulaganlar: $($qulaganlar -join ', ')" }
    return ($quladi -eq 0)
}

# ─── 3. misollar/ ──────────────────────────────────────────────────────────
Bosqich "misollar/ (15_oyna — Qt kerak, o'tkaziladi)" {
    $otdi = 0; $quladi = 0
    foreach ($f in Get-ChildItem -Path misollar -Filter *.uzpp) {
        if ($f.Name -eq "15_oyna.uzpp") { continue }
        $out = (& $UzppAbs qurish $f.FullName 2>&1) -join "`n"
        if ($out -match "MUVAFFAQIYAT") { $otdi++ } else { $quladi++; Write-Host "  FAIL: $($f.Name)" -ForegroundColor Red }
    }
    Write-Host "  $otdi / $($otdi + $quladi)"
    return ($quladi -eq 0)
}

# ─── 4. Salbiy testlar ─────────────────────────────────────────────────────
Bosqich "Salbiy testlar" {
    $out = (& bash tests/negative/run.sh $Uzpp 2>&1) -join "`n"
    Write-Host ($out -split "`n" | Select-Object -Last 5 | Out-String).TrimEnd()
    return ($out -match "missed \(regression!\):\s+0")
}

# ─── 5. @sinov / @bench ────────────────────────────────────────────────────
Bosqich "uzpp sinov / uzpp bench" {
    $s = (& $UzppAbs sinov tests/test_sinov_atribut.uzpp 2>&1) -join "`n"
    $b = (& $UzppAbs bench tests/test_sinov_atribut.uzpp 2>&1) -join "`n"
    return (($s -match "Quladi:0") -and ($b -match "ns/amal"))
}

# ─── 6. Formatlagich xavfsizligi ───────────────────────────────────────────
Bosqich "Formatlagich hech narsani buzmasligi" {
    $out = (& bash tests/formatlash_xavfsizlik.sh $Uzpp 2>&1) -join "`n"
    Write-Host ($out -split "`n" | Select-Object -Last 5 | Out-String).TrimEnd()
    return ($out -match "BUZILDI \(regressiya!\):\s+0")
}

# ─── 7. Hujjatlardagi kod misollari ────────────────────────────────────────
Bosqich "Hujjat misollari (transpilyatsiya)" {
    & python tests/darslik_tekshir.py $Uzpp
    return ($LASTEXITCODE -eq 0)
}

if (-not $Tez) {
    Bosqich "Hujjat misollari (to'liq qurish)" {
        & python tests/darslik_tekshir.py $Uzpp --qurish
        return ($LASTEXITCODE -eq 0)
    }
}

# ─── Xulosa ────────────────────────────────────────────────────────────────
Write-Host ""
Write-Host "=============================================" -ForegroundColor Cyan
$bosqichlar | ForEach-Object {
    $rang = if ($_.Holat -eq "OK") { "Green" } else { "Red" }
    Write-Host ("  {0,-40} {1}" -f $_.Nom, $_.Holat) -ForegroundColor $rang
}
Write-Host "=============================================" -ForegroundColor Cyan

if ($yiqilgan -eq 0) {
    Write-Host "Hammasi yashil." -ForegroundColor Green
    exit 0
}
Write-Host "$yiqilgan bosqich yiqildi." -ForegroundColor Red
exit 1
