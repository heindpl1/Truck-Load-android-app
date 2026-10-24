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
$metadata=[ordered]@{versionCode=$code;versionName=$version;apkUrl="https://github.com/$repository/releases/download/v$version/Truckload-Android.apk";sha256=(Get-FileHash $apk -Algorithm SHA256).Hash.ToLowerInvariant();sizeBytes=(Get-Item $apk).Length;notes='Cleaner lighting page with mode-specific controls, collapsed strip setup and testing sections, saved zone summaries and clear acknowledgement-based save feedback. Load examples removed. Android startup and UI checks passed before publication. Compatible with firmware 0.6.4; no controller update needed.'}
$metadata | ConvertTo-Json | Set-Content (Join-Path $release 'truckload-update.json') -Encoding utf8
Write-Host "Upload both files in $release to a published GitHub release tagged v$version."
