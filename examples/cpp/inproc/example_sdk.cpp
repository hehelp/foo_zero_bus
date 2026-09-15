#include "foo_zero_bus/foo_zero_bus_api.h"

#include <future>
#include <iostream>
#include <string>

// Same-process sample. Third-party foobar components should use the ABI
// instead of linking foo_zero_bus_core.

int main() {
    foo_zero_bus::bus bus;
    foo_zero_bus::executor workers(1);

    foo_zero_bus::service localize(
        bus,
        "plugin.localize",
        workers,
        [](foo_zero_bus::request incoming,
           std::shared_ptr<foo_zero_bus::responder> outgoing) {
            if (incoming.cancelled() || !outgoing) {
                return;
            }
            static_cast<void>(outgoing->reply("hello:" + incoming.payload()));
        });

    const auto handle = bus.request_async(
        "plugin.localize",
        "zh-CN",
        [](foo_zero_bus::response value) {
            if (value.ok()) {
                std::cout << "async: " << value.payload() << '\n';
            } else {
                std::cout << "error " << value.message() << '\n';
            }
        },
        foo_zero_bus::default_request_timeout_ms);

    static_cast<void>(handle);

    auto future = bus.request_future("plugin.localize", "en-US");
    const auto future_response = future.get();
    if (future_response.ok()) {
        std::cout << "future: " << future_response.payload() << '\n';
    }

    // request_sync() blocks; use a dedicated client worker in GUI plugins.
    auto sync = std::async(std::launch::async, [&bus] {
        return bus.request_sync("plugin.localize", "ja-JP");
    });
    try {
        std::cout << "sync: " << sync.get() << '\n';
    } catch (const foo_zero_bus::request_error& error) {
        std::cout << "sync error " << error.what() << '\n';
    }

    workers.wait();
    return 0;
}
