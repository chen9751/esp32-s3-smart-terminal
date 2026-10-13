#!/usr/bin/env python3
"""Rasterize four official Remix Icon SVGs into small LVGL ARGB8888 assets.
Requires: pip install cairosvg pillow. Output is generated, no TF card required.
"""
from pathlib import Path
from io import BytesIO
import cairosvg
from PIL import Image

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/"components/launcher_ui/generated"
OUT.mkdir(parents=True,exist_ok=True)
SPECS=[("robot","robot-2-line",54),("remote","remote-control-line",54),
       ("home","home-wifi-line",54),("sd","sd-card-mini-line",54)]
lines=['#include "launcher_icons.h"','#include "lvgl.h"']
hdr=['#pragma once','#include "lvgl.h"']
for key, name, size in SPECS:
    svg=(ROOT/"assets/remix"/f"{name}.svg").read_text(encoding="utf-8").replace('fill="currentColor"','fill="#B6E6FA"')
    png=cairosvg.svg2png(bytestring=svg.encode(),output_width=size,output_height=size)
    rgba=Image.open(BytesIO(png)).convert("RGBA")
    # Little endian ARGB8888 (BGRA byte order); premultiply channels as LVGL expects.
    data=[]
    for r,g,b,a in rgba.getdata():
        data.extend((b*a//255,g*a//255,r*a//255,a))
    name_id=f"launcher_icon_{key}"
    hdr.append(f"extern const lv_image_dsc_t {name_id};")
    lines.append(f"static const uint8_t {name_id}_data[] = {{")
    for start in range(0,len(data),24):
        lines.append("  "+",".join(str(x) for x in data[start:start+24])+",")
    lines.extend(["};",f"const lv_image_dsc_t {name_id} = {{",
                  f"  .header = {{ .magic=LV_IMAGE_HEADER_MAGIC, .cf=LV_COLOR_FORMAT_ARGB8888, .w={size}, .h={size}, .stride={size*4} }},",
                  f"  .data_size=sizeof({name_id}_data), .data={name_id}_data",
                  "};"])
(OUT/"launcher_icons.h").write_text("\n".join(hdr)+"\n")
(OUT/"launcher_icons.c").write_text("\n".join(lines)+"\n")
print("Generated 4 embedded LVGL Remix icons")
