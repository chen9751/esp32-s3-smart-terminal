# Troubleshooting log

Record each repeatable issue as: symptoms, board/ESP-IDF version, reproduction steps, root cause with official source, fix, regression test.

## Booting into wrong image

Cause to check: app's build generated a different partition table, or a generic `idf.py flash` overwrote the intended slot. Verify flasher_args.json, OTA data and per-slot images. Use a common partitions CSV across all four builds.

## WakeNet model exceeds 3.5 MiB

This is an open capacity risk, not a confirmed bug. Measure the chosen ESP-SR model(s) and format/partition expectations; reassign flash budget before firmware release.
