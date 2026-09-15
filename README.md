**English** · [中文](README_zh.md)

# Zero Bus (foo_zero_bus)

Zero Bus is an in-process asynchronous message bus designed specifically for foobar2000. It provides a unified Request / Response / Event communication model for native C++ components and WebSocket clients. The bus routes the envelope only and never parses the business payload.

The current version is production-ready `0.1.0`. It supports foobar2000 1.x / 2.x on Windows (32-bit and 64-bit) and macOS 11+.

This repository hosts compiled installers, public ABI headers, and sample code. Download the installer from **[Releases](https://github.com/hehelp/foo_zero_bus/releases)**.

---

## Changelog

### v0.1.0 (2026-09-15)

- In-process C++20 bus with REQUEST, RESPONSE, EVENT, and NOTIFICATION, plus timeout and cooperative cancellation.
- Other foobar components integrate through the `zero_message_bus_v1` ABI. Copy `sdk/foo_zero_bus/abi/` only; do not statically link the bus core.
- Local WebSocket listens on `127.0.0.1:17890` by default (loopback only; authentication is off by default).
- Preferences page: **Tools → Zero Bus**. Change the port, start or stop the service, view live connections, and optionally inspect the communication log.
- Payload must be a string. Serialize business JSON before placing it in the payload.
- In-process C++ SDK request helpers: `request_async()`, `request_future()`, `request_sync()`, and C++20 `request_awaitable()`. These are for source-tree tools and tests only.
- Restarting the bus restores previously registered ABI services. Register, connect, and disconnect events are always written to the log.

---

## Requirements

| Category | Details |
| --- | --- |
| **Operating System** | Windows 10 / 11; macOS 11+ |
| **Player Version** | **Windows**: foobar2000 1.x and 2.x (32-bit and 64-bit). **Mac**: foobar2000 2.6 and later |
| **Limitations** | Do not mix Windows DLLs of different bitness or mix Mac bundles. WebSocket listens on loopback only |

The 32-bit and 64-bit Windows builds are different DLLs. Do not mix them.

| Player Version | Component Filename | Default Installation Directory |
| --- | --- | --- |
| foobar2000 **32-bit** | `foo_zero_bus.dll` | `%APPDATA%\foobar2000-v2\user-components\foo_zero_bus\` |
| foobar2000 **64-bit** | `foo_zero_bus.dll` | `%APPDATA%\foobar2000-v2\user-components-x64\foo_zero_bus\` |
| foobar2000 **Mac** | `foo_zero_bus.component` | `~/Library/foobar2000-v2/user-components/` |

---

## Install

1. Open [Releases](https://github.com/hehelp/foo_zero_bus/releases) and download `foo_zero_bus-0.1.0.fb2k-component`.
2. In foobar2000: **File → Preferences → Components → Install**, then select the `.fb2k-component` file. foobar2000 2.x picks the matching bitness automatically.
3. Or copy the matching DLL into the directory above and **restart** foobar2000.

Typical 64-bit Windows path:

```text
C:\Users\<username>\AppData\Roaming\foobar2000-v2\user-components-x64\foo_zero_bus\foo_zero_bus.dll
```

After installation, the service starts with foobar2000 and listens on `ws://127.0.0.1:17890` by default. Change the port under **File → Preferences → Tools → Zero Bus**.

## Third-Party Integration

| Caller | Recommended Path | Notes |
| --- | --- | --- |
| **Other foobar2000 components** | `zero_message_bus_v1` ABI | Copy the headers in [`sdk/foo_zero_bus/abi/`](sdk/foo_zero_bus/abi/) |
| **Same-process tools or tests** | C++ SDK | Do not link this into a regular foobar component |
| **Browser / Electron / scripts** | WebSocket envelope | Connect to `ws://127.0.0.1:<port>` |

Full integration notes: [English SDK](docs/sdk.md) · [中文 SDK](docs/sdk_zh.md). Samples: [`examples/`](examples/).

Callers must follow these rules:

1. `payload` must be a string. Run `JSON.stringify` on business objects first.
2. Service callbacks must return quickly. Blocking longer than 50 ms is a violation.
3. Reply to each `REQUEST` exactly once.
4. Timeout does not kill business threads. Check `cancelled()` inside the handler.
5. Do not create detached threads with `detach()`.
6. Do not statically link `foo_zero_bus_core` into another foobar component.

## Preferences

Path: **File → Preferences → Tools → Zero Bus**

- **General**: customize the WebSocket port (listens on `127.0.0.1` only).
- **Service control**: start or stop the bus.
- **Status**: view the number of active connections.
- **Log** child page: optional communication logging, a read-only console, and one-click clear.
