#include "mode_manager.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_system.h"
#include "esp_log.h"
static const char *TAG = "mode_manager";
const char *mode_manager_label(terminal_mode_t mode) {
    switch (mode) {
    case MODE_LAUNCHER: return "factory";
    case MODE_AI_CHAT: return "ota_0";
    case MODE_HOME_ASSISTANT: return "ota_1";
    case MODE_SONY_REMOTE: return "ota_2";
    default: return NULL;
    }
}
esp_err_t mode_manager_select_and_restart(terminal_mode_t mode) {
    const char *label = mode_manager_label(mode);
    if (!label) return ESP_ERR_INVALID_ARG;
    const esp_partition_t *part = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, label);
    if (!part) return ESP_ERR_NOT_FOUND;
    esp_app_desc_t desc;
    esp_err_t err = esp_ota_get_partition_description(part, &desc);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Partition %s lacks valid app description: %s", label, esp_err_to_name(err));
        return err;
    }
    err = esp_ota_set_boot_partition(part);
    if (err != ESP_OK) return err;
    ESP_LOGI(TAG, "Next boot: %s (%s)", label, desc.version);
    esp_restart();
    return ESP_OK;
}
esp_err_t mode_manager_mark_valid_if_pending(void) {
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t state;
    if (esp_ota_get_state_partition(running, &state) == ESP_OK && state == ESP_OTA_IMG_PENDING_VERIFY) {
        return esp_ota_mark_app_valid_cancel_rollback();
    }
    return ESP_OK;
}
