# Launcher UI specification (2026-10-13)
Display: 640×172 landscape, four **equal-sized** rounded cards, no card labels. Icons in left-to-right order:
1. `robot-2-line` → AI (`ota_0`)
2. `home-wifi-line` → HA (`ota_1`)
3. `remote-control-line` → Sony Remote (`ota_2`)
4. `sd-card-mini-line` → USB storage (within Launcher, not an app partition)

Palette: pastel pale blue, approximate enclosure color; final color matching to the Waveshare housing is subject to physical review. The four 142×122 cards are evenly aligned. No text on cards.

USB screen: pale-blue rounded panel, SD icon on left, short English instructions on right:
"Connect this device to your computer with a USB cable to manage TF card files. Eject the disk on your computer before exiting."

**Physical custom key:** single-click is global Back; on USB screen returns to Launcher home, subject to safe eject. Main screen Back does nothing. Hardware GPIO input is not integrated; browser preview uses Escape / Backspace as a test substitute.

**Design / implementation:** `components/launcher_ui/launcher_ui.c` contains the LVGL screens, and `tools/generate_launcher_icons.py` rasterizes original official Remix Icon SVGs into compiled LVGL images before builds. No TF card asset files needed. UI callback is the integration point for actual firmware switching, 10-second countdown cancel, and USB service. It is not yet wired to V2 display/touch or physical key. Browser preview renders the exact shared UI layout.

**USB caution:** real USB MSC is currently a fail-closed stub; do not present as active connection. The Back key must not allow leaving while host still owns the filesystem in the final integration. See [USB_STORAGE.md](USB_STORAGE.md).

Icons from Remix Icon (Apache-2.0), originals in `assets/remix/`.

## Revision: black / outline squares

- Launcher background: pure black `#000000`.
- Four equal square cards: **124×124**, rounded 18px; black interior, 2px pastel-blue outline (`#9BD7F1`), centered light-blue Remix icons (`#B6E6FA`), **no text**.
- Coordinates: x=24/180/336/492, y=24; 640×172 screen.
- USB page: pure black background; **no panel or card**, left SD card icon, right English cable/eject instructions in pale blue.
- Icons remain embedded in firmware; TF card copy is not required.
