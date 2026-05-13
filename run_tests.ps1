$files = Get-ChildItem -Path tests -Filter *.uzpp
$failed = 0
$passed = 0
foreach ($f in $files) {
    $out = & ./build_wt/uzpp.exe qurish $f.FullName 2>&1
    $outStr = $out -join "`n"
    if ($outStr -match "MUVAFFAQIYAT") {
        $passed++
    } else {
        Write-Host "FAIL: " $f.Name
        Write-Host $outStr
        $failed++
    }
}
Write-Host "Total Passed: $passed, Total Failed: $failed"
