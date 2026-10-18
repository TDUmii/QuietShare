param([string]$CompilerRoot = '')
$ErrorActionPreference = 'Stop'
if (!$CompilerRoot) { $CompilerRoot = Split-Path -Parent (Get-Command g++.exe).Source }
$projectRoot = Split-Path -Parent $PSScriptRoot
$output = Join-Path $projectRoot 'artifacts\qa-workspace'
New-Item -ItemType Directory -Path $output -Force | Out-Null
$previousPath = $env:PATH
$env:PATH = $CompilerRoot + ';' + $env:PATH
Push-Location $projectRoot
try {
    & (Join-Path $CompilerRoot 'g++.exe') '-std=c++17' '-O2' '-Wall' '-Wextra' '-Werror' '-static' '-municode' '-DUNICODE' '-D_UNICODE' '-D_WIN32_WINNT=0x0A00' 'tests/workspace_regression.cpp' 'src/Documents.cpp' 'src/Content.cpp' '-o' (Join-Path $output 'workspace-regression.exe') '-lcrypt32' '-lwindowscodecs' '-lole32' '-luuid' '-lmsimg32' '-lgdi32' '-lcomctl32' '-luxtheme'
    if ($LASTEXITCODE) { throw 'Workspace regression compilation failed.' }
    & (Join-Path $output 'workspace-regression.exe')
    if ($LASTEXITCODE) { throw 'Workspace regression failed.' }
} finally { Pop-Location; $env:PATH = $previousPath }
