# Zero Bus Third-Party Integration API

**English** · [中文](sdk_zh.md)

> **Applicable Version**: Zero Bus `0.1.2` / ABI v1

Zero Bus is an in-process asynchronous message bus running within foobar2000. Native foobar2000 components connect via ABI, while browsers, Electron, and native scripts connect via WebSocket. Both connection methods share the exact same `REQUEST / RESPONSE / EVENT` message model.

**Core Design Principle**: The bus is solely responsible for routing envelopes and will **never parse the business Payload**. Whether using ABI or WebSocket, the Payload must always be a string; if JSON data needs to be transmitted, the business side must serialize it into a JSON string before placing it into the Payload.

---

## 1. Before You Start

Before developing and debugging, please ensure the following preparations are complete:

- Install and enable the `foo_zero_bus` component.
- Navigate to **Preferences → Tools → Zero Bus** in foobar2000 and confirm the service is running.
- Verify that the default WebSocket listening address is `ws://127.0.0.1:17890`. You can change the port in the preferences page or override it using the `FOO_ZERO_BUS_WS_PORT` environment variable.

> **⚠️ Note**: Zero Bus itself does not provide specific business services such as `plugin.localize`; service names in this documentation are for demonstration purposes only. Before making a call, you must ensure another component has registered the corresponding service, otherwise a `SERVICE_NOT_FOUND` error will be returned.

---

## 2. Choosing the Integration Method

Please select the appropriate integration method based on your development scenario:

| Your Role / Caller | Recommended Integration | Required Dependencies |
| --- | --- | --- |
| **foobar2000 Component** | `zero_message_bus_v1` ABI | [`sdk/foo_zero_bus/abi/`](../sdk/foo_zero_bus/abi/) |
| **Browser / Electron / Native Script** | WebSocket | No binary dependencies |
| **Internal Tools or Tests** | C++ SDK | `foo_zero_bus_api.h` + `foo_zero_bus_core` |

> **⛔ Strict Boundary**: Standard foobar2000 components must connect using the ABI; **never** statically link `foo_zero_bus_core` into your component. The C++ SDK is strictly for internal source code integration, internal tools, or testing, and is not a cross-DLL interface.

---

## 3. Message Model

### 3.1 Message Types

| Value | Constant | Purpose |
| --- | --- | --- |
| 1 | `REQUEST` / `ZERO_MSG_REQUEST` | Initiates a request (requires a response from the receiver) |
| 2 | `RESPONSE` / `ZERO_MSG_RESPONSE` | Request processed successfully, returning a response |
| 3 | `EVENT` / `ZERO_MSG_EVENT` | Publishes a broadcast event to registered services (no response required) |
| 4 | `NOTIFICATION` / `ZERO_MSG_NOTIFICATION` | Sends a one-way notification to a specific endpoint (no response required) |
| 5 | `ERROR` / `ZERO_MSG_ERROR` | Request processing failed, returning an error |
| 6 | `CANCEL` / `ZERO_MSG_CANCEL` | Initiates a cooperative request cancellation signal |

- The default timeout for a `REQUEST` is 5000ms.
- The `correlation_id` in `RESPONSE`, `ERROR`, and `CANCEL` messages must strictly match the `msg_id` of the original `REQUEST` message.

### 3.2 Envelope Fields

| Field | Meaning and Rules |
| --- | --- |
| `sender` | Sender identifier. **Required for ABI calls**; can be omitted in WebSocket calls, automatically overwritten by the gateway. |
| `receiver` | Receiver service name or specific endpoint, **Required**. |
| `type` | Message type (refer to the table in 3.1). |
| `msg_id` | Unique message ID. WebSocket clients can generate this themselves; if omitted, the gateway generates it automatically. |
| `correlation_id` | Used only in `RESPONSE`, `ERROR`, and `CANCEL` to point back to the original request ID. |
| `payload` | Opaque string (business data), allowed to be an empty string. |

*Service Naming Convention*: Dot-separated lowercase format is recommended (e.g., `plugin.localize`, `plugin.lyrics.search`). Service names fall under the business protocol between communicating parties; Zero Bus does not interpret their specific meanings.

### 3.3 Error Codes Reference

The following values are consistent with the ABI's `zero_error_code` and the C++ SDK's `foo_zero_bus::error_code`:

| Code | Enumeration Name | Error Meaning |
| --- | --- | --- |
| 0 | `OK` | Success |
| 1 | `SERVICE_NOT_FOUND` | Target service is not registered |
| 2 | `SERVICE_UNHEALTHY` | Service state is currently unhealthy |
| 3 | `SERVICE_QUARANTINED` | Service has been quarantined by the bus due to consecutive violations |
| 4 | `TIMEOUT` | Request timed out |
| 5 | `CANCELLED` | Request has been cancelled |
| 6 | `QUEUE_FULL` | Message queue is full, triggering backpressure |
| 7 | `INVALID_ENVELOPE` | Message envelope format is invalid |
| 8 | `INVALID_MESSAGE_TYPE` | Message type is invalid |
| 9 | `DUPLICATE_RESPONSE` | Attempted to respond multiple times to the same request |
| 10 | `TRANSPORT_ERROR` | Underlying transport failed |
| 11 | `AUTH_FAILED` | WebSocket authentication failed |
| 12 | `ACCESS_DENIED` | Unauthorized to access the target service |
| 13 | `INTERNAL_ERROR` | Internal error occurred within the bus |

