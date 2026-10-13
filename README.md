# ESP32-S3 Smart Terminal · 单固件多模式

> 2026-10-14：开发主线已切换为 **单个 ESP-IDF 工程、单个应用镜像、单套 sdkconfig**。目前处于 **UI 阶段（M1/M2）**，并非 AI/HA/BLE 功能成品；请勿把占位页面当成已经接入的业务。

硬件：Waveshare **ESP32-S3-Touch-LCD-3.49-V2**（18650 电池款），16 MiB Flash / 8 MiB OPI PSRAM / 640×172 横屏 LVGL 9.2.2。主工程：`apps/terminal`。

## 固定的产品逻辑

1. 每次正常开机或按 **RESET** 都进入 Launcher。
2. Launcher 四个图标：**AI / HA / Sony / USB-TF 工具**。前三项是三种互斥业务模式，第四项仍是 Launcher 维护工具。
3. 点击 AI/HA/Sony **不重启**，而是删除 Launcher 页面并进入对应 UI，选择后锁定当前模式；不允许从一个业务模式跳到另一个模式。
4. **只有 RESET 才返回 Launcher**。不设计复杂的模式 stop/deinit/reinit 热切换。
5. 资源严格按模式初始化：Launcher 不启 Wi-Fi/BLE/AFE；AI 启 Wi-Fi/TLS/音频但不启动 WakeNet/BLE；HA 启 Wi-Fi/WakeNet/音频但不启 BLE；Sony 只启动 BLE HID/遥控 UI，不启动 Wi-Fi、WakeNet、云聊天。
6. 自用设备一律 **USB 刷写**，不使用 OTA、ota_data、多应用分区、在线升级或 SD 刷写中转。单固件架构文件在 `partitions/single_app_16mb.csv`。

以上决策来源于项目基线《ESP32-S3_单固件多模式_项目架构与资源预算_v1.0.docx》。不经资源评估及文档修改，不应绕开这些约束。

## 当前代码状态（不能跳过）

- **已经实现**：保留 Launcher 四个图标样式；单入口 `app_manager`；同镜像进入互斥页面；AI 17 表情引擎及黑底主界面；AI 长按 GPIO0（约 800ms）进入家长设置，短按返回、10 秒无操作退出；USB/TF 按原 Launcher TinyUSB 服务入口处理。
- **UI 骨架、尚未完整开发**：AI 设置目前只有六个底部页签及说明，没有参数保存；HA 与 Sony 目前只有各自的标题占位屏，正式 UI 需逐页从旧项目选择性迁移或新写。
- **没有实现**：AI 云模型、情绪事件联动、WakeNet/HA 业务、BLE HID、音频链路、用户配置管理。
- **验证状态**：**2026-10-14 本机 `apps/terminal` 已编译成功**：`smart_terminal.bin` = `0xB75A0` bytes，8MiB 应用分区报告剩余约 91%；仍未完成真实单固件设备烧录与联调。
- **已清理主线的四个 legacy 独立工程、OTA `mode_manager`、旧分区表及旧启动文档**。完整历史源码保存在 `archive/multi-firmware-before-cleanup` 分支，不需要将旧项目重新复制入 `main`。AI UI 的旧分支已通过 PR #1 合并。

## 开发阶段：先 UI，再业务

| 阶段 | 目标 | 验收 |
|---|---|---|
| M1 单工程 | 单 sdkconfig，Launcher 四图标，三模式互斥，RESET 返回 Launcher | 编译、Flash/heap 日志、实机触摸/启动 |
| M2 全部 UI | 完成 AI 17 表情和家长设置；HA 旧 UI 精简迁移；Sony 遥控 UI；Launcher USB 工具 | 四入口显示/触摸/按钮行为正确，设置无误退 |
| M3 HA | 按需导入旧 HA/WakeNet 必要模块 | 旧项目内存基线对比，AFE/Wi-Fi 测试 |
| M4 Sony | BLE HID Peripheral | 配对、重连、操控 |
| M5 AI 云 | 阿里云实时语音 + 情绪映射 + 受限 HA 调用 | 会话、网络重连、音频/内存稳态 |
| M6 回归 | 模式互斥、长期运行、USB 更新 | Flash / 内存预算通过 |

