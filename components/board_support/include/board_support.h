#pragma once
#include "esp_err.h"
#define BOARD_BOOT_BRIGHTNESS_PERCENT 20
#define BOARD_BOOT_VOLUME_PERCENT 90
#include "lvgl.h"
// Initializes V2 display controller, expander, capacitive touch and LVGL.
// Caller must call board_support_loop() periodically from ONE task.
esp_err_t board_support_init(void);
void board_support_loop(void);
bool board_support_ready(void);

// Gamma-independent visible brightness range calibrated in the previous V2 project.
esp_err_t board_backlight_set_percent(uint8_t percent);
uint8_t board_backlight_get_percent(void);
