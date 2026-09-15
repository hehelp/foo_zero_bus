# Zero Bus Third-Party Integration API

**English** · [中文](sdk_zh.md)

> **Version**: Zero Bus `0.1.0` / ABI v1

Zero Bus is an in-process asynchronous message bus that runs inside foobar2000. Native foobar2000 components connect through the ABI. Browsers, Electron, and local scripts connect through WebSocket. Both paths share the same `REQUEST / RESPONSE / EVENT` message model.

**Design rule**: the bus routes envelopes only and **never parses the business payload**. Payload is always a string. If you need to send JSON, serialize it first, then put the resulting string into the payload.

---

## 1. Before You Start

- Install and enable the `foo_zero_bus` component.
- Open **Preferences → Tools → Zero Bus** and confirm the service is running.
- The default WebSocket address is `ws://127.0.0.1:17890`. Change the port on the preferences page, or override it with `FOO_ZERO_BUS_WS_PORT`.

> Zero Bus does not provide business services such as `plugin.localize`. Names in this document are examples. If the target service is not registered, the bus returns `SERVICE_NOT_FOUND`.

---

## 2. Choosing the Integration Method

| Caller | Recommended Path | Required Files |
| --- | --- | --- |
| **foobar2000 component** | `zero_message_bus_v1` ABI | [`sdk/foo_zero_bus/abi/`](../sdk/foo_zero_bus/abi/) |
| **Browser / Electron / local script** | WebSocket | No binary dependency |
| **Internal tools or tests** | C++ SDK | `foo_zero_bus_api.h` + `foo_zero_bus_core` |

> Regular foobar2000 components must use the ABI. **Do not** statically link `foo_zero_bus_core` into your component. The C++ SDK is only for source-tree tools, tests, or a full source integration. It is not a cross-DLL interface.

---

## 3. Message Model

### 3.1 Message Types

| Value | Constant | Purpose |
| --- | --- | --- |
| 1 | `REQUEST` / `ZERO_MSG_REQUEST` | Start a request that expects a response |
| 2 | `RESPONSE` / `ZERO_MSG_RESPONSE` | Successful reply |
| 3 | `EVENT` / `ZERO_MSG_EVENT` | Broadcast to a registered service (no reply) |
| 4 | `NOTIFICATION` / `ZERO_MSG_NOTIFICATION` | One-way message to a specific endpoint (no reply) |
| 5 | `ERROR` / `ZERO_MSG_ERROR` | Failed reply |
| 6 | `CANCEL` / `ZERO_MSG_CANCEL` | Cooperative cancel signal |

- Default `REQUEST` timeout is 5000 ms.
- `correlation_id` on `RESPONSE`, `ERROR`, and `CANCEL` must match the original `REQUEST` `msg_id`.

### 3.2 Envelope Fields

| Field | Meaning |
| --- | --- |
| `sender` | Sender id. **Required for ABI**. Optional for WebSocket; the gateway overwrites it. |
| `receiver` | Target service name or endpoint. **Required**. |
| `type` | Message type from section 3.1. |
| `msg_id` | Unique message id. WebSocket clients may generate it; the gateway fills it in if omitted. |
| `correlation_id` | Used only by `RESPONSE`, `ERROR`, and `CANCEL` to point back to the request. |
| `payload` | Opaque string. Empty string is allowed. |

Service names should use lowercase dotted form, for example `plugin.localize` or `plugin.lyrics.search`. The bus does not interpret the name.

### 3.3 Error Codes

These values match ABI `zero_error_code` and the C++ SDK `foo_zero_bus::error_code`:

| Code | Name | Meaning |
| --- | --- | --- |
| 0 | `OK` | Success |
| 1 | `SERVICE_NOT_FOUND` | Target service is not registered |
| 2 | `SERVICE_UNHEALTHY` | Service is currently unhealthy |
| 3 | `SERVICE_QUARANTINED` | Service was quarantined after repeated violations |
| 4 | `TIMEOUT` | Request timed out |
| 5 | `CANCELLED` | Request was cancelled |
| 6 | `QUEUE_FULL` | Queue is full; backpressure applied |
| 7 | `INVALID_ENVELOPE` | Envelope is invalid |
| 8 | `INVALID_MESSAGE_TYPE` | Message type is invalid |
| 9 | `DUPLICATE_RESPONSE` | More than one reply was attempted |
| 10 | `TRANSPORT_ERROR` | Transport failed |
| 11 | `AUTH_FAILED` | WebSocket authentication failed |
| 12 | `ACCESS_DENIED` | Not allowed to access the target service |
| 13 | `INTERNAL_ERROR` | Internal bus error |

WebSocket `ERROR` payloads are also JSON strings, for example `{"code":"TIMEOUT","message":"request timeout"}`.

---

## 4. foobar2000 Component ABI

### 4.1 Headers

Copy the entire [`sdk/foo_zero_bus/abi/`](../sdk/foo_zero_bus/abi/) directory into your project and add its parent directory to the include path. Include the foobar2000 SDK first, then the Zero Bus ABI:

```cpp
#include <foobar2000.h>
#include "foo_zero_bus/abi/foo_zero_bus_abi.h"
```

These headers are the integration boundary. Do not include internal headers.

### 4.2 Declare the Service GUID

Define the GUID in **exactly one** `.cpp` file in your component:

```cpp
#include <foobar2000.h>
#include "foo_zero_bus/abi/foo_zero_bus_abi.h"

ZERO_MESSAGE_BUS_DECLARE_GUID;
```

ABI v1 GUID: `{7A3E9C1D-4B62-4F8A-9C11-2E58A40DB721}`. Defining it twice or omitting it will fail at compile or link time.

### 4.3 Locate the Bus

