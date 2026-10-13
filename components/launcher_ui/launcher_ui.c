#include "launcher_ui.h"
#include "launcher_icons.h"
#include "terminal_theme.h"
#include <stdbool.h>
#include <stdint.h>

static launcher_action_cb_t callback;
static void *callback_ctx;
static lv_obj_t *home_page, *usb_page;
static bool usb_visible;

static void emit(launcher_action_t action) {
    if (callback) callback(action, callback_ctx);
}
static void home_event(lv_event_t *e) {
    launcher_action_t action=(launcher_action_t)(uintptr_t)lv_event_get_user_data(e);
    if (action == LAUNCHER_ACTION_USB) {
        usb_visible=true;
        lv_screen_load(usb_page);
    }
    emit(action);
}
static lv_obj_t *screen(void) {
    lv_obj_t *s=lv_obj_create(NULL);
    lv_obj_remove_flag(s,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(s,0,0);
    lv_obj_set_style_bg_color(s,lv_color_hex(TERMINAL_COLOR_BACKGROUND),0);
    lv_obj_set_style_border_width(s,0,0);
    return s;
}
static void icon(lv_obj_t *parent,const lv_image_dsc_t *src) {
    lv_obj_t *i=lv_image_create(parent);
    lv_image_set_src(i,src);
    lv_obj_set_style_image_recolor(i,lv_color_hex(TERMINAL_COLOR_PRIMARY),0);
    lv_obj_set_style_image_recolor_opa(i,LV_OPA_COVER,0);
    lv_obj_center(i);
}
static void tile(lv_obj_t *parent,int x,const lv_image_dsc_t *src, launcher_action_t action) {
    lv_obj_t *b=lv_button_create(parent);
    lv_obj_set_pos(b,x,22);
    lv_obj_set_size(b,128,128);
    lv_obj_set_style_radius(b,22,0);
    lv_obj_set_style_bg_color(b,lv_color_hex(TERMINAL_COLOR_BACKGROUND),0);
    lv_obj_set_style_border_width(b,3,0);
    lv_obj_set_style_border_color(b,lv_color_hex(TERMINAL_COLOR_PRIMARY),0);
    lv_obj_set_style_shadow_width(b,0,0);
    lv_obj_set_style_bg_color(b,lv_color_hex(TERMINAL_COLOR_PRESSED),LV_STATE_PRESSED);
    lv_obj_add_event_cb(b,home_event,LV_EVENT_CLICKED,(void*)(uintptr_t)action);
    icon(b,src);
}
void launcher_ui_back(void) {
    // Controller decides if leaving USB is safe. Do not navigate before ack.
    emit(LAUNCHER_ACTION_BACK);
}
void launcher_ui_confirm_usb_exit(void) {
    if (usb_visible) {
        usb_visible = false;
        lv_screen_load(home_page);
    }
}
bool launcher_ui_usb_visible(void) { return usb_visible; }
void launcher_ui_create(launcher_action_cb_t cb,void *ctx) {
    callback=cb;callback_ctx=ctx;usb_visible=false;
    home_page=screen();
    tile(home_page,28,&launcher_icon_robot,LAUNCHER_ACTION_AI);
    tile(home_page,180,&launcher_icon_home,LAUNCHER_ACTION_HA);
    tile(home_page,332,&launcher_icon_remote,LAUNCHER_ACTION_REMOTE);
    tile(home_page,484,&launcher_icon_sd,LAUNCHER_ACTION_USB);
    usb_page=screen();
    // Center the icon and copy together, without duplicate declarations.
    lv_obj_t *group = lv_obj_create(usb_page);
    lv_obj_set_size(group, 510, 110);
    lv_obj_center(group);
    lv_obj_set_style_bg_opa(group, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(group, 0, 0);
    lv_obj_set_style_pad_all(group, 0, 0);
    lv_obj_remove_flag(group, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *sd = lv_image_create(group);
    lv_image_set_src(sd,&launcher_icon_sd_large);
    lv_obj_set_style_image_recolor(sd,lv_color_hex(TERMINAL_COLOR_PRIMARY),0);
    lv_obj_set_style_image_recolor_opa(sd,LV_OPA_COVER,0);
    lv_obj_align(sd,LV_ALIGN_LEFT_MID,0,0);
    lv_obj_t *message=lv_label_create(group);
    lv_obj_set_width(message,402);
    lv_label_set_long_mode(message,LV_LABEL_LONG_WRAP);
    lv_label_set_text(message,"Connect this device to your computer\nwith a USB cable to manage TF card files.\nEject the disk before returning.");
    lv_obj_set_style_text_color(message,lv_color_hex(TERMINAL_COLOR_TEXT),0);
    lv_obj_set_style_text_font(message,&lv_font_montserrat_14,0);
    lv_obj_align(message,LV_ALIGN_LEFT_MID,108,0);
    lv_screen_load(home_page);
}
