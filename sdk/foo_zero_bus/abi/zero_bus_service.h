#pragma once

#include "foo_zero_bus/abi/zero_provider.h"

// Include foobar2000.h before this header. The interface is a foobar2000
// service_base entry point so other components can locate the bus without
// sharing a CRT or C++ ABI with foo_zero_bus.
//
// {7A3E9C1D-4B62-4F8A-9C11-2E58A40DB721}
// Define the GUID in exactly one .cpp file of the consuming plugin:
//   ZERO_MESSAGE_BUS_DECLARE_GUID;
#ifndef ZERO_MESSAGE_BUS_DECLARE_GUID
#define ZERO_MESSAGE_BUS_DECLARE_GUID                                          \
    DECLARE_CLASS_GUID(                                                        \
        zero_message_bus_v1,                                                   \
        0x7a3e9c1d,                                                            \
        0x4b62,                                                                \
        0x4f8a,                                                                \
        0x9c,                                                                  \
        0x11,                                                                  \
        0x2e,                                                                  \
        0x58,                                                                  \
        0xa4,                                                                  \
        0x0d,                                                                  \
        0xb7,                                                                  \
        0x21)
#endif

class NOVTABLE zero_message_bus_v1 : public service_base {
public:
    virtual int send_message(const zero_message* msg) = 0;
    virtual int send_request_async(
        const zero_message* msg,
        const zero_reply_receiver_v1* receiver,
        std::uint32_t timeout_ms) = 0;
    virtual int register_service(
        const char* service_name,
        const zero_service_provider_v1* provider) = 0;
    virtual int unregister_service(const char* service_name) = 0;
    virtual int cancel(const char* request_id) = 0;
    virtual int running() const = 0;

    FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(zero_message_bus_v1);
};
