# 第三方接入示例

说明见 `doc/08-第三方接入API.md`。

- `cpp/foobar_plugin/`：其他 foobar2000 组件通过 ABI 注册服务、发请求。把 `include/foo_zero_bus/abi/` 加入头文件路径，并编译 `zero_bus_guid.cpp` 一次。
- `cpp/inproc/`：同进程 SDK，演示 async / future / sync 请求；需链接
  `foo_zero_bus_core`。不要用在其他 foobar 组件里。
- `websocket/`：本机 `ws://127.0.0.1:17890`。用浏览器打开 `example.html`，或 `node example.mjs`。
