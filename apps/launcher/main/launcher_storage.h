#pragma once
#include "esp_err.h"
#include "usb_storage.h"
// USB is a utility page inside Launcher, NOT another app/OTA partition.
esp_err_t launcher_storage_open(void);
esp_err_t launcher_storage_close(void);
