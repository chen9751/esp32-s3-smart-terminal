#include "face_page.h"
#include "lvgl_kawaii_face.h"
static lv_obj_t *root;static bool active;static void noop(void){}
void face_page_hide(void){if(active){face_animation_deinit();active=false;}if(root){lv_obj_delete(root);root=NULL;}}
void face_page_show(void){if(root)return;root=lv_obj_create(lv_screen_active());lv_obj_remove_style_all(root);lv_obj_set_size(root,640,172);lv_obj_set_style_bg_color(root,lv_color_hex(0),0);lv_obj_set_style_bg_opa(root,LV_OPA_COVER,0);face_set_lvgl_lock_fns(noop,noop);face_config_t c={.parent=root,.animation_speed=50,.blink_interval=3000,.auto_blink=true};if(face_animation_init(&c)==ESP_OK){active=true;face_set_emotion(FACE_NEUTRAL,false);}}
void face_page_preview_emotion(int n){if(active&&n>=0&&n<17)face_set_emotion((face_emotion_t)n,true);}
