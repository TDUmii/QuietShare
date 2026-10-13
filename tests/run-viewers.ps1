param([string]$CompilerRoot='')
$ErrorActionPreference='Stop'
if (!$CompilerRoot) {
    $compilerCommand = Get-Command 'g++.exe' -ErrorAction SilentlyContinue
    if (!$compilerCommand) { throw 'Add MinGW-w64 to PATH or pass -CompilerRoot with its bin directory.' }
    $CompilerRoot = Split-Path -Parent $compilerCommand.Source
}
$root=Split-Path -Parent $PSScriptRoot
$qa=Join-Path $root 'artifacts/qa-viewers'
New-Item -ItemType Directory -Path $qa -Force | Out-Null
& (Join-Path $root 'prepare-viewers.ps1')
python (Join-Path $PSScriptRoot 'viewer_fixtures.py') $qa
if ($LASTEXITCODE) { throw 'PDF fixture generation failed.' }
$oldPath=$env:PATH
$env:PATH=$CompilerRoot+';'+$env:PATH
Push-Location $root
try {
    & (Join-Path $CompilerRoot 'g++.exe') -std=c++17 -O2 -s -static -municode -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0A00 -Ivendor/viewers/scintilla/include -Ivendor/viewers/lexilla/include -Ivendor/viewers/pdfium/include tests/viewer_regression.cpp src/Viewers.cpp src/Content.cpp -o (Join-Path $qa 'viewer-regression.exe') -lcomctl32 -luxtheme -luser32 -lgdi32 -lwindowscodecs -lole32 -luuid -lmsimg32
    if ($LASTEXITCODE) { throw 'Viewer regression compilation failed.' }
    foreach($name in @('Scintilla.dll','Lexilla.dll','pdfium.dll')) { Copy-Item -LiteralPath (Join-Path $root "dist/$name") -Destination (Join-Path $qa $name) -Force }
    & (Join-Path $qa 'viewer-regression.exe') $qa
    if ($LASTEXITCODE) { throw 'Native viewer regression failed.' }
} finally { Pop-Location; $env:PATH=$oldPath }
