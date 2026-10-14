# AI 设置硬件联调（开发分支）
日期：2026-10-15。此分支为**待编译及实机验收**，切勿当作已验证固件。

- 单固件进入 AI 后才启动 Wi-Fi 与 ES8311；Launcher 不初始化这两个服务。
- TF 使用 V2 SDMMC 1-bit（CMD39/D0 40/CLK41），读取 `/sdcard/config.json`。格式沿用旧项目的 `wifi.ssid`、`wifi.password`、`ai.url`；缺卡与 JSON 错误不阻止显示 UI，不创建配置热点。TF 资源由 Wi-Fi 服务持续挂载，进入 USB/TF 功能前请重启到 Launcher。
- 屏幕 PWM GPIO42 + EXIO1 沿用当前板级驱动；最低 1% 以避免触屏误关屏。
- Codec 使用已验证 ES8311、I2S0 TDM 24kHz、I2C0，滑块变更直接调用 `esp_codec_dev_set_out_vol`；音频流未接入，无法通过本轮固件播放 AI 语音。
- 电池 GPIO4/ADC1_CH3，12dB 衰减，曲线校准后 ADC 毫伏 ×3，失败显示 unavailable 而不猜值；具体数值须实机对照万用表确认。
- 音色 Liora Mira/Mia/Mione 使用本地 NVS 键 `ai_pref/voice` 存储，`terminal_ai_settings_voice()` 供未来 Omni 会话使用；当前没有真正的 Omni 连接，不宣称音色已生效。
- 设置保持 GPIO0 长按 800ms 进入/短按退出、10 秒超时；系统仅显示电池电压。云 AI 参数仍为占位，暂不加入聊天服务。
- **待办**：在 macOS ESP-IDF 5.5.4 完整 `idf.py build` + `idf.py size`，上传串口日志。检查：Wi-Fi 有卡/缺卡/密码错误/掉线；音量设置与真实播放；背光滑块；VBAT 对照万用表；音色选择重启保持；LCD/触摸/USB MSC/RESET 回 Launcher；比较内部 heap 最大连续块及 PSRAM。
- 未完成：17 表情实际模型情绪事件接口、Remix chat-voice-line 图标资源、Omni/WSS 音频、运行时资源验收。不得直接合入 main 或烧录未编译分支。

参考：旧项目 network_service.c、audio_service.c、board.c 及单固件架构 v1.0。不得在仓库内提交真实 Wi-Fi 密码/API Key。
