#pragma once
#include "esp_err.h"
typedef enum { MODE_LAUNCHER, MODE_AI_CHAT, MODE_HOME_ASSISTANT, MODE_SONY_REMOTE } terminal_mode_t;
const char *mode_manager_label(terminal_mode_t mode);
esp_err_t mode_manager_select_and_restart(terminal_mode_t mode);
esp_err_t mode_manager_arm_launcher_on_next_reset(void);
esp_err_t mode_manager_mark_valid_if_pending(void);
