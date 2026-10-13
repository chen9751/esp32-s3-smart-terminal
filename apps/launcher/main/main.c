#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "board_support.h"
#include "mode_manager.h"
#include "launcher_policy.h"
#include "launcher_storage.h"
#include "custom_key.h"
#include "ui/home_page.h"
#include "ui/settings_page.h"
static const char *TAG = "launcher";
void app_main(void) {
    ESP_LOGI(TAG, "Launcher: factory");
    ESP_ERROR_CHECK(board_support_init());
    ESP_ERROR_CHECK(mode_manager_mark_valid_if_pending());
    ESP_ERROR_CHECK(custom_key_init());
    home_page_show();
    // TODO: move this call to the first fully rendered LVGL page-ready event.
    launcher_policy_start();
    ESP_LOGW(TAG, "Touch driver absent: countdown cannot yet be canceled by a real touch");
    while (1) {
        if (custom_key_poll_click()) {
            launcher_policy_notify_activity();
            // No LVGL hardware page is initialized in this phase-0 scaffold.
            // USB exit remains gated by the storage ownership state.
            if (usb_storage_can_exit()) launcher_storage_close();
            else ESP_LOGW(TAG, "Back blocked: eject TF card from Mac first");
        }
        launcher_policy_process();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
