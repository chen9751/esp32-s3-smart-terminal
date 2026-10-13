# Launcher icon generator — local macOS build

The five compiled LVGL icon images are generated from tracked official Remix Icon SVG files. The `components/launcher_ui/generated/` directory is deliberately untracked. CMake now invokes `tools/generate_launcher_icons.py` **before** validating component include paths, so a fresh checkout can build without manually creating that directory.

Requirements in the ESP-IDF Python environment: `pillow` and `cairosvg`.

```sh
export IDF_PYTHON_ENV_PATH="$HOME/.espressif/python_env/idf5.5_py3.9_env"
. "$HOME/esp/esp-idf/export.sh"
python -m pip install pillow cairosvg
cd ~/esp32-s3-smart-terminal
git pull --ff-only
cd apps/launcher
idf.py build
```

If CairoSVG reports a missing native Cairo dynamic library on macOS, install it with `brew install cairo` and retry. No TF card asset copy is required.

Prior failure: local fresh clone had no `generated` folder, so ESP-IDF CMake failed with `Include directory .../components/launcher_ui/generated is not a directory`. CI pre-generated icons but local build did not. Both builds now use the generator.
