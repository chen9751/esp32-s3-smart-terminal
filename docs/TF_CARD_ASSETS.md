# TF Card resources

See [tf_card/README.md](../tf_card/README.md). The contents of repo `tf_card/` are copied to the **root of a FAT32 TF card**. Firmware logical root planned: `/sdcard/smart_terminal/`.

**Flash vs RAM**: putting resources on TF card avoids embedding their bytes in each firmware image, but a font loaded in full still occupies runtime memory. Later add a shared resource manager with cached/fallback assets and bounded memory use. Critical Launcher text/icons must stay in Flash so lack of a card cannot break boot choice. The ESP-SR WakeNet model currently uses a dedicated Flash partition and must **not** be moved to TF card without checking the model loader.

macOS from project root:

```sh
git pull
CARD="/Volumes/TF_CARD"  # change to actual volume name
test -d "$CARD" || exit 1
rsync -av --exclude='.DS_Store' --exclude='.gitkeep' tf_card/ "$CARD/"
```

Future work: verified V2 SDMMC/SPI mounting, LVGL FS driver, font/icon file types, manifest version validation, fallback behavior, checksums and font licenses.
