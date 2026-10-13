idf_component_register(
 SRCS "main.c" "launcher_policy.c" "launcher_storage.c" "custom_key.c"
      "ui/home_page.c" "ui/settings_page.c" "ui/usb_storage_page.c"
 INCLUDE_DIRS "." "ui"
 REQUIRES board_support mode_manager usb_storage launcher_ui esp_timer esp_driver_gpio
)
