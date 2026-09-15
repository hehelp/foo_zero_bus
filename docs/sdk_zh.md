# Zero Bus 第三方接入 API 文档

[English](sdk.md) · **中文**

> **适用版本**：Zero Bus `0.1.0` / ABI v1

Zero Bus 是一款运行在 foobar2000 进程内的异步消息总线。foobar2000 本地组件通过 ABI 接入，而浏览器、Electron 及本机脚本则通过 WebSocket 接入。这两种接入方式共用同一套 `REQUEST / RESPONSE / EVENT` 消息模型。

**核心设计原则**：总线仅负责路由信封（Envelope），**绝不解析业务负载（Payload）**。无论是通过 ABI 还是 WebSocket，Payload 必须始终为字符串；若需传递 JSON 数据，业务方必须先将其序列化为 JSON 字符串后再放入 Payload。

---

## 1. 开始之前

在进行开发与调试前，请确保完成以下准备工作：

- 安装并启用 `foo_zero_bus` 插件。
- 进入 foobar2000 的 **首选项 → Tools → Zero Bus**，确认服务处于运行状态。
- 确认 WebSocket 默认监听地址为 `ws://127.0.0.1:17890`。你可以在偏好设置页修改端口，或通过环境变量 `FOO_ZERO_BUS_WS_PORT` 进行覆盖。

> **⚠️ 注意**：Zero Bus 自身不提供诸如 `plugin.localize` 等具体的业务服务，文档中的服务名仅作演示使用。在调用前，必须确保已有其他组件注册了该服务，否则将返回 `SERVICE_NOT_FOUND` 错误。

---

## 2. 选择接入方式

请根据你的开发场景选择合适的接入方式：

| 你的角色 / 调用方 | 推荐接入方式 | 所需依赖文件 |
| --- | --- | --- |
| **foobar2000 组件** | `zero_message_bus_v1` ABI | [`sdk/foo_zero_bus/abi/`](../sdk/foo_zero_bus/abi/) |
| **浏览器 / Electron / 本机脚本** | WebSocket | 无任何二进制依赖 |
| **总线源码内的工具或测试** | C++ SDK | `foo_zero_bus_api.h` + `foo_zero_bus_core` |

> **⛔ 严禁越界**：标准的 foobar2000 组件必须使用 ABI 接入，**绝对不要**将 `foo_zero_bus_core` 静态链接进你的组件中。C++ SDK 仅供总线内部源码集成、内部工具或测试使用，绝非跨 DLL 接口。

---

## 3. 消息模型

### 3.1 消息类型

| 数值 | 常量定义 | 用途说明 |
| --- | --- | --- |
| 1 | `REQUEST` / `ZERO_MSG_REQUEST` | 发起请求（需要接收方应答） |
| 2 | `RESPONSE` / `ZERO_MSG_RESPONSE` | 请求处理成功并返回应答 |
| 3 | `EVENT` / `ZERO_MSG_EVENT` | 向已注册的服务发布广播事件（无需应答） |
| 4 | `NOTIFICATION` / `ZERO_MSG_NOTIFICATION` | 向指定的端点 (endpoint) 发送单向通知（无需应答） |
| 5 | `ERROR` / `ZERO_MSG_ERROR` | 请求处理失败并返回错误 |
| 6 | `CANCEL` / `ZERO_MSG_CANCEL` | 发起协作式的请求取消信号 |

- `REQUEST` 的默认超时时间为 5000ms。
- `RESPONSE`、`ERROR` 和 `CANCEL` 消息中的 `correlation_id` 必须与原 `REQUEST` 消息的 `msg_id` 严格对应。

### 3.2 信封字段定义

| 字段名 | 字段含义与规则 |
| --- | --- |
| `sender` | 发送方标识。**ABI 调用必填**；WebSocket 调用中可省略，由网关自动覆盖。 |
| `receiver` | 接收方服务名或具体端点，**必填**。 |
| `type` | 消息类型（参考 3.1 节表格）。 |
| `msg_id` | 消息唯一 ID。WebSocket 客户端可自行生成，若省略则由网关自动生成。 |
| `correlation_id` | 仅在 `RESPONSE`、`ERROR`、`CANCEL` 中使用，指向原请求的 ID。 |
| `payload` | 不透明字符串（业务数据），允许为空字符串。 |

