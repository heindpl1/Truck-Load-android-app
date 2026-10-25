param([string]$Java="D:\Java\corretto17\jdk17.0.17_10")
$ErrorActionPreference='Stop'
$j=Join-Path $PSScriptRoot 'app\src\main\java\com\truckload\controller'
$destination=Join-Path $PSScriptRoot 'build\test'
New-Item -ItemType Directory -Force $destination | Out-Null
& "$Java\bin\javac.exe" -encoding UTF-8 -d $destination "$j\GeoModel.java" "$j\Telemetry.java" "$j\MapProjection.java" "$j\SessionPolicy.java" "$j\UpdatePolicy.java" @(Get-ChildItem "$PSScriptRoot\tests\*.java" | ForEach-Object {$_.FullName})
if($LASTEXITCODE -ne 0){throw 'Test compilation failed'}
foreach($test in @('TelemetryTest','UpdatePolicyTest','FeatureProtocolTest','SessionPolicyTest','GeoProtocolTest')){
 & "$Java\bin\java.exe" -cp $destination $test
 if($LASTEXITCODE -ne 0){throw "Failed: $test"}
}
