[English](README.md) · **中文**

# Zero Bus (foo_zero_bus)

Zero Bus 是一款专为 foobar2000 设计的进程内异步消息总线。它为本机 C++ 组件和 WebSocket 客户端提供了一套统一的 Request / Response / Event 通信模型。总线的核心设计理念是只负责路由“信封 (Envelope)”，绝对不解析业务层面的 Payload。

当前版本为产品级 `0.1.2`。全面支持 Windows 环境下的 foobar2000 1.x / 2.x（涵盖 32 位、64 位与 ARM64EC），以及 macOS 11+ 系统。

本仓库主要用于托管编译好的安装包、对外提供的 ABI 头文件以及相关示例代码。您可以直接在仓库的 **[Releases](https://github.com/hehelp/foo_zero_bus/releases)** 页面获取安装包。

---

## 🚀 更新日志

### v0.1.2 (2026-09-27)

- macOS 新增偏好页：**工具 → Zero Bus**（端口、启停服务、界面语言、当前连接）及其「日志」子页，功能与 Windows 一致。
- macOS 的「跟随系统」改为按系统首选语言判断，简体中文系统默认显示中文。
- Windows DLL 补充版本信息（公司 klyrics.cn、版权声明）。

### v0.1.1 (2026-09-15)

- 增加英文语言支持。简体中文区默认中文，其他地区默认英文，也可在面板上手动切换。
- 安装包增加 Windows ARM（ARM64EC）组件，路径为 `arm64ec/foo_zero_bus.dll`。foobar2000 ARM 会优先加载它。
- macOS 组件明确同时包含 arm64 与 x86_64（Universal Binary）。
- 修复偏好页「语言」标签的位置和宽度。



### v0.1.0 (2026-09-15)

- 基于 C++20 构建的进程内总线，完整支持 REQUEST、RESPONSE、EVENT 与 NOTIFICATION，并内置超时控制与协作式取消机制。
- 其他 foobar 组件需通过 `zero_message_bus_v1` ABI 接入（仅需拷贝 `sdk/foo_zero_bus/abi/` 目录，严禁链接总线核心库）。
- 本机 WebSocket 默认监听 `127.0.0.1:17890`（仅限本机回环地址，默认不开启鉴权）。
- 新增偏好设置面板：**工具 → Zero Bus**，支持修改端口、启停服务、查看当前连接，并提供可选的通信日志查看功能。
- 严格规范：Payload 必须为纯字符串格式（业务 JSON 数据需提前执行 `stringify`）。
- 同进程 C++SDK 请求语法糖：++`request_async()`++、++`request_future()`++、++`request_sync()`++、C++20 `request_awaitable()`。仅供源码内工具和测试使用。
- 停止后再启动总线会恢复此前已注册的 ABI 服务。注册、连接、断开事件始终写入日志。

---



## 💻 运行环境与支持


| 类别        | 详细要求                                                                                 |
| --------- | ------------------------------------------------------------------------------------ |
| **操作系统**  | Windows 10 / 11；macOS 11+                                                            |
| **播放器版本** | **Windows**：foobar2000 1.x 与 2.x（支持 32 位、64 位与 ARM64EC） **Mac**：foobar2000 2.6 及以上版本 |
| **限制说明**  | 不支持在 Windows 下混用不同架构的 DLL 或在 Mac 下混用 bundle；WebSocket 仅监听本机回环地址                      |


**⚠️ 注意**：Windows 的 32 位、64 位与 ARM64EC 对应不同的 DLL 文件，严禁混用。


| 播放器版本                      | 组件文件名                    | 默认安装目录                                                        |
| -------------------------- | ------------------------ | ------------------------------------------------------------- |
| foobar2000 **32 位**        | `foo_zero_bus.dll`       | `%APPDATA%\foobar2000-v2\user-components\foo_zero_bus\`       |
| foobar2000 **64 位**        | `foo_zero_bus.dll`       | `%APPDATA%\foobar2000-v2\user-components-x64\foo_zero_bus\`   |
| foobar2000 **Windows ARM** | `foo_zero_bus.dll`       | `%APPDATA%\foobar2000-v2\user-components-arm64\foo_zero_bus\` |
| foobar2000 **Mac**         | `foo_zero_bus.component` | `~/Library/foobar2000-v2/user-components/`                    |


---



## 📦 安装指南

1. 前往本仓库的 **[Releases](https://github.com/hehelp/foo_zero_bus/releases)** 页面，下载最新的 `foo_zero_bus-0.1.2.fb2k-component` 安装包。
2. 打开 foobar2000，依次点击 **文件 → 首选项 → 组件 → 安装**，选择刚刚下载的 `.fb2k-component` 文件。（注：foobar 2.x 会根据软件位数自动选择正确版本）。
3. 或者，您也可以将对应的 DLL 文件手动拷贝至上表列出的安装目录，然后**重启** foobar2000。

*手动安装路径示例 (Windows 64位)*：

```text
C:\Users\<用户名>\AppData\Roaming\foobar2000-v2\user-components-x64\foo_zero_bus\foo_zero_bus.dll
```

安装完成后，服务会随 foobar2000 自动启动，并默认监听 `ws://127.0.0.1:17890`。如需修改端口，请前往 **文件 → 首选项 → 工具 → Zero Bus** 进行设置。

## 🛠️ 第三方开发者接入


| 你的角色 / 调用方              | 推荐接入方式                    | 接入说明                                                          |
| ----------------------- | ------------------------- | ------------------------------------------------------------- |
| **其他 foobar2000 组件**    | `zero_message_bus_v1` ABI | 仅需拷贝 `[sdk/foo_zero_bus/abi/](sdk/foo_zero_bus/abi/)` 目录下的头文件 |
| **同进程工具或测试**            | C++ SDK                   | 严禁将其链接到其他标准的 foobar 组件中                                       |
| **浏览器 / Electron / 脚本** | WebSocket 信封协议            | 默认连接至 `ws://127.0.0.1:<port>`                                 |


完整的接入说明请参阅 [中文 SDK](docs/sdk_zh.md) · [English SDK](docs/sdk.md)。ABI 头文件位于 `[sdk/foo_zero_bus/abi/](sdk/foo_zero_bus/abi/)`，示例代码请查看 `[examples/](examples/)` 目录。

**⛔ 接入方必须严格遵守以下规范：**

1. `payload` 字段必须是纯字符串；如果传递业务对象，必须先执行 `JSON.stringify`。
2. 服务回调函数必须尽快返回；阻塞时间超过 50ms 将被系统记录为违规。
3. 针对每个 `REQUEST`，只能进行一次应答。
4. Timeout 机制不会强制杀死业务线程；业务逻辑需在 handler 内部通过检查 `cancelled()` 来响应取消信号。
5. 绝对禁止使用 `detach()` 创建游离线程。
6. 严禁将 `foo_zero_bus_core` 核心库静态链接进其他的 foobar 组件中。



## ⚙️ 偏好设置面板

面板路径：**文件 → 首选项 → 工具 → Zero Bus**

- **常规设置**：支持自定义 WebSocket 端口（安全起见，仅监听 `127.0.0.1`），以及界面语言（跟随系统 / 中文 / English）。
- **服务控制**：支持手动启动或停止总线服务。
- **状态监控**：实时查看当前的活动连接数。
- **日志子页**：提供可选的通信日志记录功能、只读的控制台输出视图，以及一键清空日志功能。

