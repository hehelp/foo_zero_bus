## 🎉 Zero Bus 0.1.1

本次发布为产品版 `0.1.1`。提供的 `foo_zero_bus-0.1.1.fb2k-component` 安装包内含 Win32、x64、Windows ARM（ARM64EC）以及 macOS Universal（arm64 + x86_64）版本。在 foobar2000 2.x 环境下安装时，系统会根据软件架构自动选择对应版本。

*(English release notes are provided below)*

### 🚀 更新亮点

- 增加英文语言支持。简体中文区默认中文，其他地区默认英文，也可在面板上手动切换。
- **Windows ARM**：安装包新增 `arm64ec/foo_zero_bus.dll`。foobar2000 ARM 会优先加载该文件。
- **macOS Universal**：安装包新增 `mac/foo_zero_bus.component`，同一份二进制同时包含 arm64 与 x86_64。
- **布局修复**：修正偏好页「语言」标签的位置和宽度。
- **开发文档**：详细的接入说明请参阅仓库中的 [第三方接入 API](docs/sdk.md) · [中文](docs/sdk_zh.md)。

### 📦 安装指南

1. 下载本发布页提供的 `foo_zero_bus-0.1.1.fb2k-component` 安装包。
2. 打开 foobar2000，依次点击 **文件 → 首选项 → 组件 → 安装**。
3. 安装完成后，**重启 foobar2000** 即可生效。

> **💡 提示**：安装并重启后，服务将默认监听 `ws://127.0.0.1:17890`。更多详情请查阅项目 [README](README_zh.md)。

---



### 🚀 What's New

- **UI language**: Preferences are now bilingual. Simplified Chinese locales default to Chinese; others default to English. The language can also be switched on the panel.
- **Windows ARM**: The package now includes `arm64ec/foo_zero_bus.dll`. foobar2000 for ARM prefers this over the x64 build.
- **macOS Universal**: The package now includes `mac/foo_zero_bus.component`, a single Universal Binary with both arm64 and x86_64.
- **Layout fix**: Corrected the preferences “Language” label position and width.
- **Documentation**: For API details, please refer to the [English SDK](docs/sdk.md) · [中文](docs/sdk_zh.md).



### 📦 Install Guide

1. Download the `foo_zero_bus-0.1.1.fb2k-component` package from this release.
2. In foobar2000, navigate to **File → Preferences → Components → Install**.
3. **Restart foobar2000** to apply changes.

> **💡 Note**: The default listen address after installation is `ws://127.0.0.1:17890`. For more information, please read the [README](README.md).

