cmake_minimum_required(VERSION 3.16)
set(EXTRA_COMPONENT_DIRS
 "${CMAKE_CURRENT_LIST_DIR}/../../components/board_support"
 "${CMAKE_CURRENT_LIST_DIR}/../../components/launcher_ui"
 "${CMAKE_CURRENT_LIST_DIR}/../../components/ui_theme"
 "${CMAKE_CURRENT_LIST_DIR}/../../components/lvgl_kawaii_face"
 "${CMAKE_CURRENT_LIST_DIR}/../../components/usb_storage"
 "${CMAKE_CURRENT_LIST_DIR}/../../components/usb_phy_restore"
)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(smart_terminal)
