#pragma once

/**
 * @file gateway_web.h
 * @brief ESPAsyncWebServer — static UI, REST API, WebSocket chat.
 */

#include "../chat/chat_service.h"
#include "../config/config_store.h"
#include "../drivers/radio/i_radio_driver.h"
#include "../transport/reliable_transport.h"
#include <WString.h>
#include <cstdint>

namespace loralink::webserver {

struct GatewayWebContext {
    chat::ChatService* chat{nullptr};
    transport::ReliableTransport* transport{nullptr};
    radio::IRadioDriver* radio{nullptr};
    const config::AppConfig* config{nullptr};
};

/**
 * @class GatewayWeb
 * @brief HTTP + WebSocket gateway for phone chat UI.
 */
class GatewayWeb {
public:
    bool begin(const GatewayWebContext& ctx, uint16_t port = 80);
    void poll(uint32_t now_ms);
    void stop();

private:
    void onChatMessage(const chat::ChatMessage& msg);
    void broadcastStatus(uint32_t now_ms);
    String buildStatusJson(uint32_t now_ms) const;
    String buildMessagesJson() const;
    String buildHistoryEnvelope() const;
    String messageToJson(const chat::ChatMessage& msg) const;

    GatewayWebContext m_ctx{};
    bool m_running{false};
    uint32_t m_last_status_ms{0};
};

} // namespace loralink::webserver
