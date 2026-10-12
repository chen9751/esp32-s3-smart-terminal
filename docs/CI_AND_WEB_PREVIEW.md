# GitHub Actions CI and Web UI preview

Migrated technical approach from old project [esp32-ai-robot](https://github.com/chen9751/esp32-ai-robot):
ESP-IDF **v5.5.4** build action; **LVGL v9.2.2** + Emscripten **4.0.15** + SDL2 WebAssembly hosted on GitHub Pages. Old project UI/fonts/code are **not** copied, since this is a new 3-mode system.

## Firmware CI

[ESP-IDF Firmware Build](../.github/workflows/firmware-build.yml) compiles four **independent projects**: launcher (factory), ai_chat (ota_0), home_assistant (ota_1), sony_remote (ota_2). Each build uploads its own .bin, bootloader, partition table and flasher_args.json, and checks the application .bin size against the configured slot. No automatic flash/OTA or hardware test is performed.

Actions -> ESP-IDF Firmware Build -> Run workflow. Downloads appear under the run's Artifacts.

## WebAssembly preview

[UI Web Preview](../.github/workflows/ui-web-preview.yml) compiles a **browser-only placeholder** at 640×172 from `web_preview/`. It uses real LVGL 9.2.2 and supports mouse/touch in the browser. It does not yet use the Launcher hardware UI sources: the Launcher page sources currently only print logs. **It is not a visual parity test yet** and it does not perform real boot partition switching or model interactions. The next UI development stage should move hardware-independent page code into a portable component shared by firmware and preview.

Proposed URL once Pages deployment succeeds:
https://chen9751.github.io/esp32-s3-smart-terminal/

### One-time GitHub setup needed

1. Repository -> **Settings -> Pages -> Build and deployment -> Source: GitHub Actions**.
2. Repository -> **Settings -> Actions -> General**: permit GitHub Actions execution. No secrets are required for public preview.
3. **Actions -> UI Web Preview -> Run workflow** once; check both build and deploy jobs succeed before using the URL.
4. **Actions -> ESP-IDF Firmware Build -> Run workflow** and verify all four matrix jobs succeeded before downloading their artifacts.

If permissions or Pages environment policy blocks the deployment, inspect the failing Action's log and GitHub repository Pages settings. Page publication may be delayed slightly after the job completes.

## Caveats

- CI workflow runs do **not** prove the physical display/touch/boot path.
- Current apps are log-only scaffolds and hardware drivers are unimplemented.
- During launcher visual work, replace browser-only preview mock with portable actual LVGL page implementation. Keep individual pages in separate files.
- Generated firmware is not ready for production flashing; the partition layout, bootloader, V2 drivers and selection flow still need physical verification.
