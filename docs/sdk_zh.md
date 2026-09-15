# Zero Bus 第三方接入 API

[English](sdk.md) · **中文**

> **适用版本**：Zero Bus `0.1.0` / ABI v1

Zero Bus 是运行在 foobar2000 进程内的异步消息总线。本机组件通过 ABI 接入，浏览器、Electron 和本机脚本通过 WebSocket 接入。两条路径共用同一套 `REQUEST / RESPONSE / EVENT` 消息模型。

**设计原则**：总线只路由信封，**绝不解析业务 Payload**。无论 ABI 还是 WebSocket，Payload 都必须是字符串。如果要传 JSON，先序列化成字符串，再放入 Payload。

---

## 1. 开始之前

- 安装并启用 `foo_zero_bus` 组件。
- 打开 **首选项 → 工具 → Zero Bus**，确认服务在运行。
- WebSocket 默认地址是 `ws://127.0.0.1:17890`。可在偏好页改端口，也可用环境变量 `FOO_ZERO_BUS_WS_PORT` 覆盖。

> Zero Bus 本身不提供 `plugin.localize` 这类业务服务。文档里的服务名只是示例。目标服务未注册时，会返回 `SERVICE_NOT_FOUND`。

---

## 2. 选择接入方式

| 调用方 | 推荐方式 | 所需文件 |
| --- | --- | --- |
| **foobar2000 组件** | `zero_message_bus_v1` ABI | [`sdk/foo_zero_bus/abi/`](../sdk/foo_zero_bus/abi/) |
| **浏览器 / Electron / 本机脚本** | WebSocket | 无二进制依赖 |
| **内部工具或测试** | C++ SDK | `foo_zero_bus_api.h` + `foo_zero_bus_core` |

> 常规 foobar2000 组件必须走 ABI。**不要**把 `foo_zero_bus_core` 静态链接进你的组件。C++ SDK 只用于源码内工具、测试或整体源码集成，不是跨 DLL 接口。

---

## 3. 消息模型

### 3.1 消息类型

| 数值 | 常量 | 用途 |
| --- | --- | --- |
| 1 | `REQUEST` / `ZERO_MSG_REQUEST` | 发起请求，需要对方应答 |
| 2 | `RESPONSE` / `ZERO_MSG_RESPONSE` | 请求成功并返回应答 |
| 3 | `EVENT` / `ZERO_MSG_EVENT` | 向已注册服务广播（无需应答） |
| 4 | `NOTIFICATION` / `ZERO_MSG_NOTIFICATION` | 向指定端点发送单向通知（无需应答） |
| 5 | `ERROR` / `ZERO_MSG_ERROR` | 请求失败并返回错误 |
| 6 | `CANCEL` / `ZERO_MSG_CANCEL` | 协作式取消信号 |

- `REQUEST` 默认超时 5000ms。
- `RESPONSE`、`ERROR`、`CANCEL` 的 `correlation_id` 必须对应原 `REQUEST` 的 `msg_id`。

### 3.2 信封字段

| 字段 | 含义 |
| --- | --- |
| `sender` | 发送方标识。**ABI 必填**。WebSocket 可省略，由网关覆盖。 |
| `receiver` | 目标服务名或端点，**必填**。 |
| `type` | 消息类型，见 3.1。 |
| `msg_id` | 消息唯一 ID。WebSocket 客户端可自行生成；省略时由网关补齐。 |
| `correlation_id` | 仅用于 `RESPONSE`、`ERROR`、`CANCEL`，指向原请求。 |
| `payload` | 不透明字符串，允许为空。 |

服务名建议用点分小写，例如 `plugin.localize`、`plugin.lyrics.search`。总线不解释服务名含义。

### 3.3 错误码

以下数值与 ABI 的 `zero_error_code`、C++ SDK 的 `foo_zero_bus::error_code` 一致：

