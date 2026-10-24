# Truckload v0.6.4

Firmware, Android and web app versions are synchronized at 0.6.4 (Android version code 6).

- Load bars use fractional group coverage: on a seven-group strip, 50% load produces three full groups and one half-bright group. Center bars use symmetric fading edges, including odd counts. The displayed/telemetry weight is unchanged; only the light fill eases toward new readings over about 260 ms. Invalid real readings still clear the load indication immediately. Full/overload colors continue to follow the actual load percentage.
- LED render target increased from 10 to 40 updates/second; actual timing also depends on existing telemetry and strip transmission work.
- Pulse, chase and alternating effects now fade. Five additional driving effects: Comet trail, Scanner sweep, Color wave, Aurora and Theater chase. Existing six pattern command names and stored indices retain their meanings. New selections persist across restarts.
- Android and web Lighting pages use pattern dropdowns with Apply buttons. Native animated illustrations show the new effects and fading edges; illustrations remain labelled previews, not measurements. The web app also provides driving mode, pattern and brightness controls. New effects advertise FADE_PATTERNS and are gated on companion firmware support.
- Existing WRGB ordering, saved zone sizing, colors, profiles, calibration, GPS, OLED and supervised bench safeguards are retained. Each hardware group still shares one driver; fading does not create individual-LED addressing.

## Validation
Firmware built with ESP-IDF 5.5.5 and flashed on COM3 with verified hash. Android APK compiled and signed with the existing personal signing identity. Existing Android protocol suites passed (304 framing, 19 updater, 266 feature/map, 33 safety/compatibility). Existing web regressions, strip-setting tests and new dropdown/capability/apply/readback tests passed.

All 11 driving patterns were accepted/read back on the Heltec. A supervised Zone 2 demo showed the fractional load fill and five new animations; the user watched and approved the appearance. New SCANNER selection survived physical reset, saved zone counts and other driving settings were retained, and original settings were restored and verified after reset. Bench was stopped. Only Zone 2 has a physical strip attached; Zone 1 output remains physically untested.

## Installation
The connected Heltec is already updated. On the phone use App updates / Check for updates, then install v0.6.4 over the existing app. Original signing identity retained; do not uninstall. Bluetooth controls work after disconnecting the PC Bluetooth test client (now released). USB firmware update completed; the app may need to reconnect after a controller reset.

Firmware ZIP is an application/source overlay for the existing tested ESP32-S3 installation. Flash only the app at 0x10000 without erasing NVS. Source/build and original firmware backup are under C:\TruckLoadBuild. Keep backups private because saved controller data may be present.
