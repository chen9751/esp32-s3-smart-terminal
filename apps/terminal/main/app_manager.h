#pragma once
#include "launcher_ui.h"
typedef enum {TERMINAL_LAUNCHER,TERMINAL_AI,TERMINAL_HA,TERMINAL_SONY} terminal_mode_t;
void app_manager_start(void);
void app_manager_select(launcher_action_t action);
void app_manager_back(void);
terminal_mode_t app_manager_mode(void);
