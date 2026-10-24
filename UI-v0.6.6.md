Truckload Android v0.6.6 — cleaner lighting controls

- Lighting starts with a mode picker. Only settings for the selected load, driving/GPS or work mode are shown. Browsing does not operate the controller; Apply mode is explicit and waits for matching controller state.
- Strip setup is collapsed by default and shows saved zone/group/LED summaries. Unsaved edits are retained and clearly marked. Expanding or collapsing sections sends no device command.
- Testing & diagnostics is collapsed near the bottom. It contains the bench tester and controller compatibility details. The quick overview shortcut now opens Lighting.
- Removed the load-pattern example section and the redundant driving illustrations/control buttons.
- Compact connection indicator, one horizontally scrollable row of the same six tabs, reduced card spacing, consistent primary save/apply buttons, and a compact overview sensor/GPS status.
- Local save feedback distinguishes unsaved edits, waiting for acknowledgement, saved/applied state and failure/timeout. Pending writes disable the edited control. Periodic controller updates do not replace draft pattern choices. Load-pattern selection is labelled Applied, because it is not persistent in the current firmware; persistent brightness/drive settings are labelled Saved only after acknowledgement.

Validation before publishing: built and signed with the existing identity; installed over the previous app on Android 15 without clearing data. Six startup regression checks passed. Thirty new instrumented UI checks passed for all tabs, default collapsed state, independent expand/collapse, removal of examples, zone summaries, retained edits, mode-specific visibility, acknowledgement-gated mode changes, dropdown choices, save feedback, timeouts, bench gating and disconnects. Existing Java suites passed 622 framing/protocol/update/map/safety checks. Final cold launch succeeded and AndroidRuntime showed no fatal exception. Screen layouts reviewed using labelled emulator fixtures; connected-state UI tests used injected controller records, not a physical phone Bluetooth session.

Firmware and browser app v0.6.4 are unchanged and compatible. No firmware flash needed. Four load and eleven driving patterns remain supported. Existing profile, calibration, GPS, OLED, color and bench protocols are retained.

Install over the existing app through App updates or the direct release APK. The signing key is unchanged. Calibration and persistent controller settings remain on the Heltec.
