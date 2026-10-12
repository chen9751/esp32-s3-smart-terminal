#include "launcher_storage.h"
#include "launcher_policy.h"
#include "ui/home_page.h"
#include "ui/usb_storage_page.h"
#include "esp_log.h"
static const char *TAG = "launcher_storage";
esp_err_t launcher_storage_open(void) {
    launcher_policy_notify_activity();
    usb_storage_page_show();
    esp_err_t err = usb_storage_enter();
    if (err != ESP_OK)
        ESP_LOGW(TAG, "USB storage unavailable: %s", esp_err_to_name(err));
    return err;
}
esp_err_t launcher_storage_close(void) {
    if (!usb_storage_can_exit()) return ESP_ERR_INVALID_STATE;
    esp_err_t err = usb_storage_exit();
    if (err == ESP_OK) home_page_show();
    return err;
}
