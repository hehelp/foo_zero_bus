[English](README.md) · **中文**

# Zero Bus (foo_zero_bus)

Zero Bus 是一款专为 foobar2000 设计的进程内异步消息总线。它为本机 C++ 组件和 WebSocket 客户端提供统一的 Request / Response / Event 通信模型。总线只路由信封（Envelope），绝不解析业务 Payload。

当前版本为产品级 `0.1.0`。支持 Windows 上的 foobar2000 1.x / 2.x（32 位与 64 位），以及 macOS 11+。

本仓库托管编译好的安装包、对外 ABI 头文件和示例代码。请到 **[Releases](https://github.com/hehelp/foo_zero_bus/releases)** 页面下载安装包。

---

## 更新日志

### v0.1.0 (2026-09-15)

- 基于 C++20 的进程内总线，完整支持 REQUEST、RESPONSE、EVENT 与 NOTIFICATION，并内置超时与协作式取消。
- 其他 foobar 组件通过 `zero_message_bus_v1` ABI 接入。只需拷贝 `sdk/foo_zero_bus/abi/`，不要静态链接总线核心库。
- 本机 WebSocket 默认监听 `127.0.0.1:17890`（仅回环；默认不开启鉴权）。
- 偏好设置页：**工具 → Zero Bus**。可修改端口、启停服务、查看当前连接，并可选查看通信日志。
- Payload 必须是纯字符串。业务 JSON 需先序列化再放入 Payload。
- 同进程 C++ SDK 请求语法糖：`request_async()`、`request_future()`、`request_sync()`、C++20 `request_awaitable()`。仅供源码内工具和测试使用。
- 停止后再启动总线会恢复此前已注册的 ABI 服务。注册、连接、断开事件始终写入日志。

---

## 运行环境

| 类别 | 详细要求 |
| --- | --- |
| **操作系统** | Windows 10 / 11；macOS 11+ |
| **播放器版本** | **Windows**：foobar2000 1.x 与 2.x（32 位与 64 位）。**Mac**：foobar2000 2.6 及以上 |
| **限制说明** | 不要在 Windows 下混用不同位数的 DLL，也不要在 Mac 下混用 bundle。WebSocket 仅监听本机回环 |

Windows 的 32 位与 64 位对应不同的 DLL，严禁混用。

| 播放器版本 | 组件文件名 | 默认安装目录 |
| --- | --- | --- |
| foobar2000 **32 位** | `foo_zero_bus.dll` | `%APPDATA%\foobar2000-v2\user-components\foo_zero_bus\` |
| foobar2000 **64 位** | `foo_zero_bus.dll` | `%APPDATA%\foobar2000-v2\user-components-x64\foo_zero_bus\` |
| foobar2000 **Mac** | `foo_zero_bus.component` | `~/Library/foobar2000-v2/user-components/` |

---

## 安装

1. 打开 [Releases](https://github.com/hehelp/foo_zero_bus/releases)，下载 `foo_zero_bus-0.1.0.fb2k-component`。
2. 在 foobar2000 中：**文件 → 首选项 → 组件 → 安装**，选择刚下载的 `.fb2k-component`。foobar2000 2.x 会按软件位数自动选择正确版本。
3. 也可以把对应 DLL 手动拷到上表目录，然后**重启** foobar2000。

64 位 Windows 常见路径：

```text
C:\Users\<用户名>\AppData\Roaming\foobar2000-v2\user-components-x64\foo_zero_bus\foo_zero_bus.dll
```

安装后，服务会随 foobar2000 自动启动，默认监听 `ws://127.0.0.1:17890`。如需改端口，请到 **文件 → 首选项 → 工具 → Zero Bus**。

## 第三方接入

| 调用方 | 推荐方式 | 说明 |
| --- | --- | --- |
| **其他 foobar2000 组件** | `zero_message_bus_v1` ABI | 拷贝 [`sdk/foo_zero_bus/abi/`](sdk/foo_zero_bus/abi/) 下的头文件 |
| **同进程工具或测试** | C++ SDK | 不要链接到其他标准 foobar 组件 |
| **浏览器 / Electron / 脚本** | WebSocket 信封 | 连接 `ws://127.0.0.1:<port>` |

完整接入说明：[中文 SDK](docs/sdk_zh.md) · [English SDK](docs/sdk.md)。示例见 [`examples/`](examples/)。

接入方必须遵守：

1. `payload` 必须是纯字符串。业务对象要先 `JSON.stringify`。
2. 服务回调必须尽快返回。阻塞超过 50ms 记为违规。
3. 每个 `REQUEST` 只能应答一次。
4. Timeout 不会强制结束业务线程。handler 里要检查 `cancelled()`。
5. 禁止用 `detach()` 创建游离线程。
6. 不要把 `foo_zero_bus_core` 静态链接进其他 foobar 组件。

## 偏好设置

路径：**文件 → 首选项 → 工具 → Zero Bus**

- **常规设置**：自定义 WebSocket 端口（只监听 `127.0.0.1`）。
- **服务控制**：手动启动或停止总线。
- **状态监控**：查看当前活动连接数。
- **日志**子页：可选通信日志、只读控制台、一键清空。
