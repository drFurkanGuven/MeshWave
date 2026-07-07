#pragma once

/**
 * @file api_handlers.h
 * @brief REST API handlers (/api/status, /api/messages).
 */

#include "../chat/conversation_model.h"
#include "../transport/reliable_transport.h"
#include <WString.h>

namespace loralink::webserver {

struct ApiContext {
    const chat::ConversationModel* conversation{nullptr};
    const transport::TransportStats* stats{nullptr};
    int16_t rssi{0};
    int8_t  snr{0};
    bool linked{false};
    uint32_t uptime_ms{0};
    uint32_t last_peer_ms{0};
    const char* device_name{nullptr};
};

class ApiHandlers {
public:
    static String statusJson(const ApiContext& ctx);
    static String messagesJson(const chat::ConversationModel& model);
};

} // namespace loralink::webserver
