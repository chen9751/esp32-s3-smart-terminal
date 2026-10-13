#include "app_manager.h"
#include "ui/ai_face.h"
#include "ui/ai_settings.h"
#include "ui/mode_placeholder.h"
#include "esp_log.h"
static terminal_mode_t current=TERMINAL_LAUNCHER;
static const char *TAG="app_manager";
static void selection(launcher_action_t action,void *unused){(void)unused;app_manager_select(action);}
void app_manager_start(void){
 current=TERMINAL_LAUNCHER;launcher_ui_create(selection,NULL);
}
terminal_mode_t app_manager_mode(void){return current;}
void app_manager_select(launcher_action_t action){
 if(current!=TERMINAL_LAUNCHER)return;
 if(action==LAUNCHER_ACTION_AI)current=TERMINAL_AI;
 else if(action==LAUNCHER_ACTION_HA)current=TERMINAL_HA;
 else if(action==LAUNCHER_ACTION_REMOTE)current=TERMINAL_SONY;
 else return; /* USB handled as Launcher utility in later milestone. */
 ESP_LOGI(TAG,"Mode locked %d; RESET to Launcher",(int)current);
 /* Clear Launcher screen on entering mode. Preserve LVGL display & touch. */
 lv_obj_t *s=lv_screen_active();lv_obj_clean(s);
 if(current==TERMINAL_AI)terminal_ai_face_show();
 else placeholder_show(current==TERMINAL_HA?PLACEHOLDER_HA:PLACEHOLDER_SONY);
}
void app_manager_back(void){
 if(current==TERMINAL_AI&&terminal_ai_settings_visible())terminal_ai_settings_hide();
 else if(current==TERMINAL_LAUNCHER)launcher_ui_back();
}
