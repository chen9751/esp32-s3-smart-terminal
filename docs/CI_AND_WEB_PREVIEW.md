# CI 与浏览器 UI 预览（单固件主线）

## 固件 CI

[firmware-build.yml](../.github/workflows/firmware-build.yml) **只编译** `apps/terminal`（ESP-IDF 5.5.4），上传：
- `smart_terminal.bin`
- ESP-IDF 标准 `bootloader.bin`
- `partition-table.bin`
- `flasher_args.json`

CI 检查单一 factory 镜像不超过 8 MiB。不存在四应用矩阵、OTA 选择或在线 OTA 更新。标准 Bootloader 仍为必需的 ESP-IDF 启动组件，不可因取消多固件模式而删除。

## Web UI 预览

[ui-web-preview.yml](../.github/workflows/ui-web-preview.yml) 仍保留 LVGL 9.2.2 + Emscripten/SDL2 的浏览器预览路径。浏览器页面目前仅覆盖 Launcher，**尚未完整复用** `apps/terminal` 中三个业务 UI 的真实源文件；不能代替板上触摸和 GPIO0 验证。

页面启用时在仓库 Settings → Pages 选择 GitHub Actions，并手动触发 UI Web Preview workflow。

## 目前已知结果

2026-10-14：用户本地已成功编译 `apps/terminal`，`smart_terminal.bin` 报告大小为 `0xB75A0` 字节（8MiB 分区约 91% 空闲）。这并非 CI 或实机验证结果；固件烧录和新分区表迁移前仍须做好备份并测试。

## 历史记录

四个独立应用及 OTA 分区相关代码仅保留在 `archive/multi-firmware-before-cleanup` 分支，不再作为主线构建目标。
