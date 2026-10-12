idf_component_register(
  SRCS "main.c" "ui/home_page.c" "ui/settings_page.c"
  INCLUDE_DIRS "." "ui"
  REQUIRES board_support mode_manager
)
