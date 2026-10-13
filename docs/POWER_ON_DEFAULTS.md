# Power-on defaults and validation

- Intended LCD brightness: **20%**, not yet validated on this physical board.
- Intended playback volume: **90%**, a shared policy constant; Launcher initializes no codec and cannot set actual audio volume.
- LCD bring-up temporarily uses the **last known-good TCA9554 backlight enable** sequence, matching firmware 50bd0cd where display and touch were observed working. GPIO42 LEDC PWM was introduced by 3c8beba and immediately correlated with black screens; changing LEDC source clock and init order did not resolve it. Therefore remove the entire speculative PWM initializer, rather than leaving it silently in the build. **Current firmware does NOT implement the 20% physical brightness target**.
- For future brightness control, isolate backlight hardware with a dedicated test build, confirm polarity and effective duty range on the actual V2 board, then reintroduce a reusable brightness setter tested at 100% -> 50% -> 20%. Avoid modifying LVGL, LCD QSPI or SD logic in that experiment.
- When returning from native USB MSC on ESP32-S3, the USB Serial/JTAG port may not automatically recover. Restart/reconnect/enter download mode as needed.
