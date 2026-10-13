$ErrorActionPreference = 'Stop'
python (Join-Path $PSScriptRoot 'scrollbar_regression.py')
if ($LASTEXITCODE) { throw 'Native scrollbar regression failed.' }
