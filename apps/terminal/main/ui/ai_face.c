#include "ai_face.h"
#include "lvgl_kawaii_face.h"
static lv_obj_t *root;
static bool running;
static void no_lock(void){}
void terminal_ai_face_hide(void){
 if(running){face_animation_deinit();running=false;}
 if(root){lv_obj_delete(root);root=NULL;}
}
void terminal_ai_face_show(void){
 if(root)return;
 root=lv_obj_create(lv_screen_active());lv_obj_remove_style_all(root);
 lv_obj_set_size(root,640,172);lv_obj_set_style_bg_color(root,lv_color_hex(0),0);
 lv_obj_set_style_bg_opa(root,LV_OPA_COVER,0);
 face_set_lvgl_lock_fns(no_lock,no_lock);
 face_config_t cfg={.parent=root,.animation_speed=50,.blink_interval=3000,.auto_blink=true};
 if(face_animation_init(&cfg)==ESP_OK){running=true;face_set_emotion(FACE_NEUTRAL,false);}
}
void terminal_ai_face_emotion(int index){
 if(running&&index>=0&&index<17)face_set_emotion((face_emotion_t)index,true);
}