| 错误码 | 名称 | 含义 |
| --- | --- | --- |
| 0 | `OK` | 成功 |
| 1 | `SERVICE_NOT_FOUND` | 目标服务未注册 |
| 2 | `SERVICE_UNHEALTHY` | 服务当前不健康 |
| 3 | `SERVICE_QUARANTINED` | 服务因连续违规已被隔离 |
| 4 | `TIMEOUT` | 请求超时 |
| 5 | `CANCELLED` | 请求已取消 |
| 6 | `QUEUE_FULL` | 队列已满，触发背压 |
| 7 | `INVALID_ENVELOPE` | 信封格式无效 |
| 8 | `INVALID_MESSAGE_TYPE` | 消息类型无效 |
| 9 | `DUPLICATE_RESPONSE` | 对同一请求重复应答 |
| 10 | `TRANSPORT_ERROR` | 底层传输失败 |
| 11 | `AUTH_FAILED` | WebSocket 鉴权失败 |
| 12 | `ACCESS_DENIED` | 无权访问目标服务 |
| 13 | `INTERNAL_ERROR` | 总线内部错误 |

WebSocket 的 `ERROR` Payload 同样是 JSON 字符串，例如 `{"code":"TIMEOUT","message":"request timeout"}`。

---

## 4. foobar2000 组件 ABI

### 4.1 头文件

把 [`sdk/foo_zero_bus/abi/`](../sdk/foo_zero_bus/abi/) 整个目录拷进工程，并把它的上一级目录加入头文件搜索路径。必须先包含 foobar2000 SDK，再包含 Zero Bus ABI：

```cpp
#include <foobar2000.h>
#include "foo_zero_bus/abi/foo_zero_bus_abi.h"
```

这些头文件就是接入边界，不要再包含内部头文件。

### 4.2 声明服务 GUID

在接入组件中，**仅在一个** `.cpp` **文件里**定义该 GUID：

```cpp
#include <foobar2000.h>
#include "foo_zero_bus/abi/foo_zero_bus_abi.h"

ZERO_MESSAGE_BUS_DECLARE_GUID;
```

ABI v1 GUID 为 `{7A3E9C1D-4B62-4F8A-9C11-2E58A40DB721}`。重复定义或遗漏都会导致编译或链接失败。

### 4.3 获取总线实例

```cpp
service_ptr_t<zero_message_bus_v1> bus;
if (!zero_message_bus_v1::enumerate().first(bus) || !bus->running()) {
    console::print("foo_zero_bus is not running");
    return;
}
```

获取失败通常表示插件未安装、被禁用，或仍在启动。

### 4.4 发送消息与请求

- 广播或通知用 `send_message()`（`EVENT` 或 `NOTIFICATION`）。
- 需要应答的请求用 `send_request_async()`。

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

生命周期：

- 传入的 `zero_message` 和字符串指针只需活到方法返回。总线会深拷贝所需字段。
- 应答回调里收到的 `zero_message` 只在当次回调内有效。要到其他线程使用，必须自行深拷贝。

### 4.5 注册服务

用 `register_service()` 注册。组件卸载前必须调用 `unregister_service()`。

```cpp
zero_service_provider_v1 provider{};
provider.on_message = &on_message;
provider.release = &release_context;
provider.ctx = context;

if (!bus->register_service("plugin.localize", &provider)) {
    // 通常是服务名已被占用。
}
```

规则：

1. `on_message` 必须立刻返回。超过 10ms 会告警，超过 50ms 记违规，可能导致服务被隔离。
2. 异步处理时，先复制 `zero_reply_v1` 并调用 `retain(ctx)`；完成后必须调用 `release(ctx)`。
3. 一个 `REQUEST` 只能调用一次 `reply()` 或 `error()`。
4. 禁止 `std::thread::detach()`。使用有明确生命周期的线程池或 executor。
5. 不要让 C++ 异常穿过 ABI 边界。

---

## 5. WebSocket

### 5.1 连接与鉴权

- 地址：`ws://127.0.0.1:17890`（仅本机回环）。
- 若开启了 `require_auth`，建连后的首帧必须是 `{"auth":"<token>"}`。

### 5.2 发送 REQUEST

用 JSON 库生成信封，不要手写转义。Payload 必须是字符串，所以业务对象需要**两次序列化**：

