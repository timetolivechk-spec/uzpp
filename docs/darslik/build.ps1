# build.ps1 — UZ++ darslik PDF qurish (Pandoc + XeLaTeX)
# Talab: Pandoc 3.x, MiKTeX (xelatex), TeX Gyre + DejaVu shriftlar
param(
    [string]$Output = "uzpp-darslik.pdf",
    [switch]$KeepTex
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$Pandoc = "pandoc"
$XeLaTeX = "xelatex"

# Boblarni tartib bo'yicha birlashtirish
$chapters = @(
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

# Birlashtirilgan faylni temp papkaga yozish (UTF-8, BOM siz)
$merged = Join-Path $env:TEMP "uzpp-darslik-merged.md"
$utf8NoBom = New-Object System.Text.UTF8Encoding $false
$sb = New-Object System.Text.StringBuilder

foreach ($ch in $chapters) {
    $path = Join-Path $Root $ch
    if (-not (Test-Path $path)) {
        Write-Host "OGOHLANTIRISH: $ch topilmadi, o'tkazib yuborildi" -ForegroundColor Yellow
        continue
    }
    # MUHIM: Faylni xom baytlar sifatida o'qib, UTF-8 sifatida dekodlash —
    # Get-Content -Encoding UTF8 ba'zan kodlash xatolariga olib keladi.
    $bytes = [System.IO.File]::ReadAllBytes($path)
    $content = $utf8NoBom.GetString($bytes)
    # BOM bo'lsa olib tashlash
    if ($content.Length -gt 0 -and $content[0] -eq [char]0xFEFF) {
        $content = $content.Substring(1)
    }
    # Til tegi qo'yilmagan kod bloklari uchun 'cpp' belgilash —
    # juft sonli ``` chiziqlarni hisoblab, faqat ochuvchi (toq raqamli) ``` qatorni almashtiramiz.
    # Sodda yo'l: hech narsa o'zgartirmaslik, chunki .md fayllar allaqachon to'g'ri tegga ega.
    [void]$sb.AppendLine($content)
    [void]$sb.AppendLine("")
    [void]$sb.AppendLine('\newpage')
    [void]$sb.AppendLine("")
}

[System.IO.File]::WriteAllText($merged, $sb.ToString(), $utf8NoBom)
$mergedSize = (Get-Item $merged).Length
Write-Host "Birlashtirildi: $mergedSize bayt ($([math]::Round($mergedSize/1KB,1)) KB)" -ForegroundColor Cyan

# PDF qurish
$metadata = Join-Path $Root "metadata.yaml"
$template = Join-Path $Root "template.tex"
$outputPath = Join-Path $Root $Output

Write-Host "XeLaTeX orqali PDF qurilmoqda..." -ForegroundColor Cyan

$pandocArgs = @(
    $merged,
    "-o", $outputPath,
    "--from=markdown+pipe_tables+auto_identifiers+yaml_metadata_block",
    "--pdf-engine=$XeLaTeX",
    "--pdf-engine-opt=-enable-installer",
    "--template=$template",
    "--metadata-file=$metadata",
    "--top-level-division=chapter",
    "--number-sections",
    "--toc",
    "--toc-depth=2",
    "--highlight-style=tango",
    "--columns=80",
    "-V", "papersize=a4",
    "-V", "fontsize=11pt",
    "-V", "documentclass=book"
)

if ($KeepTex) {
    $texOut = [System.IO.Path]::ChangeExtension($outputPath, ".tex")
    & $Pandoc @pandocArgs "--output=$texOut"
    Write-Host "TeX manba: $texOut" -ForegroundColor DarkGray
}

& $Pandoc @pandocArgs
$pandocExit = $LASTEXITCODE

if ($pandocExit -eq 0 -and (Test-Path $outputPath)) {
    $size = (Get-Item $outputPath).Length
    Write-Host ""
    Write-Host "=== MUVAFFAQIYAT ===" -ForegroundColor Green
    Write-Host "Fayl: $outputPath"
    Write-Host "Hajm: $([math]::Round($size/1KB, 1)) KB ($([math]::Round($size/1MB, 2)) MB)"
} else {
    Write-Host ""
    Write-Host "=== XATO (chiqish kodi: $pandocExit) ===" -ForegroundColor Red
    Write-Host "Birlashtirilgan fayl: $merged"
    exit $pandocExit
}

# Tozalash
if (-not $KeepTex) {
    Remove-Item $merged -Force -ErrorAction SilentlyContinue
}
