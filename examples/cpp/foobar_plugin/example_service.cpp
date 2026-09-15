#include <foobar2000.h>
#include "foo_zero_bus/abi/foo_zero_bus_abi.h"

#include <string>

// Registers plugin.localize. Reply immediately; post heavier work yourself.
// Do not mark service implementation classes final.

namespace {

struct localize_ctx final {
    int refs{1};
};

void localize_on_message(
    void* raw,
    const zero_message* msg,
    const zero_reply_v1* reply) {
    static_cast<void>(raw);
    if (msg == nullptr || reply == nullptr || reply->reply == nullptr) {
        return;
    }
    if (msg->type == ZERO_MSG_CANCEL) {
        return;
    }
    if (reply->cancelled != nullptr && reply->cancelled(reply->ctx)) {
        return;
    }

    const char* incoming = msg->payload != nullptr ? msg->payload : "";
    const std::string outgoing = std::string("{\"text\":\"") + incoming + "\"}";
    static_cast<void>(reply->reply(reply->ctx, outgoing.c_str()));
}

void localize_release(void* raw) {
    delete static_cast<localize_ctx*>(raw);
}

class localize_initquit : public initquit {
public:
    void on_init() override {
        service_ptr_t<zero_message_bus_v1> bus;
        if (!zero_message_bus_v1::enumerate().first(bus) || !bus->running()) {
            console::print("foo_zero_bus is not running");
            return;
        }

        ctx_ = new localize_ctx();
        provider_.on_message = &localize_on_message;
        provider_.release = &localize_release;
        provider_.ctx = ctx_;
        if (!bus->register_service("plugin.localize", &provider_)) {
            console::print("failed to register plugin.localize");
            localize_release(ctx_);
            ctx_ = nullptr;
            provider_ = {};
        }
    }

    void on_quit() override {
        service_ptr_t<zero_message_bus_v1> bus;
        if (zero_message_bus_v1::enumerate().first(bus)) {
            static_cast<void>(bus->unregister_service("plugin.localize"));
        }
        ctx_ = nullptr;
        provider_ = {};
    }

private:
    localize_ctx* ctx_{nullptr};
    zero_service_provider_v1 provider_{};
};

static initquit_factory_t<localize_initquit> g_localize_initquit;

} // namespace
