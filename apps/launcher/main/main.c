#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "board_support.h"
#include "mode_manager.h"
#include "launcher_policy.h"
#include "launcher_storage.h"
#include "launcher_ui.h"
#include "custom_key.h"
#include "usb_storage.h"

static const char *TAG="launcher";
static void on_action(launcher_action_t a, void *ctx) {
    (void)ctx;
    launcher_policy_notify_activity();
    if (a==LAUNCHER_ACTION_USB) {
        esp_err_t e=launcher_storage_open();
        ESP_LOGI(TAG,"USB enter: %s",esp_err_to_name(e));
    } else if(a==LAUNCHER_ACTION_BACK) {
        if(launcher_ui_usb_visible()) {
            esp_err_t e=launcher_storage_close();
            if(e==ESP_OK) {
                // ESP-IDF 5.5: esp_restart() does not reliably restore
                // the USB Serial/JTAG controller after USB-OTG TinyUSB.
                // Do not reset a successfully running Launcher for no benefit.
                launcher_ui_confirm_usb_exit();
                ESP_LOGI(TAG,"USB safely closed; Launcher active (native console recovery unsupported)");
            }
            else ESP_LOGW(TAG,"USB remains open: %s (eject on Mac first)",esp_err_to_name(e));
        }
    } else {
        ESP_LOGI(TAG,"First-flash test: mode %d deliberately disabled",a);
    }
}
void app_main(void) {
    ESP_LOGI(TAG,"V2 standalone Launcher bring-up; no OTA switching in test");
    ESP_LOGI(TAG,"Reset reason=%d", (int)esp_reset_reason());
    ESP_ERROR_CHECK(mode_manager_mark_valid_if_pending());
    ESP_ERROR_CHECK(custom_key_init());
    ESP_ERROR_CHECK(board_support_init());
    launcher_ui_create(on_action,NULL);
    ESP_LOGI(TAG,"Main stack free low-water=%u bytes",(unsigned)uxTaskGetStackHighWaterMark(NULL));
    ESP_LOGI(TAG,"Free internal=%u PSRAM=%u",heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
                                             heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    // First-flash test: NO 10s auto HA and NO firmware switching.
    TickType_t next_report=xTaskGetTickCount()+pdMS_TO_TICKS(10000);
    while(1) {
        if(custom_key_poll_click()) {
            ESP_LOGI(TAG, "GPIO0 Back: USB state=%d", usb_storage_get_state());
            launcher_ui_back();
        }
        board_support_loop();
        if(xTaskGetTickCount()>=next_report) {
            ESP_LOGI(TAG,"Main stack low-water=%u bytes",(unsigned)uxTaskGetStackHighWaterMark(NULL));
            next_report=xTaskGetTickCount()+pdMS_TO_TICKS(10000);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
