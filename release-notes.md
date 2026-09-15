## Zero Bus 0.1.0

产品版 `0.1.0`。一份 `.fb2k-component`：Win32 + x64（本包不含 macOS）。foobar 2.x 安装时按架构自选。

English notes below.

### 更新

- 进程内 C++20 总线：REQUEST / RESPONSE / EVENT / NOTIFICATION、超时、协作取消
- 其他 foobar 组件走 `zero_message_bus_v1` ABI（只拷仓库 `sdk/foo_zero_bus/abi/`，不要链接总线核心库）
- 本机 WebSocket：`127.0.0.1:17890`（仅回环；默认不鉴权）
- 偏好页：**工具 → Zero Bus** — 端口、启动 / 停止、当前连接、可选通信日志
- Payload 必须是字符串（业务 JSON 先 `stringify`）
- 同进程 C++ SDK 请求语法糖：`request_async` / `request_future` / `request_sync` / `request_awaitable`（仅内部工具）
- 停止后再启动会恢复已注册的 ABI 服务；注册 / 连接 / 断开始终记入日志
- 接入说明见仓库 [第三方接入 API](docs/sdk.md) · [中文](docs/sdk_zh.md)

### 安装

1. 下载本页的 `foo_zero_bus-0.1.0.fb2k-component`
2. foobar：**文件 → 首选项 → 组件 → 安装**
3. 重启 foobar2000

安装后默认监听 `ws://127.0.0.1:17890`。

说明见仓库 [README](README_zh.md)。

---

### What's new

- In-process C++20 bus: REQUEST / RESPONSE / EVENT / NOTIFICATION, timeout, cooperative cancel
- Other foobar components talk through the `zero_message_bus_v1` ABI (copy `sdk/foo_zero_bus/abi/`, do not link the bus core)
- Local WebSocket on `127.0.0.1:17890` (loopback only; default no auth)
- Preferences: **Tools → Zero Bus** — port, start / stop, live connections, optional comm log
- Payload must be a string (stringify business JSON first)
- In-process C++ SDK request helpers: `request_async` / `request_future` / `request_sync` / `request_awaitable` (internal tools only)
- Restarting the bus restores registered ABI services; register / connect / disconnect are always logged
- API notes: [English](docs/sdk.md) · [中文](docs/sdk_zh.md)

### Install

1. Download `foo_zero_bus-0.1.0.fb2k-component` from this release
2. foobar: **File → Preferences → Components → Install**
3. Restart foobar2000

Default listen address after install: `ws://127.0.0.1:17890`.

Docs: [README](README.md).
