# USB TF Card MSC — Launcher utility

## Implemented (source code; not hardware verified)
- Uses official Waveshare **V2** SD card mapping: SDMMC 1-bit CLK=GPIO41, CMD=GPIO39, D0=GPIO40 (from `Arduino/examples/04_SD_Card/sdcard_bsp.cpp`).
- Initializes SDMMC and exports card via Espressif `esp_tinyusb` 2.0.1, `tinyusb_msc_new_storage_sdmmc()` and native USB OTG.
- SD card is not locally mounted while exported. Automatic formatting explicitly disabled.
- USB service belongs to Launcher only; no fourth OTA app, no Wi-Fi.
- LCD layout uses black background and a centered icon+English instruction group.

## Known incomplete / safety boundaries
**The project has no hardware-tested USB MSC yet.** The selected USB cable/port must use native S3 USB D- GPIO19 / D+ GPIO20; USB Serial/JTAG shares a PHY with USB OTG. Pin wiring/connector must be checked on Waveshare V2 schematic or hardware.

**Host-safe eject signal:** no reliable host eject acknowledgement has been connected yet. Back key must NOT tear down storage while `USB_STORAGE_EXPORTED`. To leave safely: eject from Finder, then restart the device or implement an explicit host-eject callback / safe-state indication and back key gate. Current `usb_storage_can_exit()` deliberately blocks exit during export. A USB cable disconnect alone is not interpreted as safe eject.

**Launcher hardware integration:** `board_support` is still a stub; USB is not yet triggered by a physical LVGL touch on device. Web preview simulates UI but cannot run USB MSC.

## Official references
- https://github.com/waveshareteam/ESP32-S3-Touch-LCD-3.49-V2/tree/main/Arduino/examples/04_SD_Card
- https://docs.espressif.com/projects/esp-usb/en/latest/esp32s3/usb_device.html
- https://components.espressif.com/components/espressif/esp_tinyusb/versions/2.0.1

## Physical Back button and safe eject design (2026-10-13)

The GPIO0 custom/BOOT key single-click is the requested Back key. On the USB page it sends a **request**, not an unconditional `lv_screen_load(home)`. The controller shall invoke `usb_storage_exit()`, then `launcher_ui_confirm_usb_exit()` **only after an ESP_OK result**. When storage is still exported or the host cannot be confirmed safely ejected, keep the USB page visible and show a human-readable warning. The button cannot itself instruct macOS to flush Finder's buffered writes; the user must first eject the disk in Finder, or a properly verified SCSI eject handshake must be implemented.

The ESP-IDF USB implementation was also aligned to the official esp_tinyusb MSC 2.x API: `tinyusb_msc_install_driver()` / `tinyusb_msc_uninstall_driver()` and `tinyusb_msc_storage_config_t.fat_fs.do_not_format`, using `TINYUSB_DEFAULT_CONFIG(NULL)`. This is still untested on a physical board.

Important: do not claim USB is fully complete yet: the host-eject acknowledgement, actual touchscreen callback, custom-key to LVGL controller binding and board display/touch bringup are pending. Do not use the currently generated Launcher bin on a device before those are implemented and tested.
