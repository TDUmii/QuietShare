param([string]$CompilerRoot = 'D:\Microsoft VS Code\mingw64\bin')
$ErrorActionPreference = 'Stop'
$projectRoot = $PSScriptRoot
$buildDir = Join-Path $projectRoot 'build'
$distDir = Join-Path $projectRoot 'dist'
New-Item -ItemType Directory -Path $buildDir,$distDir -Force | Out-Null
$compiler = Join-Path $CompilerRoot 'g++.exe'
$windres = Join-Path $CompilerRoot 'windres.exe'
if (!(Test-Path -LiteralPath $compiler)) { throw 'MinGW-w64 C++ compiler not found.' }
$previousPath = $env:PATH
$env:PATH = $CompilerRoot + ';' + $env:PATH
function Compile-Resource([string]$source, [string]$output) {
    for ($attempt = 1; $attempt -le 3; $attempt++) {
        & $windres '--use-temp-file' '-I.' $source '-O' 'coff' '-o' $output
        if ($LASTEXITCODE -eq 0) { return }
        if ($attempt -lt 3) { Start-Sleep -Milliseconds 700 }
    }
    throw "Resource compilation failed: $source"
}
Push-Location (Join-Path $projectRoot 'src')
try {
    Compile-Resource 'app.rc' '../build/app.res'
    & $compiler '-std=c++17' '-O2' '-s' '-static' '-static-libgcc' '-static-libstdc++' '-municode' '-mwindows' '-DUNICODE' '-D_UNICODE' '-D_WIN32_WINNT=0x0A00' 'main.cpp' 'Privacy.cpp' (Join-Path $buildDir 'app.res') '-o' (Join-Path $distDir 'QuietShare.exe') '-lcomctl32' '-ldwmapi' '-lcrypt32' '-lshell32' '-luser32' '-lgdi32'
    if ($LASTEXITCODE) { throw 'Application compilation failed.' }
    if (Test-Path -LiteralPath 'Installer.cpp') {
        Compile-Resource 'installer.rc' '../build/installer.res'
        & $compiler '-std=c++17' '-O2' '-s' '-static' '-static-libgcc' '-static-libstdc++' '-municode' '-mwindows' '-DUNICODE' '-D_UNICODE' 'Installer.cpp' (Join-Path $buildDir 'installer.res') '-o' (Join-Path $distDir 'QuietShare-Setup.exe') '-lole32' '-lshell32' '-luuid' '-ladvapi32' '-lcomctl32' '-luser32' '-lgdi32' '-lversion'
        if ($LASTEXITCODE) { throw 'Installer compilation failed.' }
    }
} finally { Pop-Location; $env:PATH = $previousPath }
Get-ChildItem -LiteralPath $distDir -Filter '*.exe' | Select-Object Name,Length
