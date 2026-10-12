# Flash layout (initial budget, **not measured**)

Board assumption: 16 MiB Flash and 8 MiB PSRAM. See partitions/three_mode_16mb.csv.

| Region | Offset | Size |
|---|---|---|
| NVS | 0x009000 | 24 KiB |
| OTA data | 0x00F000 | 8 KiB |
| PHY init | 0x011000 | 4 KiB |
| Launcher factory | 0x020000 | 1.25 MiB |
| AI ota_0 | 0x160000 | 3.5 MiB |
| HA + WakeNet ota_1 | 0x4E0000 | 3.5 MiB |
| BLE Remote ota_2 | 0x860000 | 2.5 MiB |
| ESP-SR model data | 0xAE0000 | 3.5 MiB |
| assets data | 0xE60000 | 1 MiB |
| not allocated | 0xF60000 | 640 KiB |

Partition table ends well before the first app and app offsets are 64 KiB aligned. The model region is not automatically populated; its subtype/loader must be made compatible with the selected ESP-SR version. **Do not flash a model until ESP-SR loader settings are confirmed.** No extra full-size OTA staging slot exists: these ota_N entries are fixed application roles, NOT A/B redundancy. OTA updates need a separately designed SD staging/verification/recovery flow.

Need verify with idf.py partition-table and actual build sizes before treating the layout as final. If the ESP-SR model exceeds 3.5 MiB, repartition before adding features. Avoid changing offsets after user data has been deployed without migration.
