#include "ai_settings.h"
#include "ai_face.h"
#include "board_support.h"
#include "network_service.h"
#include "ai_audio.h"
#include "lvgl.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "nvs.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
static const char *TAG="ai_settings";
static lv_obj_t *root,*content,*tabs[6],*live;
static lv_timer_t *idle_timer;
static int tab,voice;
static int64_t active_at;
static const char *names[]={"SOUND","DISPLAY","VOICE","WI-FI","AI","SYSTEM"};
static const char *voices[]={"Liora Mira","Mia","Mione"};
static void activity(lv_event_t *e){(void)e;active_at=esp_timer_get_time();}
static lv_obj_t *label(lv_obj_t *parent,const char *value,int x,int y){
 lv_obj_t *o=lv_label_create(parent);lv_label_set_text(o,value);lv_obj_set_pos(o,x,y);
 lv_obj_set_style_text_color(o,lv_color_white(),0);return o;
}
static void store_voice(void){
 nvs_handle_t h;
 if(nvs_open("ai_pref",NVS_READWRITE,&h)==ESP_OK){
  if(nvs_set_u8(h,"voice",(uint8_t)voice)==ESP_OK)nvs_commit(h);
  nvs_close(h);
 }
}
const char *terminal_ai_settings_voice(void){return voices[voice];}
static void voice_click(lv_event_t *e){
 voice=(int)(intptr_t)lv_event_get_user_data(e);activity(e);store_voice();
 /* The voice selection applies to the NEXT Omni realtime session. */
 for(int i=0;i<3;i++){
  lv_obj_t *b=lv_obj_get_child(content,i+1);
  if(b)lv_obj_set_style_bg_color(b,lv_color_hex(i==voice?0x14576A:0x26343A),0);
 }
}
static void volume_changed(lv_event_t *e){
 uint8_t v=(uint8_t)lv_slider_get_value(lv_event_get_target(e));
 esp_err_t err=ai_audio_set_volume(v);
 if(err!=ESP_OK)ESP_LOGW(TAG,"volume: %s",esp_err_to_name(err));
 activity(e);
}
static void backlight_changed(lv_event_t *e){
 uint8_t v=(uint8_t)lv_slider_get_value(lv_event_get_target(e));
 esp_err_t err=board_backlight_set_percent(v);
 if(err!=ESP_OK)ESP_LOGW(TAG,"backlight: %s",esp_err_to_name(err));
 activity(e);
}
static lv_obj_t *slider(int value,lv_event_cb_t cb){
 lv_obj_t *s=lv_slider_create(content);lv_obj_set_pos(s,22,67);
 lv_obj_set_size(s,515,16);lv_slider_set_range(s,1,100);
 lv_slider_set_value(s,value,LV_ANIM_OFF);
 lv_obj_add_event_cb(s,cb,LV_EVENT_VALUE_CHANGED,NULL);
 return s;
}
static void live_refresh(void){
 if(!live)return;
 char buf[160];
 if(tab==3){
  network_wifi_status_t w={0};
  network_service_get_wifi_status(&w);
  if(w.connected)snprintf(buf,sizeof(buf),"SSID: %.32s  IP: %.15s",w.ssid,w.ip);
  else snprintf(buf,sizeof(buf),"SSID: %.32s  %s",w.ssid[0]?w.ssid:"--",network_service_config_status());
 } else if(tab==5){
  uint16_t mv=0;
  if(board_battery_read_mv(&mv)==ESP_OK)
   snprintf(buf,sizeof(buf),"Battery: %u.%03u V",(unsigned)(mv/1000),(unsigned)(mv%1000));
  else snprintf(buf,sizeof(buf),"Battery voltage unavailable");
 }else return;
 lv_label_set_text(live,buf);
}
static void redraw(void){
 if(content)lv_obj_delete(content);
 live=NULL;
 content=lv_obj_create(root);lv_obj_remove_style_all(content);
 lv_obj_set_size(content,584,112);lv_obj_set_pos(content,28,5);
 label(content,names[tab],12,4);
 if(tab==0){
  label(content,"Speaker volume",12,35);slider(ai_audio_get_volume(),volume_changed);
 }else if(tab==1){
  label(content,"LCD brightness",12,35);slider(board_backlight_get_percent(),backlight_changed);
 }else if(tab==2){
  label(content,"Qwen Omni voice (next session)",12,29);
  for(int i=0;i<3;i++){
   lv_obj_t *b=lv_button_create(content);lv_obj_set_size(b,168,43);lv_obj_set_pos(b,12+i*181,57);
   lv_obj_set_style_bg_color(b,lv_color_hex(i==voice?0x14576A:0x26343A),0);
   lv_obj_add_event_cb(b,voice_click,LV_EVENT_CLICKED,(void*)(intptr_t)i);
   lv_obj_t *t=label(b,voices[i],0,0);lv_obj_center(t);
  }
 }else if(tab==3){
  label(content,"TF /sdcard/config.json (read only)",12,34);
  live=label(content,"",12,69);live_refresh();
 }else if(tab==4){
  /* AI configuration is intentionally unchanged until the cloud phase. */
  label(content,"AI settings reserved for cloud integration",12,45);
 }else{
  label(content,"SYSTEM",12,34);live=label(content,"",12,68);live_refresh();
 }
 for(int i=0;i<6;i++)lv_obj_set_style_bg_color(tabs[i],lv_color_hex(i==tab?0x26343A:0x0C0E11),0);
}
static void select_tab(lv_event_t *e){tab=(int)(intptr_t)lv_event_get_user_data(e);activity(e);redraw();}
static void timeout(lv_timer_t *t){
 (void)t;if(!root)return;
 if(esp_timer_get_time()-active_at>=10000000LL){terminal_ai_settings_hide();return;}
 live_refresh();
}
bool terminal_ai_settings_visible(void){return root!=NULL;}
void terminal_ai_settings_hide(void){
 if(idle_timer){lv_timer_delete(idle_timer);idle_timer=NULL;}
 if(root){lv_obj_delete(root);root=NULL;content=NULL;live=NULL;}
 terminal_ai_face_show();
}
void terminal_ai_settings_show(void){
 if(root){active_at=esp_timer_get_time();return;}
 nvs_handle_t h;uint8_t saved=0;
 if(nvs_open("ai_pref",NVS_READONLY,&h)==ESP_OK){
  if(nvs_get_u8(h,"voice",&saved)==ESP_OK&&saved<3)voice=saved;
  nvs_close(h);
 }
 terminal_ai_face_hide();
 root=lv_obj_create(lv_screen_active());lv_obj_remove_style_all(root);
 lv_obj_set_size(root,640,172);lv_obj_set_style_bg_color(root,lv_color_hex(0),0);
 lv_obj_set_style_bg_opa(root,LV_OPA_COVER,0);lv_obj_add_flag(root,LV_OBJ_FLAG_CLICKABLE);
 lv_obj_add_event_cb(root,activity,LV_EVENT_PRESSED,NULL);
 for(int i=0;i<6;i++){
  tabs[i]=lv_button_create(root);lv_obj_set_pos(tabs[i],i*106,124);lv_obj_set_size(tabs[i],i==5?110:106,48);
  lv_obj_set_style_radius(tabs[i],0,0);lv_obj_set_style_border_width(tabs[i],0,0);
  lv_obj_add_event_cb(tabs[i],select_tab,LV_EVENT_CLICKED,(void*)(intptr_t)i);
  lv_obj_t *t=lv_label_create(tabs[i]);lv_label_set_text(t,names[i]);
  lv_obj_set_style_text_font(t,&lv_font_montserrat_14,0);
  lv_obj_set_style_text_color(t,lv_color_hex(0x78DCE8),0);lv_obj_center(t);
 }
 tab=0;redraw();active_at=esp_timer_get_time();
 idle_timer=lv_timer_create(timeout,500,NULL);
}
