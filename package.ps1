$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot
python -B (Join-Path $root 'tests\export_release_report.py')
if ($LASTEXITCODE) { throw 'Release evidence is incomplete or does not match the shipping EXE.' }
$dist = Join-Path $root 'dist'
foreach ($name in @('README.md','THIRD-PARTY.md')) {
    Copy-Item -LiteralPath (Join-Path $root $name) -Destination (Join-Path $dist $name) -Force
}
$docsFolder = Join-Path $dist 'docs'
New-Item -ItemType Directory -Path $docsFolder -Force | Out-Null
$docNames = @('README.md','USER_GUIDE.md','PRIVACY.md','TROUBLESHOOTING.md','BUILDING.md','DEPENDENCIES.md','BOOK.md','USER_MANUAL_EN.md','USER_MANUAL_VI.md','QuietShare-User-Manual-EN.pdf','QuietShare-Huong-Dan-Su-Dung-VI.pdf','quietshare-home.png','readme-banner.svg','project-status.svg')
foreach ($name in $docNames) {
    Copy-Item -LiteralPath (Join-Path (Join-Path $root 'docs') $name) -Destination $docsFolder -Force
}
$license = 'dependencies/licenses/PDFium-third-party/freetype.txt'
$licenseTarget = Join-Path $dist $license
New-Item -ItemType Directory -Path (Split-Path -Parent $licenseTarget) -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $root $license) -Destination $licenseTarget -Force
# Match Git's LF text checkout while preserving the byte-pinned installer notice.
$textPayloads = Get-ChildItem -LiteralPath $dist -File -Recurse | Where-Object {
    $_.Extension -in @('.md','.json','.txt','.ijg','.svg') -and
    $_.Name -ne 'THIRD-PARTY.md' -and
    $_.Name -notlike 'WebView2-*'
}
foreach ($payload in $textPayloads) {
    if ($payload.FullName.StartsWith((Join-Path $dist 'dependencies') + [IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)) {
        # Preserve original license encodings/BOMs while matching Git's LF.
        $byteEncoding = [Text.Encoding]::GetEncoding(28591)
        $licenseText = $byteEncoding.GetString([IO.File]::ReadAllBytes($payload.FullName)).Replace("`r`n","`n")
        [IO.File]::WriteAllBytes($payload.FullName,$byteEncoding.GetBytes($licenseText))
        continue
    }
    $text = [IO.File]::ReadAllText($payload.FullName).Replace("`r`n","`n")
    [IO.File]::WriteAllText($payload.FullName,$text,[Text.UTF8Encoding]::new($false))
}
python -B (Join-Path $root 'tests\docs_regression.py') --dist
if ($LASTEXITCODE) { throw 'Bilingual documentation or distribution links are invalid.' }
# Only release files and documentation/license folders enter the ZIP.
$allowed = @('QuietShare.exe','Scintilla.dll','Lexilla.dll','pdfium.dll','WebView2Loader.dll',
             'README.md','THIRD-PARTY.md',$license)
$entries = $allowed | ForEach-Object { Join-Path $dist $_ }
$entries += $docNames | ForEach-Object { Join-Path $docsFolder $_ }
foreach ($entry in $entries) { if (!(Test-Path -LiteralPath $entry)) { throw "Missing payload: $entry" } }
$zip = Join-Path $root 'QuietShare-v0.16.0-win-x64.zip'
# Enumerate every archive entry explicitly; stale build files cannot enter it.
Add-Type -AssemblyName System.IO.Compression.FileSystem
$payload = @($allowed) + @($docNames | ForEach-Object { 'docs/' + $_ })
$unexpectedDocs = Get-ChildItem -LiteralPath $docsFolder -File | Where-Object { $_.Name -notin $docNames }
if ($unexpectedDocs) { throw 'Unexpected files remain in dist/docs.' }
$stream = [IO.File]::Open($zip,[IO.FileMode]::Create)
$archive = [IO.Compression.ZipArchive]::new($stream,[IO.Compression.ZipArchiveMode]::Create)
try {
    foreach ($relative in $payload) {
        [IO.Compression.ZipFileExtensions]::CreateEntryFromFile($archive,(Join-Path $dist $relative),$relative,[IO.Compression.CompressionLevel]::Optimal) | Out-Null
    }
} finally { $archive.Dispose(); $stream.Dispose() }
Write-Output "Packaged: $zip"
