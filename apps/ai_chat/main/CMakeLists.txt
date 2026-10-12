idf_component_register(
  SRCS "main.c" "ui/face_page.c" "ui/connection_page.c"
  INCLUDE_DIRS "." "ui"
  REQUIRES board_support mode_manager
)
