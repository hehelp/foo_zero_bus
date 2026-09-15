#pragma once

// Public ABI for third-party foobar2000 components.
// Include foobar2000.h first, then this header. Copy sdk/foo_zero_bus/abi/
// only; do not link foo_zero_bus_core.
//
// Define ZERO_MESSAGE_BUS_DECLARE_GUID in exactly one .cpp file, then
// locate the bus with zero_message_bus_v1::enumerate().

#include "foo_zero_bus/abi/zero_constants.h"
#include "foo_zero_bus/abi/zero_message.h"
#include "foo_zero_bus/abi/zero_reply_receiver.h"
#include "foo_zero_bus/abi/zero_provider.h"
#include "foo_zero_bus/abi/zero_bus_service.h"
