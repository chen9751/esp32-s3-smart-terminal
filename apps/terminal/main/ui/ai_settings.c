/* UI-only: visual style derived from the old project's bottom-tab settings.
 * No old network/audio/Bluetooth business services are imported. */
#include "ai_settings.h"
#include "ai_face.h"
#include "lvgl.h"
#include "esp_timer.h"
#include <stdint.h>
static lv_obj_t *root,*content,*tabs[6];
static lv_timer_t *idle_timer;
static int tab;
static int64_t active_at;
static const char *names[]={"SOUND","DISPLAY","VOICE","CHILD","FACE","SYSTEM"};
static const char *descs[]={"Volume / voice","Brightness / animation","PTT / realtime","Age / language","17 emotions / blink","Network / key status"};
static void activity(lv_event_t *e){(void)e;active_at=esp_timer_get_time();}
static void select_tab(lv_event_t *e);
static void redraw(void){
 if(content)lv_obj_delete(content);
 content=lv_obj_create(root);lv_obj_remove_style_all(content);
 lv_obj_set_size(content,584,112);lv_obj_set_pos(content,28,5);
 lv_obj_t *h=lv_label_create(content);lv_label_set_text(h,names[tab]);lv_obj_set_pos(h,12,4);
 lv_obj_set_style_text_color(h,lv_color_hex(0x78DCE8),0);
 lv_obj_t *d=lv_label_create(content);lv_label_set_text(d,descs[tab]);lv_obj_set_pos(d,12,40);
 lv_obj_set_style_text_color(d,lv_color_white(),0);
 for(int i=0;i<6;i++)lv_obj_set_style_bg_color(tabs[i],lv_color_hex(i==tab?0x26343A:0x0C0E11),0);
}
static void select_tab(lv_event_t *e){tab=(int)(intptr_t)lv_event_get_user_data(e);activity(e);redraw();}
static void timeout(lv_timer_t *t){(void)t;if(root&&esp_timer_get_time()-active_at>=10000000LL)terminal_ai_settings_hide();}
bool terminal_ai_settings_visible(void){return root!=NULL;}
void terminal_ai_settings_hide(void){
 if(idle_timer){lv_timer_delete(idle_timer);idle_timer=NULL;}
 if(root){lv_obj_delete(root);root=NULL;content=NULL;}
 terminal_ai_face_show();
}
void terminal_ai_settings_show(void){
 if(root){active_at=esp_timer_get_time();return;}
 terminal_ai_face_hide();
 root=lv_obj_create(lv_screen_active());lv_obj_remove_style_all(root);
 lv_obj_set_size(root,640,172);lv_obj_set_style_bg_color(root,lv_color_hex(0),0);
 lv_obj_set_style_bg_opa(root,LV_OPA_COVER,0);lv_obj_add_flag(root,LV_OBJ_FLAG_CLICKABLE);
 lv_obj_add_event_cb(root,activity,LV_EVENT_PRESSED,NULL);
 for(int i=0;i<6;i++){
  tabs[i]=lv_button_create(root);lv_obj_set_pos(tabs[i],i*106,124);lv_obj_set_size(tabs[i],i==5?110:106,48);
  lv_obj_set_style_radius(tabs[i],0,0);lv_obj_set_style_border_width(tabs[i],0,0);
  lv_obj_add_event_cb(tabs[i],select_tab,LV_EVENT_CLICKED,(void*)(intptr_t)i);
  lv_obj_t *label=lv_label_create(tabs[i]);lv_label_set_text(label,names[i]);
  lv_obj_set_style_text_font(label,&lv_font_montserrat_14,0);
  lv_obj_set_style_text_color(label,lv_color_hex(0x78DCE8),0);lv_obj_center(label);
 }
 tab=0;redraw();active_at=esp_timer_get_time();
 idle_timer=lv_timer_create(timeout,250,NULL);
}