> *WebSocket Error Response Example*: The Payload for an ERROR is also a JSON string, e.g., `{"code":"TIMEOUT","message":"request timeout"}`.

---

## 4. foobar2000 Component ABI Integration Guide

### 4.1 Including Headers

Please copy the entire [`sdk/foo_zero_bus/abi/`](../sdk/foo_zero_bus/abi/) directory into your project and add its parent directory to your header search paths. When including, **you must include the foobar2000 SDK first, followed by the Zero Bus ABI**:

```cpp
#include <foobar2000.h>
#include "foo_zero_bus/abi/foo_zero_bus_abi.h"
```

*(The above headers represent a strict integration boundary; do not include any internal headers)*.

### 4.2 Declaring the Service GUID

You must declare this GUID within your component project, **and exactly in one** `.cpp` **file only**:

```cpp
#include <foobar2000.h>
#include "foo_zero_bus/abi/foo_zero_bus_abi.h"

ZERO_MESSAGE_BUS_DECLARE_GUID;
```

*(Note: ABI v1 GUID is `{7A3E9C1D-4B62-4F8A-9C11-2E58A40DB721}`. Duplicate definitions or omissions will cause compilation/linking failures)*.

### 4.3 Retrieving the Bus Instance

```cpp
service_ptr_t<zero_message_bus_v1> bus;
if (!zero_message_bus_v1::enumerate().first(bus) || !bus->running()) {
    console::print("foo_zero_bus is not running");
    return;
}
```

Failure to retrieve usually means the component is not installed, disabled, or still booting.

### 4.4 Sending Messages and Requests

- **Broadcast/Notification**: Use `send_message()` to handle `EVENT` or `NOTIFICATION`.
- **Initiate Request**: Use `send_request_async()` to initiate a `REQUEST` that requires a response.

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

**Memory Lifecycle Management**:

- The passed `zero_message` struct and string pointers only need to stay alive until the method returns; the bus will automatically deep-copy the necessary fields internally.
- The `zero_message` received in the response callback (Receiver) is only valid during that single callback. If needed in a background thread, **the business side must perform a deep copy itself**.

### 4.5 Service Registration and Lifecycle Rules

Use `register_service()` to register a service, and you must call `unregister_service()` before the component unloads or exits.

```cpp
zero_service_provider_v1 provider{};
provider.on_message = &on_message;
provider.release = &release_context;
provider.ctx = context;

if (!bus->register_service("plugin.localize", &provider)) {
    // Registration failed: usually because the service name is occupied by another plugin
}
```

**⚠️ Developer Pitfall Guide (Crucial)**:

1. **Fast Return Principle**: `on_message` must return instantly! Executions exceeding 10ms will output a warning, and those exceeding 50ms will be judged as violations, potentially causing the service to be quarantined.
2. **Asynchronous Response**: If data processing needs to be async, first copy `zero_reply_v1` and call `retain(ctx)`; after the task finishes, you must call the corresponding `release(ctx)`.
3. **Single Response**: A single `REQUEST` can only invoke `reply()` or `error()` once; duplicate calls will be intercepted by the bus.
4. **No Rogue Threads**: Using `std::thread::detach()` is absolutely forbidden! You must use a thread pool or Executor with explicit lifecycle management.
5. **Exception Isolation**: Strictly prohibit C++ exceptions from crossing the ABI boundary.

## 5. WebSocket Integration Guide

### 5.1 Connection and Authentication

- **Address**: `ws://127.0.0.1:17890` (listens only to the local loopback address, rejects external connections).
- **Authentication**: If the administrator has enabled `require_auth`, the first frame sent after establishing the connection must be `{"auth":"<token>"}`.

### 5.2 Sending Requests (REQUEST)

**You must use a JSON library to generate the envelope; do not manually write escapes.** Because the Payload must be a pure string, if your business data is an object, you must perform **double serialization** (first serialize the business object, then serialize the entire envelope):

```javascript
const request = {
  sender: "",
  receiver: "plugin.localize",
  type: 1,
  msg_id: "req_001",
  correlation_id: "",
  // You must execute the first JSON.stringify here
  payload: JSON.stringify({ text: "Hello", to: "zh-CN" }),
};

// Execute the second JSON.stringify to send the outer envelope
socket.send(JSON.stringify(request));
```