*服务名命名规范*：建议采用点分小写格式（例如 `plugin.localize`、`plugin.lyrics.search`）。服务名属于接入双方的业务协议范畴，Zero Bus 不关心其具体含义。

### 3.3 错误码速查表

以下数值与 ABI 的 `zero_error_code` 及 C++ SDK 的 `foo_zero_bus::error_code` 保持一致：

| 错误码 | 枚举名称 | 错误含义 |
| --- | --- | --- |
| 0 | `OK` | 成功 |
| 1 | `SERVICE_NOT_FOUND` | 目标服务未注册 |
| 2 | `SERVICE_UNHEALTHY` | 服务当前状态不健康 |
| 3 | `SERVICE_QUARANTINED` | 服务因连续违规已被总线隔离 |
| 4 | `TIMEOUT` | 请求超时 |
| 5 | `CANCELLED` | 请求已被取消 |
| 6 | `QUEUE_FULL` | 消息队列已满，触发背压机制 |
| 7 | `INVALID_ENVELOPE` | 消息信封格式错误 |
| 8 | `INVALID_MESSAGE_TYPE` | 消息类型无效 |
| 9 | `DUPLICATE_RESPONSE` | 试图对同一个请求进行多次应答 |
| 10 | `TRANSPORT_ERROR` | 底层传输失败 |
| 11 | `AUTH_FAILED` | WebSocket 鉴权失败 |
| 12 | `ACCESS_DENIED` | 无权访问目标服务 |
| 13 | `INTERNAL_ERROR` | 总线内部发生错误 |

> *WebSocket 错误响应示例*： ERROR 的 Payload 同样是一个 JSON 字符串，例如：`{"code":"TIMEOUT","message":"request timeout"}`。

---

## 4. foobar2000 组件 ABI 接入指南

### 4.1 集成头文件

请将 [`sdk/foo_zero_bus/abi/`](../sdk/foo_zero_bus/abi/) 整个目录复制到你的工程中，并将其上一级目录加入头文件搜索路径。引入时，**必须先包含 foobar2000 SDK，再包含 Zero Bus ABI**：

```cpp
#include <foobar2000.h>
#include "foo_zero_bus/abi/foo_zero_bus_abi.h"
```

*(上述头文件是严格的接入边界，切勿包含任何内部头文件)*

### 4.2 声明服务 GUID

你必须在你的组件工程中，**且仅在一个** `.cpp` **文件里**，声明该 GUID：

```cpp
#include <foobar2000.h>
#include "foo_zero_bus/abi/foo_zero_bus_abi.h"

ZERO_MESSAGE_BUS_DECLARE_GUID;
```

*(注：ABI v1 GUID 为 `{7A3E9C1D-4B62-4F8A-9C11-2E58A40DB721}`。重复定义或遗漏均会导致编译/链接失败)*。

### 4.3 获取总线实例

```cpp
service_ptr_t<zero_message_bus_v1> bus;
if (!zero_message_bus_v1::enumerate().first(bus) || !bus->running()) {
    console::print("foo_zero_bus is not running");
    return;
}
```

获取失败通常意味着插件未安装、被禁用或仍在启动中。

### 4.4 发送消息与请求

- **广播/通知**：使用 `send_message()` 处理 `EVENT` 或 `NOTIFICATION`。
- **发起请求**：使用 `send_request_async()` 发起需要应答的 `REQUEST`。

```cpp
zero_message msg{};
msg.sender = "plugin.example";
msg.receiver = "plugin.localize";
msg.type = ZERO_MSG_REQUEST;
msg.msg_id = "request-001";
msg.correlation_id = "";
msg.payload = R"({"text":"Hello","to":"zh-CN"})";

const int accepted = bus->send_request_async(&msg, &receiver, ZERO_DEFAULT_REQUEST_TIMEOUT_MS);
```

