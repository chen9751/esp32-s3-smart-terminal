#include "app_manager.h"
#include "ui/ai_face.h"
#include "ui/ai_settings.h"
#include "ui/mode_placeholder.h"
#include "esp_log.h"
#include "usb_storage.h"
#include "network_service.h"
#include "ai_audio.h"
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
 else if(action==LAUNCHER_ACTION_USB){
  esp_err_t e=usb_storage_enter();
  if(e!=ESP_OK)ESP_LOGE(TAG,"USB MSC enter failed: %s",esp_err_to_name(e));
  return;
 } else return;
 ESP_LOGI(TAG,"Mode locked %d; RESET to Launcher",(int)current);
 /* Clear Launcher screen on entering mode. Preserve LVGL display & touch. */
 lv_obj_t *s=lv_obj_create(NULL);
 lv_screen_load(s);
 launcher_ui_destroy();
 if(current==TERMINAL_AI){
   terminal_ai_face_show();
   /* Do not activate these services in Launcher/HA/Sony. */
   esp_err_t n=network_service_init();
   if(n!=ESP_OK)ESP_LOGW(TAG,"AI WiFi: %s",esp_err_to_name(n));
   esp_err_t a=ai_audio_init();
   if(a!=ESP_OK)ESP_LOGW(TAG,"AI audio: %s",esp_err_to_name(a));
 }
 else placeholder_show(current==TERMINAL_HA?PLACEHOLDER_HA:PLACEHOLDER_SONY);
}
void app_manager_back(void){
 if(current==TERMINAL_AI&&terminal_ai_settings_visible())terminal_ai_settings_hide();
 else if(current==TERMINAL_LAUNCHER){
  if(launcher_ui_usb_visible()){
   esp_err_t e=usb_storage_exit();
   if(e==ESP_OK)launcher_ui_confirm_usb_exit();
   else ESP_LOGW(TAG,"USB eject required before exit: %s",esp_err_to_name(e));
  }
 }
}
