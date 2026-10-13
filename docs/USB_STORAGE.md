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
