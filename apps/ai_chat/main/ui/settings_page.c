/* Adapted visual vocabulary from esp32-ai-robot/ui_page_settings.c.
 * No legacy network/audio/Bluetooth business code is imported. */
#include "settings_page.h"
#include "face_page.h"
#include "esp_timer.h"
#include <stdbool.h>
#include <stdint.h>
static lv_obj_t *root, *body, *tabs[6], *value;
static int selected;
static int64_t last_input;
static lv_timer_t *timeout_timer;
static const char *names[]={"SOUND","DISPLAY","VOICE","CHILD","FACE","SYSTEM"};
static const char *details[]={"Volume / voice preset","Brightness / animations","Push-to-talk / realtime","Age / response length","17 emotions / blink","Network / API status"};
#define CYAN 0x78DCE8
static void activity(lv_event_t *e){(void)e; last_input=esp_timer_get_time();}
void ai_settings_page_touch(void){last_input=esp_timer_get_time();}
static void populate(void){
 if(body)lv_obj_delete(body);
 body=lv_obj_create(root);lv_obj_remove_style_all(body);
 lv_obj_set_pos(body,28,8);lv_obj_set_size(body,584,108);
 lv_obj_t *title=lv_label_create(body);lv_label_set_text(title,names[selected]);
 lv_obj_set_style_text_color(title,lv_color_hex(CYAN),0);lv_obj_set_pos(title,12,3);
 lv_obj_t *desc=lv_label_create(body);lv_label_set_text(desc,details[selected]);
 lv_obj_set_style_text_color(desc,lv_color_white(),0);lv_obj_set_pos(desc,12,40);
 for(int i=0;i<6;i++)lv_obj_set_style_bg_color(tabs[i],lv_color_hex(i==selected?0x26343A:0x0C0E11),0);
}
static void choose(lv_event_t *e){
 selected=(int)(intptr_t)lv_event_get_user_data(e);
 activity(e);populate();
}
static void on_timeout(lv_timer_t *t){
 (void)t;
 if(root && esp_timer_get_time()-last_input>=10000000LL)ai_settings_page_hide();
}
bool ai_settings_page_visible(void){return root!=NULL;}
void ai_settings_page_hide(void){
 if(timeout_timer){lv_timer_delete(timeout_timer);timeout_timer=NULL;}
 if(root){lv_obj_delete(root);root=NULL;body=NULL;}
 face_page_show();
}
void ai_settings_page_show(void){
 if(root){ai_settings_page_touch();return;}
 face_page_hide();
 root=lv_obj_create(lv_screen_active());lv_obj_remove_style_all(root);
 lv_obj_set_size(root,640,172);lv_obj_set_style_bg_color(root,lv_color_hex(0),0);
 lv_obj_set_style_bg_opa(root,LV_OPA_COVER,0);lv_obj_add_flag(root,LV_OBJ_FLAG_CLICKABLE);
 lv_obj_add_event_cb(root,activity,LV_EVENT_PRESSED,NULL);
 for(int i=0;i<6;i++){
  tabs[i]=lv_button_create(root);lv_obj_set_pos(tabs[i],i*106,124);lv_obj_set_size(tabs[i],i==5?110:106,48);
  lv_obj_set_style_radius(tabs[i],0,0);lv_obj_set_style_border_width(tabs[i],0,0);
  lv_obj_add_event_cb(tabs[i],choose,LV_EVENT_CLICKED,(void*)(intptr_t)i);
  lv_obj_t *name=lv_label_create(tabs[i]);lv_label_set_text(name,names[i]);
  lv_obj_set_style_text_font(name,&lv_font_montserrat_12,0);
  lv_obj_set_style_text_color(name,lv_color_hex(CYAN),0);lv_obj_center(name);
 }
 selected=0;populate();ai_settings_page_touch();
 timeout_timer=lv_timer_create(on_timeout,250,NULL);
}
