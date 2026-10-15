param([string]$Sdk="D:\Android",[string]$Java="D:\Java\corretto17\jdk17.0.17_10")
$ErrorActionPreference="Stop"
$env:JAVA_HOME=$Java
$env:Path="$Java\bin;$env:Path"
$root=$PSScriptRoot
$build=Join-Path $root "build"
New-Item -ItemType Directory -Force $build,"$build\generated","$build\classes","$build\dex" | Out-Null
$bt=Join-Path $Sdk "build-tools\36.1.0"
$platform=Join-Path $Sdk "platforms\android-35\android.jar"
function Run([string]$exe,[string[]]$arguments){ & $exe @arguments; if($LASTEXITCODE -ne 0){throw "Build failed: $exe"} }
Run "$bt\aapt2.exe" @('compile','--dir',"$root\app\src\main\res",'-o',"$build\resources.zip")
Run "$bt\aapt2.exe" @('link','-I',$platform,'--manifest',"$root\app\src\main\AndroidManifest.xml",'--java',"$build\generated",'-o',"$build\unsigned.apk",'-R',"$build\resources.zip",'--auto-add-overlay')
$sources=@(Get-ChildItem "$root\app\src\main\java","$build\generated" -Filter *.java -Recurse | ForEach-Object { $_.FullName })
Run "$Java\bin\javac.exe" (@('-encoding','UTF-8','-source','8','-target','8','-classpath',$platform,'-d',"$build\classes")+$sources)
Run "$Java\bin\jar.exe" @('cf',"$build\classes.jar",'-C',"$build\classes",'.')
Run "$bt\d8.bat" @('--lib',$platform,'--min-api','26','--output',"$build\dex","$build\classes.jar")
Add-Type -AssemblyName System.IO.Compression.FileSystem
$zip=[System.IO.Compression.ZipFile]::Open("$build\unsigned.apk",'Update')
try {Get-ChildItem "$build\dex" -Filter *.dex | ForEach-Object {[System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($zip,$_.FullName,$_.Name) | Out-Null}} finally {$zip.Dispose()}
Run "$bt\zipalign.exe" @('-f','4',"$build\unsigned.apk","$build\aligned.apk")
$keyDir=Join-Path $root '.local-signing';New-Item -ItemType Directory -Force $keyDir | Out-Null
$key=Join-Path $keyDir 'truckload.jks';$password=Join-Path $keyDir 'password.txt'
if(!(Test-Path $key)) { $bytes=New-Object byte[] 32;[System.Security.Cryptography.RandomNumberGenerator]::Create().GetBytes($bytes);[IO.File]::WriteAllText($password,[Convert]::ToBase64String($bytes));Run "$Java\bin\keytool.exe" @('-genkeypair','-keystore',$key,'-storepass:file',$password,'-keypass:file',$password,'-alias','truckload','-dname','CN=Truckload Personal Build','-keyalg','RSA','-keysize','2048','-validity','10000') }
Run "$bt\apksigner.bat" @('sign','--ks',$key,'--ks-key-alias','truckload','--ks-pass',"file:$password",'--v4-signing-enabled','false','--out',"$root\Truckload-Android.apk","$build\aligned.apk")
Run "$bt\apksigner.bat" @('verify','--verbose',"$root\Truckload-Android.apk")
Write-Host "Created $root\Truckload-Android.apk"

