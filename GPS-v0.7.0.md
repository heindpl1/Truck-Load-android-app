# Truckload v0.7.0 — location lighting and ground speed

App version 0.7.0 (Android code 9), firmware 0.7.0 and web controller 0.7.0 belong together. The APK keeps the existing signing identity and installs over v0.6.6 without uninstalling.

## Use

- Open **GPS → Location & GPS rules**. Add up to 12 areas: a circle around a coordinate/current truck GPS/map center, or a boundary drawn with 3–12 corners in order. Map tiles need internet; saved rules run on the Heltec without a phone or internet connection.
- Give each area a priority and separate **While driving** / **While stopped** actions. Actions include load indication, a driving pattern with colors and brightness, work light color and brightness, off, or inheritance. Highest priority wins an overlap; equal priority uses the smaller area, then the earlier slot. Inherit skips that area's action for that motion state and permits an underlying area/default.
- Under **Outside saved areas**, choose Driving, Stopped and No GPS fix actions independently. Inherit retains normal lighting. Defaults start as inherit and automation starts off, preserving existing behavior until configured.
- Enable/resume auto when ready. Manual work/driving controls pause these rules; resume them explicitly. Rule settings and manual pause survive restart. Bench tests take precedence. Normal automatic transitions do not rewrite saved base lighting settings or write flash.
- Default stopped threshold is 1 mph; driving threshold is 3 mph. Motion between those values retains its previous state. Boundary delay is 2.5 seconds; the current area has a 5 m exit buffer. Both are adjustable. Loss of a fresh location/speed uses the No GPS fix action immediately.
- Select **Ground speed** in driving effects or a location driving action. Under **Lighting → Ground-speed lighting**, adjust patch spacing/width and independently reverse Zone 1 and Zone 2. Alignment offsets let you line the strips up. Save your actual physical LEDs/m and LEDs/group in Strip setup first; these determine distance along each strip.
- Ground speed integrates fresh Heltec GPS speed, smooths speed changes and averages light coverage over each addressable group for fading edges. It freezes below 0.6 mph or with stale speed; it does not use phone GPS or simulated live motion. WS2814 cut sections remain the hardware's addressable units: software cannot control the seven physical LEDs inside a group independently.

The web controller exposes the same saved area/default/ground settings. Polygon corners can be entered as latitude, longitude pairs; the native Android app supplies map drawing.

## Validation before publication

- ESP-IDF 5.5.5 firmware build completed; app binary flashed at 0x10000 to the connected ESP32-S3 and its hash verified. NVS/calibration and other partitions were not erased. Compiler reported one harmless unused `geoModeNames` declaration; no build errors.
- 31 isolated geometry, overlap, priority, state hysteresis, delay, no-fix, manual override and fractional ground coverage checks passed on the actual Heltec. Includes concave polygon validation and rejection of a crossing boundary.
- USB protocol tests on the controller passed: synthetic overlapping property/scale, priority/tie resolution, different stopped action, outside/no-fix inheritance, invalid width rejection, independent directions/offsets, and saved areas/settings after controller restart. Existing strip counts and driving settings matched before/after. Synthetic areas were removed and automation left off. A USB-only flush/pacing fix prevents settings bursts from losing records; USB test responses are isolated from the phone's BLE connection.
- Android release APK built and its signature verified. Android 15 emulator upgrade installation succeeded. 638 pure Java protocol/policy/model checks, six startup checks, thirty existing UI regression checks and twenty new location UI checks passed. New checks cover per-zone directions, clean readback, draft retention, sequential/matching acknowledgements, pending-write locks, area editor, separate actions, timeout/disconnect and map corner coordinates. Final cold launch completed with the process alive and an empty crash buffer.
- Thirteen web location/settings protocol checks passed, both scripts passed syntax checks, and the web page opened in the browser with the new controls and no application startup exception. Web Bluetooth operation was not physically exercised.

## Remaining truck validation

The LED strips were disconnected during this update. Physical ground-speed appearance, direction, alignment, color output and real driving through overlapping areas have **not** been verified on the truck. Android's connected test records are synthetic fixtures; actual phone-to-controller use of the new screens remains to be checked. The previously confirmed WRGB channel ordering is retained. Treat this release as ready for a controlled truck-side trial, not a completed road test.

## Firmware source overlay

`companion-v0.7.0/firmware` contains the project/main sources and configuration; retain the existing components from the working ESP-IDF project (Arduino ESP32 3.3.12, FastLED 3.10.3, TinyGPSPlus 1.0.3, U8g2 2.36.16). No third-party component trees or private device backups are included. Run `Build-Firmware.ps1` in the matching build environment. The legacy project name still produces `truck_load_v060.bin`; its embedded version is 0.7.0. The release binary is named `truckload-firmware-v0.7.0.bin` and is an application-only image at **0x10000**, not a merged factory image. Do not erase flash to install it on an existing controller.
