param([string]$CompilerRoot = '')
$ErrorActionPreference = 'Stop'
if (!$CompilerRoot) {
    $compilerCommand = Get-Command 'g++.exe' -ErrorAction SilentlyContinue
    if (!$compilerCommand) { throw 'Add MinGW-w64 to PATH or pass -CompilerRoot with its bin directory.' }
    $CompilerRoot = Split-Path -Parent $compilerCommand.Source
}
$projectRoot = $PSScriptRoot
$buildDir = Join-Path $projectRoot 'build'
$distDir = Join-Path $projectRoot 'dist'
& (Join-Path $projectRoot 'prepare-viewers.ps1')
New-Item -ItemType Directory -Path $buildDir,$distDir -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $projectRoot 'THIRD-PARTY.md') -Destination (Join-Path $distDir 'THIRD-PARTY.md') -Force
$compiler = Join-Path $CompilerRoot 'g++.exe'
$windres = Join-Path $CompilerRoot 'windres.exe'
if (!(Test-Path -LiteralPath $compiler)) { throw 'MinGW-w64 C++ compiler not found.' }
$previousPath = $env:PATH
$env:PATH = $CompilerRoot + ';' + $env:PATH
function Compile-Resource([string]$source, [string]$output) {
    for ($attempt = 1; $attempt -le 3; $attempt++) {
        $resourceProcess = Start-Process -FilePath $windres -ArgumentList @('--use-temp-file','-I.',$source,'-O','coff','-o',$output) -WorkingDirectory (Get-Location).Path -WindowStyle Hidden -PassThru -Wait
        if ($resourceProcess.ExitCode -eq 0) { return }
        if ($attempt -lt 3) { Start-Sleep -Milliseconds 700 }
    }
    throw "Resource compilation failed: $source (exit $($resourceProcess.ExitCode))"
}
Push-Location (Join-Path $projectRoot 'src')
try {
    Compile-Resource 'app.rc' '../build/app.res'
    & $compiler '-std=c++17' '-O2' '-s' '-static' '-static-libgcc' '-static-libstdc++' '-municode' '-mwindows' '-DUNICODE' '-D_UNICODE' '-D_WIN32_WINNT=0x0A00' 'main.cpp' 'Privacy.cpp' 'Content.cpp' 'Documents.cpp' 'DocumentBar.cpp' 'DocumentHost.cpp' 'Viewers.cpp' 'Mirror.cpp' '-I../vendor/viewers/scintilla/include' '-I../vendor/viewers/lexilla/include' '-I../vendor/viewers/pdfium/include' (Join-Path $buildDir 'app.res') '-o' (Join-Path $distDir 'QuietShare.exe') '-lcomctl32' '-ldwmapi' '-lcrypt32' '-lshell32' '-luser32' '-lgdi32' '-luxtheme' '-lcomdlg32' '-lwindowscodecs' '-lole32' '-luuid' '-lmsimg32' '-ld3d11' '-ldxgi' '-lruntimeobject'
    if ($LASTEXITCODE) { throw 'Application compilation failed.' }
    & $compiler '-std=c++17' '-O2' '-DQUIETSHARE_QA' '-s' '-static' '-static-libgcc' '-static-libstdc++' '-municode' '-mwindows' '-DUNICODE' '-D_UNICODE' '-D_WIN32_WINNT=0x0A00' 'main.cpp' 'Privacy.cpp' 'Content.cpp' 'Documents.cpp' 'DocumentBar.cpp' 'DocumentHost.cpp' 'Viewers.cpp' 'Mirror.cpp' '-I../vendor/viewers/scintilla/include' '-I../vendor/viewers/lexilla/include' '-I../vendor/viewers/pdfium/include' (Join-Path $buildDir 'app.res') '-o' (Join-Path $buildDir 'QuietShare-QA.exe') '-lcomctl32' '-ldwmapi' '-lcrypt32' '-lshell32' '-luser32' '-lgdi32' '-luxtheme' '-lcomdlg32' '-lwindowscodecs' '-lole32' '-luuid' '-lmsimg32' '-ld3d11' '-ldxgi' '-lruntimeobject'
    if ($LASTEXITCODE) { throw 'QA application compilation failed.' }
    foreach ($runtimeName in @('Scintilla.dll','Lexilla.dll','pdfium.dll')) { Copy-Item -LiteralPath (Join-Path $distDir $runtimeName) -Destination (Join-Path $buildDir $runtimeName) -Force }
    if (Test-Path -LiteralPath 'Installer.cpp') {
        Compile-Resource 'installer.rc' '../build/installer.res'
        & $compiler '-std=c++17' '-O2' '-s' '-static' '-static-libgcc' '-static-libstdc++' '-municode' '-mwindows' '-DUNICODE' '-D_UNICODE' 'Installer.cpp' (Join-Path $buildDir 'installer.res') '-o' (Join-Path $distDir 'QuietShare-Setup.exe') '-lole32' '-lshell32' '-luuid' '-ladvapi32' '-lcomctl32' '-luser32' '-lgdi32' '-lversion'
        if ($LASTEXITCODE) { throw 'Installer compilation failed.' }
    }
} finally { Pop-Location; $env:PATH = $previousPath }
Get-ChildItem -LiteralPath $distDir -Filter '*.exe' | Select-Object Name,Length
