idf_component_register(
  SRCS "main.c" "ui/remote_page.c" "ui/bluetooth_page.c"
  INCLUDE_DIRS "." "ui"
  REQUIRES board_support mode_manager
)
