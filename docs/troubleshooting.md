# Troubleshooting log

Record each repeatable issue as: symptoms, board/ESP-IDF version, reproduction steps, root cause with official source, fix, regression test.

## Booting into wrong image

Cause to check: app's build generated a different partition table, or a generic `idf.py flash` overwrote the intended slot. Verify flasher_args.json, OTA data and per-slot images. Use a common partitions CSV across all four builds.

## WakeNet model exceeds 3.5 MiB

This is an open capacity risk, not a confirmed bug. Measure the chosen ESP-SR model(s) and format/partition expectations; reassign flash budget before firmware release.

## GitHub Actions failed to resolve LVGL (2026-10-13)

Failure in ESP-IDF Firmware Build run 38393411873: `Failed to resolve component 'lvgl' required by component 'launcher_ui': unknown name`. The WebAssembly preview succeeded because its independent FetchContent downloaded LVGL, but firmware did not declare a managed dependency.

Official fix: `apps/launcher/main/idf_component.yml` pins `lvgl/lvgl: 9.2.2` using IDF Component Manager. Additionally, individual projects now enumerate only their own `EXTRA_COMPONENT_DIRS`: other OTA apps no longer discover/compile the Launcher-only LVGL UI component. See https://docs.espressif.com/projects/idf-component-manager/en/latest/reference/manifest_file.html and https://lvgl.io/docs/open/9.2/integration/chip/espressif.html .

Check the new Actions run before marking this resolved.

## USB page compile failure (2026-10-13)
Run 38395348167 (Launcher) and 38395348193 (Web): `sd` / `group` undeclared in `launcher_ui.c` after partial UI refactor. Restored LVGL container and image declarations before use. Always inspect BOTH ESP-IDF and Web logs after changing shared UI.

## V2 custom key
Waveshare V2 factory `button_bsp.c` maps BOOT/custom key via `EXAMPLE_PIN_NUM_BOOT0` = GPIO0 (active low). Power button maps `EXAMPLE_PIN_NUM_SYS_OUT` = GPIO16. Added debounced GPIO0 single-click input to Launcher. Reset is hardware-only; don't treat it as the Back key. Hardware behavior still requires testing. Source: https://github.com/waveshareteam/ESP32-S3-Touch-LCD-3.49-V2/tree/main/ESP-IDF/11_FactoryProgram .

## 2026-10-13 first-flash reboot loop (PSRAM disabled)
Actual on-device serial log: `lcd_panel.axs15231b: send init commands success`, then `ESP_ERR_NO_MEM` from `board_support_init()`, and a reboot loop. Root cause: `apps/launcher/sdkconfig.defaults` did not enable external PSRAM, but `board_support.c` allocates **two 640×172 RGB565 framebuffers** (2×220160 bytes) with `MALLOC_CAP_SPIRAM`. `flash_id` detecting embedded 8 MB PSRAM does NOT mean ESP-IDF enabled it.

Fix: explicitly set `CONFIG_SPIRAM=y`, `CONFIG_SPIRAM_MODE_OCT=y`, and `CONFIG_SPIRAM_USE_MALLOC=y` in launcher defaults; log free/largest PSRAM prior to allocations. For an existing local `sdkconfig`, **defaults do not override previous settings**: run `idf.py reconfigure` and confirm `grep '^CONFIG_SPIRAM' sdkconfig`; if `# CONFIG_SPIRAM is not set` persists, back up or remove the **generated local sdkconfig** and run `idf.py build` so defaults regenerate it. Never remove project `sdkconfig.defaults`.

Boot log before fix confirms app partition at 0x20000 and successful LCD init commands; display/touch and DMA remain unverified, so verify after next flash.

## 2026-10-13 USB eject and physical Back fix
Real-device test: screen/touch, 8MB PSRAM, Mac Finder SD mount succeeded. When TinyUSB MSC takes the native USB peripheral, the USB-Serial/JTAG monitor disconnects (`Device not configured`). That does not by itself indicate an ESP32 reboot.

The cause of Back remaining blocked after Finder eject was that MSC state never changed from EXPORTED to SAFE_TO_EXIT. Espressif esp_tinyusb 2.x handles host SCSI START STOP UNIT eject by transferring storage ownership from USB to APP. Subscribe to `tinyusb_msc_set_storage_callback`: only `TINYUSB_MSC_EVENT_MOUNT_COMPLETE` with `TINYUSB_MSC_STORAGE_MOUNT_APP` marks SAFE_TO_EXIT. On mount failure keep USB page locked. GPIO0 Back can now clean up after that event; navigate only on ESP_OK. Check with an expendable TF card and monitor re-enumeration on macOS. If the SD FAT cannot be APP-mounted after eject, the UI is intentionally blocked until a proper recovery path is added.
