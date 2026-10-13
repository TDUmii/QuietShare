param([string]$CompilerRoot = '')
$ErrorActionPreference = 'Stop'
if (!$CompilerRoot) {
    $compilerCommand = Get-Command 'g++.exe' -ErrorAction SilentlyContinue
    if (!$compilerCommand) { throw 'Add MinGW-w64 to PATH or pass -CompilerRoot with its bin directory.' }
    $CompilerRoot = Split-Path -Parent $compilerCommand.Source
}
# GUI tests are deliberately serialized because the shipping app is single-instance.
foreach ($scriptName in @('run-content.ps1','run-documents.ps1','run-viewers.ps1','run-mirror.ps1')) {
    & (Join-Path $PSScriptRoot $scriptName) -CompilerRoot $CompilerRoot
}
& (Join-Path $PSScriptRoot 'run-local.ps1') -SkipContent
& (Join-Path $PSScriptRoot 'run-integration.ps1')
& (Join-Path $PSScriptRoot 'run-installer-smoke.ps1')
