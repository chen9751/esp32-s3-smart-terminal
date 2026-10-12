#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "mode_manager.h"
#define LAUNCHER_AUTO_HA_TIMEOUT_MS 10000U
void launcher_policy_start(void);
void launcher_policy_notify_activity(void);
uint32_t launcher_policy_remaining_seconds(void);
bool launcher_policy_auto_enabled(void);
esp_err_t launcher_policy_select_mode(terminal_mode_t mode);
esp_err_t launcher_policy_process(void);
