#pragma once
#include "esp_err.h"
#include "lvgl.h"
// Initializes V2 display controller, expander, capacitive touch and LVGL.
// Caller must call board_support_loop() periodically from ONE task.
esp_err_t board_support_init(void);
void board_support_loop(void);
bool board_support_ready(void);
