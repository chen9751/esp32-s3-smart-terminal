# Default power-on display and audio levels

For ESP32-S3-Touch-LCD-3.49-V2, hardware boot display brightness is **20%**. The V2 Waveshare reference configures GPIO42 LEDC PWM at 50kHz, 8-bit with an **inverted duty**: 255 = darkest, 0 = brightest. The current launcher initializes PWM duty 204 (20% brightness) before bringing up the LCD and enabling the TCA9554 backlight gate.

Default future audio playback volume is **90%** (`BOARD_BOOT_VOLUME_PERCENT`), stored as a shared board constant. The first-flash Launcher does **not** initialize an audio codec (it has no audio UI or playback), so this is a policy value, **not a claim of setting a live codec volume**. Future AI/HA apps must initialize their playback codec and apply the 90% volume using `esp_codec_dev_set_out_vol(..., BOARD_BOOT_VOLUME_PERCENT)` after successful codec init. Do not mistake this playback volume for microphone gain.

USB note: ESP32-S3 native USB MSC may not restore the USB Serial/JTAG console after exit. Software restart does not guarantee host console enumeration; use the RESET button and reopen the current `/dev/cu.usbmodem*` port before flashing if necessary. Don't use `brew uninstall --ignore-dependencies cairo`: it is needed by other installed Homebrew packages.
