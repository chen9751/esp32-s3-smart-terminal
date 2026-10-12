idf_component_register(
  SRCS "main.c" "ui/clock_page.c" "ui/device_page.c" "ui/listening_page.c" "ui/result_page.c"
  INCLUDE_DIRS "." "ui"
  REQUIRES board_support mode_manager
)
