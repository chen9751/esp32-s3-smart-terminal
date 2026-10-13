# ESP32-S3 Smart Terminal

基于 Waveshare ESP32-S3-Touch-LCD-3.49-V2 的三固件智能终端。

> 项目正在搭建基础框架，尚未完成硬件驱动、联网、WakeNet 或 BLE HID 功能；当前提交不是可直接使用的成品。

## 固定启动策略（2026-10-13）

每次正常开机先进入 Launcher，倒计时 **10 秒**。无触摸活动则**固定自动进入 HA**（从不记忆上次模式）。有触摸则取消本次自动启动，保留 Launcher 供手动选择。进入 AI/HA/Remote 后即把下次重启目标恢复到 factory，不影响本次运行。参见 [启动机制及限制](docs/BOOT_FLOW.md)。

**当前 UI/触摸驱动尚未接入：实际屏幕触摸还不能取消倒计时，暂不可作为完整 UI 固件使用。** 软件实现也不等同于异常早期掉电下的严格 bootloader 保证。

## 系统架构

- **Launcher（factory）**：三个大卡片（AI / HA / Sony Remote）+ 一个小型 USB TF 卡入口；10 秒无触摸自动进入 HA。USB MSC 仅为 Launcher 内部维护工具，不增加应用分区，也不做 Wi-Fi 传输。
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


## CI / Web 预览

参见 [GitHub Actions 编译与 Web 预览指南](docs/CI_AND_WEB_PREVIEW.md)。四工程独立自动编译；WebAssembly 640×172 LVGL 浏览器技术预览（目前仅占位页面，与正式 Launcher 还未共享 UI 源码）。发布需要在仓库 Settings → Pages 选择 GitHub Actions，并手动触发首次部署。

## TF 卡资源目录

仓库内 `tf_card/` 是 TF 卡根目录的资源模板：`tf_card/smart_terminal/fonts`、`icons`、`images`、`audio`、`themes`，并包含 `manifest.json`。在 Mac 上 `git pull` 后，把 **tf_card 里面的内容** 增量复制到 TF 卡根目录。详细命令、格式及注意事项见 [TF 卡资源说明](docs/TF_CARD_ASSETS.md)。目前仅创建目录，还没有加载器或真实字体图标资源。

## USB TF 卡管理（Launcher 内）

入口布局：**3 个大卡片 + 1 个小型 USB 存储按钮**。插线并手动进入后，计划由 Mac Finder 直接管理 TF 卡，需先在 Finder 中安全弹出再退出。已建立 `components/usb_storage` 与 Launcher 独立页面和流程接口，**当前只是拒绝未实现操作的安全占位，尚不能让 Mac 识别为磁盘**。后续实现见 [USB 存储设计](docs/USB_STORAGE.md)。没有第四个功能固件、没有 Wi-Fi 文件管理。

## Launcher UI 视觉规范（2026-10-13）

640×172 浅蓝色主题，**四个等大的圆角图标卡片，卡片不显示文字**，顺序为 robot-2-line（AI）、home-wifi-line（HA）、remote-control-line（Sony）、sd-card-mini-line（USB）。USB 页面左侧为 SD 图标，右侧英语连接说明；自定义实体键单击全局返回。真实硬件按键/显示/USB MSC 尚待接入，浏览器预览已改为共享 LVGL UI 模块。全部四个图标为内置资源，**无需拷贝到 TF 卡**。详见 [LAUNCHER_UI.md](docs/LAUNCHER_UI.md)。
