#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "board_support.h"
#include "mode_manager.h"
#include "ui/face_page.h"
#include "ui/settings_page.h"
#include "ui/ai_key.h"
void app_main(void){ESP_ERROR_CHECK(mode_manager_mark_valid_if_pending());ESP_ERROR_CHECK(mode_manager_arm_launcher_on_next_reset());ai_key_init();ESP_ERROR_CHECK(board_support_init());face_page_show();while(1){if(ai_key_poll_long()&&!ai_settings_page_visible())ai_settings_page_show();if(ai_key_poll_short()&&ai_settings_page_visible())ai_settings_page_hide();board_support_loop();vTaskDelay(pdMS_TO_TICKS(10));}}