**内存生命周期管理**：

- 传入的 `zero_message` 结构体及字符串指针只需存活到该方法返回即可，总线内部会自动深拷贝所需字段。
- 应答回调（Receiver）中收到的 `zero_message` 仅在单次回调内有效。若需在后台线程使用，**业务方必须自行深拷贝**。

### 4.5 服务注册与生命周期规范

使用 `register_service()` 注册服务，在组件卸载或退出前必须调用 `unregister_service()`。

```cpp
zero_service_provider_v1 provider{};
provider.on_message = &on_message;
provider.release = &release_context;
provider.ctx = context;

if (!bus->register_service("plugin.localize", &provider)) {
    // 注册失败：通常因为服务名已被其他插件占用
}
```

**⚠️ 开发者避坑指南（极度重要）**：

1. **快速返回原则**：`on_message` 必须瞬间返回！执行超过 10ms 系统会输出警告，超过 50ms 会被判定违规并可能导致服务被隔离（Quarantined）。
2. **异步响应**：如果需要异步处理数据，请先复制 `zero_reply_v1` 并调用 `retain(ctx)`；任务执行完毕后必须调用对应的 `release(ctx)`。
3. **单次应答**：一个 `REQUEST` 只能调用一次 `reply()` 或 `error()`，重复调用将被总线拦截。
4. **禁止野线程**：绝对禁止使用 `std::thread::detach()`！必须使用具备明确生命周期管理的线程池或 Executor。
5. **异常隔离**：严禁让 C++ 异常抛出穿越 ABI 边界。

## 5. WebSocket 接入指南

### 5.1 连接与鉴权

- **地址**：`ws://127.0.0.1:17890`（仅监听本机回环地址，拒绝外网连接）。
- **鉴权**：若管理员开启了 `require_auth`，建立连接后发送的首帧必须为 `{"auth":"<token>"}`。

### 5.2 发送请求 (REQUEST)

**请务必使用 JSON 库生成信封，切勿手写转义。** 因为 Payload 必须是纯字符串，所以如果你的业务数据是对象，需要进行**两次序列化**（先序列化业务对象，再序列化整体信封）：

```javascript
const request = {
  sender: "",
  receiver: "plugin.localize",
  type: 1,
  msg_id: "req_001",
  correlation_id: "",
  // 必须在此处执行第一次 JSON.stringify
  payload: JSON.stringify({ text: "Hello", to: "zh-CN" }),
};

// 执行第二次 JSON.stringify 发送外层信封
socket.send(JSON.stringify(request));
```

> ❌ **常见错误示例**：直接将对象赋值给 payload 字段会导致 `INVALID_ENVELOPE` 错误。 `payload: { text: "Hello", to: "zh-CN" } // 错误！`

*注：WebSocket 端当前采用固定的 5000ms 默认超时，不支持在信封中自定义超时时间；Payload 默认体积上限为 1MB*。

### 5.3 接收应答与取消请求

- **处理应答**：监听返回帧，`type === 2` 为成功，`type === 5` 为失败。通过比对 `correlation_id` 与原请求的 `msg_id` 来匹配回调。
- **取消请求**：发送一条 `type === 6` (CANCEL) 的消息，并将原请求的 `msg_id` 填入 `correlation_id` 即可： `{"receiver":"plugin.localize","type":6,"correlation_id":"req_001","payload":""}`。

## 6. 同进程 C++ SDK (仅供内部工具)

> **声明**：本接口仅适用于 Zero Bus 源码内部工具、测试模块及整体源码集成，**不适用于独立的 foobar2000 组件开发**。

引入命名空间：`#include "foo_zero_bus/foo_zero_bus_api.h"` -> `namespace msgbus = foo_zero_bus;`

核心 API 概览：

