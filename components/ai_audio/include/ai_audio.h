#pragma once
#include <stdint.h>
#include "esp_err.h"
esp_err_t ai_audio_init(void);
esp_err_t ai_audio_set_volume(uint8_t volume);
uint8_t ai_audio_get_volume(void);
