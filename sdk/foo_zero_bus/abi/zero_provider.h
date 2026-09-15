#pragma once

#include "foo_zero_bus/abi/zero_reply_receiver.h"

// Service callback table. on_message must return quickly; move long work
// off the Dispatcher. Call unregister_service() before the plugin unloads.
struct zero_service_provider_v1 final {
    void (*on_message)(
        void* ctx,
        const zero_message* msg,
        const zero_reply_v1* reply);
    void (*release)(void* ctx);
    void* ctx;
};
