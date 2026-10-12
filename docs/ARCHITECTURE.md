# Architecture — three independent functional applications

| Boot target | Project | Intended behavior |
|---|---|---|
| factory | apps/launcher | 3 cards, settings, reboot to chosen app |
| ota_0 | apps/ai_chat | LVGL animated faces and audio cloud pipeline, no WakeNet |
| ota_1 | apps/home_assistant | WakeNet always available while touch dashboard/clock run |
| ota_2 | apps/sony_remote | BLE HID + touch remote + IMU control |

The 4 binaries are separate ESP-IDF projects. Shared code belongs in components/, never copy-paste page implementations. Page files live under each app's main/ui/. A page handles only presentation and user interaction; a service/controller handles Wi-Fi, HA API, mic stream, audio playback and HID.

## Planned submodules

- **board_support**: pin mappings based on verified V2 schematics; LCD/touch init; audio codec/I2S; battery, IMU; PSRAM allocation. This component currently only exposes a placeholder interface.
- **mode_manager**: look up factory/ota_0/ota_1/ota_2 partition by label, set boot partition, and restart. Used by all apps for 'return to launcher'.
- **AI**: ui/face_page + ui/connection_page; future services/audio_transport and remote backend adapters.
- **HA**: ui/clock_page + ui/device_page + ui/listening_page + ui/result_page; future services/wakenet and HA client.
- **Remote**: ui/remote_page + ui/bluetooth_page + future services/ble_hid and imu_pointer.

## Important limitations of phase 0

UI page functions are placeholders that only log names; they do not draw LVGL widgets. No real screen/touch/audio/IMU/BLE/Wi-Fi support is claimed. Flash space estimates are budgets, not measured binary sizes. Button events and navigation are not wired. This staged approach avoids incorrectly assuming the V2 pinout/drivers.

## Switch lifecycle

Each app checks the ESP-IDF OTA app state on boot and, if pending verification, marks it valid once its minimum self-test succeeds. Returning to Launcher sets factory as next boot partition and reboots. Launcher sets selected ota partition and reboots. **A partition without a flashed valid image must not be selected**; mode_manager performs a basic image descriptor check but does not guarantee runtime stability. A hardware/recovery boot path must be designed before deployment.
