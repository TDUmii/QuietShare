param([string]$CompilerRoot = '')
$ErrorActionPreference = 'Stop'
if (!$CompilerRoot) {
    $compilerCommand = Get-Command 'g++.exe' -ErrorAction SilentlyContinue
    if (!$compilerCommand) { throw 'Add MinGW-w64 to PATH or pass -CompilerRoot with its bin directory.' }
    $CompilerRoot = Split-Path -Parent $compilerCommand.Source
}
$projectRoot = Split-Path $PSScriptRoot -Parent
$output = Join-Path $projectRoot 'artifacts\qa-projection-notice'
New-Item -ItemType Directory -Path $output -Force | Out-Null
$compiler = Join-Path $CompilerRoot 'g++.exe'
$previousPath = $env:PATH
$env:PATH = $CompilerRoot + ';' + $env:PATH
Push-Location $projectRoot
try {
    & $compiler '-std=c++17' '-O2' '-static' '-municode' '-DUNICODE' '-D_UNICODE' '-D_WIN32_WINNT=0x0A00' `
        'tests/projection_notice_regression.cpp' 'src/Browser.cpp' 'src/Privacy.cpp' 'src/Content.cpp' `
        'src/Documents.cpp' 'src/DocumentBar.cpp' 'src/DocumentHost.cpp' 'src/Viewers.cpp' 'src/Mirror.cpp' 'src/MeetingRecorder.cpp' 'src/MeetingTranscriber.cpp' `
        '-Ivendor/webview2/build/native/include' '-Ivendor/viewers/scintilla/include' `
        '-Ivendor/viewers/lexilla/include' '-Ivendor/viewers/pdfium/include' `
        '-o' (Join-Path $output 'projection-notice-regression.exe') `
        '-lcomctl32' '-ldwmapi' '-lcrypt32' '-lshell32' '-luser32' '-lgdi32' '-luxtheme' '-lcomdlg32' `
        '-lwindowscodecs' '-lole32' '-luuid' '-lmsimg32' '-lgdiplus' '-ld3d11' '-ldxgi' '-lruntimeobject' '-lsapi'
    if ($LASTEXITCODE) { throw 'Projection notice regression compilation failed.' }
    & (Join-Path $output 'projection-notice-regression.exe') | Out-File -LiteralPath (Join-Path $output 'projection-notice-regression.json') -Encoding ascii
    if ($LASTEXITCODE) { throw 'Projection notice regression failed.' }
    Get-Content -LiteralPath (Join-Path $output 'projection-notice-regression.json')
} finally { Pop-Location; $env:PATH = $previousPath }
