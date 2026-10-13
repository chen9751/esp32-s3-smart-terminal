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
