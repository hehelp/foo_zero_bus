#include <foobar2000.h>
#include "foo_zero_bus/abi/foo_zero_bus_abi.h"

#include <string>

namespace {

struct reply_ctx final {};

void on_reply(void* raw, const zero_message* msg) {
    static_cast<void>(raw);
    if (msg == nullptr) {
        return;
    }
    const char* payload = msg->payload != nullptr ? msg->payload : "";
    if (msg->type == ZERO_MSG_ERROR) {
        FB2K_console_formatter() << "zero_bus error: " << payload;
    } else {
        FB2K_console_formatter() << "zero_bus reply: " << payload;
    }
}

void release_reply(void* raw) {
    delete static_cast<reply_ctx*>(raw);
}

void request_localize(const char* text) {
    service_ptr_t<zero_message_bus_v1> bus;
    if (!zero_message_bus_v1::enumerate().first(bus) || !bus->running()) {
        console::print("foo_zero_bus is not running");
        return;
    }

    auto* ctx = new reply_ctx();
    zero_reply_receiver_v1 receiver{};
    receiver.on_message = &on_reply;
    receiver.release = &release_reply;
    receiver.ctx = ctx;

    zero_message msg{};
    msg.sender = "plugin.example";
    msg.receiver = "plugin.localize";
    msg.type = ZERO_MSG_REQUEST;
    msg.msg_id = "req_demo_1";
    msg.correlation_id = "";
    msg.payload = text;

    if (!bus->send_request_async(&msg, &receiver, ZERO_DEFAULT_REQUEST_TIMEOUT_MS)) {
        console::print("zero_bus request rejected");
        release_reply(ctx);
    }
}

void publish_event(const char* service, const char* payload) {
    service_ptr_t<zero_message_bus_v1> bus;
    if (!zero_message_bus_v1::enumerate().first(bus) || !bus->running()) {
        return;
    }
    zero_message msg{};
    msg.sender = "plugin.example";
    msg.receiver = service;
    msg.type = ZERO_MSG_EVENT;
    msg.payload = payload;
    static_cast<void>(bus->send_message(&msg));
}

} // namespace
