#include "custom_key.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#define CUSTOM_GPIO GPIO_NUM_0
#define DEBOUNCE_US 30000LL
#define SINGLE_MIN_US 40000LL
#define LONG_US 800000LL
static bool stable_pressed, sample_pressed;
static int64_t transition_at, pressed_at;
esp_err_t custom_key_init(void) {
    gpio_config_t c = {
        .pin_bit_mask = (1ULL << CUSTOM_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t err = gpio_config(&c);
    if (err != ESP_OK) return err;
    sample_pressed = stable_pressed = (gpio_get_level(CUSTOM_GPIO) == 0);
    transition_at = pressed_at = esp_timer_get_time();
    return ESP_OK;
}
bool custom_key_poll_click(void) {
    int64_t now = esp_timer_get_time();
    bool current = (gpio_get_level(CUSTOM_GPIO) == 0);
    if (current != sample_pressed) {
        sample_pressed = current;
        transition_at = now;
    }
    if (stable_pressed == sample_pressed || now-transition_at < DEBOUNCE_US) return false;
    stable_pressed = sample_pressed;
    if (stable_pressed) {
        pressed_at = now;
        return false;
    }
    int64_t duration = now - pressed_at;
    return duration >= SINGLE_MIN_US && duration < LONG_US;
}
