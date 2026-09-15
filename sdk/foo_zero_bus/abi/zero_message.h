#pragma once

#include "foo_zero_bus/abi/zero_constants.h"

#include <cstdint>
#include <type_traits>

// Flat C envelope. String pointers only need to live until the ABI call
// returns; the bus deep-copies the fields it keeps. Callbacks receive a
// view that is valid only for that callback.
struct zero_message final {
    const char* sender;
    const char* receiver;
    std::uint32_t type;
    const char* msg_id;
    const char* correlation_id;
    const char* payload;
};

static_assert(std::is_standard_layout_v<zero_message>);
static_assert(std::is_trivially_copyable_v<zero_message>);
