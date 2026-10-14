param([string]$SetupPath = '')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
if (!$SetupPath) { $SetupPath = Join-Path $projectRoot 'dist\QuietShare-Setup.exe' }
$fixtureRoot = Join-Path $projectRoot 'artifacts\qa-installer-smoke'
if (!(Test-Path -LiteralPath $SetupPath)) { throw 'Build QuietShare-Setup.exe before running installer smoke.' }
function Get-AccountState {
    $localData = [Environment]::GetFolderPath('LocalApplicationData')
    $accountRoot = Join-Path $localData 'Programs\QuietShare'
    $paths = @('QuietShare.exe','QuietShare-Setup.exe','Scintilla.dll','Lexilla.dll','pdfium.dll','WebView2Loader.dll','THIRD-PARTY.md') | ForEach-Object { Join-Path $accountRoot $_ }
    $paths += Join-Path $localData 'QuietShare\notes.dpapi'
    $paths += Join-Path ([Environment]::GetFolderPath('Desktop')) 'QuietShare.lnk'
    $paths += Join-Path ([Environment]::GetFolderPath('Programs')) 'QuietShare.lnk'
    $state = [ordered]@{}
    foreach ($filePath in $paths) {
        $state[$filePath] = if (Test-Path -LiteralPath $filePath -PathType Leaf) { (Get-FileHash -LiteralPath $filePath -Algorithm SHA256).Hash } else { $null }
    }
    $registryPath = 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Uninstall\QuietShare'
    $state['Registry'] = if (Test-Path -LiteralPath $registryPath) { (Get-ItemProperty -LiteralPath $registryPath | ConvertTo-Json -Compress) } else { $null }
    return ($state | ConvertTo-Json -Compress)
}
$accountBefore = Get-AccountState
$process = Start-Process -FilePath $SetupPath -ArgumentList @('--qa-install-smoke', ('"' + $fixtureRoot + '"')) -PassThru -WindowStyle Hidden
if (!$process.WaitForExit(60000)) {
    Stop-Process -Id $process.Id
    throw 'Installer smoke timed out; inspect only its isolated artifact fixture.'
}
if ($process.ExitCode) { throw "Installer smoke failed with exit code $($process.ExitCode); see artifacts\qa-installer-smoke\installer-smoke.json." }
$report = Get-Content -LiteralPath (Join-Path $fixtureRoot 'installer-smoke.json') -Raw | ConvertFrom-Json
if ($report.failedChecks -ne 0) { throw 'Installer smoke report contains failures.' }
$invalidRoot = Join-Path $fixtureRoot 'unapproved-target'
$negative = Start-Process -FilePath $SetupPath -ArgumentList @('--qa-install-smoke', ('"' + $invalidRoot + '"')) -PassThru -WindowStyle Hidden
if (!$negative.WaitForExit(10000)) { Stop-Process -Id $negative.Id; throw 'Negative installer fixture validation timed out.' }
$invalidRejected = $negative.ExitCode -eq 40 -and !(Test-Path -LiteralPath $invalidRoot)
$accountUnchanged = $accountBefore -eq (Get-AccountState)
$extraChecks = @(
    [pscustomobject]@{ name = 'unapproved_fixture_root_rejected_without_writes'; passed = $invalidRejected },
    [pscustomobject]@{ name = 'actual_account_payload_notes_shortcuts_registry_unchanged'; passed = $accountUnchanged }
)
$report.checks += $extraChecks
$report.passedChecks += @($extraChecks | Where-Object passed).Count
$report.failedChecks += @($extraChecks | Where-Object { !$_.passed }).Count
$report | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $fixtureRoot 'installer-smoke.json') -Encoding utf8
if ($report.failedChecks -ne 0) { throw 'Installer smoke isolation verification failed.' }
Write-Output "Installer smoke: $($report.passedChecks) passed, $($report.failedChecks) failed (isolated actual install/upgrade/uninstall)."
