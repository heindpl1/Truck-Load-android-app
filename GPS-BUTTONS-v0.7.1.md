# Android v0.7.1 — GPS button feedback and recovery

This Android-only patch works with the existing firmware and web controller v0.7.0. No Heltec flash is needed. Install the signed APK over the existing app; its version code is 10.

GPS buttons previously used a static background without a press animation, and the location Auto/Pause/Off controls did not show the selected controller state. Refresh also shared the same settings-ready restriction as save buttons, so a missed initial settings reply disabled recovery.

- Standard buttons now show a visible ripple on press while retaining their readable dark backgrounds.
- Auto/Pause/Off highlights the state reported by the controller. The pressed action shows a waiting label until acknowledged. A press does not falsely claim a state change before readback.
- Completed saves show an explicit confirmation; rejection, timeout and disconnect retain clear failure feedback and unsaved edits.
- Refresh remains available while connected, including when settings have not loaded. Reading, completion and incomplete-reply messages explain what is happening. Incomplete initial reads retry up to three times.
- Settings-dependent buttons look dimmed when unavailable. Creating or editing areas requires a complete area-list readback, protecting saved areas from being overwritten when an initial list reply was missed.
- Zoom reports the changed zoom level. Follow highlights the local follow preference and explains when it is waiting for a Heltec GPS fix. Map controls can respond locally without a Bluetooth connection or GPS fix; those actions do not fabricate a truck location.
- Original plant and simple GPS controls show a controller acknowledgement when saved.

Validation: signed release build and upgrade install on the Android 15 emulator; 638 pure Java protocol/policy/model checks, six startup checks, thirty existing UI regression checks and thirty-five location UI checks passed. New checks exercise actual button clicks, selected state before/after readback, waiting text, ripple/background colors, refresh with missing settings, incomplete-list protection, bounded retries, and Zoom/Follow without GPS. Final cold start was checked for app exceptions. Connected records in UI tests are synthetic fixtures; physical phone-to-Heltec operation still needs user confirmation.
