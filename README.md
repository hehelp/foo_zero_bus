**English** · [中文](README_zh.md)

# Zero Bus (foo_zero_bus)

Zero Bus is an in-process asynchronous message bus designed specifically for foobar2000. It provides a unified Request / Response / Event communication model for native C++ components and WebSocket clients. The core design philosophy of the bus is to solely route the "Envelope," absolutely refusing to parse the Payload at the business level.

The current version is production-ready `0.1.1`. It fully supports foobar2000 1.x / 2.x in Windows environments (covering 32-bit, 64-bit, and ARM64EC), as well as macOS 11+ systems.

This repository is primarily used for hosting compiled installation packages, provided ABI header files, and relevant sample code. You can obtain the installation packages directly from the repository's **[Releases](https://github.com/hehelp/foo_zero_bus/releases)** page.

---

## 🚀 Changelog

### v0.1.1 (2026-09-15)

- Preferences page is now bilingual. Simplified Chinese locales default to Chinese; all others default to English. The language can also be switched manually.
- The installer now includes a Windows ARM (ARM64EC) binary at `arm64ec/foo_zero_bus.dll`. foobar2000 for ARM prefers this over the x64 build.
- The macOS component is an explicit Universal Binary (arm64 + x86_64).
- Fixed the preferences “Language” label position and width.

### v0.1.0 (2026-09-15)

- In-process bus built on C++20, fully supporting REQUEST, RESPONSE, EVENT, and NOTIFICATION, with built-in timeout control and cooperative cancellation mechanisms.
- Other foobar components must integrate via the `zero_message_bus_v1` ABI (only need to copy the `sdk/foo_zero_bus/abi/` directory; statically linking the bus core library is strictly prohibited).
- Native WebSocket listens by default at `127.0.0.1:17890` (restricted to local loopback address, authentication disabled by default).
- Added preference panel: **Tools → Zero Bus**, allowing port modification, starting/stopping the service, viewing current connections, and providing an optional communication log viewing feature.
- Strict specification: Payload must be in pure string format (business JSON data must be executed with `stringify` beforehand).
- In-process C++ SDK request helpers: `request_async()`, `request_future()`, `request_sync()`, and C++20 `request_awaitable()`. These are for source-tree tools and tests only.
- Restarting the bus restores previously registered ABI services. Register, connect, and disconnect events are always written to the log.

---

## 💻 Operating Environment & Support

| Category | Detailed Requirements |
| --- | --- |
| **Operating System** | Windows 10 / 11; macOS 11+ |
| **Player Version** | **Windows**: foobar2000 1.x and 2.x (32-bit, 64-bit, and ARM64EC) **Mac**: foobar2000 2.6 and above |
| **Limitations** | Does not support mixing DLLs of different architectures on Windows or mixing bundles on Mac; WebSocket only listens on the local loopback address |

**⚠️ Note**: The 32-bit, 64-bit, and ARM64EC builds for Windows are different DLL files; mixing them is strictly prohibited.

| Player Version | Component Filename | Default Installation Directory |
| --- | --- | --- |
| foobar2000 **32-bit** | `foo_zero_bus.dll` | `%APPDATA%\foobar2000-v2\user-components\foo_zero_bus\` |
| foobar2000 **64-bit** | `foo_zero_bus.dll` | `%APPDATA%\foobar2000-v2\user-components-x64\foo_zero_bus\` |
| foobar2000 **Windows ARM** | `foo_zero_bus.dll` | `%APPDATA%\foobar2000-v2\user-components-arm64\foo_zero_bus\` |
| foobar2000 **Mac** | `foo_zero_bus.component` | `~/Library/foobar2000-v2/user-components/` |

---

## 📦 Installation Guide

1. Head to the **[Releases](https://github.com/hehelp/foo_zero_bus/releases)** page of this repository and download the latest `foo_zero_bus-0.1.1.fb2k-component` installation package.
2. Open foobar2000, navigate to **File → Preferences → Components → Install**, and select the `.fb2k-component` file you just downloaded. (Note: foobar 2.x will automatically select the correct version based on the software's bitness).
3. Alternatively, you can manually copy the corresponding DLL file into the installation directory listed in the table above, then **restart** foobar2000.

*Manual Installation Path Example (Windows 64-bit)*:

```text
C:\Users\<username>\AppData\Roaming\foobar2000-v2\user-components-x64\foo_zero_bus\foo_zero_bus.dll
```

After installation, the service will start automatically with foobar2000 and listen on `ws://127.0.0.1:17890` by default. If you need to modify the port, please go to **File → Preferences → Tools → Zero Bus** to configure it.

## 🛠️ Third-Party Developer Integration

| Your Role / Caller | Recommended Integration | Integration Note |
| --- | --- | --- |
| **Other foobar2000 Components** | `zero_message_bus_v1` ABI | Only need to copy the header files in the [`sdk/foo_zero_bus/abi/`](sdk/foo_zero_bus/abi/) directory |
| **In-Process Tools or Tests** | C++ SDK | Strictly prohibited to link this into other standard foobar components |
| **Browser / Electron / Scripts** | WebSocket Envelope Protocol | Connects by default to `ws://127.0.0.1:<port>` |

For complete integration instructions, please refer to the [English SDK](docs/sdk.md) · [中文 SDK](docs/sdk_zh.md). ABI header files are located in the [`sdk/foo_zero_bus/abi/`](sdk/foo_zero_bus/abi/) directory, and sample code can be found in the [`examples/`](examples/) directory.

**⛔ Integrators must strictly adhere to the following rules:**

1. The `payload` field must be a pure string; if passing business objects, you must execute `JSON.stringify` first.
2. Service callback functions must return as quickly as possible; blocking for more than 50ms will be recorded by the system as a violation.
3. For each `REQUEST`, only one response is permitted.
4. The Timeout mechanism will not forcefully kill business threads; business logic must respond to cancellation signals internally by checking `cancelled()` within the handler.
5. Creating detached threads using `detach()` is absolutely prohibited.
6. Statically linking the `foo_zero_bus_core` core library into other foobar components is strictly forbidden.

## ⚙️ Preferences Panel

Panel Path: **File → Preferences → Tools → Zero Bus**

- **General Settings**: Supports customizing the WebSocket port (for security reasons, listens only on `127.0.0.1`), and the UI language (Follow system / 中文 / English).
- **Service Control**: Supports manually starting or stopping the bus service.
- **Status Monitoring**: View the number of active connections in real-time.
- **Log Subpage**: Provides optional communication logging, a read-only console output view, and a one-click log clearing feature.
