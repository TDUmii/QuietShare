$ErrorActionPreference = 'Stop'
$browserSdkDir = Join-Path $PSScriptRoot 'vendor/webview2'
New-Item -ItemType Directory -Path $browserSdkDir -Force | Out-Null
$archive = Join-Path $browserSdkDir 'sdk.zip'
$sdkHash = '56f7f4b8bf9aee4b8efefbbdd4f67d5f74ebd1b100ed0806da71bf76af481aa9'
if (!(Test-Path -LiteralPath $archive)) {
    Invoke-WebRequest 'https://api.nuget.org/v3-flatcontainer/microsoft.web.webview2/1.0.4258.31/microsoft.web.webview2.1.0.4258.31.nupkg' -OutFile $archive
}
if ((Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash -ne $sdkHash) { throw 'WebView2 SDK checksum mismatch.' }
Expand-Archive -LiteralPath $archive -DestinationPath $browserSdkDir -Force
$loader = Join-Path $browserSdkDir 'build/native/x64/WebView2Loader.dll'
if ((Get-FileHash -LiteralPath $loader -Algorithm SHA256).Hash -ne '3426dcc55fdfb8b5e7ac623cf33b4ccf283fbf68a0c5a65bddfdb4d749a8abee') { throw 'WebView2 loader checksum mismatch.' }
Copy-Item -LiteralPath $loader -Destination (Join-Path $PSScriptRoot 'dist/WebView2Loader.dll') -Force
# MinGW lacks MSVC's interface UUID metadata. Generate equivalent GUID constants
# from the byte-pinned official SDK rather than maintaining handwritten UUIDs.
$header = [IO.File]::ReadAllText((Join-Path $browserSdkDir 'build/native/include/WebView2.h'))
$browserGuidText = "#pragma once`n template<class T> GUID WebGuid();`n"
foreach ($match in [regex]::Matches($header,'MIDL_INTERFACE\("([A-Fa-f0-9-]+)"\)\s+(ICore\w+)\s*:')) {
    $g = [Guid]::Parse($match.Groups[1].Value).ToByteArray()
    $tail = (($g[8..15] | ForEach-Object { '0x{0:x2}' -f $_ }) -join ',')
    $browserGuidText += ('template<> inline GUID WebGuid<{0}>() {{ return {{0x{1:x8},0x{2:x4},0x{3:x4},{{{4}}}}}; }}' -f $match.Groups[2].Value,[BitConverter]::ToUInt32($g,0),[BitConverter]::ToUInt16($g,4),[BitConverter]::ToUInt16($g,6),$tail) + "`n"
}
[IO.File]::WriteAllText((Join-Path $browserSdkDir 'WebGuids.h'),$browserGuidText,[Text.UTF8Encoding]::new($false))
Copy-Item -LiteralPath (Join-Path $browserSdkDir 'LICENSE.txt') -Destination (Join-Path $PSScriptRoot 'dependencies/WebView2-LICENSE.txt') -Force
Write-Output 'Pinned WebView2 SDK and x64 loader verified; browser runtime is optional.'
