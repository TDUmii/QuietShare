param([string]$CompilerRoot = '')
$ErrorActionPreference = 'Stop'
if (!$CompilerRoot) {
    $compilerCommand = Get-Command 'g++.exe' -ErrorAction SilentlyContinue
    if (!$compilerCommand) { throw 'Add MinGW-w64 to PATH or pass -CompilerRoot with its bin directory.' }
    $CompilerRoot = Split-Path -Parent $compilerCommand.Source
}
$projectRoot = Split-Path $PSScriptRoot -Parent
$output = Join-Path $projectRoot 'artifacts\qa-meeting-transcriber'
New-Item -ItemType Directory -Path $output -Force | Out-Null
$compiler = Join-Path $CompilerRoot 'g++.exe'
$previousPath = $env:PATH
$env:PATH = $CompilerRoot + ';' + $env:PATH
Push-Location $projectRoot
try {
    & $compiler '-std=c++17' '-O2' '-static' '-municode' '-DUNICODE' '-D_UNICODE' '-D_WIN32_WINNT=0x0A00' `
        'tests/meeting_transcriber_regression.cpp' 'src/MeetingTranscriber.cpp' `
        '-o' (Join-Path $output 'meeting-transcriber-regression.exe') '-lole32' '-luuid' '-lsapi'
    if ($LASTEXITCODE) { throw 'Meeting transcriber regression compilation failed.' }
    & (Join-Path $output 'meeting-transcriber-regression.exe') | Out-File -LiteralPath (Join-Path $output 'meeting-transcriber-regression.txt') -Encoding ascii
    if ($LASTEXITCODE) { throw 'Meeting transcriber regression failed.' }
    Get-Content -LiteralPath (Join-Path $output 'meeting-transcriber-regression.txt')
} finally { Pop-Location; $env:PATH = $previousPath }
