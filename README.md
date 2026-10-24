# Truckload for Android

This is a native Android application: native Android screens and Bluetooth LE, with no WebView or website server. It controls the existing TruckLoad-Heltec firmware using the same Nordic UART service as the web app.

## Install on your phone

1. Download **Truckload-Android.apk** from https://github.com/heindpl1/Truck-Load-android-app/releases/latest on your phone.
2. Open the APK from Downloads. If Android asks, allow that browser or file manager to install apps, then tap Install. You can turn that permission off afterward.
3. Open **Truckload**. Power the Heltec, and disconnect the web app first; the controller accepts one connection at a time.
4. Tap **Connect**, allow **Nearby devices**, and select **TruckLoad-Heltec**. On Android 8–11, Bluetooth discovery also requires the system Location permission and Location switch; this app does not read the phone's GPS.
5. Keep **Sensor input: Off** while the pressure sensor is disconnected. The load display stays blank until the enabled sensor has a valid, calibrated reading.

Android 8.0 or newer, with Bluetooth LE, is required. Bluetooth controls work without an account, internet service or PC. Internet access is used for map tiles while viewing GPS, and for app update checks/downloads. Bluetooth controls and bench tests work without internet. The phone must remain within Bluetooth range. The app is intended for foreground use; connection survival while Android suspends or kills it is not guaranteed. While the app is open it remembers the last successfully connected Heltec and makes up to three direct reconnect attempts (after 2, 8 and 20 seconds), without repeated scans. Intentional Disconnect/Cancel suppresses retries for that app session. Connect remains available. Reopening the app allows retrying again; permission denial or disabled Bluetooth never starts a scan or a permission loop.

## Controls

- **Overview:** live weight, pressure, load progress, quick trailer selection, large work-light toggle, guided calibration, sensor enable, Bluetooth/sensor/GPS status and compatibility.
- **Trailers:** create/select trailers, target and overload reference, capture calibration against known scale weights.
- **Lighting:** manual/GPS driving controls, six driving patterns, brightness, two driving colors, work lights, and animated examples of all four load patterns.
- **Colors:** six mode colors, a visual preset palette, selected-color preview and custom six-digit hexadecimal input, saved on the Heltec.
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

The result is Truckload-Android.apk. The application id is com.truckload.controller, version 0.6.6 (8), minimum API 26, target API 35. SDK and Java paths can be changed with the parameters above. Build products go into build/.

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

## Version 0.4.0 changes

The existing dark/mint layout, map, OLED pages, trailer settings, lighting patterns and app updater remain. The palette is shared by Colors and both driving-color controls. Swatches update the preview immediately; Save sends the original color command without changing lighting mode. Work lights use the existing WORK_LIGHT command, saved color/brightness, and controller-confirmed state; while waiting for confirmation the large toggle is disabled. Exit Bench Mode first.

Overview provides direct selection among saved trailers through PROFILE_SELECT. Guided calibration also opens from Trailers. It enables the real sensor only explicitly, requires fresh READY sensor and pressure records, blocks bench values, shows the actual saved count, and captures known scale weights using CAL_CAPTURE. At least two points at meaningfully different pressures are needed. The guide checks a minimum 2 PSI separation between points captured in the current guide session; the protocol does not expose pressures of existing saved points. It never clears saved points or changes the controller calibration math. An empty trailer's known scale weight need not be zero.

The existing Bench Mode now contains zone selection, direct Off/Red/Green/Blue/White tests, and diagnostics. Companion firmware **0.6.2** supports independent GPIO43/GPIO44 zones, actual raw 12-bit ADC counts and firmware identification. Firmware **0.6.1 remains compatible** for its existing both-zone load/driving/work/off tests; independent zone/color controls remain disabled. Unsupported controls never send speculative commands. A dedicated isolated white-channel test remains disabled; White tests the current FastLED RGB-to-RGBW conversion. RSSI is shown as unavailable. Stale/missing diagnostic readings are labelled unavailable.

Capabilities and firmware identity are read from the controller; the app does not guess a version from the feature set. On firmware 0.6.1 the version is unavailable because it is not reported. The Overview identifies Compatible or Limited features, explaining which controls require an update. Android 8–11 and 12+ permission declarations are unchanged. No background service or repeated scanning was added. Bench state is never restarted automatically after a reconnect.

Run all JVM checks with `.\Test.ps1` (same optional -Java parameter as Build.ps1). Test results and remaining phone/physical hardware checks accompany this review build. Version 0.4.0 is distributed through the existing GitHub Releases app updater.


## v0.6.3 strip sizing

Lighting now includes saved sizes for both zones: addressable groups, physical LEDs/meter, and physical LEDs/group. Enter group counts directly or calculate from meters, review the represented size, then save. Reconnect reads the controller settings. Settings persist on the Heltec. The current Zone 2 test strip is 7 groups of 7 LEDs at 84 LEDs/meter (49 LEDs total, about 0.583 meters). Other strips can use different group sizes. Existing trailer calibration, GPS, OLED and lighting controls remain available. Firmware 0.6.3 is required for sizing. Companion firmware and browser sources are under companion-v0.6.3. Color order is WRGB, established from four user-observed channels; post-update physical verification is still pending.


## v0.6.4 fading and pattern dropdowns

Lighting now uses dropdowns for load and driving patterns. Five new effects join the existing six: Comet trail, Scanner sweep, Color wave, Aurora and Theater chase. Progress and center load bars fade their boundary groups and ease toward new readings. All effects use the saved group counts. Firmware 0.6.4 is required for the new patterns; the currently attached Heltec is updated. The Zone 2 demonstration was observed and approved by the user, and a new pattern was verified across reset. Zone 1 is untested physically. See companion-v0.6.4/CHANGES.md and release assets.


## Android v0.6.5 startup hotfix

Removes an obsolete update loop that still indexed the removed load-pattern buttons, causing v0.6.4 to crash at startup. Dropdowns and new patterns are retained. The crash was reproduced on Android 15, then the fixed APK installed over v0.6.4 without clearing app data. Cold launch, all six tabs, repeated refresh and dropdown readback passed instrumented tests. Test-Startup.ps1 builds and runs the startup regression tests on emulator-5556. Existing firmware/web app v0.6.4 remains compatible and needs no update.


## Android v0.6.6 UI cleanup

Lighting has a mode chooser with relevant controls, collapsed Strip setup with saved zone summaries, and collapsed Testing & diagnostics. The load example section is removed. Save feedback waits for controller acknowledgement and preserves unsaved edits. The same tabs are now in a single scrollable navigation row. Firmware/web v0.6.4 remains compatible. See UI-v0.6.6.md for validation and Test-UI.ps1 for the Android UI regression suite.
