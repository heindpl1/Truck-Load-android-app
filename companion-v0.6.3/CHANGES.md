# Truckload v0.6.3

Firmware, web app and local Android app report version 0.6.3.

- Independent persistent strip configuration for both zones: addressable IC group count, physical LEDs per meter, physical LEDs per group. Stored atomically as one NVS blob in the existing namespace, with validation and a checked save result. Existing trailer/profile/color/GPS/sensor settings are retained.
- User-confirmed Zone 2 test strip: 7 groups, 7 physical LEDs per group, 84 physical LEDs/m = 49 LEDs, approximately 0.583 m. Zone 1 retains its prior 120-group default pending measurement. Maximums remain 240 / 120 groups for Zones 1 / 2.
- Lighting page accepts group counts or calculates them from length. Length calculations round to whole addressable groups and show the represented physical size before saving. Firmware readback is the source of truth; user edits are preserved until acknowledged or explicitly discarded.
- All renderers use saved counts; unused tails are sent black every frame and on resizing.
- WS2814 byte order changed from RGBW to WRGB (FastLED RGB + W0, exact RGBW conversion). Observed on old firmware: Red -> White, Green -> Red, Blue -> Green, White -> Blue. Fourth-channel observation supplied by the user on Zone 2 before implementing the correction. Exact conversion sends equal RGB input solely through W; pure colors stay on their respective channels.
- Existing BLE commands remain available. New commands: GET_STRIP_CONFIG; SET_STRIP_CONFIG|count1|count2|density1|density2|LEDsPerGroup1|LEDsPerGroup2. STRIP_CONFIG readback appends maximum count1/count2. Counts and metadata must be positive integers; density <=1000 and LEDs per group <=30. Invalid or incomplete records are rejected before state changes. Failed NVS save is reported as an error.
- USB newline commands use the same main-loop handler and validation as BLE. Responses/telemetry are mirrored to USB. Replies to USB commands are isolated from BLE notifications, so diagnostic validation errors do not appear on the phone. Bench still requires a BLE connection and retains its existing disconnect/heartbeat shutdown.

## Validation
Firmware built with installed ESP-IDF 5.5.5 and flashed to COM3 with verified hash. Native Android APK built and signed with the existing local signing identity. Existing web regression tests and strip-setting DOM tests passed. Android suites: 304 framing/protocol, 19 update policy, 266 feature/map and 33 safety/compatibility checks passed. Hardware readback identified firmware 0.6.3 and the expected Zone 2 settings; 11 invalid strip configurations rejected without changing readback.

Physical-reset persistence verified with nondefault Zone 1 count 119; restored 120/7 and verified after a second reset. Existing driving configuration retained; bench stayed off. Hardware appearance: see strip-hardware-v063.txt for recorded results. Zone 1 physical output is untested because only Zone 2 has a test strip attached.

## Files and recovery
Firmware source: firmware/main. Browser app: webapp/index.html (serve through localhost or HTTPS for Web Bluetooth). Native app: android/Truckload-Android.apk. Install over the existing app to preserve phone settings; no phone was connected by USB during this work, so automatic installation was unavailable.

Backup: backups/before-v063 contains the previous source and a COM3 flash snapshot of addresses 0x0 through 0x10ffff, covering the old app, partition table, bootloader and NVS. The update flashes only the application at 0x10000 and does not erase NVS. Keep the backup private; it contains saved controller settings.
