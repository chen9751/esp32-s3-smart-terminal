idf_component_register(
 SRCS "main.c" "app_manager.c"
      "ui/ai_face.c" "ui/ai_settings.c" "ui/mode_placeholder.c"
      "ui/terminal_key.c"
 INCLUDE_DIRS "." "ui"
 REQUIRES board_support launcher_ui lvgl_kawaii_face usb_storage
          esp_timer esp_driver_gpio
)
