# UZ++ darslik — PDF qurish skripti
# Talab: Pandoc 3.6+ va WeasyPrint (pip install weasyprint)

$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot

$PANDOC = "$env:TEMP/pandoc/pandoc-3.6.4/pandoc.exe"
$OUTPUT = "uzpp_dasturlash_tili.pdf"

$chapters = @(
    "00-mundarija.md",
    "01-kirish.md",
    "02-ozgaruvchilar-va-turlar.md",
    "03-operatorlar.md",
    "04-boshqaruv.md",
    "05-funksiyalar.md",
    "06-korsatkichlar.md",
    "07-tuplamlar.md",
    "08-sinflar.md",
    "09-shablonlar.md",
    "10-xatolik.md",
    "11-matn.md",
    "12-fayllar.md",
    "13-kop-oqimlilik.md",
    "14-stdlib.md",
    "15-gui.md",
    "16-ilovalar.md"
)

Write-Host "UZ++ darslik PDF qurilmoqda..." -ForegroundColor Green
Write-Host "Bo'limlar: $($chapters.Count) ta" -ForegroundColor Cyan

# Combine all chapters into a single input
$inputFiles = $chapters -join " "

$cmd = "& `"$PANDOC`" $inputFiles metadata.yaml --from=markdown+smart --to=pdf --pdf-engine=weasyprint --highlight-style=tango --css=style.css -o `"$OUTPUT`" 2>&1"
Write-Host "Buyruq: $cmd" -ForegroundColor DarkGray

Invoke-Expression $cmd

if (Test-Path $OUTPUT) {
    $size = [math]::Round((Get-Item $OUTPUT).Length / 1MB, 1)
    Write-Host "MUVAFFAQIYAT: $OUTPUT ($size MB)" -ForegroundColor Green
} else {
    Write-Host "XATO: PDF yaratilmadi" -ForegroundColor Red
}
