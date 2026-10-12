# ESP32-S3 Smart Terminal

基于 Waveshare ESP32-S3-Touch-LCD-3.49-V2 的三固件智能终端。

> 项目正在搭建基础框架，尚未完成硬件驱动、联网、WakeNet 或 BLE HID 功能；当前提交不是可直接使用的成品。

## 系统架构

- **Launcher（factory）**：LVGL 模式选择、系统设置、分区切换。
- **AI Chat（ota_0）**：17 种表情、按键/实时语音、NUC11 与阿里云服务。**不加载 WakeNet**。
- **Home Assistant（ota_1）**：WakeNet 唤醒、时钟、聆听状态、HA 语音执行反馈，以及灯光/设备触控页面；切换页面不重启。
- **Sony Remote（ota_2）**：BLE HID、方向/确认/返回、IMU 体感方向、连接与配对页面；与 Wi-Fi 模式隔离。

模式之间通过 ESP-IDF OTA 应用分区选择 API 与重启切换，并非重写二级 Bootloader。`factory` 常驻 Launcher。每套固件的 GUI 页面必须按文件拆分，不允许合并到单个 `main.c`。

## 目录与构建

参见 `docs/ARCHITECTURE.md`、`docs/PARTITIONS.md`、`docs/DEVELOPMENT.md`。四个独立 ESP-IDF 工程在 `apps/launcher`、`apps/ai_chat`、`apps/home_assistant`、`apps/sony_remote`，公共组件在 `components/`，分区配置在 `partitions/`。

当前为**接口/页面占位骨架**：尚未接入微雪 V2 显示/触摸/音频/IMU 驱动，故运行时仅打印日志，不会渲染屏幕；不可把构建骨架误认为四模式已完成。分区预算在测试 WakeNet 模型大小之后重新确认。

## 设计纪律

1. UI **一个页面一个 `.c/.h`**，页面间通过明确接口调用，业务逻辑与视图分离。
2. 共享驱动在 `components/board_support`，共享分区切换在 `components/mode_manager`，不复制到四个工程。
3. HA 语音+触控在同一个应用，AI 不启用 WakeNet；BLE 遥控不初始化 Wi-Fi。
4. API 密钥、HA Token、Wi-Fi 密码等不提交到 Git。使用 NVS/安全配置流程，严禁硬编码。
5. 每次修改硬件配置、分区或遇到可复现问题，更新 README 或 docs/troubleshooting.md。
6. 重大功能接入前先查 Waveshare V2 原理图/示例、乐鑫 ESP-IDF/ESP-SR 官方文档，再对照实际编译、硬件测试结果。

## 官方资源

- [Waveshare ESP32-S3-Touch-LCD-3.49-V2 示例](https://github.com/waveshareteam/ESP32-S3-Touch-LCD-3.49-V2)
- [ESP-IDF 分区表](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-guides/partition-tables.html)
- [ESP-IDF OTA](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/system/ota.html)
- [ESP-SR](https://github.com/espressif/esp-sr)
