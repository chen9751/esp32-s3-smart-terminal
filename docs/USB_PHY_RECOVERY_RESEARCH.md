# ESP32-S3 USB-OTG MSC -> USB Serial/JTAG recovery (2026-10-13)

## Confirmed on this board
Launcher works; Finder can safely eject TF card; GPIO0 Back returns to UI. However the original USB Serial/JTAG console stays disconnected until a hardware reset. `esp_restart()` on MSC exit was already tested and did not restore it; removed in commit bc66f23.

## Official references
1. ESP-IDF issue https://github.com/espressif/esp-idf/issues/15912 — regression present since v5.3.3; uninstall TinyUSB then `usb_new_phy(USB_PHY_CTRL_SERIAL_JTAG)` returns success but Mac does not enumerate the hardware console.
2. **New lead**: Espressif's own Arduino-ESP32 `cores/esp32/esp32-hal-tinyusb.c` function `usb_switch_to_cdc_jtag()`: https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal-tinyusb.c . Unlike the simple public PHY API it resets the USB OTG peripheral, restores RTC USB PHY selection and USB Serial/JTAG pad routing, forces D+/D- low for host re-enumeration, enables hardware USB-JTAG interrupt, and waits for BUS_RESET. Its usage is tied to RESTART_BOOTLOADER; this is **not** a published ESP-IDF 5.5.4 drop-in API and must not be pasted into Launcher without a separate recovery component and hardware test.
3. ESP-IDF native USB console docs: https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-guides/usb-serial-jtag-console.html
4. IDF USB-OTG console docs: https://github.com/espressif/esp-idf/blob/master/docs/en/api-guides/usb-otg-console.rst — native USB OTG CDC and TinyUSB are incompatible as separate stacks. Composite TinyUSB CDC+MSC keeps a serial endpoint but this is NOT the same as hardware USB Serial/JTAG and does not automatically preserve ROM flashing behavior.

## Recommended implementation plan
- Retain the working MSC eject/cleanup state machine and LCD/power/brightness/audio code unchanged.
- Prototype a **separate, opt-in** ESP-IDF S3 USB PHY restore helper using the supported HAL/SoC paths consistent with the Arduino-ESP32 implementation, called ONLY after successful MSC shutdown. Never write eFuses. Ensure no interrupts, handles or pad conflicts are left behind. Add explicit error results and a fallback that leaves Launcher running.
- First hardware test: Finder eject -> GPIO0 Back -> Mac detects the original USB Serial/JTAG interface -> open a fresh `idf.py monitor` -> press reset with monitor attached -> confirm esptool can flash without manually touching RESET. Repeatedly re-enter MSC and exit.
- Since Mac may not reattach a pre-existing serial-file handle, verify device re-enumeration independently from `idf.py monitor` auto-reconnect.
- Alternative architecture: persistent TinyUSB CDC+MSC composite exposes its own debug serial continuously but requires refactoring console and deployment; it will NOT make the hardware Serial/JTAG show up during MSC.

Research does not constitute a successful recovery. Do not claim fixed until test evidence arrives.