- `bus::request_async(service, payload, handler, timeout_ms)`：回调式异步请求，返回 `request_handle`。
- `bus::request_future(service, payload, timeout_ms)`：返回 `std::future<response>`。
- `bus::request_sync(service, payload, timeout_ms)`：阻塞当前工作线程；失败时抛出 `request_error`。
- `bus::request_awaitable(executor, service, payload, timeout_ms)`：C++20 `co_await`，完成后从指定 executor 恢复。
- `bus::request(service, payload, timeout_ms, handler)`：发起异步请求并返回句柄（原始回调接口的兼容别名）。
- `bus::publish(service, payload)`：发布 EVENT。
- `bus::notify(endpoint, payload)`：发送 NOTIFICATION。
- `bus::cancel_request(id)`：协作式取消请求。
- `service(bus, name, executor, handler)`：注册服务（利用 RAII，析构时自动安全注销）。
- `responder::reply()` / `error()`：执行请求应答（仅限成功调用一次）。
- `executor::post()`：将任务安全推送到受控的工作线程中执行。

服务 handler 在指定 executor 上执行。超时或取消后，总线不会强制结束业务任务，应通过 `request::cancelled()` 自行退出。

`request_sync()` 只能在独立后台线程调用，不能阻塞 foobar2000 UI、Dispatcher，或目标服务完成本次请求所依赖的单线程 executor。`request_awaitable()` 必须显式提供恢复 executor，避免协程后续代码跑在 Dispatcher 上。

```cpp
auto handle = bus.request_async(
    "plugin.localize",
    R"({"text":"Hello"})",
    [](msgbus::response value) {
        if (value.ok()) {
            use_result(value.payload());
        } else {
            report_error(value.code(), value.message());
        }
    },
    5000);

const std::string translated = bus.request_sync(
    "plugin.localize",
    R"({"text":"Hello"})",
    5000);

std::string result = co_await bus.request_awaitable(
    workers,
    "plugin.localize",
    R"({"text":"Hello"})",
    5000);
```

完整示例见 [`example_sdk.cpp`](../examples/cpp/inproc/example_sdk.cpp)。

## 7. 常见问题排查 (FAQ)

- **WebSocket 无法连接？** 请确认 foobar2000 正在运行。前往 **首选项 → Tools → Zero Bus** 检查服务是否启动及端口配置。需注意 `FOO_ZERO_BUS_WS_PORT` 环境变量可能会覆盖 UI 面板的端口设置。
- **频繁收到** `SERVICE_NOT_FOUND`**？** 目标服务未注册，或者提供该服务的组件尚未完成启动加载。
- **频繁收到** `INVALID_ENVELOPE`**？** 请严格检查 WebSocket 发送帧。必须使用 `JSON.stringify()` 生成外层信封，确保 `receiver` 非空，且 `payload` 是一个纯字符串而非 JSON Object。
- **请求总是触发** `TIMEOUT`**？** 接收方服务必须针对每个 `REQUEST` 调用一次 `reply()` 或 `error()`。需注意，总线触发超时仅仅意味着结束了总线内的排队/等待状态，它**不会**主动杀死仍在后台运行的业务线程。
- **服务状态变成了** `UNHEALTHY` **或** `QUARANTINED`**？** 这说明你的服务回调阻塞了总线的分发线程。请立刻将所有的网络请求、文件 IO 等耗时任务移交给自己管理的 `executor` 线程池处理，确保总线的入口回调瞬间返回。

## 8. 官方示例索引

| 示例文件路径 | 演示内容说明 |
| --- | --- |
| [`zero_bus_guid.cpp`](../examples/cpp/foobar_plugin/zero_bus_guid.cpp) | GUID 定义示范（每个组件仅需编译一次） |
| [`example_service.cpp`](../examples/cpp/foobar_plugin/example_service.cpp) | 注册并实现 `plugin.localize` 服务端 |
| [`example_client.cpp`](../examples/cpp/foobar_plugin/example_client.cpp) | 发起 ABI `REQUEST` 请求客户端 |
| [`example_sdk.cpp`](../examples/cpp/inproc/example_sdk.cpp) | 同进程 C++ SDK 的使用方法，含请求语法糖 |
| [`example.html`](../examples/websocket/example.html) | 基于浏览器的 WebSocket 调试页 |
| [`example.mjs`](../examples/websocket/example.mjs) | Node.js / Electron 环境下的调用客户端 |
