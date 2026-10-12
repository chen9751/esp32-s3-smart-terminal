#include "preview_home.h"
#include "lvgl.h"
// Browser-only placeholder. Real Launcher LVGL page will replace/share this
// module during UI design. No claim of hardware-equivalent navigation here.
static void show_message(lv_event_t *e) {
    const char *name = (const char *)lv_event_get_user_data(e);
    lv_obj_t *message = lv_obj_get_child(lv_screen_active(), -1);
    lv_label_set_text(message, name);
}
void preview_home_create(void) {
    lv_obj_t *root = lv_screen_active();
    lv_obj_set_style_bg_color(root, lv_color_hex(0x101820), 0);
    const char *labels[] = {"AI CHAT", "HOME ASSISTANT", "SONY REMOTE"};
    for (int i = 0; i < 3; ++i) {
        lv_obj_t *btn = lv_button_create(root);
        lv_obj_set_size(btn, 184, 85);
        lv_obj_set_pos(btn, 16 + i * 211, 34);
        lv_obj_add_event_cb(btn, show_message, LV_EVENT_CLICKED, (void *)labels[i]);
        lv_obj_t *text = lv_label_create(btn);
        lv_label_set_text(text, labels[i]);
        lv_obj_center(text);
    }
    lv_obj_t *msg = lv_label_create(root);
    lv_label_set_text(msg, "UI scaffold | preview only - no real firmware switching");
    lv_obj_set_style_text_color(msg, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(msg, LV_ALIGN_BOTTOM_MID, 0, -8);
}
