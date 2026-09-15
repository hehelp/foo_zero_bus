## 🎉 Zero Bus 0.1.0 (正式版 / Production Release)

本次发布为产品版 `0.1.0`。提供的 `foo_zero_bus.fb2k-component` 安装包内含 Win32 与 x64 版本（注：本安装包不包含 macOS 版本）。在 foobar2000 2.x 环境下安装时，系统会根据软件架构自动选择对应版本。

*(English release notes are provided below)*

### 🚀 更新亮点

- **核心引擎**：引入进程内 C++20 消息总线，完整支持 REQUEST、RESPONSE、EVENT 与 NOTIFICATION 模型，并原生支持超时控制与协作式取消机制。
- **开发者接入**：其他 foobar 组件需通过 `zero_message_bus_v1` ABI 接入。请仅拷贝仓库中的 `sdk/foo_zero_bus/abi/` 目录，**切勿**直接链接总线核心库。
- **网络服务**：内置本机 WebSocket 服务，默认监听 `127.0.0.1:17890`。为保障安全，该服务仅限本机回环地址访问，且默认不开启鉴权。
- **UI 面板**：新增偏好设置页面（路径：**工具 → Zero Bus**）。支持修改端口、启停服务、实时查看当前连接数，并提供可选的通信日志功能。
- **强制规范**：所有消息的 Payload 必须为纯字符串格式。如果业务数据为 JSON，请务必先执行 `stringify` 序列化。
- **请求语法糖**：同进程 C++ SDK 提供 `request_async` / `request_future` / `request_sync` / `request_awaitable`（仅供内部工具）。
- **服务恢复**：停止后再启动总线会恢复此前已注册的 ABI 服务。注册、连接、断开事件始终写入日志。
- **开发文档**：详细的接入说明请参阅仓库中的 [第三方接入 API](docs/sdk.md) · [中文](docs/sdk_zh.md)。

### 📦 安装指南

1. 下载本发布页提供的 `foo_zero_bus-0.1.0.fb2k-component` 安装包。
2. 打开 foobar2000，依次点击 **文件 → 首选项 → 组件 → 安装**。
3. 安装完成后，**重启 foobar2000** 即可生效。

> **💡 提示**：安装并重启后，服务将默认监听 `ws://127.0.0.1:17890`。更多详情请查阅项目 [README](README_zh.md)。

---

### 🚀 What's New

- **Core Engine**: Introduced an in-process C++20 message bus supporting REQUEST, RESPONSE, EVENT, and NOTIFICATION models, alongside built-in timeout and cooperative cancellation mechanisms.
- **Developer Integration**: Other foobar components must integrate via the `zero_message_bus_v1` ABI. Please strictly copy the `sdk/foo_zero_bus/abi/` directory and **do not** link the bus core library.
- **Network Service**: Built-in local WebSocket listening on `127.0.0.1:17890`. This is restricted to loopback only, with no authentication required by default.
- **UI Panel**: Added a Preferences page located at **Tools → Zero Bus**. Features include port configuration, start/stop controls, live connection tracking, and an optional communication log.
- **Strict Constraint**: The Payload must be a string. You must stringify your business JSON first before sending.
- **Request Helpers**: In-process C++ SDK helpers `request_async` / `request_future` / `request_sync` / `request_awaitable` (internal tools only).
- **Service Restore**: Restarting the bus restores previously registered ABI services. Register, connect, and disconnect events are always logged.
- **Documentation**: For API details, please refer to the [English SDK](docs/sdk.md) · [中文](docs/sdk_zh.md).

### 📦 Install Guide

1. Download the `foo_zero_bus-0.1.0.fb2k-component` package from this release.
2. In foobar2000, navigate to **File → Preferences → Components → Install**.
3. **Restart foobar2000** to apply changes.

> **💡 Note**: The default listen address after installation is `ws://127.0.0.1:17890`. For more information, please read the [README](README.md).
