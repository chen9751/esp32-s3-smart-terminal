idf_component_register(SRCS "usb_phy_restore.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_common esp_timer esp_driver_gpio esp_hw_support hal soc)
