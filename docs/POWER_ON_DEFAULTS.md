# Boot defaults (Waveshare ESP32-S3-Touch-LCD-3.49-V2)

Display brightness **20%** is applied when Launcher initializes; volume **90%** is the shared playback policy for audio-capable modes.

## Brightness implementation

This firmware uses the proven brightness mapping from previous user project [chen9751/esp32-ai-robot](https://github.com/chen9751/esp32-ai-robot/blob/main/components/board/board.c), specifically `board_backlight_set_percent`. GPIO42 is active-low PWM at 50kHz, 8 bits. Low raw duty output had been below the V2 panel's visible threshold, which caused the newer firmware to appear black when 20% was naively converted to duty 204.

The old project's visible-range calibration maps positive user brightness to raw brightness **90..255**. Output duty = 255 - raw brightness. A requested 20% corresponds to ~PWM duty **133**. `0%` disables through the TCA9554 EXIO1 backlight gate.

LCD panel and PSRAM initialization and the TCA9554 battery SYS_EN latch are unchanged. After panel initialization configure LEDC off at duty255; after LVGL display setup set brightness to the user default 20% using a reusable setter. This is source parity with the old project, and **requires first physical verification on this new firmware**.

## Volume

The old project's [audio_service.c](https://github.com/chen9751/esp32-ai-robot/blob/main/components/audio/audio_service.c) uses `AUDIO_DEFAULT_VOLUME=90` and sets `esp_codec_dev_set_out_vol(playback,90)` after ES8311 initialization. The independent Launcher has no audio codec driver or playback service; it stores the same value in `BOARD_BOOT_VOLUME_PERCENT` but cannot apply volume on the hardware until AI/HA audio service creation. Do not initialize the mic or power amplifiers needlessly on launcher.
