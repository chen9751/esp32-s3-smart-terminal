# Launcher-only USB TF-card storage mode (planned)

**Final architecture decision:** NO fourth functional app, NO new OTA partition, NO Wi-Fi file transfer, NO server or syncing daemon. USB Mass Storage Class (MSC) lives inside Launcher as a seldom-used **utility** accessed through a **small fourth entry**. Three large cards are AI Chat, Home Assistant and Sony Remote.

- Launcher always starts its 10-second inactivity countdown targeting HA.
- Any tap cancels the countdown. A tap on the USB shortcut cancels it and opens a dedicated USB storage page.
- Launcher USB mode is never automatically entered; it must be explicitly selected.
- Desired Mac workflow: plug USB cable, enter USB storage, Finder mounts TF card, copy assets, **Eject** disk in Finder, safely leave USB mode. No card removal.
- Exclusive storage ownership: ESP FATFS must be unmounted before exposing writable MSC; block ESP filesystem reads/writes while host owns card. Host eject/disconnect must be coordinated before remount. A raw USB disconnect alone is NOT guaranteed safe if writes were in progress.
- Launcher must keep minimum UI fonts/icons in internal Flash so it can show the storage/recovery pages without TF resources.
- Future implementation uses native USB OTG + TinyUSB MSC with hardware-verified Waveshare V2 USB wiring and SD interface. Avoid assumptions about USB Serial/JTAG port. MSC may replace active USB serial logging.
- **Current code is intentionally fail-closed**: `components/usb_storage` is an API/ownership boundary only. `usb_storage_enter()` returns `ESP_ERR_NOT_SUPPORTED` until real drivers, exclusive ownership and safe eject are implemented. Finder will **not** see a disk yet. Do not present this as a working USB transfer implementation.

Expected UI: three prominent mode cards plus a small USB icon/link; USB page shows card/host status and disallows exit until safe. No visual styling frozen yet.

References:
- https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-guides/usb-otg-console.html
- https://docs.espressif.com/projects/esp-usb/en/latest/esp32s3/usb_device.html
- https://github.com/waveshareteam/ESP32-S3-Touch-LCD-3.49-V2

Acceptance tests after hardware bring-up: Mac Finder mounts; repeated copy/eject cycles preserve FAT; disconnect mid-transfer reports recovery; long files copy correctly; Launcher 10s HA policy remains unaffected; no SD filesystem access races.