**目前严格停留在 UI 阶段**；不得为填充页面提前开启网络、WakeNet、BLE 或 AI 服务。

## Flash 与内存预算

- 应用分区暂定 **8 MiB**，WakeNet 独立模型数据分区 **1 MiB（待 ESP-SR loader 验证）**；其他空闲 Flash 不强行划分。没有 OTA 或 assets 分区。
- 旧项目 `esp32-ai-robot` 曾测得 WakeNet/AFE 后内部可用 **28,715 B**、最大连续内部块约 **9 KiB**、PSRAM 剩余 **7,318,124 B**；这不是新单固件实测。
- 新项目 Launcher 之前实测内部空闲 **264,375 B**、PSRAM 空闲 **7,941,524 B**；合并后的静态内存需要重新量化。
- 工程预估（非实测）：单镜像约 **2.8–4.5 MiB**；PSRAM 剩余 Launcher **7.2–7.55 MiB**、AI **6.2–7.1 MiB**、HA **6.0–7.0 MiB**、Sony **7.0–7.5 MiB**。
- 内部空闲验收建议：Launcher ≥150KiB；AI ≥80KiB；HA ≥40KiB；Sony ≥80KiB。必须同步记录最大连续内部块与 DMA 内存。HA 是首要风险。
- 避免把非当前模式的大缓冲定义为内部静态数组；LVGL、网络非 DMA 缓冲、音频环形缓存优先按需 PSRAM；DMA 必须按能力分配。

## Mac 本地编译（新主线）

```sh
cd ~/esp32-s3-smart-terminal
git switch main
git pull --ff-only origin main
export IDF_PYTHON_ENV_PATH="$HOME/.espressif/python_env/idf5.5_py3.9_env"
source ~/esp/esp-idf/export.sh
python tools/generate_launcher_icons.py
cd apps/terminal
idf.py build
idf.py size
```

**分区表与当前设备上的旧多固件布局不同。** 不要仅用 `0x20000` 或 `0x160000` 的旧偏移刷写单固件；分区迁移需要等构建验证后受控操作，备份旧固件/数据后，使用 `idf.py flash` 完整烧录新 bootloader、partition-table 与 factory 镜像。切勿在本轮未验证时执行 `erase_flash`。

## 资料与约束

- [旧项目 HA/内存基线](https://github.com/chen9751/esp32-ai-robot)：`docs/memory-audit-2026-10-09.md`、`docs/psram-memory-experiment.md`、`docs/memory-optimization-test.md`。
- [Waveshare 官方 V2](https://github.com/waveshareteam/ESP32-S3-Touch-LCD-3.49-V2)；[ESP-IDF 官方](https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32s3/)。
- 17 表情引擎源于 [0015/lvgl_kawaii_face](https://github.com/0015/lvgl_kawaii_face)，迁移的是旧项目针对 LVGL 9 和 PSRAM 优化版本，许可保留在 `components/lvgl_kawaii_face/LICENSE`。
- 一页一个 .c/.h；业务逻辑不写入 UI；API Key、HA Token、Wi-Fi 密码不得提交到 GitHub。TF 可拆卸，不能提供密钥机密性。
- 每次发现可复现的问题，先查 README、官方文档与案例，再将解决办法写入 `docs/troubleshooting.md`；新增功能应核对资源预算与项目基线，避免偏离主线。

## 历史工程

现有 `apps/launcher` 等多工程文件为迁移前参考，不代表仍使用四固件。旧 `docs/PARTITIONS.md`、`docs/ARCHITECTURE.md` 等记录仅作为历史材料；**本 README 与单固件架构基线优先**。

## 仓库清理（2026-10-14）

- 本主线只保留 `apps/terminal`；ESP-IDF **标准 bootloader** 仍是启动所必需，不等于旧自定义 OTA 多固件框架，不能删除 `build/bootloader/bootloader.bin` 的烧录步骤。
- 已删除旧的四个独立 `apps/*` 应用、`components/mode_manager` 和 `partitions/three_mode_16mb.csv`；备份分支 `archive/multi-firmware-before-cleanup` 可供按需查找历史代码。
- 新单固件构建成功：`smart_terminal.bin` 0xB75A0；**新分区表尚未完成板端烧录核验**。当前阶段继续优先完成三种模式 UI。
