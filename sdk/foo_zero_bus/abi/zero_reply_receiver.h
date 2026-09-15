#pragma once

#include "foo_zero_bus/abi/zero_message.h"

// Reply handle passed into a service on_message callback. reply() or
// error() may succeed only once. For async work, retain(ctx) first and
// release(ctx) when the work finishes.
struct zero_reply_v1 final {
    int (*reply)(void* ctx, const char* payload);
    int (*error)(void* ctx, std::uint32_t code, const char* text);
    int (*cancelled)(void* ctx);
    void (*retain)(void* ctx);
    void (*release)(void* ctx);
    void* ctx;
};

// Client callback table for send_request_async().
struct zero_reply_receiver_v1 final {
    void (*on_message)(void* ctx, const zero_message* msg);
    void (*release)(void* ctx);
    void* ctx;
};
