# Launcher boot policy

**Fixed requirement:** always choose HA automatically after 10 seconds without interaction on Launcher. Never auto-choose the last-used mode. Any actual touch/interaction during these 10 seconds cancels automatic selection for this Launcher session; user then chooses manually. GUI has not been implemented yet.

## Why boot target must be re-armed

ESP-IDF persists OTA app selection in `otadata`; without an explicit reset-to-factory action, selecting an app once makes subsequent cold boots launch it directly.

Implementation:
1. Factory Launcher is the standard boot target.
2. Launcher selects an OTA app via `esp_ota_set_boot_partition(ota_N)`, then calls `esp_restart()`.
3. Each OTA app, immediately after startup/verification, sets **factory as the NEXT boot partition**, without restarting. The current app continues running.
4. Next ordinary software reset or power cycle boots Launcher, which always starts a fresh 10-second inactivity timer.
5. Missing/invalid HA image -> switching API returns an error and Launcher remains running; no retry loop.

## Important limits (must resolve before hardware release)

- There is a brief interval after selecting a mode but before it re-arms factory. A power loss or crash in this interval may cause one extra boot into that mode. Therefore **this is not strict bootloader-enforced factory-first under all fault conditions**. A custom bootloader or rescue strategy would be needed for an absolute guarantee.
- The touch driver / LVGL input layer is not yet implemented. Once integrated, call `launcher_policy_notify_activity()` on the first genuine touch (even if it does not activate a menu item).
- Countdown currently starts when the placeholder `home_page_show()` returns, not when a physical screen is drawn. Move timer startup to the real LVGL page-ready callback.
- Unstable mode code that crashes before factory is re-armed can boot-loop. Design and test a recovery GPIO/bootloader feature only after checking official Waveshare V2 safe pins.
- If ESP-IDF OTA rollback is enabled, app-validation logic must be tested separately before production.

## API and acceptance tests

`launcher_policy_start()`: start fresh 10000ms deadline.
`launcher_policy_notify_activity()`: cancel this session's auto launch (no 10s restart).
`launcher_policy_remaining_seconds()`: rounded-up countdown for UI.
`launcher_policy_select_mode(MODE_*)`: manual select and reboot.
`launcher_policy_process()`: nonblocking periodic poll, HA after deadline.

Test: power-on shows Launcher; untouched -> HA at 10s after ready; any touch -> remains Launcher indefinitely; manual AI/Remote/HA works; normal power-cycle from any running mode -> Launcher; HA absent -> stay in Launcher.

ESP-IDF reference: https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/system/ota.html
