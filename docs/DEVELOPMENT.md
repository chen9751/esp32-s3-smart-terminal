# Getting started (phase 0)

ESP-IDF 5.5 target esp32s3; run commands in macOS terminal after `source ~/esp/esp-idf/export.sh`.

Build each project independently:

```sh
cd apps/launcher && idf.py set-target esp32s3 && idf.py build
cd ../ai_chat && idf.py set-target esp32s3 && idf.py build
cd ../home_assistant && idf.py set-target esp32s3 && idf.py build
cd ../sony_remote && idf.py set-target esp32s3 && idf.py build
```

**WARNING**: Never run `idf.py flash` from each project indiscriminately: default flash may overwrite the factory slot / partition table or erase previously flashed images. First verify each binary and flash offsets. From repo root, after compiling and confirming partition sizes, use esptool to write only the intended images:

```text
0x20000  -> apps/launcher/build/launcher.bin
0x160000 -> apps/ai_chat/build/ai_chat.bin
0x4E0000 -> apps/home_assistant/build/home_assistant.bin
0x860000 -> apps/sony_remote/build/sony_remote.bin
```

Flash bootloader and partition table only from one consistent project's build, verifying bootloader/table offsets from its generated flasher_args.json. When changing partition layout, update all applications together. The firmware is currently a console/log scaffold and not intended for user-facing flash/testing until verified board init and LVGL integration are added.

## Size checks

`idf.py size`, `idf.py size-components`, `idf.py partition-table`; inspect compiled `*.bin` bytes against slot capacity. Record actual outputs in this document when available.

## Phases

0. Shared partition layout, independent build projects, mode manager and page modules (this commit).
1. Bring-up official Waveshare V2 LCD/touch/audio/IMU drivers; LVGL + PSRAM; Launcher cards/settings UI.
2. Three independent image boot/switch tests, recovery entry.
3. AI face pages + audio stream to NUC11/cloud.
4. HA clock/device pages + WakeNet/model + execution feedback.
5. BLE HID pairing/remote UI/IMU mapping; Sony XR-55X91J tests.
6. OTA/update safety, credentials, power state, diagnostics.
