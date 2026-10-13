#pragma once
#include "lvgl.h"
typedef enum {
    LAUNCHER_ACTION_AI,
    LAUNCHER_ACTION_HA,
    LAUNCHER_ACTION_REMOTE,
    LAUNCHER_ACTION_USB,
    LAUNCHER_ACTION_BACK
} launcher_action_t;
typedef void (*launcher_action_cb_t)(launcher_action_t action, void *ctx);
// Call only with an initialized LVGL display. Every UI operation is on LVGL thread.
void launcher_ui_create(launcher_action_cb_t cb, void *ctx);
void launcher_ui_back(void); // bind custom physical key single-click globally
bool launcher_ui_usb_visible(void);

// Call only after USB shutdown and storage checks return ESP_OK.
void launcher_ui_confirm_usb_exit(void);
