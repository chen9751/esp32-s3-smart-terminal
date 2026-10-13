idf_component_register(SRCS "board_support.c" INCLUDE_DIRS "include"
 REQUIRES esp_system esp_timer esp_lcd esp_driver_spi esp_driver_i2c esp_driver_ledc
 esp_io_expander_tca9554 esp_lcd_axs15231b lvgl)
