cmake_minimum_required(VERSION 3.16)
set(EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/../../components/board_support"
    "${CMAKE_CURRENT_LIST_DIR}/../../components/mode_manager"
)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ai_chat)
