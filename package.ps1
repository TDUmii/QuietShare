$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot
python -B (Join-Path $root 'tests\export_release_report.py')
if ($LASTEXITCODE) { throw 'Release evidence is incomplete or does not match the shipping EXE.' }
$dist = Join-Path $root 'dist'
foreach ($name in @('README.md','CHANGELOG.md','THIRD-PARTY.md')) {
    Copy-Item -LiteralPath (Join-Path $root $name) -Destination (Join-Path $dist $name) -Force
}
$inventory = Get-Content -LiteralPath (Join-Path $root 'artifacts\release-inventory.json') -Raw | ConvertFrom-Json
$manifest = [ordered]@{ version=$inventory.version; platform=$inventory.platform; files=[ordered]@{} }
foreach ($file in $inventory.files.PSObject.Properties) {
    $manifest.files[$file.Name]=[ordered]@{ bytes=$file.Value.bytes; sha256=$file.Value.sha256 }
}
[IO.File]::WriteAllText((Join-Path $dist 'release-manifest.json'),($manifest | ConvertTo-Json -Depth 6)+"`n",[Text.UTF8Encoding]::new($false))
$docsFolder = Join-Path $dist 'docs'
New-Item -ItemType Directory -Path $docsFolder -Force | Out-Null
$docNames = @('quietshare-settings.png','Browser_Integration_Study.md','QuietShare_v0.6.0_Implementation_Plan.md','QuietShare_v0.7.0_Settings_Spec.md','QuietShare_v0.8.0_Implementation_Plan.md','QuietShare_v0.8.1_Implementation_Plan.md','QuietShare_v0.4.0_Codex_Implementation_Plan.md')
foreach ($name in $docNames) {
    Copy-Item -LiteralPath (Join-Path (Join-Path $root 'docs') $name) -Destination $docsFolder -Force
}
Copy-Item -LiteralPath (Join-Path $root 'dependencies') -Destination $dist -Recurse -Force
# Match Git's LF text checkout while preserving the byte-pinned installer notice/spec.
$textPayloads = Get-ChildItem -LiteralPath $dist -File -Recurse | Where-Object {
    $_.Extension -in @('.md','.json','.txt','.ijg') -and
    $_.Name -ne 'THIRD-PARTY.md' -and $_.Name -ne 'QuietShare_v0.4.0_Codex_Implementation_Plan.md' -and
    $_.Name -notlike 'WebView2-*'
}
foreach ($payload in $textPayloads) {
    $text = [IO.File]::ReadAllText($payload.FullName).Replace("`r`n","`n")
    [IO.File]::WriteAllText($payload.FullName,$text,[Text.UTF8Encoding]::new($false))
}
# Only release files and documentation/license folders enter the ZIP.
$allowed = @('QuietShare.exe','QuietShare-Setup.exe','Scintilla.dll','Lexilla.dll','pdfium.dll','WebView2Loader.dll',
             'README.md','CHANGELOG.md','THIRD-PARTY.md','release-manifest.json','dependencies')
$entries = $allowed | ForEach-Object { Join-Path $dist $_ }
$entries += $docNames | ForEach-Object { Join-Path $docsFolder $_ }
foreach ($entry in $entries) { if (!(Test-Path -LiteralPath $entry)) { throw "Missing payload: $entry" } }
$zip = Join-Path $root 'QuietShare-v0.8.1-win-x64.zip'
# Preserve the docs/ hierarchy, but never scoop up stale QA files from dist.
$shippingDocs = Get-ChildItem -LiteralPath $docsFolder -File | Where-Object { $_.Name -notin $docNames }
if ($shippingDocs) { throw 'Unexpected files remain in dist/docs; review release contents before packaging.' }
$entries = $allowed | ForEach-Object { Join-Path $dist $_ }
$entries += $docsFolder
Compress-Archive -LiteralPath $entries -DestinationPath $zip -CompressionLevel Optimal -Force
Write-Output "Packaged: $zip"
