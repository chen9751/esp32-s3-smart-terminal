#pragma once
#include <stdbool.h>
#include "esp_err.h"
// Waveshare V2 factory firmware: BOOT/custom button GPIO0, active low.
// GPIO16 is the power button and RESET is not a programmable user key.
esp_err_t custom_key_init(void);
// Poll every ~10ms; returns true only on a debounced single short click.
// No LVGL calls from this low-level input module.
bool custom_key_poll_click(void);
