#include "mode_placeholder.h"
#include "lvgl.h"
/* Independent UI boundaries; actual HA/remote pages are next UI milestone. */
void placeholder_show(placeholder_mode_t mode){
 lv_obj_t *s=lv_screen_active();
 lv_obj_clean(s);lv_obj_set_style_bg_color(s,lv_color_hex(0),0);
 lv_obj_t *title=lv_label_create(s);
 lv_label_set_text(title,mode==PLACEHOLDER_HA?"HOME ASSISTANT":"SONY REMOTE");
 lv_obj_set_style_text_color(title,lv_color_hex(0x78DCE8),0);
 lv_obj_center(title);
}
