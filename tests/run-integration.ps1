$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
python -B (Join-Path $PSScriptRoot 'production_integration.py')
if($LASTEXITCODE){throw 'Production integration regression failed. See artifacts/production-integration.json.'}
