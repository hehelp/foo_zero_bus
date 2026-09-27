## 🎉 Zero Bus 0.1.2

本次发布为产品版 `0.1.2`。提供的 `foo_zero_bus-0.1.2.fb2k-component` 安装包内含 Win32、x64、Windows ARM（ARM64EC）以及 macOS Universal（arm64 + x86_64）版本。在 foobar2000 2.x 环境下安装时，系统会根据软件架构自动选择对应版本。

*(English release notes are provided below)*

### 🚀 更新亮点

- **macOS 偏好页**：新增 **工具 → Zero Bus** 页面（端口、启停服务、界面语言、当前连接）及其「日志」子页，功能与 Windows 一致。
- **macOS 语言**：「跟随系统」改为按系统首选语言判断，简体中文系统默认显示中文。
- **Windows 版本信息**：DLL 属性中补充公司（klyrics.cn）与版权声明。
- **开发文档**：详细的接入说明请参阅仓库中的 [第三方接入 API](docs/sdk.md) · [中文](docs/sdk_zh.md)。

### 📦 安装指南

1. 下载本发布页提供的 `foo_zero_bus-0.1.2.fb2k-component` 安装包。
2. 打开 foobar2000，依次点击 **文件 → 首选项 → 组件 → 安装**。
3. 安装完成后，**重启 foobar2000** 即可生效。

> **💡 提示**：安装并重启后，服务将默认监听 `ws://127.0.0.1:17890`。更多详情请查阅项目 [README](README_zh.md)。

---

### 🚀 What's New

- **macOS preferences**: New **Tools → Zero Bus** page (port, start/stop, UI language, active connections) plus a “Log” sub-page, matching Windows.
- **macOS language**: “Follow system” now uses the system preferred language, so Simplified Chinese systems default to Chinese.
- **Windows version info**: The DLL properties now show the company (klyrics.cn) and copyright.
- **Documentation**: For API details, please refer to the [English SDK](docs/sdk.md) · [中文](docs/sdk_zh.md).

### 📦 Install Guide

1. Download the `foo_zero_bus-0.1.2.fb2k-component` package from this release.
2. In foobar2000, navigate to **File → Preferences → Components → Install**.
3. **Restart foobar2000** to apply changes.

> **💡 Note**: The default listen address after installation is `ws://127.0.0.1:17890`. For more information, please read the [README](README.md).
