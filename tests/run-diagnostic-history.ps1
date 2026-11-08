param([string]$CompilerRoot='')
$ErrorActionPreference='Stop'
if(!$CompilerRoot){$CompilerRoot=Split-Path -Parent (Get-Command g++.exe).Source}
$root=Split-Path -Parent $PSScriptRoot
$output=Join-Path $root 'artifacts/qa-diagnostic-history'
New-Item -ItemType Directory -Path $output -Force | Out-Null
& (Join-Path $CompilerRoot 'g++.exe') -std=c++17 -O2 -Wall -Wextra -Werror -static -municode -DUNICODE -D_UNICODE (Join-Path $PSScriptRoot 'diagnostic_history_regression.cpp') -o (Join-Path $output 'diagnostic-history-regression.exe')
if($LASTEXITCODE){throw 'Diagnostic history compilation failed'}
$lines=& (Join-Path $output 'diagnostic-history-regression.exe') $output
$result=$LASTEXITCODE
$lines | Write-Output
$summary=$lines | Select-Object -Last 1
[IO.File]::WriteAllText((Join-Path $output 'diagnostic-history-regression.json'),$summary,[Text.UTF8Encoding]::new($false))
if($result){throw 'Diagnostic history regression failed'}
