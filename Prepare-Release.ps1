param([string]$Sdk="D:\Android",[string]$Java="D:\Java\corretto17\jdk17.0.17_10",[switch]$SkipBuild)
$ErrorActionPreference="Stop"
if(!$SkipBuild){& "$PSScriptRoot\Build.ps1" -Sdk $Sdk -Java $Java}
[xml]$manifest=Get-Content "$PSScriptRoot\app\src\main\AndroidManifest.xml"
$ns='http://schemas.android.com/apk/res/android'
$version=$manifest.manifest.GetAttribute('versionName',$ns)
$code=[long]$manifest.manifest.GetAttribute('versionCode',$ns)
$javaSource=Get-Content "$PSScriptRoot\app\src\main\java\com\truckload\controller\AppUpdates.java" -Raw
$repository=[regex]::Match($javaSource,'DEFAULT_REPOSITORY="([^"]+)"').Groups[1].Value
if(!$repository -or $repository -notmatch '^[A-Za-z0-9-]+/[A-Za-z0-9_.-]+$'){throw 'Set a valid DEFAULT_REPOSITORY first.'}
$release=Join-Path $PSScriptRoot 'release'
New-Item -ItemType Directory -Force $release | Out-Null
$apk=Join-Path $PSScriptRoot 'Truckload-Android.apk'
Copy-Item $apk (Join-Path $release 'Truckload-Android.apk') -Force
$metadata=[ordered]@{versionCode=$code;versionName=$version;apkUrl="https://github.com/$repository/releases/download/v$version/Truckload-Android.apk";sha256=(Get-FileHash $apk -Algorithm SHA256).Hash.ToLowerInvariant();sizeBytes=(Get-Item $apk).Length;notes='Fixes the v0.6.4 startup crash caused by obsolete pattern-button updates after moving to dropdowns. Verified by reproducing the crash and installing this update over v0.6.4 on Android, then testing startup and all tabs. Firmware 0.6.4 remains compatible and needs no reflash.'}
$metadata | ConvertTo-Json | Set-Content (Join-Path $release 'truckload-update.json') -Encoding utf8
Write-Host "Upload both files in $release to a published GitHub release tagged v$version."
