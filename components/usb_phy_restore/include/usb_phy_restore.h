#pragma once
#include "esp_err.h"
// Only invoke after TinyUSB MSC has been safely ejected and completely stopped.
// ESP32-S3 native PHY restoration is experimental and hardware-test dependent.
esp_err_t usb_phy_restore_serial_jtag(void);
