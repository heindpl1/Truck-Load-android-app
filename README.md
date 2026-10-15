# Truckload for Android

This is a native Android application: native Android screens and Bluetooth LE, with no WebView or website server. It controls the existing TruckLoad-Heltec firmware using the same Nordic UART service as the web app.

## Install on your phone

1. Download **Truckload-Android.apk** from https://github.com/heindpl1/Truck-Load-android-app/releases/latest on your phone.
2. Open the APK from Downloads. If Android asks, allow that browser or file manager to install apps, then tap Install. You can turn that permission off afterward.
3. Open **Truckload**. Power the Heltec, and disconnect the web app first; the controller accepts one connection at a time.
4. Tap **Connect**, allow **Nearby devices**, and select **TruckLoad-Heltec**. On Android 8–11, Bluetooth discovery also requires the system Location permission and Location switch; this app does not read the phone's GPS.
5. Keep **Sensor input: Off** while the pressure sensor is disconnected. The load display stays blank until the enabled sensor has a valid, calibrated reading.

Android 8.0 or newer, with Bluetooth LE, is required. Bluetooth controls work without an account, internet service or PC. Internet access is used for map tiles while viewing GPS, and for app update checks/downloads. Bluetooth controls and bench tests work without internet. The phone must remain within Bluetooth range. The app is intended for foreground use; connection survival while Android suspends or kills it is not guaranteed. Reopen and reconnect when needed.

## Controls

- **Overview:** live weight, pressure, load progress, selected trailer, sensor enable.
- **Trailers:** create/select trailers, target and overload reference, capture calibration against known scale weights.
- **Lighting:** manual/GPS driving controls, six driving patterns, brightness, two driving colors, work lights, and animated examples of all four load patterns.
- **Colors:** six mode colors, RGB sliders and hexadecimal input, saved on the Heltec.
- **OLED:** choose one of four pages. No automatic cycling; selection is saved on the controller.
- **GPS:** native map with the Heltec position, saved plant/geofence, drag, zoom and follow; satellite status, speed and plant controls. It uses the Heltec GPS.

The existing controller firmware implements calibration and lighting. Installing this APK does not replace or flash firmware. Firmware color/OLED/sensor controls must be present (the current TruckLoadBuild firmware is compatible). Invalid readings are displayed as blanks; this app does not fabricate weights.

## Build on Windows

The source includes Build.ps1. It uses the official Android SDK tools directly, without Gradle or third-party Android libraries.

Prerequisites: JDK 17, Android SDK platform android-35 and build-tools 36.1.0.

```powershell
Set-Location "PATH_TO_EXTRACTED_ANDROID_SOURCE"
.\Build.ps1 -Sdk "D:\Android" -Java "D:\Java\corretto17\jdk17.0.17_10"
```

The result is Truckload-Android.apk. The application id is com.truckload.controller, version 0.3.0 (3), minimum API 26, target API 35. SDK and Java paths can be changed with the parameters above. Build products go into build/.

Build.ps1 creates a private personal signing identity in .local-signing/ on its first run. Keep this folder private and backed up to sign compatible updates. It is deliberately excluded from the source download. A build signed with a different key cannot update an existing installation; uninstall the old app first. Controller settings and calibration are stored on the Heltec, not in the Android installation.

Protocol tests:

```powershell
$jdk = "D:\Java\corretto17\jdk17.0.17_10\bin"
& "$jdk\javac.exe" -encoding UTF-8 -d build\test app\src\main\java\com\truckload\controller\Telemetry.java tests\TelemetryTest.java
& "$jdk\java.exe" -cp build\test TelemetryTest
```

## Driving lights, map and bench testing

Version 0.3.0 uses companion firmware 0.6.1 for the new driving and bench commands. The Heltec connected to this PC was updated over USB. Other boards can use Truckload-firmware-v0.6.1.zip from this release. Older firmware keeps the new hardware controls disabled and shows that an update is required. Existing Bluetooth controls remain compatible.

Lighting → Driving lights offers manual on, return to load mode, or automatic GPS control. Choose solid, gentle pulse, chase, alternating, breathing or rainbow. Primary and secondary colors and driving brightness are saved on the controller. Chase and alternating use both colors; rainbow generates its own spectrum. Preview strips are labelled examples, not measured loads.

GPS shows the Heltec's valid position in blue and the saved plant/geofence in green. Drag the map, zoom in/out, or choose Follow Heltec. No fake default location is used without a GPS fix. Native map imagery comes from OpenStreetMap over HTTPS, with visible attribution and a local tile cache that honours HTTP caching. Viewing tiles tells the provider your IP address and the requested map area. No phone GPS permission is added. OpenStreetMap tile policy: https://operations.osmfoundation.org/policies/tiles/ .

Overview → Bench mode (also available from Lighting) opens a temporary tester. Tap Start bench test. Test load, driving, work or off outputs. The SIMULATED load slider operates bench lights; the separate SIMULATED pressure slider previews the selected trailer's calibration and target settings. Neither is published as real weight or pressure. Real sensor testing has a separate explicit switch and shows ADC voltage, divided-back sensor voltage, status and real filtered pressure.

Bench sensor enable does not change the normal saved sensor setting. Calibration capture is locked during a test. Closing the tester, leaving the app or disconnecting stops the test, and firmware stops it after 15 seconds without a heartbeat. Bench mode never survives a restart. Normal lighting mode resumes after bench mode ends. Changes you explicitly make to saved driving colors, brightness or patterns remain saved. Real LED appearance and sensor accuracy still need checking with the actual wired hardware.

## In-app updates

Install the updater-enabled app once. Then open Overview → App updates → Check for updates. The app checks the latest published GitHub release. When a newer version is available, tap Download update, allow Truckload to install updates if Android asks, then approve Update. Do not uninstall the app. Updates keep Android settings; calibration and lighting settings remain on the Heltec.

Repository: heindpl1/Truck-Load-android-app. Keep it public. No GitHub token is stored in the app. Downloads require HTTPS. The app checks size, SHA-256, package name, newer version and the matching signing certificate before opening Android's installer. Cancellation prevents installation. Invalid or unavailable releases leave the existing installation intact.

To publish a future version: increase versionCode and versionName in AndroidManifest.xml, keep the same private .local-signing folder, and run Prepare-Release.ps1. Create a GitHub release tagged v plus versionName. Upload release/Truckload-Android.apk and release/truckload-update.json and publish it as the latest stable release. Never upload the signing folder or its contents. Phone users choose when to check and approve updates.

## Privacy

INTERNET permission is used for GPS map imagery and user-initiated update checks/APK downloads. REQUEST_INSTALL_PACKAGES lets the app hand verified updates to Android for approval. No analytics, advertising, account or phone-GPS reads. BLE Nearby devices permissions are used to scan and connect. Android 8–11 additionally requires Location permission for BLE discovery. The local selected tab is stored on the phone; configuration is read from and written to the Heltec.