> ❌ **Common Error Example**: Assigning an object directly to the payload field will cause an `INVALID_ENVELOPE` error. `payload: { text: "Hello", to: "zh-CN" } // ERROR!`

*Note: The WebSocket endpoint currently uses a fixed 5000ms default timeout and does not support custom timeout configurations within the envelope; Payload size is limited to 1MB by default*.

### 5.3 Receiving Responses and Canceling Requests

- **Processing Responses**: Listen to the return frame; `type === 2` indicates success, `type === 5` indicates failure. Match the callback by comparing the `correlation_id` with the original request's `msg_id`.
- **Canceling Requests**: Send a message with `type === 6` (CANCEL) and fill the original request's `msg_id` into the `correlation_id`: `{"receiver":"plugin.localize","type":6,"correlation_id":"req_001","payload":""}`.

## 6. In-Process C++ SDK (Internal Tools Only)

> **Declaration**: This interface is applicable only to Zero Bus source code internal tools, testing modules, and overall source code integration; **it is not applicable to independent foobar2000 component development**.

Include namespace: `#include "foo_zero_bus/foo_zero_bus_api.h"` -> `namespace msgbus = foo_zero_bus;`

Core API Overview:

- `bus::request_async(service, payload, handler, timeout_ms)`: Callback-style async request; returns a `request_handle`.
- `bus::request_future(service, payload, timeout_ms)`: Returns `std::future<response>`.
- `bus::request_sync(service, payload, timeout_ms)`: Blocks the current worker thread; throws `request_error` on failure.
- `bus::request_awaitable(executor, service, payload, timeout_ms)`: C++20 `co_await`; resumes on the given executor.
- `bus::request(service, payload, timeout_ms, handler)`: Initiates an async request and returns a handle (compatibility alias for the original callback API).
- `bus::publish(service, payload)`: Publishes an EVENT.
- `bus::notify(endpoint, payload)`: Sends a NOTIFICATION.
- `bus::cancel_request(id)`: Cooperative cancellation.
- `service(bus, name, executor, handler)`: Registers a service (utilizes RAII, automatically unregisters safely upon destruction).
- `responder::reply()` / `error()`: Executes a request response (can only be called successfully once).
- `executor::post()`: Safely pushes a task to be executed in a managed worker thread.

The service handler runs on the specified executor. After timeout or cancellation, the bus will not forcibly stop the business task; check `request::cancelled()` and exit yourself.

`request_sync()` may only be called on an independent background thread. Do not block the foobar2000 UI, the Dispatcher, or the single-thread executor that the target service needs to finish this request. `request_awaitable()` must be given an explicit resume executor so later coroutine code does not run on the Dispatcher.

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

See [`example_sdk.cpp`](../examples/cpp/inproc/example_sdk.cpp) for a complete example.

## 7. Troubleshooting (FAQ)

- **Cannot connect via WebSocket?** Please confirm foobar2000 is running. Go to **Preferences → Tools → Zero Bus** to check if the service has started and verify the port configuration. Note that the `FOO_ZERO_BUS_WS_PORT` environment variable might override the UI panel's port setting.
- **Frequently receiving** `SERVICE_NOT_FOUND`**?** The target service is not registered, or the component providing the service has not finished loading during boot.
- **Frequently receiving** `INVALID_ENVELOPE`**?** Please strictly check your WebSocket send frames. You must use `JSON.stringify()` to generate the outer envelope, ensure `receiver` is non-empty, and ensure the `payload` is a pure string, not a JSON Object.
- **Requests always triggering** `TIMEOUT`**?** The receiving service must invoke `reply()` or `error()` once for every `REQUEST`. Note that a bus timeout merely means the queueing/waiting state within the bus has ended; it **will not** actively kill business threads still running in the background.
- **Service state changed to** `UNHEALTHY` **or** `QUARANTINED`**?** This indicates your service callback blocked the bus's dispatcher thread. Please immediately move all time-consuming tasks like network requests and file IO to your managed `executor` thread pool to ensure the bus's entry callback returns instantaneously.

## 8. Official Example Index

| Example File Path | Demonstration Content |
| --- | --- |
| [`zero_bus_guid.cpp`](../examples/cpp/foobar_plugin/zero_bus_guid.cpp) | GUID definition example (needs to be compiled only once per component) |
| [`example_service.cpp`](../examples/cpp/foobar_plugin/example_service.cpp) | Registering and implementing the `plugin.localize` server |
| [`example_client.cpp`](../examples/cpp/foobar_plugin/example_client.cpp) | Initiating an ABI `REQUEST` client |
| [`example_sdk.cpp`](../examples/cpp/inproc/example_sdk.cpp) | Usage methods for the in-process C++ SDK, including request helpers |
| [`example.html`](../examples/websocket/example.html) | Browser-based WebSocket debugging page |
| [`example.mjs`](../examples/websocket/example.mjs) | Calling client in Node.js / Electron environments |
