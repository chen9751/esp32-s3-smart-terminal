# macOS local build: Cairo search-path troubleshooting

On macOS with Homebrew, `cairocffi` may load Cairo successfully when invoked
directly, but still fail in the ESP-IDF CMake requirement subprocess. This is
caused by differences in dynamic-library search environments, not by an absent
Python wheel.

The CMake component now **skips SVG conversion whenever the generated files
already exist**, and explicitly supplies Homebrew's Cairo library path for
Apple Silicon if generation is necessary. The recommended first-checkout fix is
to generate once in your regular terminal before `idf.py build`:

```sh
export IDF_PYTHON_ENV_PATH="$HOME/.espressif/python_env/idf5.5_py3.9_env"
. "$HOME/esp/esp-idf/export.sh"
export DYLD_FALLBACK_LIBRARY_PATH="$(brew --prefix cairo)/lib"
cd ~/esp32-s3-smart-terminal
python tools/generate_launcher_icons.py
test -s components/launcher_ui/generated/launcher_icons.c
cd apps/launcher
idf.py build
```

The generated `launcher_icons.c` and `launcher_icons.h` are LVGL image data,
not runtime TF card dependencies. Long-term improvement: check in those
generated files to remove Cairo from the firmware build requirements.

Previously observed: on 2026-10-13 Python CLI printed `CairoSVG OK`, but the
ESP-IDF CMake subprocess raised `OSError: no library called "cairo-2" was
found`. Generate assets in the top-level user shell as above to bypass it.
