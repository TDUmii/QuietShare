param([string]$CompilerRoot = '')
$ErrorActionPreference = 'Stop'
if (!$CompilerRoot) {
    $compilerCommand = Get-Command 'g++.exe' -ErrorAction SilentlyContinue
    if (!$compilerCommand) { throw 'Add MinGW-w64 to PATH or pass -CompilerRoot with its bin directory.' }
    $CompilerRoot = Split-Path -Parent $compilerCommand.Source
}
$projectRoot = Split-Path $PSScriptRoot -Parent
$output = Join-Path $projectRoot 'artifacts\qa-preferences'
New-Item -ItemType Directory -Path $output -Force | Out-Null
$compiler = Join-Path $CompilerRoot 'g++.exe'
$previousPath = $env:PATH
$env:PATH = $CompilerRoot + ';' + $env:PATH
Push-Location $projectRoot
try {
    & $compiler '-std=c++17' '-O2' '-static' '-municode' '-DUNICODE' '-D_UNICODE' '-D_WIN32_WINNT=0x0A00' `
        'tests/preferences_regression.cpp' '-o' (Join-Path $output 'preferences-regression.exe') '-luser32'
    if ($LASTEXITCODE) { throw 'Preferences regression compilation failed.' }
    & (Join-Path $output 'preferences-regression.exe') | Out-File -LiteralPath (Join-Path $output 'preferences-regression.json') -Encoding ascii
    if ($LASTEXITCODE) { throw 'Preferences regression failed.' }
    Get-Content -LiteralPath (Join-Path $output 'preferences-regression.json')
} finally { Pop-Location; $env:PATH = $previousPath }
