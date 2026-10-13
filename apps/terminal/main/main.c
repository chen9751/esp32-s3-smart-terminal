#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "board_support.h"
#include "app_manager.h"
#include "ui/ai_settings.h"
#include "ui/terminal_key.h"
static const char *TAG="terminal";
void app_main(void){
 ESP_ERROR_CHECK(board_support_init());
 terminal_key_init();
 app_manager_start();
 ESP_LOGI(TAG,"Single factory image; no OTA; RESET returns to Launcher");
 while(1){
  bool short_key=false,long_key=false;
  terminal_key_poll(&short_key,&long_key);
  if(app_manager_mode()==TERMINAL_AI && long_key && !terminal_ai_settings_visible())
   terminal_ai_settings_show();
  if(short_key)app_manager_back();
  board_support_loop();
  vTaskDelay(pdMS_TO_TICKS(10));
 }
}
