# Smart Terminal Global UI Color Theme

All screens use **pure black #000000** for the entire background, blending visually with the black screen bezel. The housing is vivid blue; the primary UI color is **glacier cyan / slightly greenish blue #78DCE8**. It is used for icons, card borders and primary controls.

| Usage | Hex |
| --- | --- |
| Background (mandatory) | `#000000` |
| Main accent | `#78DCE8` |
| Text and highlight | `#A1F1FA` |
| Secondary labels | `#3A7895` |
| Pressed card only | `#102A37` |
| Disabled state | `#35505B` |

Source of truth: `components/ui_theme/include/terminal_theme.h`. All future UI pages for Launcher, AI Chat, Home Assistant and Sony Remote **must** use this palette. Never substitute navy/teal for the pure-black full-screen background. Color fills are restricted to local interactions like pressed tiles.

Launcher recolors existing compiled image icons at runtime and the SVG generator uses the same theme color for new generated icon assets. No layout, interaction, backlight, audio or USB changes.
