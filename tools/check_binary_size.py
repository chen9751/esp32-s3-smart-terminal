#!/usr/bin/env python3
"""Verify built app images fit the fixed three-mode 16 MiB partition slots."""
from pathlib import Path
import csv
import sys

ROOT = Path(__file__).resolve().parents[1]
SLOTS = {"launcher": "factory", "ai_chat": "ota_0",
         "home_assistant": "ota_1", "sony_remote": "ota_2"}

def check(app, filename):
    if app not in SLOTS:
        raise ValueError(f"Unknown app: {app}")
    rows = []
    with (ROOT / "partitions/three_mode_16mb.csv").open(newline="") as f:
        for row in csv.reader(f):
            if row and not row[0].strip().startswith("#") and len(row) >= 5:
                rows.append([v.strip() for v in row])
    size = next((int(r[4], 0) for r in rows if r[0] == SLOTS[app]), None)
    if size is None:
        raise ValueError("Slot missing from partitions CSV")
    image = Path(filename)
    if not image.exists():
        raise FileNotFoundError(image)
    used = image.stat().st_size
    print(f"{app}: image {used:,} bytes / slot {size:,} bytes ({used/size:.1%})")
    if used > size:
        raise SystemExit(f"ERROR: {app} exceeds its {SLOTS[app]} slot")
    if used > size * .90:
        print(f"WARNING: {app} is using over 90% of its slot")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        raise SystemExit("usage: check_binary_size.py APP IMAGE.bin")
    check(sys.argv[1], sys.argv[2])