```cpp
service_ptr_t<zero_message_bus_v1> bus;
if (!zero_message_bus_v1::enumerate().first(bus) || !bus->running()) {
    console::print("foo_zero_bus is not running");
    return;
}
```

Failure usually means the component is not installed, is disabled, or is still starting.

### 4.4 Send Messages and Requests

- Broadcast or notify with `send_message()` (`EVENT` or `NOTIFICATION`).
- Start a request that needs a reply with `send_request_async()`.

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

Lifetime rules:

- The `zero_message` and its string pointers only need to stay alive until the method returns. The bus deep-copies what it needs.
- A `zero_message` received in a reply callback is valid only for that callback. Deep-copy it before using it on another thread.

### 4.5 Register a Service

Register with `register_service()`. Call `unregister_service()` before the component unloads.

```cpp
zero_service_provider_v1 provider{};
provider.on_message = &on_message;
provider.release = &release_context;
provider.ctx = context;

if (!bus->register_service("plugin.localize", &provider)) {
    // Usually the service name is already taken.
}
```

Rules:

1. `on_message` must return immediately. Over 10 ms produces a warning. Over 50 ms is a violation and may quarantine the service.
2. For async work, copy `zero_reply_v1` and call `retain(ctx)`. Call `release(ctx)` when the work finishes.
3. A `REQUEST` may call `reply()` or `error()` only once.
4. Do not use `std::thread::detach()`. Use a thread pool or executor with a defined lifetime.
5. Do not let C++ exceptions cross the ABI boundary.

---

## 5. WebSocket

### 5.1 Connect and Authenticate

- Address: `ws://127.0.0.1:17890` (loopback only).
- If `require_auth` is enabled, the first frame after connect must be `{"auth":"<token>"}`.

### 5.2 Send a REQUEST

Generate the envelope with a JSON library. Do not hand-write escapes. Payload must be a string, so business objects need **two** serializations:

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

Do not assign an object to `payload`. That produces `INVALID_ENVELOPE`.

The WebSocket endpoint currently uses a fixed 5000 ms timeout and does not accept a custom timeout in the envelope. Default payload limit is 1 MB.

### 5.3 Receive Replies and Cancel

- `type === 2` is success. `type === 5` is failure. Match `correlation_id` to the original `msg_id`.
- To cancel, send `type === 6` and put the original `msg_id` in `correlation_id`:

```json
{"receiver":"plugin.localize","type":6,"correlation_id":"req_001","payload":""}
```

---

## 6. In-Process C++ SDK (Internal Tools Only)

This interface is for Zero Bus source-tree tools, tests, and full source integration. It is **not** for standalone foobar2000 components.

```cpp
#include "foo_zero_bus/foo_zero_bus_api.h"
namespace msgbus = foo_zero_bus;
```

| API | Purpose |
| --- | --- |
| `bus::request_async(service, payload, handler, timeout_ms)` | Callback-style async request; returns `request_handle` |
| `bus::request_future(service, payload, timeout_ms)` | Returns `std::future<response>` |
| `bus::request_sync(service, payload, timeout_ms)` | Blocks the worker thread; throws `request_error` on failure |
| `bus::request_awaitable(executor, service, payload, timeout_ms)` | C++20 `co_await`; resumes on the given executor |
| `bus::request(...)` | Compatibility alias for the original callback API |
| `bus::publish(service, payload)` | Publish an EVENT |
| `bus::notify(endpoint, payload)` | Send a NOTIFICATION |
| `bus::cancel_request(id)` | Cooperative cancel |
| `service(bus, name, executor, handler)` | Register a service; unregisters on destruction |
| `responder::reply()` / `error()` | Complete a request; succeeds only once |
| `executor::post()` | Post work to a managed worker thread |

Service handlers run on the supplied executor. After timeout or cancel, the bus does not kill business work. Check `request::cancelled()` and exit yourself.

`request_sync()` may run only on a dedicated background thread. Do not block the foobar2000 UI thread, the bus Dispatcher, or the single-thread executor that the target service needs to finish the request. `request_awaitable()` requires an explicit resume executor so continuation code does not run on the Dispatcher.

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

See [`example_sdk.cpp`](../examples/cpp/inproc/example_sdk.cpp).

---

## 7. FAQ

- **WebSocket will not connect?** Confirm foobar2000 is running. Check **Preferences → Tools → Zero Bus** for service state and port. `FOO_ZERO_BUS_WS_PORT` can override the UI port.
- **Frequent `SERVICE_NOT_FOUND`?** The target service is not registered, or the providing component has not finished loading.
- **Frequent `INVALID_ENVELOPE`?** Build the outer envelope with `JSON.stringify()`. `receiver` must be non-empty. `payload` must be a string, not a JSON object.
- **Requests always time out?** The service must call `reply()` or `error()` once for every `REQUEST`. Timeout only ends the wait inside the bus. It does not kill background work.
- **Service became `UNHEALTHY` or `QUARANTINED`?** The callback blocked the Dispatcher. Move network I/O and file I/O to a managed executor so the entry callback returns immediately.

---

## 8. Examples

| File | What it shows |
| --- | --- |
| [`zero_bus_guid.cpp`](../examples/cpp/foobar_plugin/zero_bus_guid.cpp) | GUID definition (compile in one file per component) |
| [`example_service.cpp`](../examples/cpp/foobar_plugin/example_service.cpp) | Register and implement `plugin.localize` |
| [`example_client.cpp`](../examples/cpp/foobar_plugin/example_client.cpp) | ABI `REQUEST` client |
| [`example_sdk.cpp`](../examples/cpp/inproc/example_sdk.cpp) | In-process C++ SDK, including request helpers |
| [`example.html`](../examples/websocket/example.html) | Browser WebSocket debug page |
| [`example.mjs`](../examples/websocket/example.mjs) | Node.js / Electron client |
