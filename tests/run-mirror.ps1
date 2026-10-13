param([string]$CompilerRoot = '')
$ErrorActionPreference = 'Stop'
if (!$CompilerRoot) {
    $compilerCommand = Get-Command 'g++.exe' -ErrorAction SilentlyContinue
    if (!$compilerCommand) { throw 'Add MinGW-w64 to PATH or pass -CompilerRoot with its bin directory.' }
    $CompilerRoot = Split-Path -Parent $compilerCommand.Source
}
$projectRoot = Split-Path $PSScriptRoot -Parent
$output = Join-Path $projectRoot 'artifacts\qa-mirror'
New-Item -ItemType Directory -Path $output -Force | Out-Null
$compiler = Join-Path $CompilerRoot 'g++.exe'
$previousPath = $env:PATH
$env:PATH = $CompilerRoot + ';' + $env:PATH
Push-Location $projectRoot
try {
    & $compiler '-std=c++17' '-O2' '-Wall' '-Wextra' '-Werror' '-static' '-municode' '-DUNICODE' '-D_UNICODE' '-D_WIN32_WINNT=0x0A00' 'tests/mirror_regression.cpp' 'src/Mirror.cpp' 'src/Privacy.cpp' '-o' (Join-Path $output 'mirror-regression.exe') '-ld3d11' '-ldxgi' '-ldwmapi' '-lruntimeobject' '-lole32' '-luuid' '-luser32' '-lgdi32' '-lshell32'
    if ($LASTEXITCODE) { throw 'Mirror regression compilation failed.' }
    & (Join-Path $output 'mirror-regression.exe') (Join-Path $output 'mirror-regression.json')
    if ($LASTEXITCODE) { throw 'Mirror regression failed; inspect actual WGC result.' }
    & $compiler '-std=c++17' '-O2' '-Wall' '-Wextra' '-Werror' '-static' '-municode' '-DUNICODE' '-D_UNICODE' '-D_WIN32_WINNT=0x0A00' 'tests/mirror_render_regression.cpp' 'src/Privacy.cpp' '-o' (Join-Path $output 'mirror-render-regression.exe') '-ld3d11' '-ldxgi' '-ldwmapi' '-lruntimeobject' '-lole32' '-luuid' '-luser32' '-lgdi32' '-lshell32'
    if ($LASTEXITCODE) { throw 'Mirror renderer regression compilation failed.' }
    & (Join-Path $output 'mirror-render-regression.exe') (Join-Path $output 'mirror-render-regression.json')
    if ($LASTEXITCODE) { throw 'Mirror renderer regression failed.' }
} finally { Pop-Location; $env:PATH = $previousPath }
