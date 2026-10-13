idf_component_register(SRCS "usb_storage.c" INCLUDE_DIRS "include"
  REQUIRES esp_common sdmmc fatfs esp_driver_sdmmc esp_tinyusb)
