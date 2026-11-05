param([string]$CompilerRoot = '')
$ErrorActionPreference = 'Stop'
if (!$CompilerRoot) { $CompilerRoot = Split-Path -Parent (Get-Command g++.exe).Source }
$projectRoot = Split-Path -Parent $PSScriptRoot
$output = Join-Path $projectRoot 'artifacts\qa-speech'
New-Item -ItemType Directory -Path $output -Force | Out-Null
Push-Location $projectRoot
try {
    & (Join-Path $CompilerRoot 'g++.exe') '-std=c++17' '-O2' '-static' '-municode' '-DUNICODE' '-D_UNICODE' '-D_WIN32_WINNT=0x0A00' 'tests/speech_runtime_regression.cpp' '-o' (Join-Path $output 'speech-runtime-regression.exe') '-lcrypt32'
    if ($LASTEXITCODE) { throw 'Speech runtime regression compilation failed.' }
    $fixtures = Join-Path $projectRoot 'artifacts\speech-test'
    $english = Join-Path $projectRoot 'artifacts\speech-build\whisper.cpp-f049fff95a089aa9969deb009cdd4892b3e74916\samples\jfk.wav'
    if ((Test-Path -LiteralPath (Join-Path $fixtures 'ggml-small.bin')) -and (Test-Path -LiteralPath (Join-Path $fixtures 'vi.wav')) -and (Test-Path -LiteralPath $english)) {
        $speech = Join-Path $output 'Speech'
        New-Item -ItemType Directory -Path $speech -Force | Out-Null
        Copy-Item -LiteralPath (Join-Path $projectRoot 'artifacts\speech-addon\whisper-cli.exe') -Destination $speech -Force
        if (!(Test-Path -LiteralPath (Join-Path $speech 'ggml-small.bin'))) { New-Item -ItemType HardLink -Path (Join-Path $speech 'ggml-small.bin') -Target (Join-Path $fixtures 'ggml-small.bin') | Out-Null }
        & (Join-Path $output 'speech-runtime-regression.exe') $output $english (Join-Path $fixtures 'vi.wav')
    } else { & (Join-Path $output 'speech-runtime-regression.exe') }
    if ($LASTEXITCODE) { throw 'Speech runtime regression failed.' }
} finally { Pop-Location }
