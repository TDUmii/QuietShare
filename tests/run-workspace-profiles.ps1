param([string]$CompilerRoot='')
$ErrorActionPreference='Stop'
if (!$CompilerRoot) { $CompilerRoot=Split-Path -Parent (Get-Command g++.exe).Source }
$projectRoot=Split-Path -Parent $PSScriptRoot
$output=Join-Path $projectRoot 'artifacts\qa-workspace-profiles'
New-Item -ItemType Directory -Path $output -Force | Out-Null
Push-Location $projectRoot
try {
    & (Join-Path $CompilerRoot 'g++.exe') '-std=c++17' '-O2' '-static' '-municode' '-DUNICODE' '-D_UNICODE' 'tests/workspace_profiles_regression.cpp' '-o' (Join-Path $output 'workspace-profiles-regression.exe') '-lcrypt32'
    if ($LASTEXITCODE) { throw 'Workspace profiles compilation failed.' }
    & (Join-Path $output 'workspace-profiles-regression.exe')
    if ($LASTEXITCODE) { throw 'Workspace profiles regression failed.' }
} finally { Pop-Location }
