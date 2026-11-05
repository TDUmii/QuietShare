param([string]$CompilerRoot = '')
$ErrorActionPreference = 'Stop'
if (!$CompilerRoot) { $CompilerRoot = Split-Path -Parent (Get-Command g++.exe).Source }
$projectRoot = Split-Path -Parent $PSScriptRoot
$output = Join-Path $projectRoot 'artifacts\qa-reading-meeting'
New-Item -ItemType Directory -Path $output -Force | Out-Null
Push-Location $projectRoot
try {
    & (Join-Path $CompilerRoot 'g++.exe') '-std=c++17' '-O2' '-static' '-municode' '-DUNICODE' '-D_UNICODE' 'tests/reading_meeting_regression.cpp' '-o' (Join-Path $output 'reading-meeting-regression.exe') '-lcrypt32'
    if ($LASTEXITCODE) { throw 'Reading / notebook regression compilation failed.' }
    & (Join-Path $output 'reading-meeting-regression.exe')
    if ($LASTEXITCODE) { throw 'Reading / notebook regression failed.' }
} finally { Pop-Location }
