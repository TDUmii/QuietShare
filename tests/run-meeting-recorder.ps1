param([string]$CompilerRoot = '')
$ErrorActionPreference = 'Stop'
if (!$CompilerRoot) {
    $compilerCommand = Get-Command 'g++.exe' -ErrorAction SilentlyContinue
    if (!$compilerCommand) { throw 'Add MinGW-w64 to PATH or pass -CompilerRoot with its bin directory.' }
    $CompilerRoot = Split-Path -Parent $compilerCommand.Source
}
$projectRoot = Split-Path $PSScriptRoot -Parent
$output = Join-Path $projectRoot 'artifacts\qa-meeting-recorder'
New-Item -ItemType Directory -Path $output -Force | Out-Null
$compiler = Join-Path $CompilerRoot 'g++.exe'
$previousPath = $env:PATH
$env:PATH = $CompilerRoot + ';' + $env:PATH
Push-Location $projectRoot
try {
    & $compiler '-std=c++17' '-O2' '-static' '-municode' '-DUNICODE' '-D_UNICODE' '-D_WIN32_WINNT=0x0A00' `
        'tests/meeting_recorder_regression.cpp' 'src/MeetingRecorder.cpp' `
        '-o' (Join-Path $output 'meeting-recorder-regression.exe') '-lole32' '-luuid'
    if ($LASTEXITCODE) { throw 'Meeting recorder regression compilation failed.' }
    & (Join-Path $output 'meeting-recorder-regression.exe') | Out-File -LiteralPath (Join-Path $output 'meeting-recorder-regression.txt') -Encoding ascii
    if ($LASTEXITCODE) { throw 'Meeting recorder regression failed.' }
    Get-Content -LiteralPath (Join-Path $output 'meeting-recorder-regression.txt')
} finally { Pop-Location; $env:PATH = $previousPath }
