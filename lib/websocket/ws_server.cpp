#include "ws_server.h"

namespace loralink::websocket {

bool WsServer::begin(uint16_t, WsMessageHandler handler) {
    m_handler = handler;
    return true;
}

void WsServer::broadcast(const WsOutbound&) {}
void WsServer::broadcastStatus(int16_t, bool, uint32_t, uint32_t) {}

} // namespace loralink::websocket
