#include "usb_storage.h"
#include "esp_log.h"
static const char *TAG = "usb_storage";
static usb_storage_state_t s_state = USB_STORAGE_IDLE;
usb_storage_state_t usb_storage_get_state(void) { return s_state; }
bool usb_storage_can_exit(void) { return s_state == USB_STORAGE_IDLE || s_state == USB_STORAGE_SAFE_TO_EXIT; }
esp_err_t usb_storage_enter(void) {
    // Fail closed: never advertise a writable MSC device before exclusive
    // SD ownership, V2 USB pins and host eject handling are implemented.
    ESP_LOGW(TAG, "USB MSC not yet implemented: waiting for verified V2 SD/USB drivers");
    s_state = USB_STORAGE_ERROR;
    return ESP_ERR_NOT_SUPPORTED;
}
esp_err_t usb_storage_exit(void) {
    if (s_state == USB_STORAGE_EXPORTED || s_state == USB_STORAGE_STARTING)
        return ESP_ERR_INVALID_STATE; // host must eject first
    s_state = USB_STORAGE_IDLE;
    return ESP_OK;
}
