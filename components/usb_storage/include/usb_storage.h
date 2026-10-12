#pragma once
#include <stdbool.h>
#include "esp_err.h"

typedef enum {
    USB_STORAGE_IDLE = 0,
    USB_STORAGE_STARTING,
    USB_STORAGE_EXPORTED,
    USB_STORAGE_SAFE_TO_EXIT,
    USB_STORAGE_ERROR
} usb_storage_state_t;

// Launcher-only service. Intended contract:
// 1. ensure exclusive access to SD card
// 2. unmount ESP-side FATFS
// 3. export block device using USB OTG / TinyUSB MSC
// 4. after host eject, stop MSC and remount FATFS before leaving page.
esp_err_t usb_storage_enter(void);
esp_err_t usb_storage_exit(void);
usb_storage_state_t usb_storage_get_state(void);
bool usb_storage_can_exit(void);
