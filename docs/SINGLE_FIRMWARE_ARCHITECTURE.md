# 单固件架构实施基线（2026-10-14）

> 主线文档：用户确认的《ESP32-S3_单固件多模式_项目架构与资源预算_v1.0.docx》。本文件只记录源码对应关系及 UI 阶段执行顺序，不替代正式预算文档。

## 唯一启动路径

- `apps/terminal/main/main.c`：公共板级初始化一次、GPIO0 输入轮询、LVGL 单任务处理。
- `apps/terminal/main/app_manager.[ch]`：启动 Launcher；三个业务模式只允许选择一次，UI 当场切换，无重启；RESET 重新启动单镜像并进入 Launcher。
- `components/launcher_ui`：既有四卡片 Launcher 视觉资源，前三张卡片选模式；USB 是 Launcher 内部维护页。
- `apps/terminal/main/ui/ai_face.[ch]`：优化版 17 种表情，主页面仅表情，无后台网络。
- `apps/terminal/main/ui/ai_settings.[ch]`：基础设置标签、10 秒超时及实体返回键；仍属视觉骨架。
- `apps/terminal/main/ui/mode_placeholder.[ch]`：HA/Sony 暂时只有标题，下一阶段必须迁移/完成真实 UI。不得宣称 UI 已完成。
- `components/board_support`：共用 LCD、触摸、背光与 LVGL；后续音频单独按需初始化。

## 资源归属与架构红线

| 状态 | 启动 UI | 网络 | WakeNet | BLE | 音频 |
|---|---|---|---|---|---|
| Launcher | 四卡片 | 不启动 | 不加载 | 不启动 | 不启动 |
| AI Chat | 17 表情/家长设置 | 后续 Wi-Fi/WSS | 不加载 | 不启动 | 后续 I2S |
| HA | 旧 HA 必要 UI | 后续 Wi-Fi | 后续 AFE | 不启动 | 后续 I2S |
| Sony | 遥控器 UI | 不启动 | 不加载 | 后续 NimBLE HID | 不启动 |

- 资源在用户选择模式以后再初始化。大缓存动态 PSRAM 分配；DMA-capable 内部内存单独统计。
- 运行期间不允许重新选择业务模式。按 RESET 回到 Launcher，不需要跨模式 stop/restart 状态机。
- Flash：单 factory 应用分区 8MiB；model 暂定 1MiB，仅为 WakeNet loader 条件预算；空闲空间不额外创造 OTA / assets 分区。
- 主线工程唯一的 sdkconfig 位于 `apps/terminal/sdkconfig`（本地生成），默认项在 `sdkconfig.defaults`。旧 `apps/*` 分工程保留为历史对照，**不再更新**。
- UI 验收之前，不迁移 HA 业务、模型、BLE HID 或云端调用。先检查完整页面、字体、触摸、长短按和 10 秒设置页返回。

## UI 实施顺序

1. 确认单工程可编译，统一 PSRAM 配置和分区；本轮仅源码提交、尚未实机验收。
2. Launcher：保留四入口视觉、确认点击后不会触发固件重启；验证 USB MSC 安全退出。
3. AI：完整 17 表情预览与各状态联动（先本地模拟）；设置页按旧版风格实现**真实可操作**的设置控件（业务写入后续）。
4. HA：从旧 `esp32-ai-robot` 仅迁移必需 UI 页面及资源；不复制全部旧代码、网络/音频服务。
5. Sony：实现横屏可触控遥控器页面、按键/旋钮交互视觉，不接 BLE 业务。
6. 完成三种模式 UI 测试及实时内存记录，再开始 WakeNet / BLE / 云模型任务。

## 构建与 Flash 安全

`apps/terminal` 使用与旧项目完全不同的应用分区布局，切忌把单固件应用直接写到旧 `0x20000` factory、`0x160000` AI 分区。**需要完整编译验证，然后备份并重新烧录 bootloader + 单分区表 + 单应用镜像。** 本轮只建立源码，不代表已安全可烧录。
