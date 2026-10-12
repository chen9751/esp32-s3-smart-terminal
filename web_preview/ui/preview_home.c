#include "preview_home.h"
#include "lvgl.h"
// Browser-only placeholder. Final Launcher UI is not yet designed.
// Click targets are previews, not firmware switches or USB MSC activation.
static lv_obj_t *s_status;
static void selected(lv_event_t *event) {
    const char *label = (const char *)lv_event_get_user_data(event);
    lv_label_set_text(s_status, label);
}
static void add_button(lv_obj_t *root, const char *title,
                       const char *message, int x, int y, int w, int h) {
    lv_obj_t *button = lv_button_create(root);
    lv_obj_set_size(button, w, h);
    lv_obj_set_pos(button, x, y);
    lv_obj_add_event_cb(button, selected, LV_EVENT_CLICKED, (void *)message);
    lv_obj_t *label = lv_label_create(button);
    lv_label_set_text(label, title);
    lv_obj_center(label);
}
void preview_home_create(void) {
    lv_obj_t *root = lv_screen_active();
    lv_obj_set_style_bg_color(root, lv_color_hex(0x101820), 0);
    add_button(root, "AI CHAT", "AI selected (preview only)", 12, 28, 170, 90);
    add_button(root, "HOME ASSISTANT", "HA selected (preview only)", 192, 28, 170, 90);
    add_button(root, "SONY REMOTE", "Remote selected (preview only)", 372, 28, 170, 90);
    add_button(root, "USB", "USB storage not implemented; Mac disk not exposed", 552, 55, 76, 46);
    s_status = lv_label_create(root);
    lv_label_set_text(s_status, "3 app cards + USB utility | UI concept placeholder");
    lv_obj_set_style_text_color(s_status, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(s_status, LV_ALIGN_BOTTOM_MID, 0, -8);
}
