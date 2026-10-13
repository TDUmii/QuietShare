$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$artifactRoot = Join-Path $projectRoot 'artifacts'
$exe = Join-Path $projectRoot 'dist\QuietShare.exe'
$setup = Join-Path $projectRoot 'dist\QuietShare-Setup.exe'
$runs = @(@('--qa-visible', 'qa-visible'), @('--qa-excluded', 'qa-excluded'), @('--qa-storage', 'qa-storage'))
foreach ($run in $runs) {
    $directory = Join-Path $artifactRoot $run[1]
    New-Item -ItemType Directory -Path $directory -Force | Out-Null
    $arguments = $run[0] + ' "' + $directory + '"'
    $process = Start-Process -FilePath $exe -ArgumentList $arguments -WindowStyle Hidden -PassThru -Wait
    if ($process.ExitCode) { throw "Native QA failed: $($run[1]), exit $($process.ExitCode)" }
    Write-Output "$($run[1]): exit 0"
}
Copy-Item -LiteralPath (Join-Path $artifactRoot 'qa-storage\storage-check.json') -Destination (Join-Path $artifactRoot 'storage-check.json')
$reportPath = Join-Path $artifactRoot 'installer-self-test.json'
$process = Start-Process -FilePath $setup -ArgumentList ('--self-test --report "' + $reportPath + '"') -WindowStyle Hidden -PassThru -Wait
if ($process.ExitCode) { throw 'Installer self-test failed.' }
$installerImage = Join-Path $artifactRoot 'installer-visible.bmp'
$process = Start-Process -FilePath $setup -ArgumentList ('--qa-visible --report "' + $installerImage + '"') -WindowStyle Hidden -PassThru -Wait
if ($process.ExitCode) { throw 'Installer UI capture failed.' }
$uninstallerImage = Join-Path $artifactRoot 'uninstaller-visible.bmp'
$process = Start-Process -FilePath $setup -ArgumentList ('--uninstall --qa-visible --report "' + $uninstallerImage + '"') -WindowStyle Hidden -PassThru -Wait
if ($process.ExitCode) { throw 'Uninstaller UI capture failed.' }
python (Join-Path $PSScriptRoot 'verify_local.py')
if ($LASTEXITCODE) { throw 'Artifact verification failed.' }
python (Join-Path $PSScriptRoot 'release_inventory.py')
if ($LASTEXITCODE) { throw 'Release inventory verification failed.' }
python (Join-Path $PSScriptRoot 'lifecycle_regression.py')
if ($LASTEXITCODE) { throw 'Production EXE lifecycle regression failed.' }
