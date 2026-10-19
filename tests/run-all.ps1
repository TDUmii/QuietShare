param([string]$CompilerRoot = '', [switch]$LongSession)
$ErrorActionPreference = 'Stop'
if (!$CompilerRoot) {
    $compilerCommand = Get-Command 'g++.exe' -ErrorAction SilentlyContinue
    if (!$compilerCommand) { throw 'Add MinGW-w64 to PATH or pass -CompilerRoot with its bin directory.' }
    $CompilerRoot = Split-Path -Parent $compilerCommand.Source
}
# GUI tests are deliberately serialized because the shipping app is single-instance.
$previousPath = $env:PATH
$env:PATH = $CompilerRoot + ';' + $env:PATH
try {
& (Join-Path $PSScriptRoot 'run-preferences.ps1') -CompilerRoot $CompilerRoot
& (Join-Path $PSScriptRoot 'run-workspace.ps1') -CompilerRoot $CompilerRoot
foreach ($scriptName in @('run-content.ps1','run-documents.ps1','run-viewers.ps1')) {
    & (Join-Path $PSScriptRoot $scriptName) -CompilerRoot $CompilerRoot
}
& (Join-Path $PSScriptRoot 'run-mirror.ps1') -CompilerRoot $CompilerRoot -SoakSeconds $(if ($LongSession) { 720 } else { 0 })
& (Join-Path $PSScriptRoot 'run-mirror-window-guard.ps1') -CompilerRoot $CompilerRoot
& (Join-Path $PSScriptRoot 'run-projection-notice.ps1') -CompilerRoot $CompilerRoot
& (Join-Path $PSScriptRoot 'run-local.ps1') -SkipContent
& (Join-Path $PSScriptRoot 'run-integration.ps1')
python -B (Join-Path $PSScriptRoot 'workspace_integration.py')
if ($LASTEXITCODE) { throw 'Workspace integration failed.' }
python -B (Join-Path $PSScriptRoot 'browser_regression.py')
if ($LASTEXITCODE) { throw 'Browser integration regression failed.' }
& (Join-Path $PSScriptRoot 'run-browser-newtab.ps1') -CompilerRoot $CompilerRoot
python -B (Join-Path $PSScriptRoot 'settings_regression.py')
if ($LASTEXITCODE) { throw 'Consolidated Settings regression failed.' }
& (Join-Path $PSScriptRoot 'run-installer-smoke.ps1')
python -B (Join-Path $PSScriptRoot 'docs_regression.py')
if ($LASTEXITCODE) { throw 'Documentation regression failed.' }
}
finally { $env:PATH = $previousPath }
