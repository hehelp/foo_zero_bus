#pragma once

#include <cstdint>

// Message types and error codes shared by the foobar ABI and the WebSocket
// envelope. Payload is always an opaque string; the bus never parses it.
enum zero_message_type : std::uint32_t {
    ZERO_MSG_REQUEST = 1,
    ZERO_MSG_RESPONSE = 2,
    ZERO_MSG_EVENT = 3,
    ZERO_MSG_NOTIFICATION = 4,
    ZERO_MSG_ERROR = 5,
    ZERO_MSG_CANCEL = 6,
};

// Stable numeric codes. WebSocket error payloads use the matching name
// string, for example {"code":"TIMEOUT","message":"request timeout"}.
enum zero_error_code : std::uint32_t {
    ZERO_OK = 0,
    ZERO_SERVICE_NOT_FOUND = 1,
    ZERO_SERVICE_UNHEALTHY = 2,
    ZERO_SERVICE_QUARANTINED = 3,
    ZERO_TIMEOUT = 4,
    ZERO_CANCELLED = 5,
    ZERO_QUEUE_FULL = 6,
    ZERO_INVALID_ENVELOPE = 7,
    ZERO_INVALID_MESSAGE_TYPE = 8,
    ZERO_DUPLICATE_RESPONSE = 9,
    ZERO_TRANSPORT_ERROR = 10,
    ZERO_AUTH_FAILED = 11,
    ZERO_ACCESS_DENIED = 12,
    ZERO_INTERNAL_ERROR = 13,
};

inline constexpr std::uint32_t ZERO_DEFAULT_REQUEST_TIMEOUT_MS = 5000;
inline constexpr std::uint16_t ZERO_DEFAULT_WEBSOCKET_PORT = 17890;
