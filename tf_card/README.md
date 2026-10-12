# TF 卡资源根目录

此目录的**内容**用于复制到 TF 卡根目录（不是把 tf_card 目录本身复制进去）。

预计设备挂载 FAT32 TF 卡到 `/sdcard`，资源路径统一约定为：
`/sdcard/smart_terminal/...`。
**挂载点和 LVGL 文件系统盘符尚未实际接入、验证**，后续以 V2 硬件测试为准。

## 目录

- `smart_terminal/fonts/`：LVGL 二进制字体子集（思源黑体/数字等），目前为空。
- `smart_terminal/icons/`：Remix Icon 子集或图标素材，目前为空。
- `smart_terminal/images/`：图片资源，目前为空。
- `smart_terminal/audio/`：提示音与短音效，目前为空。
- `smart_terminal/themes/`：颜色、主题等配置，目前为空。
- `smart_terminal/manifest.json`：资源清单及版本。

Git 不跟踪空目录，子目录中的 `.gitkeep` 仅用于保留目录层级，后续可删除；设备运行时无需解析这些文件。

## macOS 同步

从仓库根目录执行（先运行 `git pull`）：

```bash
# 替换为 Finder 中 TF 卡的实际卷名；请先确认目标路径！
CARD="/Volumes/TF_CARD"
test -d "$CARD" || { echo "TF 卡卷不存在: $CARD"; exit 1; }
rsync -av --exclude='.DS_Store' --exclude='.gitkeep' tf_card/ "$CARD/"
```

上面是**增量复制，不会删除卡上现有文件**。建议首次仅复制到空卡。Mac 磁盘工具建议格式化为 FAT32（MBR），确保 ESP-IDF SD FAT 挂载兼容；格式化会清空数据，请先备份。

注意：密钥、Token、Wi-Fi 密码不要放进本公开仓库或这份 TF 卡样板。大字体资源入库前需检查许可证。
