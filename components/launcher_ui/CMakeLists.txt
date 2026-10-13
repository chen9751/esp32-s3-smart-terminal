# Generate Remix assets only if missing. A fresh macOS checkout can pre-generate
# outside ESP-IDF CMake, avoiding macOS SIP filtering DYLD_* variables.
set(_icon_c "${CMAKE_CURRENT_LIST_DIR}/generated/launcher_icons.c")
set(_icon_h "${CMAKE_CURRENT_LIST_DIR}/generated/launcher_icons.h")
if(NOT EXISTS "${_icon_c}" OR NOT EXISTS "${_icon_h}")
    if(DEFINED ENV{IDF_PYTHON_ENV_PATH})
        set(_icon_python "$ENV{IDF_PYTHON_ENV_PATH}/bin/python")
    else()
        find_package(Python3 REQUIRED COMPONENTS Interpreter)
        set(_icon_python "${Python3_EXECUTABLE}")
    endif()
    # Homebrew installs libcairo outside macOS system lookup directories.
    # Avoid relying on inherited DYLD_* env from the parent process.
    set(_generate_command "${_icon_python}" "${CMAKE_CURRENT_LIST_DIR}/../../tools/generate_launcher_icons.py")
    if(APPLE AND EXISTS "/opt/homebrew/opt/cairo/lib/libcairo.2.dylib")
        set(_generate_command "${CMAKE_COMMAND}" -E env
            "DYLD_FALLBACK_LIBRARY_PATH=/opt/homebrew/opt/cairo/lib"
            "${_icon_python}" "${CMAKE_CURRENT_LIST_DIR}/../../tools/generate_launcher_icons.py")
    endif()
    execute_process(
        COMMAND ${_generate_command}
        RESULT_VARIABLE _icons_result
        ERROR_VARIABLE _icons_stderr
    )
    if(NOT _icons_result EQUAL 0)
        message(FATAL_ERROR
            "Launcher icon generation failed: ${_icons_stderr}\n"
            "Run from repository root in your normal terminal:\n"
            "  python tools/generate_launcher_icons.py\n"
            "Then rerun idf.py build.")
    endif()
endif()
idf_component_register(SRCS "launcher_ui.c" "generated/launcher_icons.c"
 INCLUDE_DIRS "include" "generated" REQUIRES lvgl ui_theme)
