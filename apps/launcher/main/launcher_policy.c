#include "launcher_policy.h"
#include "esp_timer.h"
#include "esp_log.h"
static const char *TAG = "launcher_policy";
static int64_t deadline_us;
static bool running;
static bool canceled;
void launcher_policy_start(void) {
    deadline_us = esp_timer_get_time() + (int64_t)LAUNCHER_AUTO_HA_TIMEOUT_MS * 1000;
    running = true; canceled = false;
    ESP_LOGI(TAG, "Starting 10 second inactivity timer; HA is permanent default");
}
void launcher_policy_notify_activity(void) {
    if (running && !canceled) {
        canceled = true;
        ESP_LOGI(TAG, "Interaction cancels this auto-launch; no countdown restart");
    }
}
bool launcher_policy_auto_enabled(void) { return running && !canceled; }
uint32_t launcher_policy_remaining_seconds(void) {
    if (!launcher_policy_auto_enabled()) return 0;
    int64_t remaining = deadline_us - esp_timer_get_time();
    return remaining > 0 ? (uint32_t)((remaining + 999999) / 1000000) : 0;
}
esp_err_t launcher_policy_select_mode(terminal_mode_t mode) {
    launcher_policy_notify_activity();
    return mode_manager_select_and_restart(mode);
}
esp_err_t launcher_policy_process(void) {
    if (!launcher_policy_auto_enabled() || esp_timer_get_time() < deadline_us) return ESP_OK;
    running = false; // only one automatic attempt: no endless retry if HA missing
    ESP_LOGI(TAG, "No interaction for 10 seconds; switch to HA");
    esp_err_t err = mode_manager_select_and_restart(MODE_HOME_ASSISTANT);
    if (err != ESP_OK) ESP_LOGE(TAG, "HA launch failed: %s; stay in Launcher", esp_err_to_name(err));
    return err;
}
