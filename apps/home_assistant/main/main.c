#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "board_support.h"
#include "mode_manager.h"
#include "ui/clock_page.h"
#include "ui/device_page.h"
#include "ui/listening_page.h"
#include "ui/result_page.h"
static const char *TAG = "home_assistant";
void app_main(void) {
    ESP_LOGI(TAG, "Starting Home Assistant: ota_1 (phase-0 scaffold)");
    ESP_ERROR_CHECK(board_support_init());
    ESP_ERROR_CHECK(mode_manager_mark_valid_if_pending());
    clock_page_show();
    ESP_LOGW(TAG, "Display/touch/navigation are not yet wired");
    while (1) vTaskDelay(pdMS_TO_TICKS(1000));
}
