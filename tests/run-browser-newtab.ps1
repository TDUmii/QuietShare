param([string]$CompilerRoot = '')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
if (!$CompilerRoot) {
    $compilerCommand = Get-Command 'g++.exe' -ErrorAction SilentlyContinue
    if (!$compilerCommand) { throw 'Add MinGW-w64 to PATH or pass -CompilerRoot with its bin directory.' }
    $CompilerRoot = Split-Path -Parent $compilerCommand.Source
}
$compiler = Join-Path $CompilerRoot 'g++.exe'
$outputDirectory = Join-Path $projectRoot 'artifacts\qa-browser-newtab'
New-Item -ItemType Directory -Path $outputDirectory -Force | Out-Null
$fixture = Join-Path $outputDirectory 'browser-newtab-fixture.exe'
& $compiler '-std=c++17' '-O2' '-static' '-static-libgcc' '-static-libstdc++' '-municode' '-DUNICODE' '-D_UNICODE' `
    '-D_WIN32_WINNT=0x0A00' (Join-Path $PSScriptRoot 'browser_newtab_fixture.cpp') `
    ('-I'+(Join-Path $projectRoot 'vendor\webview2\build\native\include')) '-o' $fixture '-lole32' '-luuid' '-luser32'
if ($LASTEXITCODE) { throw 'New Tab rendering fixture compilation failed.' }
Copy-Item -LiteralPath (Join-Path $projectRoot 'dist\WebView2Loader.dll') -Destination (Join-Path $outputDirectory 'WebView2Loader.dll') -Force
$rawReport = Join-Path $outputDirectory 'browser-newtab-fixture.raw.json'
$errorOutput = Join-Path $outputDirectory 'browser-newtab-fixture.stderr.txt'
$process = Start-Process -FilePath $fixture -ArgumentList @('"'+$outputDirectory+'"') -WorkingDirectory $outputDirectory `
    -WindowStyle Hidden -RedirectStandardOutput $rawReport -RedirectStandardError $errorOutput -PassThru -Wait
if (!(Test-Path -LiteralPath $rawReport)) { throw 'New Tab rendering fixture produced no report.' }
$raw = Get-Content -LiteralPath $rawReport -Raw | ConvertFrom-Json
$report = [ordered]@{method=$raw.method;checks=@($raw.checks);passed=$raw.passed;failed=$raw.failed;
    generatedAtUtc=[DateTime]::UtcNow.ToString('o');
    fixtureSha256=(Get-FileHash -LiteralPath $fixture -Algorithm SHA256).Hash.ToLowerInvariant();
    sourceSha256=[ordered]@{};screenshots=@()}
foreach ($relative in @('src\Browser.cpp','src\Browser.h','tests\browser_newtab_fixture.cpp')) {
    $report.sourceSha256[$relative.Replace('\','/')] = (Get-FileHash -LiteralPath (Join-Path $projectRoot $relative) -Algorithm SHA256).Hash.ToLowerInvariant()
}
foreach ($imageName in @('newtab-vi-358x197.png','newtab-en-358x197.png')) {
    $imagePath = Join-Path $outputDirectory $imageName
    if (Test-Path -LiteralPath $imagePath) {
        $report.screenshots += @{name=$imageName;sha256=(Get-FileHash -LiteralPath $imagePath -Algorithm SHA256).Hash.ToLowerInvariant()}
    }
}
$reportPath = Join-Path $outputDirectory 'browser-newtab-fixture.json'
[IO.File]::WriteAllText($reportPath,($report | ConvertTo-Json -Depth 10),(New-Object Text.UTF8Encoding($false)))
Write-Output ("New Tab rendering / DOM fixture: {0} passed, {1} failed. Synthetic canary gate - not privacy proof." -f $report.passed,$report.failed)
if ($process.ExitCode -ne 0 -or $report.failed -ne 0) { throw "New Tab rendering / DOM regression failed. See $reportPath" }
