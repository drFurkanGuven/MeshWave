#pragma once

/**
 * @file ws_server.h
 * @brief WebSocket server for live phone chat (ESPAsyncWebServer).
 */

#include "ws_protocol.h"
#include "../chat/chat_service.h"
#include <functional>

namespace loralink::websocket {

using WsMessageHandler = std::function<void(const WsInbound&)>;

/**
 * @class WsServer
 * @brief WebSocket endpoint at /ws
 */
class WsServer {
public:
    bool begin(uint16_t port, WsMessageHandler on_message);
    void broadcast(const WsOutbound& msg);
    void broadcastStatus(int16_t rssi, bool linked, uint32_t tx, uint32_t rx);
    size_t clientCount() const { return m_clients; }

private:
    WsMessageHandler m_handler;
    size_t m_clients{0};
};

} // namespace loralink::websocket
