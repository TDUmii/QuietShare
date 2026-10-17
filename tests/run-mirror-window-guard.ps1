param([string]$CompilerRoot = '')
$ErrorActionPreference = 'Stop'
if (!$CompilerRoot) {
    $compilerCommand = Get-Command 'g++.exe' -ErrorAction SilentlyContinue
    if (!$compilerCommand) { throw 'Add MinGW-w64 to PATH or pass -CompilerRoot with its bin directory.' }
    $CompilerRoot = Split-Path -Parent $compilerCommand.Source
}
$projectRoot = Split-Path $PSScriptRoot -Parent
$output = Join-Path $projectRoot 'artifacts\qa-mirror-window-guard'
New-Item -ItemType Directory -Path $output -Force | Out-Null
$compiler = Join-Path $CompilerRoot 'g++.exe'
$previousPath = $env:PATH
$env:PATH = $CompilerRoot + ';' + $env:PATH
Push-Location $projectRoot
try {
    & $compiler '-std=c++17' '-O2' '-Wall' '-Wextra' '-Werror' '-static' '-municode' '-DUNICODE' '-D_UNICODE' '-D_WIN32_WINNT=0x0A00' 'tests/mirror_window_guard_regression.cpp' 'src/Privacy.cpp' '-o' (Join-Path $output 'mirror-window-guard-regression.exe') '-ld3d11' '-ldxgi' '-ldwmapi' '-lruntimeobject' '-lole32' '-luuid' '-luser32' '-lgdi32' '-lshell32'
    if ($LASTEXITCODE) { throw 'Mirror window guard regression compilation failed.' }
    & (Join-Path $output 'mirror-window-guard-regression.exe') (Join-Path $output 'mirror-window-guard-regression.json')
    if ($LASTEXITCODE) { throw 'Mirror window guard regression failed.' }
} finally { Pop-Location; $env:PATH = $previousPath }
