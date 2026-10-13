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

## 2026-10-13: Mac serial monitor remains disconnected after safe USB eject
Hardware test with firmware 50bd0cd confirms proper security gating: GPIO0 Back is blocked until Finder eject, and returns to Launcher once Finder eject completes. But idf.py monitor never reconnects after USB MSC shutdown.

Espressif USB docs: ESP32-S3 USB OTG and fixed-function USB Serial/JTAG share the same internal PHY, and cannot both operate on it simultaneously. Additionally, an Espressif issue (espressif/esp-idf#15912) reports unreliable restoration of the Serial/JTAG function after TinyUSB uninstall in IDF 5.3.3+.

First-flash workaround: **only after host eject AND successful usb_storage_exit()**, navigate to Launcher and perform esp_restart(). The factory bootloader re-enters Launcher; the serial console can enumerate again. If MSC is still EXPORTED or uninstall fails, do not navigate/restart. Users may need to reopen the monitor on the re-enumerated Mac port; do not assume an existing idf.py monitor handle can always recover. Do not burn any USB PHY eFuses; no hardware changes.

References:
- https://docs.espressif.com/projects/esp-usb/en/latest/esp32s3/usb_device.html
- https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-guides/usb-serial-jtag-console.html
- https://github.com/espressif/esp-idf/issues/15912

## 2026-10-13 black screen after backlight default 20%
Device with commit 3c8beba started successfully and logged AXS15231B init, 8MB PSRAM allocation and LVGL startup, but screen went black around one second after restart and USB console disconnected/reconnected. No panic/backtrace in provided excerpt. Causality not confirmed. The only newly introduced display change was inverted GPIO42 LEDC duty (204 for 20% per official Waveshare V2 mapping). Diagnostic revision temporarily drives full brightness (duty=0) for two seconds **after LCD init**, then restores requested 20% (duty=204), with serial markers and reset reason. Compare visible behavior and capture full reset lines. Remove the two-second diagnostic after hardware verification. Do not assume that a USB monitor disconnection alone proves a panic or CPU reset.

## 2026-10-13 main task stack overflow (confirmed)
Firmware e9a54e5 repeatedly rebooted with `***ERROR*** A stack overflow in task main has been detected`, and a `vApplicationStackOverflowHook` backtrace. LCD and PSRAM initialized successfully. Actual cause is the default ESP-IDF main task stack being too small for LVGL rendering in `app_main`; backlight duty is not the crash reason. Waveshare V2 LVGL 9 sample uses a separate 8KB worker task. The first-flash Launcher sets `CONFIG_ESP_MAIN_TASK_STACK_SIZE=16384` and logs the main task stack low-water mark, removes the temporary full-brightness demonstration, and keeps 20% backlight. NOTE: existing local `sdkconfig` overrides `sdkconfig.defaults`; set the config with `idf.py menuconfig`, or rename the generated `sdkconfig` and rebuild before flashing. Confirm `grep '^CONFIG_ESP_MAIN_TASK_STACK_SIZE=' sdkconfig` outputs 16384.

## 2026-10-13 display black while firmware remains responsive
User confirmed firmware 0700bfd reaches 41 seconds, PSRAM is healthy, main task low-water 12884 bytes, touch triggers mode action. Do NOT diagnose as a restart loop. Regression began with GPIO42 LEDC PWM backlight implementation (20% target). Compare against Waveshare V2 official ESP-IDF/10_LVGL_V9_Test/components/lcd_bl_pwm_bsp: uses LEDC_TIMER_3, LEDC_CHANNEL_1, LEDC_LOW_SPEED_MODE, 8-bit PWM, 50kHz, **LEDC_SLOW_CLK_RC_FAST**. Prior code used LEDC_AUTO_CLK instead. First remedy: reproduce official clock choice, initialize PWM after LCD panel initialization while keeping TCA9554 backlight gate disabled, then enable gate. This is a source-level correction, not verified on hardware. The requested 20% maps to duty 204 for the factory example's inverted 8-bit mapping. If hardware still blacks out, obtain photo/LED current or test direct GPIO42 and panel backlight before further changes; do not claim proven root cause.

## 2026-10-13 native USB monitor loss after MSC (confirmed limitation)
After Finder eject and GPIO0 Back, the user confirmed the Launcher returns but /dev/cu.usbmodem serial console does not recover, including after esp_restart(). This follows the shared internal PHY on ESP32-S3: USB-OTG TinyUSB and fixed-function USB Serial/JTAG cannot both drive the same PHY. Official Espressif esp-idf issue #15912 (closed, root driver regression from 5.3.3) reproduces that tinyusb_driver_uninstall + usb_new_phy(SERIAL_JTAG) does not switch back on 5.3.3/5.4.1/5.5-dev. There is no validated 5.5.4 in-place software restoration for this board. Do NOT assume esp_restart() restores the port and do NOT write undocumented USB peripheral registers or eFuses. Remove that ineffective reboot from Launcher. Keep the safety gate for MSC eject and cleanup, leaving app running. For reflash, physically reset the board (or BOOT+RESET for ROM download), then identify /dev/cu.usbmodem* and run idf.py flash. Console persistence across MSC in a future version requires a composite TinyUSB CDC+MSC and corresponding debug/flashing redesign, or dedicated external USB-UART / PHY hardware; TinyUSB CDC is not equivalent to the ROM USB Serial/JTAG download port.
References: https://docs.espressif.com/projects/esp-usb/en/latest/esp32s3/usb_device.html ; https://github.com/espressif/esp-idf/issues/15912

## 单固件迁移记录（2026-10-14）

- **旧 AI 固件重启回 Launcher**：串口日志确认先跳转 `ota_0` 成功，随后 AI 的独立 `sdkconfig` 没有启用 OPI PSRAM，`board_support_init()` 因 `PSRAM=0`、`ESP_ERR_NO_MEM` 中止。单固件只维护 `apps/terminal/sdkconfig.defaults`，但仍需在新工程实际 `sdkconfig` 与启动日志核对 `CONFIG_SPIRAM`、8MiB PSRAM。
- **AI 设置页首次编译错误**：`lv_font_montserrat_12` 未启用，已改为现有 `lv_font_montserrat_14` 并删除未使用变量。
- **编译环境**：macOS 若 ESP-IDF 误选不存在的 Python 3.14 venv，先设置正确的 `IDF_PYTHON_ENV_PATH`（此前为 `$HOME/.espressif/python_env/idf5.5_py3.9_env`）再 source；Cairo 生成错误参见 `docs/LOCAL_BUILD.md`。
- **分区迁移**：旧设备 `factory=0x20000`、`ota_0=0x160000`；新单固件 factory 草案 `0x10000`，且无 OTA。必须等待新工程编译及分区验证通过、备份旧版以后才实施 USB 全量烧录，严禁把单应用镜像直接写进旧多固件 slot。
