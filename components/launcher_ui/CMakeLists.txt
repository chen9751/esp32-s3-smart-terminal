# Generate assets before IDF validates SRCS / INCLUDE_DIRS (also works after git clone).
# The icon generator requires Pillow + CairoSVG in the active Python environment.
if(DEFINED ENV{IDF_PYTHON_ENV_PATH})
    set(_icon_python "$ENV{IDF_PYTHON_ENV_PATH}/bin/python")
else()
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    set(_icon_python "${Python3_EXECUTABLE}")
endif()
execute_process(
    COMMAND "${_icon_python}" "${CMAKE_CURRENT_LIST_DIR}/../../tools/generate_launcher_icons.py"
    RESULT_VARIABLE _icons_result
    OUTPUT_VARIABLE _icons_stdout
    ERROR_VARIABLE _icons_stderr
)
if(NOT _icons_result EQUAL 0)
    message(FATAL_ERROR "Failed to generate embedded Launcher icons.\n"
        "Install requirements in the active ESP-IDF Python environment:\n"
        "  ${_icon_python} -m pip install pillow cairosvg\n"
        "Then rerun idf.py build. Generator output:\n${_icons_stderr}")
endif()
idf_component_register(SRCS "launcher_ui.c" "generated/launcher_icons.c"
 INCLUDE_DIRS "include" "generated" REQUIRES lvgl)
