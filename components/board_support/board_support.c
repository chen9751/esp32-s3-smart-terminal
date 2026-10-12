#include "board_support.h"
#include "esp_log.h"
static const char *TAG = "board_support";
esp_err_t board_support_init(void) {
    ESP_LOGW(TAG, "V2 hardware drivers not integrated; console scaffold only");
    return ESP_OK;
}
