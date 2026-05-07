# Negative test runner (PowerShell)
# Har bir tests/negative/*.uzpp fayli kompilyatsiyada xatoga olib kelishi kerak.
#
# Usage:
#   pwsh tests/negative/run.ps1 [path\to\uzpp.exe]

param(
    [string]$UzppPath = ""
)

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot  = Split-Path -Parent (Split-Path -Parent $scriptDir)

if ([string]::IsNullOrEmpty($UzppPath)) {
    $candidate = Join-Path $repoRoot "build\uzpp.exe"
    if (Test-Path $candidate) { $UzppPath = $candidate }
    else {
        Write-Error "uzpp binary topilmadi. Awwal: cmake --build build"
        exit 2
    }
}

$caught = 0
$missed = 0
$missedFiles = @()

# pending/ ichidagilar — bilingan kompilyator kamchiliklari. Top-level only.
Get-ChildItem -Path $scriptDir -Filter "*.uzpp" -File | ForEach-Object {
    $f = $_.FullName
    & $UzppPath qurish $f *> $null
    if ($LASTEXITCODE -eq 0) {
        Write-Host "  MISS  $($_.Name)  (kompilyator xatoni topa olmadi)"
        $missed++
        $missedFiles += $_.Name
    } else {
        $caught++
    }
}

Write-Host ""
Write-Host "============================================="
Write-Host " Salbiy testlar | Negative tests"
Write-Host "  caught (xatolik aniqlandi):  $caught"
Write-Host "  missed (regression!):        $missed"
Write-Host "============================================="

if ($missed -gt 0) {
    Write-Host "Topilmagan testlar: $($missedFiles -join ' ')"
    exit 1
}
exit 0
