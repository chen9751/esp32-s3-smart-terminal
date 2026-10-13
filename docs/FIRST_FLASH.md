# First flash — standalone Launcher (test-only)
Official Waveshare V2 `ESP-IDF/10_LVGL_V9_Test` electrical mapping, independently implemented using Espressif drivers AXS15231B and TCA9554.

Initial test profile: 640×172 four-card UI, GPIO0 Back, 10-second HA auto-switch and OTA selection disabled, USB experimental. Display uses PSRAM full RGB565 buffer, 16-row internal DMA staging with on-color-trans-done semaphore. No old project code.

Do **not** erase flash or use generic `idf.py erase-flash` for this test. Build and flash only after GitHub Actions reports success and sources reviewed:
```sh
export IDF_PYTHON_ENV_PATH="$HOME/.espressif/python_env/idf5.5_py3.9_env"
. "$HOME/esp/esp-idf/export.sh"
git clone https://github.com/chen9751/esp32-s3-smart-terminal.git
cd esp32-s3-smart-terminal/apps/launcher
idf.py build
idf.py -p /dev/cu.usbmodem201401 flash monitor
```
Physical touch orientation, LCD signal integrity, USB MSC, and GPIO0 behavior remain **unverified** until first flash. Capture full boot log, any panic, PSRAM sizes, screen/keys and return behavior.
