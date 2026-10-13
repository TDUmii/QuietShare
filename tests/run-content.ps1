param([string]$CompilerRoot = '')
$ErrorActionPreference = 'Stop'
if (!$CompilerRoot) {
    $compilerCommand = Get-Command 'g++.exe' -ErrorAction SilentlyContinue
    if (!$compilerCommand) { throw 'Add MinGW-w64 to PATH or pass -CompilerRoot with its bin directory.' }
    $CompilerRoot = Split-Path -Parent $compilerCommand.Source
}
$projectRoot = Split-Path -Parent $PSScriptRoot
$artifactRoot = Join-Path $projectRoot 'artifacts\qa-content'
$compiler = Join-Path $CompilerRoot 'g++.exe'
if (!(Test-Path -LiteralPath $compiler)) { throw 'MinGW-w64 C++ compiler not found.' }
New-Item -ItemType Directory -Path $artifactRoot -Force | Out-Null
$testExe = Join-Path $artifactRoot 'content-regression.exe'
$previousPath = $env:PATH
$env:PATH = $CompilerRoot + ';' + $env:PATH
try {
    & $compiler '-std=c++17' '-O2' '-s' '-static' '-static-libgcc' '-static-libstdc++' '-municode' '-DUNICODE' '-D_UNICODE' '-D_WIN32_WINNT=0x0A00' (Join-Path $PSScriptRoot 'content_regression.cpp') (Join-Path $projectRoot 'src\Content.cpp') '-o' $testExe '-lwindowscodecs' '-lole32' '-luuid' '-lmsimg32' '-lgdi32'
    if ($LASTEXITCODE) { throw 'Content regression compilation failed.' }
    & $testExe
    if ($LASTEXITCODE) { throw 'Content regression failed; see artifacts\qa-content\content-regression.json.' }
} finally { $env:PATH = $previousPath }
