# First flash — standalone Launcher (test-only)
Source from Waveshare **V2** `ESP-IDF/10_LVGL_V9_Test` hardware example and Espressif official AXS15231B + TCA9554 components; this repo independently implements board support.

**IMPORTANT: First bringup incomplete until LCD flush DMA completion is made safe.** Current `board_support.c` flush uses an asynchronous SPI transaction while reusing its DMA buffer. This can corrupt video or DMA, so **do not flash this candidate yet** until the semaphore fix is merged and CI succeeds.

Test profile: 640×172 UI with four black/blue cards; GPIO0 short-click Back; 10-second mode-switch and OTA selection disabled; USB MSC experimental and requires Mac eject; no old project assets. Never run full-flash erase unless explicitly requested.

After safe-build approval, in a Mac terminal with IDF_PYTHON_ENV_PATH:
```sh
git clone https://github.com/chen9751/esp32-s3-smart-terminal.git
cd esp32-s3-smart-terminal/apps/launcher
idf.py build
idf.py -p /dev/cu.usbmodem201401 flash monitor
```
