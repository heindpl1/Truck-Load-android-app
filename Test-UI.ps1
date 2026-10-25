$ErrorActionPreference='Stop'
$env:JAVA_HOME='D:\Java\corretto17\jdk17.0.17_10'
$env:Path="$env:JAVA_HOME\bin;$env:Path"
$root='C:\TruckLoadBuild\android\ui-tests'
$bt='D:\Android\build-tools\36.1.0';$platform='D:\Android\platforms\android-35\android.jar'
New-Item -ItemType Directory -Force "$root\classes","$root\dex" | Out-Null
function Run([string]$exe,[string[]]$arguments){ & $exe @arguments; if($LASTEXITCODE -ne 0){throw "Test build failed: $exe"} }
Run "$bt\aapt2.exe" @('link','-I',$platform,'--manifest',"$root\AndroidManifest.xml",'-o',"$root\unsigned.apk")
Run "$env:JAVA_HOME\bin\javac.exe" @('-encoding','UTF-8','-source','8','-target','8','-classpath',"$platform;C:\TruckLoadBuild\android\build\classes.jar",'-d',"$root\classes","$root\UiInstrumentation.java")
Run "$env:JAVA_HOME\bin\jar.exe" @('cf',"$root\test.jar",'-C',"$root\classes",'.')
Run "$bt\d8.bat" @('--lib',$platform,'--classpath','C:\TruckLoadBuild\android\build\classes.jar','--min-api','26','--output',"$root\dex","$root\test.jar")
Add-Type -AssemblyName System.IO.Compression.FileSystem
$zip=[System.IO.Compression.ZipFile]::Open("$root\unsigned.apk",'Update')
try{[System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($zip,"$root\dex\classes.dex",'classes.dex') | Out-Null}finally{$zip.Dispose()}
Run "$bt\zipalign.exe" @('-f','4',"$root\unsigned.apk","$root\aligned.apk")
Run "$bt\apksigner.bat" @('sign','--ks','C:\TruckLoadBuild\android\.local-signing\truckload.jks','--ks-key-alias','truckload','--ks-pass','file:C:\TruckLoadBuild\android\.local-signing\password.txt','--v4-signing-enabled','false','--out',"$root\feature-tests.apk","$root\aligned.apk")
Run 'D:\Android\platform-tools\adb.exe' @('-s','emulator-5556','install','-r',"$root\feature-tests.apk")
Run 'D:\Android\platform-tools\adb.exe' @('-s','emulator-5556','shell','am','instrument','-w','com.truckload.controller.uitests/.UiInstrumentation')

