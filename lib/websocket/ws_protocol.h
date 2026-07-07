#pragma once

/**
 * @file ws_protocol.h
 * @brief WebSocket JSON message types for phone chat.
 */

#include <cstdint>

namespace loralink::websocket {

enum class WsMessageType : uint8_t {
    Message  = 0,
    Status   = 1,
    History  = 2,
    Error    = 3,
};

struct WsInbound {
    WsMessageType type{WsMessageType::Message};
    char text[512]{};
};

struct WsOutbound {
    WsMessageType type{WsMessageType::Message};
    char text[512]{};
    int16_t rssi{0};
    bool linked{false};
    const char* dir{"in"};
};

} // namespace loralink::websocket
