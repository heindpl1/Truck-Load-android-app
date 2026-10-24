Truckload Android v0.6.5 startup hotfix

Reproduced v0.6.4 startup failure on Android 15: IndexOutOfBoundsException in MainActivity.update, caused by an obsolete loop indexing load-pattern buttons after they were replaced with dropdowns. Removed the stale button list and refresh loop; OLED buttons refresh independently.

Verified: install v0.6.5 over v0.6.4 with the same signing identity and without clearing data; cold launch succeeds. Instrumented tests passed for all six tabs, four load/eleven driving dropdown choices, disconnected control gating, scanner/center controller readback, and repeated status refresh. Existing Java protocol suites also passed. Subsequent cold launch remains running and AndroidRuntime reports no fatal exceptions.

Firmware/web v0.6.4 remains unchanged and compatible. No controller reflash required. Install the APK directly over the existing app; uninstalling is unnecessary. If already uninstalled, install this normally and reconnect; calibration and controller settings remain on the Heltec.