```javascript
const request = {
  sender: "",
  receiver: "plugin.localize",
  type: 1,
  msg_id: "req_001",
  correlation_id: "",
  payload: JSON.stringify({ text: "Hello", to: "zh-CN" }),
};

socket.send(JSON.stringify(request));
```

不要把对象直接赋给 `payload`，否则会得到 `INVALID_ENVELOPE`。

WebSocket 端当前固定 5000ms 超时，信封里不能自定义超时。Payload 默认上限 1MB。

### 5.3 接收应答与取消

- `type === 2` 表示成功，`type === 5` 表示失败。用 `correlation_id` 匹配原请求的 `msg_id`。
- 取消时发送 `type === 6`，并把原 `msg_id` 填进 `correlation_id`：

```json
{"receiver":"plugin.localize","type":6,"correlation_id":"req_001","payload":""}
```

---

## 6. 同进程 C++ SDK（仅供内部工具）

本接口用于 Zero Bus 源码内的工具、测试或整体源码集成，**不适用于独立 foobar2000 组件**。

```cpp
#include "foo_zero_bus/foo_zero_bus_api.h"
namespace msgbus = foo_zero_bus;
```

| API | 用途 |
| --- | --- |
| `bus::request_async(service, payload, handler, timeout_ms)` | 回调式异步请求，返回 `request_handle` |
| `bus::request_future(service, payload, timeout_ms)` | 返回 `std::future<response>` |
| `bus::request_sync(service, payload, timeout_ms)` | 阻塞工作线程；失败时抛出 `request_error` |
| `bus::request_awaitable(executor, service, payload, timeout_ms)` | C++20 `co_await`，从指定 executor 恢复 |
| `bus::request(...)` | 原始回调接口的兼容别名 |
| `bus::publish(service, payload)` | 发布 EVENT |
| `bus::notify(endpoint, payload)` | 发送 NOTIFICATION |
| `bus::cancel_request(id)` | 协作式取消 |
| `service(bus, name, executor, handler)` | 注册服务，析构时自动注销 |
| `responder::reply()` / `error()` | 完成请求，只能成功一次 |
| `executor::post()` | 向受控工作线程提交任务 |

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

---

## 7. 常见问题

- **WebSocket 连不上？** 确认 foobar2000 正在运行。到 **首选项 → 工具 → Zero Bus** 检查服务和端口。`FOO_ZERO_BUS_WS_PORT` 可能覆盖界面上的端口。
- **经常收到 `SERVICE_NOT_FOUND`？** 目标服务未注册，或提供该服务的组件还没加载完。
- **经常收到 `INVALID_ENVELOPE`？** 外层信封必须用 `JSON.stringify()` 生成。`receiver` 不能为空。`payload` 必须是字符串，不能是 JSON 对象。
- **请求总是超时？** 服务必须对每个 `REQUEST` 调用一次 `reply()` 或 `error()`。超时只结束总线内的等待，不会杀死后台业务线程。
- **服务变成 `UNHEALTHY` 或 `QUARANTINED`？** 回调阻塞了 Dispatcher。把网络和文件 IO 移到受控 executor，保证入口回调立刻返回。

---

## 8. 官方示例

| 文件 | 说明 |
| --- | --- |
| [`zero_bus_guid.cpp`](../examples/cpp/foobar_plugin/zero_bus_guid.cpp) | GUID 定义（每个组件只编译一次） |
| [`example_service.cpp`](../examples/cpp/foobar_plugin/example_service.cpp) | 注册并实现 `plugin.localize` |
| [`example_client.cpp`](../examples/cpp/foobar_plugin/example_client.cpp) | ABI `REQUEST` 客户端 |
| [`example_sdk.cpp`](../examples/cpp/inproc/example_sdk.cpp) | 同进程 C++ SDK，含请求语法糖 |
| [`example.html`](../examples/websocket/example.html) | 浏览器 WebSocket 调试页 |
| [`example.mjs`](../examples/websocket/example.mjs) | Node.js / Electron 客户端 |
