param([switch]$Monitor)
$ErrorActionPreference='Stop'
if (-not $env:IDF_PATH) {
  $profilePath='C:\Espressif\tools\Microsoft.v5.5.5.PowerShell_profile.ps1'
  if (!(Test-Path -LiteralPath $profilePath)) { throw 'Open the ESP-IDF 5.5.5 terminal, then run this script again.' }
  . $profilePath
}
Push-Location (Join-Path $PSScriptRoot 'firmware')
try {
  & python (Join-Path $env:IDF_PATH 'tools\idf.py') build
  if ($LASTEXITCODE -ne 0) {throw 'Build failed. See the messages above.'}
} finally {Pop-Location}
