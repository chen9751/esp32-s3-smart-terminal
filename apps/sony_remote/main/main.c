#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "board_support.h"
#include "mode_manager.h"
#include "ui/remote_page.h"
#include "ui/bluetooth_page.h"
static const char *TAG = "sony_remote";
void app_main(void) {
    ESP_LOGI(TAG, "Starting Sony Remote: ota_2 (phase-0 scaffold)");
    ESP_ERROR_CHECK(board_support_init());
    ESP_ERROR_CHECK(mode_manager_mark_valid_if_pending());
    // Current mode runs normally, but NEXT power-on/reset starts Launcher.
    ESP_ERROR_CHECK(mode_manager_arm_launcher_on_next_reset());
    remote_page_show();
    ESP_LOGW(TAG, "Display/touch/navigation are not yet wired");
    while (1) vTaskDelay(pdMS_TO_TICKS(1000));
}
