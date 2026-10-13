param([switch]$Force)
$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot
$base = Join-Path $root 'vendor/viewers'
$dist = Join-Path $root 'dist'
New-Item -ItemType Directory -Path $base,$dist -Force | Out-Null
$archives = @(
    @{Name='wscite567.zip'; Url='https://www.scintilla.org/wscite567.zip'; Hash='24b6c63fc61d8fb57b782bac6149352dde262212a9588367bde4b2def599f353'},
    @{Name='scintilla567.zip'; Url='https://www.scintilla.org/scintilla567.zip'; Hash='2309181e59e7da4659f890dbc128db7451ccd094748770e7be674b830494587c'},
    @{Name='lexilla554.zip'; Url='https://www.scintilla.org/lexilla554.zip'; Hash='5e0a21356e4e2f19d6a38ecd1c8172d5a2b7b4542940568b023386080db90fe0'},
    @{Name='pdfium-win-x64.tgz'; Url='https://github.com/bblanchon/pdfium-binaries/releases/download/chromium/8100/pdfium-win-x64.tgz'; Hash='51621bcc1d9be9ea448f4b84b0516862b928e3968d3fca899af1a6d6e2848d6d'}
)
foreach ($archive in $archives) {
    $path = Join-Path $base $archive.Name
    if ($Force -or !(Test-Path -LiteralPath $path)) { Invoke-WebRequest -Uri $archive.Url -OutFile $path }
    if ((Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash -ne $archive.Hash) { throw "Dependency checksum mismatch: $($archive.Name)" }
    if ($archive.Name.EndsWith('.zip')) { Expand-Archive -LiteralPath $path -DestinationPath $base -Force }
    else {
        $pdf = Join-Path $base 'pdfium'
        New-Item -ItemType Directory -Path $pdf -Force | Out-Null
        & tar.exe -xf $path -C $pdf
        if ($LASTEXITCODE) { throw 'PDFium extraction failed.' }
    }
}
foreach ($name in @('Scintilla.dll','Lexilla.dll')) { Copy-Item -LiteralPath (Join-Path $base "wscite/$name") -Destination (Join-Path $dist $name) -Force }
Copy-Item -LiteralPath (Join-Path $base 'pdfium/bin/pdfium.dll') -Destination (Join-Path $dist 'pdfium.dll') -Force
$manifest = Get-Content -LiteralPath (Join-Path $root 'dependencies/viewers-lock.json') -Raw | ConvertFrom-Json
foreach ($dependency in $manifest.runtime) {
    $path = Join-Path $dist $dependency.file
    if ((Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash -ne $dependency.sha256) { throw "Runtime checksum mismatch: $($dependency.file)" }
    $bytes = [IO.File]::ReadAllBytes($path)
    $pe = [BitConverter]::ToInt32($bytes,60)
    if ([BitConverter]::ToUInt16($bytes,$pe+4) -ne 0x8664) { throw "Runtime is not x64: $($dependency.file)" }
}
Write-Output 'Pinned Scintilla, Lexilla and non-V8 PDFium x64 dependencies verified.'
