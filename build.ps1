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
& (Join-Path $projectRoot 'prepare-browser.ps1')
New-Item -ItemType Directory -Path $buildDir,$distDir -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $projectRoot 'THIRD-PARTY.md') -Destination (Join-Path $distDir 'THIRD-PARTY.md') -Force
$compiler = Join-Path $CompilerRoot 'g++.exe'
$windres = Join-Path $CompilerRoot 'windres.exe'
if (!(Test-Path -LiteralPath $compiler)) { throw 'MinGW-w64 C++ compiler not found.' }
$previousPath = $env:PATH
$env:PATH = $CompilerRoot + ';' + $env:PATH
function Compile-Resource([string]$source, [string]$output) {
    for ($attempt = 1; $attempt -le 3; $attempt++) {
        # Separate RC parsing from COFF conversion; direct large-payload COFF
        # emission can crash this MinGW windres build during heap cleanup.
        $nativeResource = $output + '.native'
        $resourceProcess = Start-Process -FilePath $windres -ArgumentList @('-I.',$source,'-O','res','-o',$nativeResource) -WorkingDirectory (Get-Location).Path -WindowStyle Hidden -PassThru -Wait
        if ($resourceProcess.ExitCode -eq 0) {
            $resourceProcess = Start-Process -FilePath $windres -ArgumentList @('-J','res','-O','coff','-i',$nativeResource,'-o',$output) -WorkingDirectory (Get-Location).Path -WindowStyle Hidden -PassThru -Wait
            if ($resourceProcess.ExitCode -eq 0) { return }
        }
        if ($attempt -lt 3) { Start-Sleep -Milliseconds 700 }
    }
    # The res-to-COFF converter can also hit this toolchain's heap bug. Retry
    # the same small manifest/icon/font input through the direct RC path.
    $resourceProcess = Start-Process -FilePath $windres -ArgumentList @('-I.',$source,'-O','coff','-o',$output) -WorkingDirectory (Get-Location).Path -WindowStyle Hidden -PassThru -Wait
    if ($resourceProcess.ExitCode -eq 0) { return }
    throw "Resource compilation failed: $source (exit $($resourceProcess.ExitCode))"
}
Push-Location (Join-Path $projectRoot 'src')
try {
    Compile-Resource 'app.rc' '../build/app.res'
    & $compiler '-std=c++17' '-O2' '-s' '-static' '-static-libgcc' '-static-libstdc++' '-municode' '-mwindows' '-DUNICODE' '-D_UNICODE' '-D_WIN32_WINNT=0x0A00' 'main.cpp' 'Browser.cpp' 'Privacy.cpp' 'Content.cpp' 'Documents.cpp' 'DocumentBar.cpp' 'DocumentHost.cpp' 'Viewers.cpp' 'Mirror.cpp' 'MeetingRecorder.cpp' '-I../vendor/webview2/build/native/include' '-I../vendor/viewers/scintilla/include' '-I../vendor/viewers/lexilla/include' '-I../vendor/viewers/pdfium/include' (Join-Path $buildDir 'app.res') '-o' (Join-Path $distDir 'QuietShare.exe') '-lcomctl32' '-ldwmapi' '-lcrypt32' '-lshell32' '-luser32' '-lgdi32' '-luxtheme' '-lcomdlg32' '-lwindowscodecs' '-lole32' '-luuid' '-lmsimg32' '-lgdiplus' '-ld3d11' '-ldxgi' '-lruntimeobject'
    if ($LASTEXITCODE) { throw 'Application compilation failed.' }
    & $compiler '-std=c++17' '-O2' '-DQUIETSHARE_QA' '-s' '-static' '-static-libgcc' '-static-libstdc++' '-municode' '-mwindows' '-DUNICODE' '-D_UNICODE' '-D_WIN32_WINNT=0x0A00' 'main.cpp' 'Browser.cpp' 'Privacy.cpp' 'Content.cpp' 'Documents.cpp' 'DocumentBar.cpp' 'DocumentHost.cpp' 'Viewers.cpp' 'Mirror.cpp' 'MeetingRecorder.cpp' '-I../vendor/webview2/build/native/include' '-I../vendor/viewers/scintilla/include' '-I../vendor/viewers/lexilla/include' '-I../vendor/viewers/pdfium/include' (Join-Path $buildDir 'app.res') '-o' (Join-Path $buildDir 'QuietShare-QA.exe') '-lcomctl32' '-ldwmapi' '-lcrypt32' '-lshell32' '-luser32' '-lgdi32' '-luxtheme' '-lcomdlg32' '-lwindowscodecs' '-lole32' '-luuid' '-lmsimg32' '-lgdiplus' '-ld3d11' '-ldxgi' '-lruntimeobject'
    if ($LASTEXITCODE) { throw 'QA application compilation failed.' }
    foreach ($runtimeName in @('Scintilla.dll','Lexilla.dll','pdfium.dll','WebView2Loader.dll')) { Copy-Item -LiteralPath (Join-Path $distDir $runtimeName) -Destination (Join-Path $buildDir $runtimeName) -Force }
    if (Test-Path -LiteralPath 'Installer.cpp') {
        Compile-Resource 'installer.rc' '../build/installer.res'
        & $compiler '-std=c++17' '-O2' '-s' '-static' '-static-libgcc' '-static-libstdc++' '-municode' '-mwindows' '-DUNICODE' '-D_UNICODE' 'Installer.cpp' (Join-Path $buildDir 'installer.res') '-o' (Join-Path $distDir 'QuietShare-Setup.exe') '-lole32' '-lshell32' '-luuid' '-ladvapi32' '-lcomctl32' '-luser32' '-lgdi32' '-lmsimg32' '-lversion'
        if ($LASTEXITCODE) { throw 'Installer compilation failed.' }
        if (-not ('QuietShareBuildResources' -as [type])) {
            Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
public static class QuietShareBuildResources {
    [DllImport("kernel32.dll", CharSet=CharSet.Unicode, SetLastError=true)]
    public static extern IntPtr BeginUpdateResource(string path, bool deleteExisting);
    [DllImport("kernel32.dll", SetLastError=true)]
    public static extern bool UpdateResource(IntPtr update, IntPtr type, IntPtr name, ushort language, byte[] bytes, uint size);
    [DllImport("kernel32.dll", SetLastError=true)]
    public static extern bool EndUpdateResource(IntPtr update, bool discard);
}
'@
        }
        $setupOutput = Join-Path $distDir 'QuietShare-Setup.exe'
        $resourceUpdate = [QuietShareBuildResources]::BeginUpdateResource($setupOutput,$false)
        if ($resourceUpdate -eq [IntPtr]::Zero) { throw 'Cannot open installer resource update.' }
        $resourceCommitted = $false
        try {
            $payloadNames = @('QuietShare.exe','Scintilla.dll','Lexilla.dll','pdfium.dll','THIRD-PARTY.md','WebView2Loader.dll')
            for ($index = 0; $index -lt $payloadNames.Count; $index++) {
                $payloadBytes = [IO.File]::ReadAllBytes((Join-Path $distDir $payloadNames[$index]))
                if (-not [QuietShareBuildResources]::UpdateResource($resourceUpdate,[IntPtr]10,[IntPtr](301+$index),1033,$payloadBytes,$payloadBytes.Length)) {
                    throw "Cannot embed installer payload: $($payloadNames[$index])"
                }
            }
            $resourceCommitted = [QuietShareBuildResources]::EndUpdateResource($resourceUpdate,$false)
            if (-not $resourceCommitted) { throw 'Cannot commit installer payload resources.' }
        } finally {
            if (-not $resourceCommitted) { [QuietShareBuildResources]::EndUpdateResource($resourceUpdate,$true) | Out-Null }
        }
    }
} finally { Pop-Location; $env:PATH = $previousPath }
Get-ChildItem -LiteralPath $distDir -Filter '*.exe' | Select-Object Name,Length
